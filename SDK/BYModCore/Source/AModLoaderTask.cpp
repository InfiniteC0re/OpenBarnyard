#include "pch.h"
#include "AHooks.h"
#include "AModLoaderTask.h"
#include "AHTTPClient.h"
#include "json.hpp"

#include <BYardSDK/THookedRenderD3DInterface.h>
#include <BYardSDK/AGUI2FontManager.h>
#include <BYardSDK/AGUI2.h>

#include <filesystem>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AModLoaderTask );

extern const T2CommandLine* g_pCommandLine;

void AModLoaderTask::AGUI2MainPostRenderCallback()
{
	if ( AGlobalModLoaderTask::Get()->m_pTextBox )
	{
		AGlobalModLoaderTask::Get()->m_pTextBox->PreRender();
		AGlobalModLoaderTask::Get()->m_pTextBox->Render();
		AGlobalModLoaderTask::Get()->m_pTextBox->PostRender();
	}
}

AModLoaderTask::AModLoaderTask()
{
	m_fTotalTime = 0.0f;
	m_bLoaded    = TFALSE;
	m_pTextBox   = TNULL;
	m_uiNumMods  = 0;

	AHooks::AddHook( Hook_AGUI2_MainPostRenderCallback, HookType_Before, AGUI2MainPostRenderCallback );
}

AModLoaderTask::~AModLoaderTask()
{
	m_LoadedMods.DeleteAll();
	// TODO: remove hook
}

TBOOL AModLoaderTask::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_pTextBox && m_pTextBox->IsVisible() )
	{
		TFLOAT fAlpha = ( m_fTotalTime - 3.0f ) / 0.8f;
		TMath::Clip( fAlpha, 0.0f, 1.0f );

		fAlpha *= fAlpha;
		m_pTextBox->SetAlpha( 1.0f - fAlpha );

		if ( fAlpha == 1.0f )
		{
			m_pTextBox->Hide();
		}
	}

	for ( auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++ )
	{
		TBOOL bResult = it->OnUpdate( a_fDeltaTime );
		TASSERT( bResult != TFALSE );
	}

	m_fTotalTime += a_fDeltaTime;
	return TTRUE;
}

TBOOL AModLoaderTask::OnCreate()
{
	LoadMods();

	return TTRUE;
}

void AModLoaderTask::LoadMods()
{
	auto pchCommandLine = GetCommandLineA();

	TINFO( "Checking mods versions are up to date...\n" );

	// Update mods
	for ( const auto& entry : std::filesystem::directory_iterator( GetModsDirectory() ) )
	{
		if ( entry.path().extension().compare( L".dll" ) == 0 )
		{
			HMODULE hModule               = LoadLibraryExW( entry.path().native().c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES );
			auto    fnGetModAutoUpdateURL = TREINTERPRETCAST( t_GetModAutoUpdateURL, GetProcAddress( hModule, "GetModAutoUpdateURL" ) );
			auto    fnGetVersion          = TREINTERPRETCAST( t_GetModVersion, GetProcAddress( hModule, "GetModVersion" ) );
			auto    fnGetName             = TREINTERPRETCAST( t_GetModName, GetProcAddress( hModule, "GetModName" ) );

			// Skip mods that don't have auto updates enabled
			if ( !fnGetModAutoUpdateURL || !fnGetVersion )
			{
				FreeLibrary( hModule );
				continue;
			}

			// Obtain the update url
			TString8 updateXmlPath = fnGetModAutoUpdateURL();
			TString8 strModName    = ( fnGetName ) ? fnGetName() : "Unknown Mod";
			TVersion modVersion    = fnGetVersion();
			
			// We don't need the dll to be attached anymore
			FreeLibrary( hModule );

			TINFO( "Checking for updates of the '%s' mod...\n", strModName.GetString() );

			if ( !updateXmlPath.Length() ) continue;
			
			AHTTPClient httpClient;
			httpClient.Create( updateXmlPath.GetString() );

			// Do the request
			httplib::Result result = httpClient.Get();
			
			if ( result.error() != httplib::Error::Success ) continue;
			if ( result->status != 200 ) continue;

			// Parse json
			nlohmann::json json = json.parse( result->body );

			if ( json.is_null() || !json.is_object() )
				continue;

			auto version = json.find( "version" );
			auto latest  = json.find( "latest" );

			if ( version == json.end() || latest == json.end() ) continue;
			if ( !version->is_array() || !latest->is_string() ) continue;

			if ( version->size() != 2 || !version->at( 0 ).is_number_integer() || !version->at( 1 ).is_number_integer() )
				continue;

			TVersion latestVersion = TVERSION( version->at( 0 ).get<TINT>(), version->at( 1 ).get<TINT>() );

			if ( modVersion.Parts.Major < latestVersion.Parts.Major ||
			     modVersion.Parts.Minor < latestVersion.Parts.Minor )
			{
				TINFO( "New version (v%u.%u) of the '%s' (v%u.%u) mod is available\n", latestVersion.Parts.Major, latestVersion.Parts.Minor, strModName.GetString(), modVersion.Parts.Major, modVersion.Parts.Minor );
				
				TString8 strDescription = TString8::VarArgs(
				    "New version of the '%s' mod is available!\n\n"
				    "Current version: v%u.%u\n"
				    "Latest version:  v%u.%u\n\n"
					"Would you like to update now?",
				    strModName.GetString(),
				    modVersion.Parts.Major,
				    modVersion.Parts.Minor,
				    latestVersion.Parts.Major,
				    latestVersion.Parts.Minor
				);

				const TINT msgBoxResult = MessageBoxA( NULL, strDescription.GetString(), "Barnyard ModLoader", MB_YESNO );

				if ( msgBoxResult == IDYES )
				{
					TINFO( "User agreed to update mod\n" );

					httpClient.Create( latest->get<std::string>().c_str() );
					httplib::Result downloadResult = httpClient.Get();

					if ( downloadResult.error() != httplib::Error::Success ||
						 downloadResult->status != 200 )
					{
						TERROR( "Unable to download new version of the mod\n" );
						TASSERT( TFALSE );
						MessageBoxA( NULL, "Unable to download new version of the mod! Canceling...", "Barnyard ModLoader", MB_OK );
						continue;
					}

					TINFO( "Downloaded new version of the mod!\n" );

					HANDLE hFile = ::CreateFileW( entry.path().native().c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

					if ( hFile == INVALID_HANDLE_VALUE )
					{
						TERROR( "Unable to open file for writing. Error: %u\n", GetLastError() );
						TASSERT( TFALSE );
						MessageBoxA( NULL, "Unable to download new version of the mod! Canceling...", "Barnyard ModLoader", MB_OK );
						continue;
					}

					DWORD dwNumWritten;
					BOOL  bWritten = WriteFile( hFile, downloadResult->body.c_str(), downloadResult->body.size(), &dwNumWritten, NULL );
					
					if ( !bWritten || dwNumWritten != downloadResult->body.size() )
					{
						TERROR( "An unknown error has occured during updating the mod. Error: %u\n", GetLastError() );
						TASSERT( TFALSE );
						MessageBoxA( NULL, "An unknown error has occured during updating the mod.", "Barnyard ModLoader", MB_OK );
					}
					
					::CloseHandle( hFile );

					TINFO( "Successfully updated the mod!\n" );
				}
			}
		}
	}

	TINFO( "Loading mods...\n" );

	// Load the mods
	for ( const auto& entry : std::filesystem::directory_iterator( GetModsDirectory() ) )
	{
		if ( entry.path().extension().compare( L".dll" ) == 0 )
		{
			const wchar_t* dll = entry.path().native().c_str();

			char dllPath[ MAX_PATH ];
			TStringManager::StringUnicodeToChar( dllPath, entry.path().native().c_str(), -1 );

			HMODULE hModule             = LoadLibraryW( dll );
			auto    fnCreateModInstance = TREINTERPRETCAST( t_CreateModInstance, GetProcAddress( hModule, "CreateModInstance" ) );
			auto    fnGetVersion        = TREINTERPRETCAST( t_GetModVersion, GetProcAddress( hModule, "GetModVersion" ) );
			auto    fnGetName           = TREINTERPRETCAST( t_GetModName, GetProcAddress( hModule, "GetModName" ) );

			// Load the mod
			if ( fnCreateModInstance )
			{
				AModInstance* pModInstance = fnCreateModInstance( g_pCommandLine );
				pModInstance->m_strName    = ( fnGetName ) ? fnGetName() : "Unknown Mod";
				pModInstance->m_uiVersion  = ( fnGetVersion ) ? fnGetVersion() : TVERSION( 1, 0 );

				if ( pModInstance )
				{
					TINFO( "Trying to initialise '%s'\n", pModInstance->GetName() );
					pModInstance->m_hModule = hModule;

					TLOGUP()
					{
						if ( pModInstance->OnLoad() )
						{
							TINFO( "Successfully initialised!\n" );
							m_LoadedMods.PushBack( pModInstance );
							m_uiNumMods += 1;
						}
						else
						{
							TERROR( "Couldn't initialise!\n" );
							delete pModInstance;
							FreeLibrary( hModule );
						}

						TLOGDOWN();
					}
				}
				else
				{

					TERROR( "%s: CreateModInstance returned TFALSE!\n", dllPath );
				}
			}
			else
			{
				TERROR( "%s: CreateModInstance is not found!\n", dllPath );
				FreeLibrary( hModule );
			}
		}
	}

	for ( auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++ )
	{
		it->OnAllModsLoaded();
	}

	m_bLoaded = TTRUE;
}

void AModLoaderTask::UnloadMods()
{
	for ( auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++ )
	{
		it->OnUnload();
	}

	m_LoadedMods.Clear();
}

void AModLoaderTask::OnAGUI2Ready()
{
	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	auto pFont = AGUI2FontManager::FindFont( "Rekord18" );
	m_pTextBox = AGUI2TextBox::CreateFromEngine();
	m_pTextBox->SetAttachment( AGUI2Element::Anchor_MiddleCenter, AGUI2Element::Pivot_TopCenter );
	m_pTextBox->Create( pFont, 300.0f );
	m_pTextBox->SetTransform( 0, -fHeight / 2 + 64.0f );
	m_pTextBox->SetText( L"ModLoader works!" );
	m_pTextBox->SetInFront();

	if ( m_bLoaded )
	{
		static wchar_t s_wcsBuffer[ 64 ];
		const wchar_t* wcsFormat = ( m_uiNumMods != 1 ) ? L"Loaded %d mods!\n%ls" : L"Loaded %d mod!\n%ls";
		TStringManager::String16Format( s_wcsBuffer, TARRAYSIZE( s_wcsBuffer ), wcsFormat, m_uiNumMods, L"" );
		m_pTextBox->SetText( s_wcsBuffer );
	}

	for ( auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++ )
	{
		it->OnAGUI2Ready();
	}
}

void AModLoaderTask::OnRenderInterfaceReady()
{
	auto pRender = THookedRenderD3DInterface::GetSingleton();
	TASSERT( TNULL != pRender );

	for ( auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++ )
	{
		it->OnRenderInterfaceReady( pRender );
	}
}

void AModLoaderTask::OnAppRendererReady()
{
	for ( auto it = m_LoadedMods.Begin(); it != m_LoadedMods.End(); it++ )
	{
		it->OnAppRendererReady();
	}
}

T2DList<AModInstance>& AModLoaderTask::GetMods()
{
	return m_LoadedMods;
}

AGlobalModLoaderTask::AGlobalModLoaderTask()
{
	auto pScheduler = CALL_THIS( 0x006bbc10, TSystemManager*, TScheduler*, (TSystemManager*)0x007ce640 );
	m_pTask         = CALL_THIS( 0x006bcbf0, TScheduler*, AModLoaderTask*, pScheduler, TClass*, &TGetClass( AModLoaderTask ), TTask*, TNULL );
}

TBOOL AGlobalModLoaderTask::Create()
{
	TASSERT( TNULL != m_pTask );
	return m_pTask->Create();
}

void AGlobalModLoaderTask::Destroy()
{
	TASSERT( TNULL != m_pTask );
	m_pTask->UnloadMods();
}

AModInstance* AGlobalModLoaderTask::FindMod( const char* a_szModName )
{
	auto pModLoader = Get();
	auto pMods      = &pModLoader->GetMods();

	for ( auto it = pMods->Begin(); it != pMods->End(); it++ )
	{
		if ( TStringManager::String8Compare( it->GetName(), a_szModName ) == 0 )
		{
			return it;
		}
	}

	return TNULL;
}
