#include "ToshiPCH.h"
#include "CD3DFont.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// The fixed function pipeline for D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 vertices, stage 0 modulates
static const TCHAR s_szFontShader[] = R"(
cbuffer Constants : register( b0 )
{
	float4 g_vScreenToClip : packoffset( c0 ); // scale (xy) and offset (zw) from viewport pixels to clip space
	float4 g_vAlphaTest    : packoffset( c1 );
};

struct VS_IN
{
	float4 vPosition : POSITION;
	float4 vColour : COLOR;
	float2 vUV : TEXCOORD;
};

struct PS_IN
{
	float4 vPosition : SV_POSITION;
	float4 vColour : COLOR;
	float2 vUV : TEXCOORD0;
};

PS_IN vs_main( VS_IN In )
{
	// Pixel centres are on integer coordinates in D3D8 and on half integers in D3D11
	float2 vPixel = In.vPosition.xy + 0.5f;

	PS_IN Out;
	Out.vPosition = float4( vPixel * g_vScreenToClip.xy + g_vScreenToClip.zw, In.vPosition.z, 1.0f ) / In.vPosition.w;
	Out.vColour   = In.vColour;
	Out.vUV       = In.vUV;
	return Out;
}

Texture2D    g_Texture : register( t0 );
SamplerState g_Sampler : register( s0 );

float4 ps_main( PS_IN In ) : SV_TARGET
{
	float4 vColour = saturate( g_Texture.Sample( g_Sampler, In.vUV ) * In.vColour );
	AlphaTest( vColour.a, g_vAlphaTest );
	return vColour;
}
)";

CD3DFont::CD3DFont( const TCHAR* a_szFontName, DWORD a_dwHeight, DWORD a_dwFlags )
{
	strcpy( m_szFontName, a_szFontName );
	m_dwFontHeight = a_dwHeight;
	m_dwFontFlags  = a_dwFlags;

	m_pRenderer     = TNULL;
	m_pTexture      = TNULL;
	m_pVB           = TNULL;
	m_pVertexShader = TNULL;
	m_pPixelShader  = TNULL;
	m_pInputLayout  = TNULL;

	m_bStateBlocksRecorded = TFALSE;
}

CD3DFont::~CD3DFont()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();
}

HRESULT CD3DFont::InitDeviceObjects( TRenderD3DInterface* a_pRenderer )
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

	// If the texture is too big for the device, scale the font down
	if ( m_dwTexWidth > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION )
	{
		m_fTextScale = TFLOAT( D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ) / TFLOAT( m_dwTexWidth );
		m_dwTexWidth = m_dwTexHeight = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	}

	// Render the characters with GDI
	DWORD*     pBitmapBits;
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
	DWORD dwBold   = ( m_dwFontFlags & D3DFONT_BOLD ) ? FW_BOLD : FW_NORMAL;
	DWORD dwItalic = ( m_dwFontFlags & D3DFONT_ITALIC ) ? TRUE : FALSE;
	HFONT hFont    = CreateFontA( iHeight, 0, 0, 0, dwBold, dwItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, m_szFontName );

	if ( hFont == NULL )
		return E_FAIL;

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

	// Copy the alpha of the bitmap to the texture (D3DFMT_A4R4G4B4)
	WORD* pTexels = TSTATICCAST( WORD, TMalloc( m_dwTexWidth * m_dwTexHeight * sizeof( WORD ) ) );
	WORD* pDst16  = pTexels;

	for ( y = 0; y < m_dwTexHeight; y++ )
	{
		for ( x = 0; x < m_dwTexWidth; x++ )
		{
			BYTE bAlpha = BYTE( ( pBitmapBits[ m_dwTexWidth * y + x ] & 0xff ) >> 4 );

			if ( bAlpha > 0 )
				*pDst16++ = WORD( ( bAlpha << 12 ) | 0x0fff );
			else
				*pDst16++ = 0x0000;
		}
	}

	m_pTexture = m_pRenderer->CreateTexture( m_dwTexWidth, m_dwTexHeight, DXGI_FORMAT_B4G4R4A4_UNORM, pTexels, TRenderD3DInterface::TEXTUREFLAGS_NONE, D3D11_USAGE_IMMUTABLE, 0, 1 );

	TFree( pTexels );
	DeleteObject( hbmBitmap );
	DeleteDC( hDC );
	DeleteObject( hFont );

	return m_pTexture ? S_OK : E_FAIL;
}

HRESULT CD3DFont::RestoreDeviceObjects()
{
	m_pVB = m_pRenderer->CreateBuffer( D3D11_BIND_VERTEX_BUFFER, MAX_NUM_VERTICES * sizeof( FONT2DVERTEX ), TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );

	if ( !m_pVB )
		return E_FAIL;

	ID3DBlob* pVSBlob = TRenderD3DInterface::CompileShader( s_szFontShader, "vs_main", "vs_4_0", TNULL );
	ID3DBlob* pPSBlob = TRenderD3DInterface::CompileShader( s_szFontShader, "ps_main", "ps_4_0", TNULL );

	if ( !pVSBlob || !pPSBlob ||
	     !TRenderD3DInterface::CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader ) ||
	     !TRenderD3DInterface::CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader ) )
	{
		return E_FAIL;
	}

	static constexpr D3D11_INPUT_ELEMENT_DESC s_aInputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	m_pRenderer->GetDevice()->CreateInputLayout( s_aInputLayout, TARRAYSIZE( s_aInputLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );

	pVSBlob->Release();
	pPSBlob->Release();

	// Record the saved state block and the one used for drawing text
	TRenderD3DInterface::STATEBLOCK oCurrentStates;
	m_pRenderer->CaptureStateBlock( oCurrentStates );

	for ( TUINT i = 0; i < 2; i++ )
	{
		m_pRenderer->SetTexture( 0, m_pTexture );
		m_pRenderer->SetZEnable( ( m_dwFontFlags & D3DFONT_ZENABLE ) ? TTRUE : TFALSE );
		m_pRenderer->SetAlphaBlendEnable( TTRUE );
		m_pRenderer->SetSrcBlend( D3D11_BLEND_SRC_ALPHA );
		m_pRenderer->SetDestBlend( D3D11_BLEND_INV_SRC_ALPHA );
		m_pRenderer->SetAlphaTestEnable( TTRUE );
		m_pRenderer->SetAlphaRef( 0x08 );
		m_pRenderer->SetAlphaFunc( D3D11_COMPARISON_GREATER_EQUAL );
		m_pRenderer->SetCullMode( D3D11_CULL_BACK );
		m_pRenderer->SetStencil( TFALSE, D3D11_COMPARISON_ALWAYS, 0, 0xFF, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP );
		m_pRenderer->SetFogEnable( TFALSE );

		m_pRenderer->SetTextureOp( TRenderD3DInterface::TEXTUREOP_MODULATE );
		m_pRenderer->SetTextureMinFilter( 0, TRenderD3DInterface::TEXTUREFILTER_POINT );
		m_pRenderer->SetTextureMagFilter( 0, TRenderD3DInterface::TEXTUREFILTER_POINT );
		m_pRenderer->SetTextureMipFilter( 0, TRenderD3DInterface::TEXTUREFILTER_NONE );

		if ( i == 0 )
			m_pRenderer->CaptureStateBlock( m_oSavedStateBlock );
		else
			m_pRenderer->CaptureStateBlock( m_oDrawTextStateBlock );
	}

	// Recording a state block doesn't change the device states
	m_pRenderer->ApplyStateBlock( oCurrentStates );
	m_bStateBlocksRecorded = TTRUE;

	return S_OK;
}

HRESULT CD3DFont::InvalidateDeviceObjects()
{
	ID3D11DeviceChild** appObjects[] = {
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pVB ),
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pVertexShader ),
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pPixelShader ),
		TREINTERPRETCAST( ID3D11DeviceChild**, &m_pInputLayout ),
	};

	for ( auto ppObject : appObjects )
	{
		if ( *ppObject )
		{
			( *ppObject )->Release();
			*ppObject = TNULL;
		}
	}

	m_bStateBlocksRecorded = TFALSE;

	return S_OK;
}

HRESULT CD3DFont::DeleteDeviceObjects()
{
	if ( m_pTexture )
	{
		if ( m_pRenderer )
			m_pRenderer->InvalidateShaderResource( m_pTexture );

		m_pTexture->Release();
		m_pTexture = TNULL;
	}

	m_pRenderer = TNULL;

	return S_OK;
}

HRESULT CD3DFont::GetTextExtent( const TCHAR* a_szText, SIZE* a_pSize )
{
	if ( a_szText == TNULL || a_pSize == TNULL )
		return E_FAIL;

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

	a_pSize->cx = TINT( fWidth );
	a_pSize->cy = TINT( fHeight );

	return S_OK;
}

static TFORCEINLINE CD3DFont::FONT2DVERTEX InitFont2DVertex( TFLOAT a_fX, TFLOAT a_fY, DWORD a_dwColour, TFLOAT a_fTU, TFLOAT a_fTV )
{
	CD3DFont::FONT2DVERTEX oVertex;
	oVertex.fX       = a_fX;
	oVertex.fY       = a_fY;
	oVertex.fZ       = 0.9f;
	oVertex.fRHW     = 1.0f;
	oVertex.uiColour = a_dwColour;
	oVertex.fTU      = a_fTU;
	oVertex.fTV      = a_fTV;
	return oVertex;
}

void CD3DFont::DrawTriangles( DWORD a_dwNumTriangles )
{
	D3D11_VIEWPORT oViewport;
	UINT           uiNumViewports = 1;
	m_pRenderer->GetDeviceContext()->RSGetViewports( &uiNumViewports, &oViewport );

	const TVector4 vScreenToClip(
	    2.0f / oViewport.Width,
	    -2.0f / oViewport.Height,
	    -1.0f - 2.0f * oViewport.TopLeftX / oViewport.Width,
	    1.0f + 2.0f * oViewport.TopLeftY / oViewport.Height
	);

	const TVector4 vAlphaTest = m_pRenderer->GetAlphaTestParams();
	m_pRenderer->SetVec4InVSBuffer( 0, &vScreenToClip );
	m_pRenderer->SetVec4InVSBuffer( 1, &vAlphaTest );

	m_pRenderer->DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pVB, a_dwNumTriangles * 3, sizeof( FONT2DVERTEX ), 0, 0 );
}

HRESULT CD3DFont::DrawText( TFLOAT a_fX, TFLOAT a_fY, DWORD a_dwColour, const TCHAR* a_szText, DWORD a_dwFlags )
{
	if ( m_pRenderer == TNULL || !m_bStateBlocksRecorded )
		return E_FAIL;

	ID3D11DeviceContext* pDeviceContext = m_pRenderer->GetDeviceContext();

	// Set up the render states
	m_pRenderer->CaptureStateBlock( m_oSavedStateBlock );
	m_pRenderer->ApplyStateBlock( m_oDrawTextStateBlock );
	m_pRenderer->SetShaders( m_pVertexShader, m_pPixelShader, m_pInputLayout );

	if ( a_dwFlags & D3DFONT_FILTERED )
	{
		m_pRenderer->SetTextureMinFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
		m_pRenderer->SetTextureMagFilter( 0, TRenderD3DInterface::TEXTUREFILTER_LINEAR );
	}

	TFLOAT fStartX = a_fX;

	D3D11_MAPPED_SUBRESOURCE oMapped;
	DWORD                    dwNumTriangles = 0;
	pDeviceContext->Map( m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
	FONT2DVERTEX* pVertices = TSTATICCAST( FONT2DVERTEX, oMapped.pData );

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
					pDeviceContext->Unmap( m_pVB, 0 );
					DrawTriangles( dwNumTriangles );
					pDeviceContext->Map( m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &oMapped );
					pVertices      = TSTATICCAST( FONT2DVERTEX, oMapped.pData );
					dwNumTriangles = 0;
				}
			}

			a_fX += w;
		}
	}

	pDeviceContext->Unmap( m_pVB, 0 );

	if ( dwNumTriangles > 0 )
		DrawTriangles( dwNumTriangles );

	// Restore the render states
	m_pRenderer->ApplyStateBlock( m_oSavedStateBlock );

	return S_OK;
}

TOSHI_NAMESPACE_END
