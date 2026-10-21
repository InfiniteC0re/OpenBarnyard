#include "ToshiPCH.h"

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )

#  include "TDevScreenshot_DX11.h"
#  include "Toshi/T2String8.h"

#  include <wincodec.h>
#  pragma comment( lib, "windowscodecs.lib" )
#  pragma comment( lib, "ole32.lib" )

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

static constexpr TINT MAX_SHOTS = 64;

// Copies the argument that follows a_szSwitch (which includes the trailing space) into a_szOut.
// Returns TFALSE if the switch isn't on the command line.
static TBOOL DevShot_GetArg( const TCHAR* a_szSwitch, TCHAR* a_szOut, TINT a_iSize )
{
	const TCHAR* szArg = T2String8::FindString( GetCommandLineA(), a_szSwitch );
	if ( !szArg ) return TFALSE;

	szArg += T2String8::Length( a_szSwitch );
	while ( *szArg == ' ' ) szArg++;

	const TBOOL bQuoted = *szArg == '"';
	if ( bQuoted ) szArg++;

	TINT iLength = 0;
	while ( szArg[ iLength ] != '\0' && iLength < a_iSize - 1 && ( bQuoted ? szArg[ iLength ] != '"' : szArg[ iLength ] != ' ' ) )
	{
		a_szOut[ iLength ] = szArg[ iLength ];
		iLength++;
	}

	a_szOut[ iLength ] = '\0';
	return TTRUE;
}

// Seconds since the process was created
static TFLOAT DevShot_GetTimeSinceStart()
{
	FILETIME oCreation, oExit, oKernel, oUser, oNow;
	GetProcessTimes( GetCurrentProcess(), &oCreation, &oExit, &oKernel, &oUser );
	GetSystemTimeAsFileTime( &oNow );

	ULARGE_INTEGER uiCreation, uiNow;
	uiCreation.LowPart  = oCreation.dwLowDateTime;
	uiCreation.HighPart = oCreation.dwHighDateTime;
	uiNow.LowPart       = oNow.dwLowDateTime;
	uiNow.HighPart      = oNow.dwHighDateTime;

	return TFLOAT( uiNow.QuadPart - uiCreation.QuadPart ) / 10000000.0f;
}

void TDevScreenshot::Update( ID3D11Device* a_pDevice, ID3D11DeviceContext* a_pContext, ID3D11Texture2D* a_pBackBuffer )
{
	static TBOOL  s_bInitialised = TFALSE;
	static TBOOL  s_bEnabled     = TFALSE;
	static TCHAR  s_szPrefix[ MAX_PATH ];
	static TFLOAT s_aTimes[ MAX_SHOTS ];
	static TBOOL  s_aTaken[ MAX_SHOTS ];
	static TINT   s_iNumTimes = 0;
	static TFLOAT s_fInterval = 0.0f;
	static TFLOAT s_fNextShot = 0.0f;

	if ( !s_bInitialised )
	{
		s_bInitialised = TTRUE;
		s_bEnabled     = DevShot_GetArg( "-devshot ", s_szPrefix, sizeof( s_szPrefix ) ) && s_szPrefix[ 0 ] != '\0';

		if ( !s_bEnabled ) return;

		TCHAR szList[ 512 ];
		if ( DevShot_GetArg( "-devshotat ", szList, sizeof( szList ) ) )
		{
			// Comma separated list of seconds
			const TCHAR* pCursor = szList;

			while ( *pCursor != '\0' && s_iNumTimes < MAX_SHOTS )
			{
				TCHAR* pEnd;
				TFLOAT fTime = TFLOAT( strtod( pCursor, &pEnd ) );

				if ( pEnd == pCursor ) break;

				s_aTimes[ s_iNumTimes ] = fTime;
				s_aTaken[ s_iNumTimes ] = TFALSE;
				s_iNumTimes++;

				pCursor = ( *pEnd == ',' ) ? pEnd + 1 : pEnd;
			}
		}

		if ( s_iNumTimes == 0 )
		{
			TCHAR szInterval[ 32 ];
			s_fInterval = DevShot_GetArg( "-devshotevery ", szInterval, sizeof( szInterval ) ) ? TFLOAT( atof( szInterval ) ) : 15.0f;
			if ( s_fInterval < 1.0f ) s_fInterval = 1.0f;
			s_fNextShot = s_fInterval;
		}

		TINFO( "[devshot] saving screenshots to %s_<seconds>.png\n", s_szPrefix );
	}

	if ( !s_bEnabled || !a_pBackBuffer ) return;

	const TFLOAT fTime = DevShot_GetTimeSinceStart();
	TFLOAT       fShotTime;

	if ( s_iNumTimes != 0 )
	{
		// One shot per frame: the earliest of the times that have passed
		TINT iShot = -1;

		for ( TINT i = 0; i < s_iNumTimes; i++ )
		{
			if ( !s_aTaken[ i ] && s_aTimes[ i ] <= fTime && ( iShot == -1 || s_aTimes[ i ] < s_aTimes[ iShot ] ) )
				iShot = i;
		}

		if ( iShot == -1 ) return;

		s_aTaken[ iShot ] = TTRUE;
		fShotTime         = s_aTimes[ iShot ];
	}
	else
	{
		if ( fTime < s_fNextShot ) return;

		fShotTime = s_fNextShot;
		s_fNextShot += s_fInterval;
		if ( s_fNextShot <= fTime ) s_fNextShot = fTime + s_fInterval;
	}

	TCHAR szFileName[ MAX_PATH + 32 ];
	if ( fShotTime == TFLOAT( TINT( fShotTime ) ) )
		T2String8::Format( szFileName, sizeof( szFileName ), "%s_%d.png", s_szPrefix, TINT( fShotTime ) );
	else
		T2String8::Format( szFileName, sizeof( szFileName ), "%s_%g.png", s_szPrefix, fShotTime );

	if ( Capture( a_pDevice, a_pContext, a_pBackBuffer, szFileName ) )
		TINFO( "[devshot] %s saved (%.1fs)\n", szFileName, fTime );
	else
		TWARN( "[devshot] couldn't save %s (%.1fs)\n", szFileName, fTime );

	fflush( stdout );
}

// Writes 24 bit BGR pixels as a PNG file
static TBOOL DevShot_WritePNG( const TCHAR* a_szFileName, TINT a_iWidth, TINT a_iHeight, TUINT8* a_pBGR )
{
	const HRESULT hrCOM = CoInitializeEx( NULL, COINIT_MULTITHREADED );

	WCHAR wszFileName[ MAX_PATH + 32 ];
	MultiByteToWideChar( CP_ACP, 0, a_szFileName, -1, wszFileName, MAX_PATH + 32 );

	IWICImagingFactory*    pFactory = TNULL;
	IWICStream*            pStream  = TNULL;
	IWICBitmapEncoder*     pEncoder = TNULL;
	IWICBitmapFrameEncode* pFrame   = TNULL;

	HRESULT hr = CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &pFactory ) );
	if ( SUCCEEDED( hr ) ) hr = pFactory->CreateStream( &pStream );
	if ( SUCCEEDED( hr ) ) hr = pStream->InitializeFromFilename( wszFileName, GENERIC_WRITE );
	if ( SUCCEEDED( hr ) ) hr = pFactory->CreateEncoder( GUID_ContainerFormatPng, NULL, &pEncoder );
	if ( SUCCEEDED( hr ) ) hr = pEncoder->Initialize( pStream, WICBitmapEncoderNoCache );
	if ( SUCCEEDED( hr ) ) hr = pEncoder->CreateNewFrame( &pFrame, NULL );
	if ( SUCCEEDED( hr ) ) hr = pFrame->Initialize( NULL );
	if ( SUCCEEDED( hr ) ) hr = pFrame->SetSize( a_iWidth, a_iHeight );

	WICPixelFormatGUID oFormat = GUID_WICPixelFormat24bppBGR;
	if ( SUCCEEDED( hr ) ) hr = pFrame->SetPixelFormat( &oFormat );
	if ( SUCCEEDED( hr ) && oFormat != GUID_WICPixelFormat24bppBGR ) hr = E_FAIL;
	if ( SUCCEEDED( hr ) ) hr = pFrame->WritePixels( a_iHeight, a_iWidth * 3, a_iWidth * a_iHeight * 3, a_pBGR );
	if ( SUCCEEDED( hr ) ) hr = pFrame->Commit();
	if ( SUCCEEDED( hr ) ) hr = pEncoder->Commit();

	if ( pFrame ) pFrame->Release();
	if ( pEncoder ) pEncoder->Release();
	if ( pStream ) pStream->Release();
	if ( pFactory ) pFactory->Release();

	if ( SUCCEEDED( hrCOM ) ) CoUninitialize();

	return SUCCEEDED( hr );
}

TBOOL TDevScreenshot::Capture( ID3D11Device* a_pDevice, ID3D11DeviceContext* a_pContext, ID3D11Texture2D* a_pTexture, const TCHAR* a_szFileName )
{
	D3D11_TEXTURE2D_DESC oDesc;
	a_pTexture->GetDesc( &oDesc );

	// The back buffer can't be mapped, so copy it into a staging texture first
	D3D11_TEXTURE2D_DESC oStagingDesc = oDesc;
	oStagingDesc.MipLevels            = 1;
	oStagingDesc.ArraySize            = 1;
	oStagingDesc.SampleDesc.Count     = 1;
	oStagingDesc.SampleDesc.Quality   = 0;
	oStagingDesc.Usage                = D3D11_USAGE_STAGING;
	oStagingDesc.BindFlags            = 0;
	oStagingDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_READ;
	oStagingDesc.MiscFlags            = 0;

	ID3D11Texture2D* pStaging = TNULL;
	if ( FAILED( a_pDevice->CreateTexture2D( &oStagingDesc, NULL, &pStaging ) ) )
		return TFALSE;

	if ( oDesc.SampleDesc.Count > 1 )
		a_pContext->ResolveSubresource( pStaging, 0, a_pTexture, 0, oDesc.Format );
	else
		a_pContext->CopyResource( pStaging, a_pTexture );

	D3D11_MAPPED_SUBRESOURCE oMapped;
	if ( FAILED( a_pContext->Map( pStaging, 0, D3D11_MAP_READ, 0, &oMapped ) ) )
	{
		pStaging->Release();
		return TFALSE;
	}

	const TINT iWidth  = TINT( oDesc.Width );
	const TINT iHeight = TINT( oDesc.Height );
	TUINT8*    pBGR    = new TUINT8[ iWidth * iHeight * 3 ];
	TBOOL      bKnown  = TTRUE;

	// Byte order of the pixels: RGBA or BGRA
	TBOOL bIsBGRA = TFALSE;

	switch ( oDesc.Format )
	{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			break;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			bIsBGRA = TTRUE;
			break;
		default:
			bKnown = TFALSE;
			break;
	}

	if ( bKnown )
	{
		for ( TINT iY = 0; iY < iHeight; iY++ )
		{
			const TUINT8* pRow = TSTATICCAST( const TUINT8, oMapped.pData ) + iY * oMapped.RowPitch;
			TUINT8*       pOut = pBGR + iY * iWidth * 3;

			for ( TINT iX = 0; iX < iWidth; iX++, pOut += 3 )
			{
				const TUINT8* pPixel = pRow + iX * 4;
				pOut[ 0 ]            = bIsBGRA ? pPixel[ 0 ] : pPixel[ 2 ];
				pOut[ 1 ]            = pPixel[ 1 ];
				pOut[ 2 ]            = bIsBGRA ? pPixel[ 2 ] : pPixel[ 0 ];
			}
		}
	}

	a_pContext->Unmap( pStaging, 0 );
	pStaging->Release();

	TBOOL bSaved = TFALSE;

	if ( bKnown )
		bSaved = DevShot_WritePNG( a_szFileName, iWidth, iHeight, pBGR );
	else
		TWARN( "[devshot] unsupported back buffer format %d\n", oDesc.Format );

	delete[] pBGR;
	return bSaved;
}

TOSHI_NAMESPACE_END

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
