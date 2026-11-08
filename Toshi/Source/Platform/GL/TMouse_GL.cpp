#include "ToshiPCH.h"
#include "TMouse_GL.h"
#include "TRenderInterface_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TMouse_GL );

TMouse_GL::~TMouse_GL()
{
	if ( m_hCursor )
		DestroyCursor( m_hCursor );
}

TBOOL TMouse_GL::SetCursor( const TWCHAR* a_wszFileName )
{
	TCHAR szFileName[ 260 ];
	TStringManager::StringUnicodeToChar( szFileName, a_wszFileName, -1 );

	TFile* pFile = TFile::Create( szFileName );

	if ( !pFile )
		return TFALSE;

	const TUINT uiSize = pFile->GetSize();
	TBYTE*      pData  = new TBYTE[ uiSize ];
	pFile->Read( pData, uiSize );
	pFile->Destroy();

	TUINT32 uiWidth  = 0;
	TUINT32 uiHeight = 0;
	TUINT8* pPixels  = TRenderGLInterface::DecodeImage( pData, uiSize, uiWidth, uiHeight );
	delete[] pData;

	if ( !pPixels )
		return TFALSE;

	BITMAPV5HEADER oHeader = {};
	oHeader.bV5Size        = sizeof( BITMAPV5HEADER );
	oHeader.bV5Width       = 32;
	oHeader.bV5Height      = -32;
	oHeader.bV5Planes      = 1;
	oHeader.bV5BitCount    = 32;
	oHeader.bV5Compression = BI_BITFIELDS;
	oHeader.bV5RedMask     = 0x00FF0000;
	oHeader.bV5GreenMask   = 0x0000FF00;
	oHeader.bV5BlueMask    = 0x000000FF;
	oHeader.bV5AlphaMask   = 0xFF000000;

	TUINT32* pBits   = TNULL;
	HDC      hDC     = GetDC( NULL );
	HBITMAP  hColour = CreateDIBSection( hDC, TREINTERPRETCAST( BITMAPINFO*, &oHeader ), DIB_RGB_COLORS, (void**)&pBits, NULL, 0 );
	ReleaseDC( NULL, hDC );

	if ( !hColour )
	{
		TFree( pPixels );
		return TFALSE;
	}

	for ( TUINT y = 0; y < 32; y++ )
	{
		for ( TUINT x = 0; x < 32; x++ )
		{
			const TUINT8* pSrc = pPixels + ( y * uiHeight / 32 * uiWidth + x * uiWidth / 32 ) * 4;
			const TUINT32 uiColour = ( TUINT32( pSrc[ 3 ] ) << 24 ) | ( TUINT32( pSrc[ 0 ] ) << 16 ) | ( TUINT32( pSrc[ 1 ] ) << 8 ) | pSrc[ 2 ];

			// The colour key (opaque black) becomes transparent black
			pBits[ y * 32 + x ] = ( uiColour == 0xff000000 ) ? 0x00000000 : uiColour;
		}
	}

	TFree( pPixels );

	HBITMAP hMask = CreateBitmap( 32, 32, 1, 1, TNULL );

	ICONINFO oIconInfo = {};
	oIconInfo.fIcon    = FALSE;
	oIconInfo.xHotspot = 0;
	oIconInfo.yHotspot = 0;
	oIconInfo.hbmMask  = hMask;
	oIconInfo.hbmColor = hColour;

	HCURSOR hCursor = CreateIconIndirect( &oIconInfo );

	DeleteObject( hColour );
	DeleteObject( hMask );

	if ( !hCursor )
		return TFALSE;

	if ( m_hCursor )
		DestroyCursor( m_hCursor );

	m_hCursor = hCursor;
	::SetCursor( m_hCursor );

	return TTRUE;
}

TOSHI_NAMESPACE_END
