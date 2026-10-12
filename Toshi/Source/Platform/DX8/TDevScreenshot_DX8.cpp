#include "ToshiPCH.h"

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )

#  include "TDevScreenshot_DX8.h"
#  include "Toshi/T2String8.h"

#  define STB_IMAGE_WRITE_IMPLEMENTATION
#  define STB_IMAGE_WRITE_STATIC
#  include <stb_image_write.h>

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

void TDevScreenshot::Update( IDirect3DDevice8* a_pDevice )
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

	if ( !s_bEnabled ) return;

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

	if ( Capture( a_pDevice, szFileName ) )
		TINFO( "[devshot] %.1fs: saved %s\n", fTime, szFileName );
	else
		TWARN( "[devshot] %.1fs: couldn't save %s\n", fTime, szFileName );

	fflush( stdout );
}

TBOOL TDevScreenshot::Capture( IDirect3DDevice8* a_pDevice, const TCHAR* a_szFileName )
{
	IDirect3DSurface8* pBackBuffer = TNULL;
	if ( FAILED( a_pDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer ) ) )
		return TFALSE;

	D3DSURFACE_DESC oDesc;
	pBackBuffer->GetDesc( &oDesc );

	// The back buffer can't be locked, so copy it into a system memory surface first
	IDirect3DSurface8* pImage = TNULL;
	if ( FAILED( a_pDevice->CreateImageSurface( oDesc.Width, oDesc.Height, oDesc.Format, &pImage ) ) )
	{
		pBackBuffer->Release();
		return TFALSE;
	}

	HRESULT hRes = a_pDevice->CopyRects( pBackBuffer, NULL, 0, pImage, NULL );
	pBackBuffer->Release();

	D3DLOCKED_RECT oLocked;
	if ( FAILED( hRes ) || FAILED( pImage->LockRect( &oLocked, NULL, D3DLOCK_READONLY ) ) )
	{
		pImage->Release();
		return TFALSE;
	}

	const TINT iWidth  = TINT( oDesc.Width );
	const TINT iHeight = TINT( oDesc.Height );
	TUINT8*    pRGB    = new TUINT8[ iWidth * iHeight * 3 ];
	TBOOL      bKnown  = TTRUE;

	for ( TINT iY = 0; iY < iHeight && bKnown; iY++ )
	{
		const TUINT8* pRow = TSTATICCAST( const TUINT8, oLocked.pBits ) + iY * oLocked.Pitch;
		TUINT8*       pOut = pRGB + iY * iWidth * 3;

		for ( TINT iX = 0; iX < iWidth; iX++, pOut += 3 )
		{
			switch ( oDesc.Format )
			{
				case D3DFMT_X8R8G8B8:
				case D3DFMT_A8R8G8B8:
				{
					const TUINT8* pPixel = pRow + iX * 4;
					pOut[ 0 ]            = pPixel[ 2 ];
					pOut[ 1 ]            = pPixel[ 1 ];
					pOut[ 2 ]            = pPixel[ 0 ];
					break;
				}
				case D3DFMT_R8G8B8:
				{
					const TUINT8* pPixel = pRow + iX * 3;
					pOut[ 0 ]            = pPixel[ 2 ];
					pOut[ 1 ]            = pPixel[ 1 ];
					pOut[ 2 ]            = pPixel[ 0 ];
					break;
				}
				case D3DFMT_R5G6B5:
				{
					const TUINT16 uiPixel = TREINTERPRETCAST( const TUINT16*, pRow )[ iX ];
					pOut[ 0 ]             = TUINT8( ( ( uiPixel >> 11 ) & 0x1F ) * 255 / 31 );
					pOut[ 1 ]             = TUINT8( ( ( uiPixel >> 5 ) & 0x3F ) * 255 / 63 );
					pOut[ 2 ]             = TUINT8( ( uiPixel & 0x1F ) * 255 / 31 );
					break;
				}
				case D3DFMT_X1R5G5B5:
				case D3DFMT_A1R5G5B5:
				{
					const TUINT16 uiPixel = TREINTERPRETCAST( const TUINT16*, pRow )[ iX ];
					pOut[ 0 ]             = TUINT8( ( ( uiPixel >> 10 ) & 0x1F ) * 255 / 31 );
					pOut[ 1 ]             = TUINT8( ( ( uiPixel >> 5 ) & 0x1F ) * 255 / 31 );
					pOut[ 2 ]             = TUINT8( ( uiPixel & 0x1F ) * 255 / 31 );
					break;
				}
				default:
					bKnown = TFALSE;
					break;
			}

			if ( !bKnown ) break;
		}
	}

	pImage->UnlockRect();
	pImage->Release();

	TBOOL bSaved = TFALSE;

	if ( bKnown )
		bSaved = stbi_write_png( a_szFileName, iWidth, iHeight, 3, pRGB, iWidth * 3 ) != 0;
	else
		TWARN( "[devshot] unsupported back buffer format %d\n", oDesc.Format );

	delete[] pRGB;
	return bSaved;
}

TOSHI_NAMESPACE_END

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
