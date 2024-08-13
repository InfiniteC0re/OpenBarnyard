#include "pch.h"
#include "AOptions.h"
#include "Locale/ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AOptions );

AOptions::AOptions()
{
	m_eUpdatedSettings = UpdatedSettings_None;
	m_eLanguage		   = ALocaleManager::Lang_UNKNOWN;

	ReloadSettings();
}

TBOOL AOptions::ApplySettings()
{
	TIMPLEMENT();

	if ( m_eUpdatedSettings & UpdatedSettings_Audio )
	{
	}

	if ( m_eUpdatedSettings & UpdatedSettings_Language )
	{
		T2Locale::GetSingleton()->SetLanguage( ALocaleManager::Lang_EnglishUK );
		m_eLanguage = ALocaleManager::Lang_EnglishUK;
	}

	if ( m_eUpdatedSettings & UpdatedSettings_Input )
	{
	}

	m_eUpdatedSettings = UpdatedSettings_None;
	return TTRUE;
}

void AOptions::LoadSettings()
{
	m_fGamma			 = 0.420849f;
	m_iWidth			 = 800;
	m_iHeight			 = 600;
	m_fSensitivity		 = 0.5f;
	m_bHighDetailGrass	 = TTRUE;
	m_bControllerEnabled = TFALSE;
	m_bForcedWindowed	 = TFALSE;

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
		dwSize		= sizeof( iWidth );
		if ( ERROR_SUCCESS == RegQueryValueExA( hKey, "Width", NULL, &dwType, (LPBYTE)&dwData, &dwSize ) &&
			 dwType == REG_DWORD )
		{
			iWidth = dwData;
		}

		TINT iHeight = 0;
		dwSize		 = sizeof( iHeight );
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
