#include "ToshiPCH.h"
#include "TGLFont.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// Vertices in screen pixels, stage 0 modulates
static const TCHAR s_szFontShader[] = R"(
layout( std140 ) uniform Constants
{
	vec4 g_vScreenToClip; // scale (xy) and offset (zw) from viewport pixels to clip space
	vec4 g_vAlphaTest;
};

#ifdef VERTEX_SHADER

layout( location = 0 ) in vec4 a_vPosition;
layout( location = 1 ) in vec4 a_vColour;
layout( location = 2 ) in vec2 a_vUV;

out vec4 v_vColour;
out vec2 v_vUV;

void main()
{
	gl_Position = ToClipSpace( vec4( a_vPosition.xy * g_vScreenToClip.xy + g_vScreenToClip.zw, a_vPosition.z, 1.0 ) / a_vPosition.w );
	v_vColour   = a_vColour.bgra;
	v_vUV       = a_vUV;
}

#else

in vec4 v_vColour;
in vec2 v_vUV;

uniform sampler2D g_Texture0;

void main()
{
	vec4 vColour = saturate( texture( g_Texture0, v_vUV ) * v_vColour );
	AlphaTest( vColour.a, g_vAlphaTest );
	o_vColour = vColour;
}

#endif
)";

static const TGLVertexAttribute s_aFontAttributes[] = {
	{ 0, 0, 4, GL_FALSE, GL_FLOAT, 0 },
	{ 1, 0, 4, GL_TRUE, GL_UNSIGNED_BYTE, 16 },
	{ 2, 0, 2, GL_FALSE, GL_FLOAT, 20 },
};

static const TGLInputLayout s_oFontLayout = { s_aFontAttributes, TARRAYSIZE( s_aFontAttributes ) };

TGLFont::TGLFont( const TCHAR* a_szFontName, TUINT a_dwHeight, TUINT a_dwFlags )
{
	strcpy( m_szFontName, a_szFontName );
	m_dwFontHeight = a_dwHeight;
	m_dwFontFlags  = a_dwFlags;

	m_pRenderer     = TNULL;
	m_uiFontTexture = 0;
	m_uiProgram     = 0;

	m_bStateBlocksRecorded = TFALSE;
}

TGLFont::~TGLFont()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();
}

TBOOL TGLFont::InitDeviceObjects( TRenderGLInterface* a_pRenderer )
{
	m_pRenderer  = a_pRenderer;
	m_fTextScale = 1.0f;

	// Large fonts need larger textures
	if ( m_dwFontHeight > 40 )
		m_dwTexWidth = m_dwTexHeight = 1024;
	else if ( m_dwFontHeight > 20 )
		m_dwTexWidth = m_dwTexHeight = 512;
	else
		m_dwTexWidth = m_dwTexHeight = 256;

	// Render the characters with GDI
	TUINT32*   pBitmapBits;
	BITMAPINFO oBitmapInfo;
	TUtil::MemClear( &oBitmapInfo.bmiHeader, sizeof( BITMAPINFOHEADER ) );
	oBitmapInfo.bmiHeader.biSize        = sizeof( BITMAPINFOHEADER );
	oBitmapInfo.bmiHeader.biWidth       = TINT( m_dwTexWidth );
	oBitmapInfo.bmiHeader.biHeight      = -TINT( m_dwTexHeight );
	oBitmapInfo.bmiHeader.biPlanes      = 1;
	oBitmapInfo.bmiHeader.biCompression = BI_RGB;
	oBitmapInfo.bmiHeader.biBitCount    = 32;

	HDC     hDC       = CreateCompatibleDC( NULL );
	HBITMAP hbmBitmap = CreateDIBSection( hDC, &oBitmapInfo, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0 );
	SetMapMode( hDC, MM_TEXT );

	TINT  iHeight  = -MulDiv( m_dwFontHeight, TINT( GetDeviceCaps( hDC, LOGPIXELSY ) * m_fTextScale ), 72 );
	DWORD dwBold   = ( m_dwFontFlags & GLFONT_BOLD ) ? FW_BOLD : FW_NORMAL;
	DWORD dwItalic = ( m_dwFontFlags & GLFONT_ITALIC ) ? TRUE : FALSE;
	HFONT hFont    = CreateFontA( iHeight, 0, 0, 0, dwBold, dwItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, m_szFontName );

	if ( hFont == NULL )
		return TFALSE;

	SelectObject( hDC, hbmBitmap );
	SelectObject( hDC, hFont );

	SetTextColor( hDC, RGB( 255, 255, 255 ) );
	SetBkColor( hDC, 0x00000000 );
	SetTextAlign( hDC, TA_TOP );

	DWORD x          = 0;
	DWORD y          = 0;
	TCHAR szStr[ 2 ] = "x";
	SIZE  oSize;

	for ( TCHAR c = 32; c < 127; c++ )
	{
		szStr[ 0 ] = c;
		GetTextExtentPoint32A( hDC, szStr, 1, &oSize );

		if ( DWORD( x + oSize.cx + 1 ) > m_dwTexWidth )
		{
			x = 0;
			y += oSize.cy + 1;
		}

		ExtTextOutA( hDC, x + 0, y + 0, ETO_OPAQUE, NULL, szStr, 1, NULL );

		m_afTexCoords[ c - 32 ][ 0 ] = TFLOAT( x + 0 ) / m_dwTexWidth;
		m_afTexCoords[ c - 32 ][ 1 ] = TFLOAT( y + 0 ) / m_dwTexHeight;
		m_afTexCoords[ c - 32 ][ 2 ] = TFLOAT( x + 0 + oSize.cx ) / m_dwTexWidth;
		m_afTexCoords[ c - 32 ][ 3 ] = TFLOAT( y + 0 + oSize.cy ) / m_dwTexHeight;

		x += oSize.cx + 1;
	}

	// Copy the alpha of the bitmap to the texture, quantised like D3DFMT_A4R4G4B4
	TUINT32* pTexels = TSTATICCAST( TUINT32, TMalloc( m_dwTexWidth * m_dwTexHeight * sizeof( TUINT32 ) ) );

	for ( y = 0; y < m_dwTexHeight; y++ )
	{
		for ( x = 0; x < m_dwTexWidth; x++ )
		{
			const TUINT32 uiAlpha = ( pBitmapBits[ m_dwTexWidth * y + x ] & 0xff ) >> 4;

			pTexels[ m_dwTexWidth * y + x ] = uiAlpha > 0 ? ( ( uiAlpha * 17 ) << 24 ) | 0x00ffffff : 0;
		}
	}

	m_uiFontTexture = m_pRenderer->CreateTexture( m_dwTexWidth, m_dwTexHeight, pTexels, TRenderGLInterface::TEXTUREFLAGS_NONE );

	TFree( pTexels );
	DeleteObject( hbmBitmap );
	DeleteDC( hDC );
	DeleteObject( hFont );

	return m_uiFontTexture != 0;
}


TBOOL TGLFont::RestoreDeviceObjects()
{
	m_uiProgram = TRenderGLInterface::CompileProgram( s_szFontShader );

	if ( !m_uiProgram )
		return TFALSE;

	// Record the saved state block and the one used for drawing text
	TRenderGLInterface::STATEBLOCK oCurrentStates;
	m_pRenderer->CaptureStateBlock( oCurrentStates );

	for ( TUINT i = 0; i < 2; i++ )
	{
		m_pRenderer->SetTexture( 0, m_uiFontTexture );
		m_pRenderer->SetZEnable( ( m_dwFontFlags & GLFONT_ZENABLE ) ? TTRUE : TFALSE );
		m_pRenderer->SetAlphaBlendEnable( TTRUE );
		m_pRenderer->SetSrcBlend( GL_SRC_ALPHA );
		m_pRenderer->SetDestBlend( GL_ONE_MINUS_SRC_ALPHA );
		m_pRenderer->SetAlphaTestEnable( TTRUE );
		m_pRenderer->SetAlphaRef( 0x08 );
		m_pRenderer->SetAlphaFunc( GL_GEQUAL );
		m_pRenderer->SetCullMode( TRenderGLInterface::CULL_BACK );
		m_pRenderer->SetStencil( TFALSE, GL_ALWAYS, 0, 0xFF, GL_KEEP, GL_KEEP, GL_KEEP );
		m_pRenderer->SetFogEnable( TFALSE );

		m_pRenderer->SetTextureOp( TRenderGLInterface::TEXTUREOP_MODULATE );
		m_pRenderer->SetTextureMinFilter( 0, TRenderGLInterface::TEXTUREFILTER_POINT );
		m_pRenderer->SetTextureMagFilter( 0, TRenderGLInterface::TEXTUREFILTER_POINT );
		m_pRenderer->SetTextureMipFilter( 0, TRenderGLInterface::TEXTUREFILTER_NONE );

		if ( i == 0 )
			m_pRenderer->CaptureStateBlock( m_oSavedStateBlock );
		else
			m_pRenderer->CaptureStateBlock( m_oDrawTextStateBlock );
	}

	// Recording a state block doesn't change the device states
	m_pRenderer->ApplyStateBlock( oCurrentStates );
	m_bStateBlocksRecorded = TTRUE;

	return TTRUE;
}

TBOOL TGLFont::InvalidateDeviceObjects()
{
	if ( m_uiProgram )
	{
		TRenderGLInterface::DestroyProgram( m_uiProgram );
		m_uiProgram = 0;
	}

	m_bStateBlocksRecorded = TFALSE;

	return TTRUE;
}

TBOOL TGLFont::DeleteDeviceObjects()
{
	if ( m_uiFontTexture )
	{
		if ( m_pRenderer )
			m_pRenderer->DestroyTexture( m_uiFontTexture );

		m_uiFontTexture = 0;
	}

	m_pRenderer = TNULL;

	return TTRUE;
}

TBOOL TGLFont::GetTextExtent( const TCHAR* a_szText, TINT* a_pWidth, TINT* a_pHeight )
{
	if ( a_szText == TNULL || a_pWidth == TNULL || a_pHeight == TNULL )
		return TFALSE;

	TFLOAT fRowWidth  = 0.0f;
	TFLOAT fRowHeight = ( m_afTexCoords[ 0 ][ 3 ] - m_afTexCoords[ 0 ][ 1 ] ) * m_dwTexHeight;
	TFLOAT fWidth     = 0.0f;
	TFLOAT fHeight    = fRowHeight;

	while ( *a_szText )
	{
		TCHAR c = *a_szText++;

		if ( c == '\n' )
		{
			fRowWidth = 0.0f;
			fHeight += fRowHeight;
		}
		else if ( c >= ' ' )
		{
			TFLOAT tx1 = m_afTexCoords[ c - 32 ][ 0 ];
			TFLOAT tx2 = m_afTexCoords[ c - 32 ][ 2 ];

			fRowWidth += ( tx2 - tx1 ) * m_dwTexWidth;

			if ( fRowWidth > fWidth )
				fWidth = fRowWidth;
		}
	}

	*a_pWidth  = TINT( fWidth );
	*a_pHeight = TINT( fHeight );

	return TTRUE;
}

static TFORCEINLINE TGLFont::FONT2DVERTEX InitFont2DVertex( TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_dwColour, TFLOAT a_fTU, TFLOAT a_fTV )
{
	TGLFont::FONT2DVERTEX oVertex;
	oVertex.fX       = a_fX;
	oVertex.fY       = a_fY;
	oVertex.fZ       = 0.9f;
	oVertex.fRHW     = 1.0f;
	oVertex.uiColour = a_dwColour;
	oVertex.fTU      = a_fTU;
	oVertex.fTV      = a_fTV;
	return oVertex;
}

void TGLFont::DrawTriangles( TUINT a_dwNumTriangles )
{
	TINT aViewport[ 4 ];
	m_pRenderer->GetViewport( aViewport[ 0 ], aViewport[ 1 ], aViewport[ 2 ], aViewport[ 3 ] );

	const TFLOAT fWidth  = TFLOAT( aViewport[ 2 ] );
	const TFLOAT fHeight = TFLOAT( aViewport[ 3 ] );

	const TVector4 vScreenToClip(
	    2.0f / fWidth,
	    -2.0f / fHeight,
	    -1.0f - 2.0f * aViewport[ 0 ] / fWidth,
	    1.0f + 2.0f * aViewport[ 1 ] / fHeight
	);

	const TVector4 vAlphaTest = m_pRenderer->GetAlphaTestParams();
	m_pRenderer->SetVec4InVSBuffer( 0, &vScreenToClip );
	m_pRenderer->SetVec4InVSBuffer( 1, &vAlphaTest );

	m_pRenderer->DrawNonIndexedImmediate( GL_TRIANGLES, m_aVertices, a_dwNumTriangles * 3, sizeof( FONT2DVERTEX ) );
}

TBOOL TGLFont::DrawText( TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_dwColour, const TCHAR* a_szText, TUINT a_dwFlags )
{
	if ( m_pRenderer == TNULL || !m_bStateBlocksRecorded )
		return TFALSE;

	// Set up the render states
	m_pRenderer->CaptureStateBlock( m_oSavedStateBlock );
	m_pRenderer->ApplyStateBlock( m_oDrawTextStateBlock );
	m_pRenderer->SetShaders( m_uiProgram, &s_oFontLayout );

	if ( a_dwFlags & GLFONT_FILTERED )
	{
		m_pRenderer->SetTextureMinFilter( 0, TRenderGLInterface::TEXTUREFILTER_LINEAR );
		m_pRenderer->SetTextureMagFilter( 0, TRenderGLInterface::TEXTUREFILTER_LINEAR );
	}

	TFLOAT fStartX = a_fX;

	TUINT         dwNumTriangles = 0;
	FONT2DVERTEX* pVertices      = m_aVertices;

	while ( *a_szText )
	{
		TCHAR c = *a_szText++;

		if ( c == '\n' )
		{
			a_fX = fStartX;
			a_fY += ( m_afTexCoords[ 0 ][ 3 ] - m_afTexCoords[ 0 ][ 1 ] ) * m_dwTexHeight;
		}
		else if ( c >= ' ' )
		{
			TFLOAT tx1 = m_afTexCoords[ c - 32 ][ 0 ];
			TFLOAT ty1 = m_afTexCoords[ c - 32 ][ 1 ];
			TFLOAT tx2 = m_afTexCoords[ c - 32 ][ 2 ];
			TFLOAT ty2 = m_afTexCoords[ c - 32 ][ 3 ];

			TFLOAT w = ( tx2 - tx1 ) * m_dwTexWidth / m_fTextScale;
			TFLOAT h = ( ty2 - ty1 ) * m_dwTexHeight / m_fTextScale;

			if ( c != ' ' )
			{
				*pVertices++ = InitFont2DVertex( a_fX + 0 - 0.5f, a_fY + h - 0.5f, a_dwColour, tx1, ty2 );
				*pVertices++ = InitFont2DVertex( a_fX + 0 - 0.5f, a_fY + 0 - 0.5f, a_dwColour, tx1, ty1 );
				*pVertices++ = InitFont2DVertex( a_fX + w - 0.5f, a_fY + h - 0.5f, a_dwColour, tx2, ty2 );
				*pVertices++ = InitFont2DVertex( a_fX + w - 0.5f, a_fY + 0 - 0.5f, a_dwColour, tx2, ty1 );
				*pVertices++ = InitFont2DVertex( a_fX + w - 0.5f, a_fY + h - 0.5f, a_dwColour, tx2, ty2 );
				*pVertices++ = InitFont2DVertex( a_fX + 0 - 0.5f, a_fY + 0 - 0.5f, a_dwColour, tx1, ty1 );
				dwNumTriangles += 2;

				if ( dwNumTriangles * 3 > ( MAX_NUM_VERTICES - 6 ) )
				{
					// Flush the full buffer
					DrawTriangles( dwNumTriangles );
					pVertices      = m_aVertices;
					dwNumTriangles = 0;
				}
			}

			a_fX += w;
		}
	}

	if ( dwNumTriangles > 0 )
		DrawTriangles( dwNumTriangles );

	// Restore the render states
	m_pRenderer->ApplyStateBlock( m_oSavedStateBlock );

	return TTRUE;
}

TOSHI_NAMESPACE_END
