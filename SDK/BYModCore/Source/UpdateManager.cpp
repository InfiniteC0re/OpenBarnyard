#include "pch.h"
#include "UpdateManager.h"
#include "AHTTPClient.h"
#include "json.hpp"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TVersion g_uiCurrentVersion = TVERSION( 1, 0 );

#define NEW_DLL_NAME    L"BYModCore.dll"
#define OLD_DLL_NAME    L"BYModCore.dll_old"
#define UPDATE_INFO_URL "http://storage.opentoshi.net/BYModCore/update.json"

void UpdateManager::CleanUp()
{
	DeleteFileW( OLD_DLL_NAME );
}

void UpdateManager::AskAutoUpdate()
{
	TINFO( "Checking for new updates of the ModCore...\n" );

	VersionInfo versionInfo;
	TBOOL       bOutDated = CheckVersion( UPDATE_INFO_URL, g_uiCurrentVersion, &versionInfo );

	if ( bOutDated )
	{
		TINFO( "New version (v%u.%u) of the ModCore (v%u.%u) is available\n", versionInfo.uiVersion.Parts.Major, versionInfo.uiVersion.Parts.Minor, g_uiCurrentVersion.Parts.Major, g_uiCurrentVersion.Parts.Minor );

		TString8 strDescription = TString8::VarArgs(
		    "New version of the ModCore is available!\n\n"
		    "Current version: v%u.%u\n"
		    "Latest version:  v%u.%u\n\n"
		    "Would you like to update now?",
		    g_uiCurrentVersion.Parts.Major,
		    g_uiCurrentVersion.Parts.Minor,
		    versionInfo.uiVersion.Parts.Major,
		    versionInfo.uiVersion.Parts.Minor
		);

		const TINT msgBoxResult = MessageBoxA( NULL, strDescription.GetString(), "Barnyard ModLoader", MB_YESNO );

		if ( msgBoxResult == IDYES )
		{
			TINFO( "User agreed to update the ModCore\n" );

			// Allocate console so user can see something during update
			AllocConsole();
			FILE* fDummy;
			freopen_s( &fDummy, "CONOUT$", "w", stdout );
			printf( "Downloading the update...\n" );

			AHTTPClient httpClient;
			httpClient.Create( versionInfo.strUpdateUrl.GetString() );
			httplib::Result downloadResult = httpClient.Get();

			// Redirect once
			if ( downloadResult->get_header_value_count( "location" ) )
			{
				httpClient.Create( downloadResult->get_header_value( "location" ).c_str() );
				downloadResult = httpClient.Get();
			}

			if ( downloadResult.error() != httplib::Error::Success ||
			     downloadResult->status != 200 )
			{
				TERROR( "Unable to download new version of the ModCore\n" );
				TASSERT( TFALSE );
				MessageBoxA( NULL, "Unable to download new version of the ModCore! Canceling...", "Barnyard ModLoader", MB_OK );
				return;
			}

			TINFO( "Downloaded new version of the ModCore!\n" );

			CleanUp();
			BOOL bRenamedOldFile = MoveFileW( NEW_DLL_NAME, OLD_DLL_NAME );

			if ( !bRenamedOldFile )
			{
				TERROR( "Unable to rename the old core file. Error: %u\n", GetLastError() );
				TASSERT( TFALSE );
				MessageBoxA( NULL, "Unable to install the update! Canceling...", "Barnyard ModLoader", MB_OK );
				return;
			}

			HANDLE hFile = ::CreateFileW( NEW_DLL_NAME, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

			if ( hFile == INVALID_HANDLE_VALUE )
			{
				TERROR( "Unable to open file for writing. Error: %u\n", GetLastError() );
				TASSERT( TFALSE );
				MessageBoxA( NULL, "Unable to open file for writing! Canceling...", "Barnyard ModLoader", MB_OK );
				return;
			}

			DWORD dwNumWritten;
			BOOL  bWritten = WriteFile( hFile, downloadResult->body.c_str(), downloadResult->body.size(), &dwNumWritten, NULL );

			if ( !bWritten || dwNumWritten != downloadResult->body.size() )
			{
				TERROR( "An unknown error has occured during updating the ModCore. Error: %u\n", GetLastError() );
				TASSERT( TFALSE );
				MessageBoxA( NULL, "An unknown error has occured during updating the ModCore.", "Barnyard ModLoader", MB_OK );
			}

			::CloseHandle( hFile );

			TINFO( "Successfully updated the ModCore!\n" );
			MessageBoxA( NULL, "Successfully updated the ModCore!\n\nRestart the game to continue.", "Barnyard ModLoader", MB_OK );
			
			ExitProcess( 0 );
		}
	}
}

TBOOL UpdateManager::CheckVersion( Toshi::T2StringView strUpdateInfoUrl, Toshi::TVersion uiCurrentVersion, VersionInfo* pOutVersionInfo )
{
	AHTTPClient httpClient;
	httpClient.Create( strUpdateInfoUrl.Get() );

	// Do the request
	httplib::Result result = httpClient.Get();

	if ( result.error() != httplib::Error::Success ) return TFALSE;
	if ( result->status != 200 ) return TFALSE;

	// Parse json
	nlohmann::json json = json.parse( result->body );

	if ( json.is_null() || !json.is_object() )
		return TFALSE;

	auto version = json.find( "version" );
	auto latest  = json.find( "latest" );

	if ( version == json.end() || latest == json.end() ) return TFALSE;
	if ( !version->is_array() || !latest->is_string() ) return TFALSE;

	if ( version->size() != 2 || !version->at( 0 ).is_number_integer() || !version->at( 1 ).is_number_integer() )
		return TFALSE;

	TVersion latestVersion = TVERSION( version->at( 0 ).get<TINT>(), version->at( 1 ).get<TINT>() );

	if ( pOutVersionInfo )
	{
		pOutVersionInfo->uiVersion    = latestVersion;
		pOutVersionInfo->strUpdateUrl = latest->get<std::string>().c_str();
	}

	return ( uiCurrentVersion.Parts.Major < latestVersion.Parts.Major || uiCurrentVersion.Parts.Minor < latestVersion.Parts.Minor );
}
