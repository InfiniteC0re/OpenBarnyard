#include "pch.h"
#include "AOptions.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMultiplexStream.h"
#include "Input/AInputHandler.h"

#ifdef BARNYARD_LATEST
#  include "GameInterface/ACameraOptionsState.h"
#  include "GameInterface/SaveLoadSKU.h"
#  include "Helpers/AGameLoader.h"

#  include <direct.h>
#endif // BARNYARD_LATEST

#include <windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( Music );
TPSTRING8_DECLARE( VO );
TPSTRING8_DECLARE( Ingame );

// NOTE: GetClass 00662d60, CreateObject 00663360, CreateObjectInPlace 006633c0
TDEFINE_CLASS( AOptions );

// $Barnyard: FUNCTION 006632d0
AOptions::AOptions()
{
	m_eUpdatedSettings = UpdatedSettings_None;
	m_iUnk04           = 1;
	m_iSfxVolume       = 204;
	m_iMusicVolume     = 128;
	m_iVOVolume        = 128;
	m_eLanguage        = ALocaleManager::Lang_UNKNOWN;
	m_eFlags           = Flags_Rumble;

	ReloadSettings();

#ifdef BARNYARD_LATEST
	m_uiCameraSpeed         = CAMERA_SPEED_DEFAULT;
	m_bWiiStyleCamera       = TTRUE;
	m_bFollowJumpsCamera    = TFALSE;
	m_bEasyDifficulty       = TFALSE;
	m_bExtraSettingsChanged = TFALSE;

#  ifdef BARNYARD_COMMUNITY_PATCH
	// "-devsavefile <path>" also moves OpenBarnyard.cfg, set it up before it's read
	AGameLoader::Dev_SetupSaveFileAtBoot();
#  endif // BARNYARD_COMMUNITY_PATCH

	LoadExtraSettings();
#endif // BARNYARD_LATEST
}

// $Barnyard: FUNCTION 00662e70
// $Barnyard: FUNCTION 00663140
AOptions::~AOptions()
{
#ifdef BARNYARD_LATEST
	// The game is closing, write the extra settings if they couldn't be written when they changed
	if ( m_bExtraSettingsChanged )
		WriteExtraSettings();
#endif // BARNYARD_LATEST
}

// $Barnyard: FUNCTION 00663160
TBOOL AOptions::ApplySettings()
{

	if ( m_eUpdatedSettings & UpdatedSettings_Audio )
	{
		ASoundManager* pSoundManager = ASoundManager::GetSingleton();
		pSoundManager->SetCategoryVolume( TPS8( Music ), m_iMusicVolume * ( 1.0f / 255.0f ), 0.0f );
		pSoundManager->SetCategoryVolume( TPS8( VO ), m_iVOVolume * ( 1.0f / 255.0f ), 0.0f );
		pSoundManager->SetCategoryVolume( TPS8( Ingame ), m_iSfxVolume * ( 1.0f / 255.0f ), 0.0f );

		// Apply the new volume to the music of the main game
		AMultiplexStream* pMultiplexStream = AMultiplexStream::IsSingletonCreated() ? AMultiplexStream::GetSingleton() : TNULL;

		if ( pMultiplexStream && pMultiplexStream->GetState() == AMultiplexStream::STATE_PLAYING )
		{
			for ( TINT i = 0; i < pMultiplexStream->GetNumSubChannels(); i++ )
				pMultiplexStream->SetVolume( pMultiplexStream->GetVolume( i ), i, TTRUE );
		}
	}

	if ( m_eUpdatedSettings & UpdatedSettings_Language )
	{
		g_pLocaleManager->SetLanguage( ALocaleManager::Lang_EnglishUK );
		m_eLanguage = ALocaleManager::Lang_EnglishUK;
	}

	if ( m_eUpdatedSettings & UpdatedSettings_Input )
	{
		AInputHandler::GetSingleton()->SetRumbleEnabled( IsRumbleEnabled() );
	}

	m_eUpdatedSettings = UpdatedSettings_None;
	return TTRUE;
}

// $Barnyard: FUNCTION 00662eb0
void AOptions::LoadSettings()
{
	m_fGamma             = 0.420849f;
	m_iWidth             = 800;
	m_iHeight            = 600;
	m_fSensitivity       = 0.5f;
	m_bHighDetailGrass   = TTRUE;
	m_bControllerEnabled = TFALSE;
	m_bForcedWindowed    = TFALSE;

	HKEY hKey;

	if ( ERROR_SUCCESS == RegOpenKeyExA( HKEY_CURRENT_USER, "Software\\THQ\\Barnyard", 0, KEY_READ, &hKey ) )
	{
		CHAR  szBuffer[ 512 ];
		DWORD dwType = REG_NONE;
		DWORD dwData = 0;
		DWORD dwSize = sizeof( dwData );

		if ( ERROR_SUCCESS == RegQueryValueExA( hKey, "RealForcedWindowed", NULL, &dwType, (LPBYTE)&dwData, &dwSize ) &&
		     dwType == REG_DWORD )
		{
			m_bForcedWindowed = dwData != FALSE;
		}

		dwSize = sizeof( dwData );
		if ( ERROR_SUCCESS == RegQueryValueExA( hKey, "HighDetailGrass", NULL, &dwType, (LPBYTE)&dwData, &dwSize ) &&
		     dwType == REG_DWORD )
		{
			m_bHighDetailGrass = dwData != FALSE;
		}

		dwSize = sizeof( szBuffer );
		if ( ERROR_SUCCESS == RegQueryValueExA( hKey, "Gamma", NULL, &dwType, (LPBYTE)szBuffer, &dwSize ) &&
		     dwType == REG_SZ )
		{
			dwSize -= 1;
			TMath::Clip( dwSize, (DWORD)0, (DWORD)16 );

			for ( DWORD i = 0; i < dwSize; i++ )
			{
				if ( szBuffer[ i ] == ',' )
				{
					szBuffer[ i ] = '.';
				}
			}

			m_fGamma = TStringManager::String8ToFloat( szBuffer );
			TMath::Clip( m_fGamma, 0.0f, 1.0f );
		}

		TINT iWidth = 0;
		dwSize      = sizeof( iWidth );
		if ( ERROR_SUCCESS == RegQueryValueExA( hKey, "Width", NULL, &dwType, (LPBYTE)&dwData, &dwSize ) &&
		     dwType == REG_DWORD )
		{
			iWidth = dwData;
		}

		TINT iHeight = 0;
		dwSize       = sizeof( iHeight );
		if ( ERROR_SUCCESS == RegQueryValueExA( hKey, "Height", NULL, &dwType, (LPBYTE)&dwData, &dwSize ) &&
		     dwType == REG_DWORD )
		{
			iHeight = dwData;
		}

		if ( IsResolutionCompatible( iWidth, iHeight ) )
		{
			m_iWidth  = iWidth;
			m_iHeight = iHeight;
		}

		dwSize = sizeof( szBuffer );
		if ( ERROR_SUCCESS == RegQueryValueExA( hKey, "Sensitivity", NULL, &dwType, (LPBYTE)szBuffer, &dwSize ) &&
		     dwType == REG_SZ )
		{
			m_fSensitivity = TStringManager::String8ToFloat( szBuffer );
			TMath::Clip( m_fSensitivity, 0.0f, 1.0f );
		}

		dwSize = sizeof( dwData );
		if ( ERROR_SUCCESS == RegQueryValueExA( hKey, "ControllerEnabled", NULL, &dwType, (LPBYTE)&dwData, &dwSize ) &&
		     dwType == REG_DWORD )
		{
			m_bControllerEnabled = dwData != FALSE;
		}
	}
}

// $Barnyard: FUNCTION 00662d90
TBOOL AOptions::IsResolutionCompatible( TINT a_iWidth, TINT a_iHeight )
{
	static constexpr struct
	{
		TINT iWidth;
		TINT iHeight;
	} s_Resolutions[] = {
		{ 600, 800 },
		{ 768, 1024 },
		{ 864, 1152 },
		{ 1024, 1280 },
		{ 640, 480 },
		{ 800, 600 },
		{ 1024, 768 },
		{ 1152, 864 },
		{ 1280, 1024 },
		{ 1600, 1200 },
	};

	for ( TSIZE i = 0; i < TARRAYSIZE( s_Resolutions ); i++ )
	{
		auto pResolution = &s_Resolutions[ i ];

		if ( a_iWidth == pResolution->iWidth && a_iHeight == pResolution->iHeight )
		{
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00662d70
TBOOL AOptions::SaveSettings( void* a_pBuffer, TINT a_iSize )
{
	TUtil::MemCopy( a_pBuffer, &m_iUnk04, SAVED_SETTINGS_SIZE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00662e90
TBOOL AOptions::LoadSavedSettings( const void* a_pBuffer, TINT a_iSize )
{
	TUtil::MemCopy( &m_iUnk04, a_pBuffer, SAVED_SETTINGS_SIZE );
	m_eUpdatedSettings = UpdatedSettings_ForceUpdate;
	return TTRUE;
}

// $Barnyard: FUNCTION 00663280
void AOptions::SetLanguage( T2Locale::Lang a_eLanguage )
{
	m_eLanguage = a_eLanguage;
	m_eUpdatedSettings |= UpdatedSettings_Language;
}

TBOOL AOptions::IsWindowed() const
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// Allow forcing the windowed mode from the command line
	if ( TStringManager::String8FindString( GetCommandLineA(), "-windowed" ) )
		return TTRUE;

	// "-background" runs (see TMSWindow::IsBackgroundMode) are always windowed
	if ( TStringManager::String8FindString( GetCommandLineA(), "-background" ) )
		return TTRUE;
#endif

	return IsDebuggerPresent() || m_bForcedWindowed;
}

#ifdef BARNYARD_LATEST

void AOptions::GetExtraSettingsPath( TCHAR ( &a_rPath )[ 260 ], TCHAR ( &a_rDir )[ 260 ] )
{
#  ifdef BARNYARD_COMMUNITY_PATCH
	const TCHAR* szDevSavePath = SaveLoadSKU::Dev_GetSavePath();

	if ( szDevSavePath[ 0 ] != '\0' )
	{
		// Development: <dev save file>.cfg
		TStringManager::String8Copy( a_rPath, szDevSavePath, sizeof( a_rPath ) - 5 );
		strcat( a_rPath, ".cfg" );
		a_rDir[ 0 ] = '\0';
		return;
	}
#  endif

	// <exe directory>Save\OpenBarnyard.cfg, next to savedata.by (SaveLoadSKU)
	GetModuleFileNameA( NULL, a_rDir, sizeof( a_rDir ) - 32 );
	a_rDir[ sizeof( a_rDir ) - 32 ] = '\0';
	TCHAR* pSlash                   = strrchr( a_rDir, '\\' );

	if ( pSlash )
		pSlash[ 1 ] = '\0';

	strcat( a_rDir, "Save" );
	strcpy( a_rPath, a_rDir );
	strcat( a_rPath, "\\OpenBarnyard.cfg" );
}

void AOptions::LoadExtraSettings()
{
	TCHAR szPath[ 260 ];
	TCHAR szDir[ 260 ];
	GetExtraSettingsPath( szPath, szDir );

	FILE* pFile = fopen( szPath, "r" );

	if ( !pFile )
		return;

	// "Name=Value" lines, unknown names are ignored
	TCHAR szLine[ 128 ];

	while ( fgets( szLine, sizeof( szLine ), pFile ) )
	{
		TCHAR* pEquals = strchr( szLine, '=' );

		if ( szLine[ 0 ] == '#' || !pEquals )
			continue;

		*pEquals    = '\0';
		TINT iValue = atoi( pEquals + 1 );

		if ( !strcmp( szLine, "CameraSpeed" ) )
		{
			// NOTE: the speed slider is hidden for now (ACameraOptionsState::SHOW_SPEED_SLIDER), so the Wii Style camera
			// always uses the default speed. The value stays in the file so the slider can come back.
			if ( iValue >= 0 && iValue <= CAMERA_SPEED_MAX && ACameraOptionsState::SHOW_SPEED_SLIDER )
				m_uiCameraSpeed = TUINT8( iValue );
		}
		else if ( !strcmp( szLine, "WiiStyleCamera" ) )
			m_bWiiStyleCamera = iValue != 0;
		else if ( !strcmp( szLine, "FollowJumpsCamera" ) )
			m_bFollowJumpsCamera = iValue != 0;
		else if ( !strcmp( szLine, "EasyDifficulty" ) )
			m_bEasyDifficulty = iValue != 0;
	}

	fclose( pFile );
	TINFO( "Loaded %s (camera: Wii Style %d, Follow Jumps %d, speed %d; easy difficulty %d)\n", szPath, m_bWiiStyleCamera, m_bFollowJumpsCamera, m_uiCameraSpeed, m_bEasyDifficulty );
}

void AOptions::SaveExtraSettings()
{
	m_bExtraSettingsChanged = !WriteExtraSettings();
}

TBOOL AOptions::WriteExtraSettings()
{
	TCHAR szPath[ 260 ];
	TCHAR szDir[ 260 ];
	GetExtraSettingsPath( szPath, szDir );

	// The save directory doesn't exist before the first save
	if ( szDir[ 0 ] != '\0' && GetFileAttributesA( szDir ) == INVALID_FILE_ATTRIBUTES )
		_mkdir( szDir );

	FILE* pFile = fopen( szPath, "w" );

	if ( !pFile )
	{
		TWARN( "Couldn't write %s\n", szPath );
		return TFALSE;
	}

	fprintf( pFile, "# OpenBarnyard settings that the original game doesn't have (savedata.by stays compatible with it)\n" );
	fprintf( pFile, "CameraSpeed=%d\n", m_uiCameraSpeed );
	fprintf( pFile, "WiiStyleCamera=%d\n", m_bWiiStyleCamera ? 1 : 0 );
	fprintf( pFile, "FollowJumpsCamera=%d\n", m_bFollowJumpsCamera ? 1 : 0 );
	fprintf( pFile, "EasyDifficulty=%d\n", m_bEasyDifficulty ? 1 : 0 );

	const TBOOL bWritten = ferror( pFile ) == 0;
	fclose( pFile );

	TINFO( "Saved %s (camera: Wii Style %d, Follow Jumps %d, speed %d; easy difficulty %d)\n", szPath, m_bWiiStyleCamera, m_bFollowJumpsCamera, m_uiCameraSpeed, m_bEasyDifficulty );
	return bWritten;
}

#endif // BARNYARD_LATEST
