#include "pch.h"
#include <AImGUI.h>
#include <ModLoader.h>
#include <AModLoaderTask.h>
#include <AHooks.h>
#include <HookHelpers.h>
#include <BYardSDK/AGUI2.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

#include <Toshi/THPTimer.h>
#include <File/TFile.h>

#include "ABYTexturePacks.h"

#include "Platform/DX8/TTextureResourceHAL_DX8.h"

TOSHI_NAMESPACE_USING

class ABINKMoviePlayer
{};

MEMBER_HOOK( 0x006059f0, ABINKMoviePlayer, ABINKMoviePlayer_SetMovieFile, void, const char* a_szMovieFile )
{
	if ( TStringManager::String8FindString( a_szMovieFile, "Attract" ) != TNULL )
	{
		TStringManager::String8Format( (char*)( TUINT( this ) + 0x37 ), 64, "%s\\BYWinterMod\\intro_new.bik", GetModsDirectory() );
		return;
	}

	CallOriginal( a_szMovieFile );
}

class AOptions
{};

MEMBER_HOOK( 0x00662eb0, AOptions, AOptions_LoadSettings, void )
{
	CallOriginal();

	// Disable high detail grass
	*(TBOOL*)( TUINT( this ) + 0x30 ) = TFALSE;
}

TFLOAT g_afLightColour[ 4 ] = {
	218 / 255.0f,
	236 / 255.0f,
	254 / 255.0f,
	255 / 255.0f
};

void TRenderInterface_SetLightColourMatrix( Toshi::TRenderInterface* a_pRenderInterface, Toshi::TMatrix44* a_pLightColour )
{
	a_pLightColour->m_f11 = g_afLightColour[ 0 ];
	a_pLightColour->m_f12 = g_afLightColour[ 1 ];
	a_pLightColour->m_f13 = g_afLightColour[ 2 ];
	a_pLightColour->m_f14 = g_afLightColour[ 3 ];
}

class ABYWinterMod : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		InstallHook<ABINKMoviePlayer_SetMovieFile>();
		InstallHook<AOptions_LoadSettings>();
		return AHooks::AddHook( Hook_TRenderInterface_SetLightColourMatrix, HookType_Before, TRenderInterface_SetLightColourMatrix );
	}

	TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override
	{
		return TTRUE;
	}

	void OnUnload() override
	{
		if ( m_pMatLib )
		{
			m_pMatLib->Destroy();
			m_pMatLib = TNULL;
		}
	}

	void OnAllModsLoaded()
	{
		auto pTexturePacksMod = TSTATICCAST( ABYTexturePacks, AGlobalModLoaderTask::FindMod( "BYTexturePacks" ) );

		if ( pTexturePacksMod )
		{
			pTexturePacksMod->SetTexturePack( "Winter" );
		}
	}

	void OnRenderInterfaceReady( Toshi::TRenderD3DInterface* a_pRenderInterface ) override
	{
		TRenderInterface::SetSingletonExplicit(
		    THookedRenderD3DInterface::GetSingleton()
		);
	}

	void OnAppRendererReady() override
	{
		static TBOOL s_bLoaded = TFALSE;

		if ( !s_bLoaded )
		{
			m_pAssetTRB = new TTRB;

			if ( TTRB::ERROR_OK == m_pAssetTRB->Load( TString8::VarArgs( "%s\\BYWinterMod\\BYWinterMod.trb", GetModsDirectory() ) ) )
			{
				m_pMatLib = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
				    "BYWinterMatlib.ttl",
				    m_pAssetTRB
				);

				AMaterialLibraryManager::GetSingleton()->CreateTextures( m_pMatLib );
			}
			else
			{
				delete m_pAssetTRB;
				m_pAssetTRB = TNULL;
			}

			s_bLoaded = TTRUE;
		}
	}

	Toshi::TTRB* GetAssetFile() override
	{
		return m_pAssetTRB;
	}

	const PBProperties* GetFileOverrides() override
	{
		if ( m_pAssetTRB )
		{
			return PBProperties::LoadFromAsset( m_pAssetTRB, "overrides" );
		}

		return TNULL;
	}

	const char* GetName() override
	{
		return "BYWinterMod";
	}

	void OnImGuiRender() override
	{
		ImGui::ColorEdit4( "Lighting Colour", g_afLightColour );
	}

	TBOOL HasSettingsUI() override
	{
		return TTRUE;
	}

private:
	Toshi::TTRB*      m_pAssetTRB = TNULL;
	AMaterialLibrary* m_pMatLib   = TNULL;
};

extern "C"
{
	MODLOADER_EXPORT AModInstance* CreateModInstance( const T2CommandLine* a_pCommandLine )
	{
		TMemory::Initialise( 2 * 1024 * 1024, 0 );

		TUtil::TOSHIParams toshiParams;
		toshiParams.szCommandLine = "";
		toshiParams.szLogFileName = "wintermod";
		toshiParams.szLogAppName  = "BYWinterMod";

		TUtil::ToshiCreate( toshiParams );

		return new ABYWinterMod();
	}
}
