#include "ToshiPCH.h"
#include "CD3DFont.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006dd750
CD3DFont::CD3DFont( const TCHAR* a_szFontName, DWORD a_dwHeight, DWORD a_dwFlags )
{
	strcpy( m_szFontName, a_szFontName );
	m_dwFontHeight = a_dwHeight;
	m_dwFontFlags  = a_dwFlags;

	m_pDevice  = TNULL;
	m_pTexture = TNULL;
	m_pVB      = TNULL;

	m_dwSavedStateBlock    = 0;
	m_dwDrawTextStateBlock = 0;
}

// $Barnyard: FUNCTION 006ddf90
CD3DFont::~CD3DFont()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();
}

// $Barnyard: FUNCTION 006dd7a0
HRESULT CD3DFont::InitDeviceObjects( IDirect3DDevice8* a_pDevice )
{
	m_pDevice    = a_pDevice;
	m_fTextScale = 1.0f;

	// Large fonts need larger textures
	if ( m_dwFontHeight > 40 )
		m_dwTexWidth = m_dwTexHeight = 1024;
	else if ( m_dwFontHeight > 20 )
		m_dwTexWidth = m_dwTexHeight = 512;
	else
		m_dwTexWidth = m_dwTexHeight = 256;

	// If the texture is too big for the device, scale the font down
	D3DCAPS8 oCaps;
	m_pDevice->GetDeviceCaps( &oCaps );

	if ( m_dwTexWidth > oCaps.MaxTextureWidth )
	{
		m_fTextScale = TFLOAT( oCaps.MaxTextureWidth ) / TFLOAT( m_dwTexWidth );
		m_dwTexWidth = m_dwTexHeight = oCaps.MaxTextureWidth;
	}

	HRESULT hRes = m_pDevice->CreateTexture( m_dwTexWidth, m_dwTexHeight, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pTexture );

	if ( FAILED( hRes ) )
		return hRes;

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

	// Copy the alpha of the bitmap to the texture
	D3DLOCKED_RECT oLockedRect;
	m_pTexture->LockRect( 0, &oLockedRect, 0, 0 );

	BYTE* pDstRow = (BYTE*)oLockedRect.pBits;

	for ( y = 0; y < m_dwTexHeight; y++ )
	{
		WORD* pDst16 = (WORD*)pDstRow;

		for ( x = 0; x < m_dwTexWidth; x++ )
		{
			BYTE bAlpha = BYTE( ( pBitmapBits[ m_dwTexWidth * y + x ] & 0xff ) >> 4 );

			if ( bAlpha > 0 )
				*pDst16++ = WORD( ( bAlpha << 12 ) | 0x0fff );
			else
				*pDst16++ = 0x0000;
		}

		pDstRow += oLockedRect.Pitch;
	}

	m_pTexture->UnlockRect( 0 );
	DeleteObject( hbmBitmap );
	DeleteDC( hDC );
	DeleteObject( hFont );

	return S_OK;
}

// $Barnyard: FUNCTION 006ddb70
HRESULT CD3DFont::RestoreDeviceObjects()
{
	HRESULT hRes = m_pDevice->CreateVertexBuffer( MAX_NUM_VERTICES * sizeof( FONT2DVERTEX ), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_pVB );

	if ( FAILED( hRes ) )
		return hRes;

	// Record the saved state block and the one used for drawing text
	for ( TUINT i = 0; i < 2; i++ )
	{
		m_pDevice->BeginStateBlock();
		m_pDevice->SetTexture( 0, m_pTexture );

		if ( m_dwFontFlags & D3DFONT_ZENABLE )
			m_pDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
		else
			m_pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );

		m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		m_pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		m_pDevice->SetRenderState( D3DRS_ALPHAREF, 0x08 );
		m_pDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
		m_pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		m_pDevice->SetRenderState( D3DRS_STENCILENABLE, FALSE );
		m_pDevice->SetRenderState( D3DRS_CLIPPING, TRUE );
		m_pDevice->SetRenderState( D3DRS_EDGEANTIALIAS, FALSE );
		m_pDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, FALSE );
		m_pDevice->SetRenderState( D3DRS_VERTEXBLEND, FALSE );
		m_pDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		m_pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );

		m_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		m_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		m_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		m_pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
		m_pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
		m_pDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );
		m_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		m_pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		m_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
		m_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

		if ( i == 0 )
			m_pDevice->EndStateBlock( &m_dwSavedStateBlock );
		else
			m_pDevice->EndStateBlock( &m_dwDrawTextStateBlock );
	}

	return S_OK;
}

// $Barnyard: FUNCTION 006dde00
HRESULT CD3DFont::InvalidateDeviceObjects()
{
	if ( m_pVB )
	{
		m_pVB->Release();
		m_pVB = TNULL;
	}

	if ( m_pDevice )
	{
		if ( m_dwSavedStateBlock )
			m_pDevice->DeleteStateBlock( m_dwSavedStateBlock );

		if ( m_dwDrawTextStateBlock )
			m_pDevice->DeleteStateBlock( m_dwDrawTextStateBlock );
	}

	m_dwSavedStateBlock    = 0;
	m_dwDrawTextStateBlock = 0;

	return S_OK;
}

// $Barnyard: FUNCTION 006dde70
HRESULT CD3DFont::DeleteDeviceObjects()
{
	if ( m_pTexture )
	{
		m_pTexture->Release();
		m_pTexture = TNULL;
	}

	m_pDevice = TNULL;

	return S_OK;
}

// $Barnyard: FUNCTION 006ddea0
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

// $Barnyard: FUNCTION 006ddfc0
HRESULT CD3DFont::DrawText( TFLOAT a_fX, TFLOAT a_fY, DWORD a_dwColour, const TCHAR* a_szText, DWORD a_dwFlags )
{
	if ( m_pDevice == TNULL )
		return E_FAIL;

	// Set up the render states
	m_pDevice->CaptureStateBlock( m_dwSavedStateBlock );
	m_pDevice->ApplyStateBlock( m_dwDrawTextStateBlock );
	m_pDevice->SetVertexShader( D3DFVF_FONT2DVERTEX );
	m_pDevice->SetPixelShader( NULL );
	m_pDevice->SetStreamSource( 0, m_pVB, sizeof( FONT2DVERTEX ) );

	if ( a_dwFlags & D3DFONT_FILTERED )
	{
		m_pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
		m_pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	}

	TFLOAT fStartX = a_fX;

	FONT2DVERTEX* pVertices      = TNULL;
	DWORD         dwNumTriangles = 0;
	m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );

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
					m_pVB->Unlock();
					m_pDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );
					pVertices = TNULL;
					m_pVB->Lock( 0, 0, (BYTE**)&pVertices, D3DLOCK_DISCARD );
					dwNumTriangles = 0;
				}
			}

			a_fX += w;
		}
	}

	m_pVB->Unlock();

	if ( dwNumTriangles > 0 )
		m_pDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, dwNumTriangles );

	// Restore the render states
	m_pDevice->ApplyStateBlock( m_dwSavedStateBlock );

	return S_OK;
}

TOSHI_NAMESPACE_END
