#include "ToshiPCH.h"
#include "TMouse_D3D8_Win.h"

#include <DirectXTex.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TMouse_D3D8_Win );

TMouse_D3D8_Win::~TMouse_D3D8_Win()
{
	if ( m_hCursor )
		DestroyCursor( m_hCursor );
}

// Loads an image like D3DXLoadSurfaceFromFileW does into a 32x32 D3DFMT_A8R8G8B8 surface
static TBOOL LoadCursorImage( const TWCHAR* a_wszFileName, DirectX::ScratchImage& a_rImage )
{
	DirectX::TexMetadata  oMetadata;
	DirectX::ScratchImage oLoaded;
	HRESULT               hRes;

	const TWCHAR* wszExtension = wcsrchr( a_wszFileName, L'.' );

	if ( wszExtension && _wcsicmp( wszExtension, L".dds" ) == 0 )
	{
		hRes = DirectX::LoadFromDDSFile( a_wszFileName, DirectX::DDS_FLAGS_NONE, &oMetadata, oLoaded );
	}
	else if ( wszExtension && _wcsicmp( wszExtension, L".tga" ) == 0 )
	{
		hRes = DirectX::LoadFromTGAFile( a_wszFileName, DirectX::TGA_FLAGS_BGR, &oMetadata, oLoaded );
	}
	else
	{
		CoInitializeEx( TNULL, COINIT_MULTITHREADED );
		hRes = DirectX::LoadFromWICFile( a_wszFileName, DirectX::WIC_FLAGS_NONE, &oMetadata, oLoaded );
	}

	if ( FAILED( hRes ) )
		return TFALSE;

	DirectX::ScratchImage oDecompressed;
	const DirectX::Image* pImage = oLoaded.GetImage( 0, 0, 0 );

	if ( DirectX::IsCompressed( pImage->format ) )
	{
		if ( FAILED( DirectX::Decompress( *pImage, DXGI_FORMAT_B8G8R8A8_UNORM, oDecompressed ) ) )
			return TFALSE;

		pImage = oDecompressed.GetImage( 0, 0, 0 );
	}

	DirectX::ScratchImage oConverted;

	if ( pImage->format != DXGI_FORMAT_B8G8R8A8_UNORM )
	{
		if ( FAILED( DirectX::Convert( *pImage, DXGI_FORMAT_B8G8R8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, oConverted ) ) )
			return TFALSE;

		pImage = oConverted.GetImage( 0, 0, 0 );
	}

	// D3DX_FILTER_LINEAR
	if ( pImage->width != 32 || pImage->height != 32 )
		return SUCCEEDED( DirectX::Resize( *pImage, 32, 32, DirectX::TEX_FILTER_LINEAR, a_rImage ) );

	return SUCCEEDED( a_rImage.InitializeFromImage( *pImage ) );
}

TBOOL TMouse_D3D8_Win::SetCursor( const TWCHAR* a_wszFileName )
{
	DirectX::ScratchImage oImage;

	if ( !LoadCursorImage( a_wszFileName, oImage ) )
		return TFALSE;

	const DirectX::Image* pImage = oImage.GetImage( 0, 0, 0 );

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
		return TFALSE;

	for ( TUINT y = 0; y < 32; y++ )
	{
		const TUINT32* pSrc = TREINTERPRETCAST( const TUINT32*, pImage->pixels + y * pImage->rowPitch );

		for ( TUINT x = 0; x < 32; x++ )
		{
			// The colour key (opaque black) becomes transparent black
			const TUINT32 uiColour = pSrc[ x ];
			pBits[ y * 32 + x ]    = ( uiColour == 0xff000000 ) ? 0x00000000 : uiColour;
		}
	}

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
