#include "pch.h"
#include "AHooks.h"
#include "AModLoaderTask.h"

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
	m_bLoaded	 = TFALSE;
	m_pTextBox	 = TNULL;
	m_uiNumMods	 = 0;

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

	for ( const auto& entry : std::filesystem::directory_iterator( GetModsDirectory() ) )
	{
		if ( entry.path().extension().compare( L".dll" ) == 0 )
		{
			const wchar_t* dll = entry.path().native().c_str();

			char dllPath[ MAX_PATH ];
			TStringManager::StringUnicodeToChar( dllPath, entry.path().native().c_str(), -1 );

			HMODULE hModule				= LoadLibraryW( dll );
			auto	fnCreateModInstance = TREINTERPRETCAST( t_CreateModInstance, GetProcAddress( hModule, "CreateModInstance" ) );

			if ( fnCreateModInstance )
			{
				AModInstance* pModInstance = fnCreateModInstance( g_pCommandLine );

				if ( pModInstance )
				{
					TINFO( "Trying to initialise '%s'\n", pModInstance->GetName() );

					pModInstance->m_hModule = hModule;

					if ( pModInstance->OnLoad() )
					{
						TINFO( "  Successfully initialised!\n" );
						m_LoadedMods.PushBack( pModInstance );
						m_uiNumMods += 1;
					}
					else
					{
						TERROR( "  Couldn't initialise!\n" );
						delete pModInstance;
						FreeLibrary( hModule );
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
	m_pTask			= CALL_THIS( 0x006bcbf0, TScheduler*, AModLoaderTask*, pScheduler, TClass*, &TGetClass( AModLoaderTask ), TTask*, TNULL );
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
	auto pMods		= &pModLoader->GetMods();

	for ( auto it = pMods->Begin(); it != pMods->End(); it++ )
	{
		if ( TStringManager::String8Compare( it->GetName(), a_szModName ) == 0 )
		{
			return it;
		}
	}

	return TNULL;
}
