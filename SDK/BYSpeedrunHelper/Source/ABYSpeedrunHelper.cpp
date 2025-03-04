#include "pch.h"
#include "ARunTimer.h"
#include "AGUISpeedometer.h"
#include "ACollisionInspector.h"
#include "ASplitsServer.h"
#include "AModSettings.h"

#include <AImGUI.h>
#include <ModLoader.h>
#include <AHooks.h>
#include <HookHelpers.h>

#include <BYardSDK/AGUI2.h>
#include <BYardSDK/AGameStateController.h>
#include <BYardSDK/THookedRenderD3DInterface.h>
#include <BYardSDK/ATerrainInterface.h>

#include <Toshi/THPTimer.h>
#include <Toshi/TScheduler.h>
#include <Input/TInputDeviceKeyboard.h>
#include <T2Locale/T2Locale.h>
#include <File/TFile.h>
#include <ToshiTools/T2CommandLine.h>

TOSHI_NAMESPACE_USING

ARunTimer       g_Timer;
AGUISpeedometer g_Speedometer;
AGUI2TextBox*   g_pExperimentalModeText = TNULL;

const T2CommandLine* g_pCommandLine;
TBOOL                g_bIsExperimentalMode = TFALSE;

class AQuestManager : public Toshi::TTask
{
public:
	TCHAR MARGIN[ 408 ];
	TINT  m_iCurTier;
	TINT  m_iCurAct;
};

MEMBER_HOOK( 0x005ce150, AQuestManager, AQuestManager_FUNC, void, TCHAR* a_pUnk1, TBOOL a_bUnk2, TBOOL a_bUpdateProgress )
{
	TINT iOldAct  = m_iCurAct;
	TINT iOldTier = m_iCurTier;

	CallOriginal( a_pUnk1, a_bUnk2, a_bUpdateProgress );

	if ( a_bUpdateProgress && ( iOldAct < m_iCurAct || iOldTier < m_iCurTier ) )
	{
		TTRACE( "Split! Old State: (%d, %d); Currect State: (%d, %d)\n", iOldAct, iOldTier, m_iCurAct, m_iCurTier );
		g_Timer.Split();
	}
}

MEMBER_HOOK( 0x006be7b0, T2Locale, T2Locale_GetString, const TWCHAR*, TINT a_iNumString )
{
	// Detect game end
	if ( a_iNumString == 588 )
	{
		if ( ABYardTerrainManager::ms_eCurrentLevel != ABYardTerrainManager::Terrain_CowTipping )
		{
			static TClass* pCowTippingGameClass = TClass::Find( "ACowTippingMiniGameState", &TGetClass( THookedObject ) );
			AGameState*    pCurrentGameState    = AGameStateController::GetSingleton()->GetCurrentState();

			if ( pCurrentGameState->IsExactly( pCowTippingGameClass ) )
			{
				g_Timer.End();
				return L"Nice run!!!";
			}
		}
	}

	return CallOriginal( a_iNumString );
}

class ALoadScreen
{};

MEMBER_HOOK( 0x0042b160, ALoadScreen, ALoadScreen_StartLoading, void, TINT a_iUnused, TBOOL a_bRender )
{
	g_Timer.SetIsLoadingScreen( TTRUE );
	CallOriginal( a_iUnused, a_bRender );
}

MEMBER_HOOK( 0x0042b260, ALoadScreen, ALoadScreen_StopLoading, void )
{
	g_Timer.SetIsLoadingScreen( TFALSE );
	CallOriginal();
}

void NewGameStarted()
{
	g_Timer.Start();
}

void AGUI2_MainPostRenderCallback()
{
	g_Timer.Render();
	g_Speedometer.Render();

	if ( g_pExperimentalModeText )
	{
		g_pExperimentalModeText->PreRender();
		g_pExperimentalModeText->Render();
		g_pExperimentalModeText->PostRender();
	}
}

class ABYSpeedrunHelper : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		if ( AHooks::AddHook( Hook_NewGameStarted, HookType_Before, NewGameStarted ) &&
		     AHooks::AddHook( Hook_AGUI2_MainPostRenderCallback, HookType_Before, AGUI2_MainPostRenderCallback ) )
		{
			g_oSettings.Load();

			InstallHook<AQuestManager_FUNC>();
			InstallHook<T2Locale_GetString>();
			InstallHook<ALoadScreen_StartLoading>();
			InstallHook<ALoadScreen_StopLoading>();

			ACollisionInspector::CreateSingleton();
			ASplitsServer::CreateSingleton();

			return TTRUE;
		}

		return TFALSE;
	}

	TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override
	{
		if ( g_bIsExperimentalMode )
		{
			// Simulate lag by pressing Z key
			TInputInterface* pInputInterface = *(TInputInterface**)( 0x007cec34 );

			if ( pInputInterface )
			{
				static TClass*        pKeyboardClass = TClass::Find( "TInputDeviceKeyboard", &TGetClass( THookedObject ) );
				TInputDeviceKeyboard* pKeyboard      = (TInputDeviceKeyboard*)pInputInterface->GetDeviceByIndex( pKeyboardClass, 0 );
				TSystemManager*       pSystemManager = (TSystemManager*)0x007ce640;

				TBOOL        bIsDown    = pKeyboard->IsDown( TInputDeviceKeyboard::KEY_Q );
				static TBOOL s_bWasDown = TFALSE;

				pSystemManager->GetScheduler()->SetDebugDeltaTime( !s_bWasDown && bIsDown, 0.5f );
				s_bWasDown = bIsDown;
			}
		}

		g_Timer.Update();
		g_Speedometer.Update();
		return TTRUE;
	}

	void OnUnload() override
	{
		g_Timer.Destroy();
	}

	void OnRenderInterfaceReady( Toshi::TRenderD3DInterface* a_pRenderInterface ) override
	{
		TRenderInterface::SetSingletonExplicit(
		    THookedRenderD3DInterface::GetSingleton()
		);
	}

	void OnAGUI2Ready() override
	{
		g_Timer.Create();
		g_Speedometer.Create();

		if ( g_bIsExperimentalMode )
		{
			TFLOAT fWidth, fHeight;
			AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

			auto pFont              = AGUI2FontManager::FindFont( "Rekord18" );
			g_pExperimentalModeText = AGUI2TextBox::CreateFromEngine();

			g_pExperimentalModeText->Create( pFont, 400.0f );
			g_pExperimentalModeText->SetText( L"Experimental mode!" );
			g_pExperimentalModeText->SetColour( TCOLOR( 255, 0, 0 ) );
			g_pExperimentalModeText->SetTransform( 0.0f, -fHeight / 2 + 32.0f );
			g_pExperimentalModeText->SetAlpha( 1.0f );
			g_pExperimentalModeText->SetTextAlign( AGUI2Font::TextAlign_Center );
			g_pExperimentalModeText->SetInFront();
			g_pExperimentalModeText->SetAttachment( AGUI2Element::Anchor_MiddleCenter, AGUI2Element::Pivot_MiddleCenter );
		}
	}

	void OnImGuiRender() override
	{
		TBOOL bChangedSettings = TFALSE;
		bChangedSettings |= ImGui::Checkbox( "Show Timer", &g_oSettings.bShowTimer );
		bChangedSettings |= ImGui::Checkbox( "Show Speedometer", &g_oSettings.bShowSpeedometer );

		ImGui::PushItemWidth( 320.0f );
		bChangedSettings |= ImGui::ColorPicker4( "##HUD Color", (float*)&g_oSettings.vecHUDColor, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar );

		if ( g_bIsExperimentalMode )
		{
			ImGui::Checkbox( "Show Collision", &ACollisionInspector::GetSingleton()->IsCollisionVisible() );

			if ( ImGui::Button( "Restart Timer" ) ) g_Timer.Start();
			ImGui::SameLine();
			if ( ImGui::Button( "Resume Timer" ) ) g_Timer.Resume();
			ImGui::SameLine();
			if ( ImGui::Button( "Pause Timer" ) ) g_Timer.Pause();
			ImGui::SameLine();
			if ( ImGui::Button( "Reset Timer" ) ) g_Timer.Reset();
		}

		if ( bChangedSettings )
			g_oSettings.Apply();

		if ( ImGui::Button( "Save" ) )
			g_oSettings.Save();

		ImGui::SameLine();
		if ( ImGui::Button( "Reset" ) )
			g_oSettings.Reset();
	}

	TBOOL HasSettingsUI() override
	{
		return TTRUE;
	}
};

extern "C"
{
	MODLOADER_EXPORT AModInstance* CreateModInstance( const T2CommandLine* a_pCommandLine )
	{
		TMemory::Initialise( 8 * 1024 * 1024, 0 );

		TUtil::TOSHIParams toshiParams;
		toshiParams.szCommandLine = "";
		toshiParams.szLogFileName = "speedrunhelper";
		toshiParams.szLogAppName  = "BYSpeedrunHelper";

		TUtil::ToshiCreate( toshiParams );

		g_pCommandLine        = a_pCommandLine;
		g_bIsExperimentalMode = g_pCommandLine->HasParameter( "-experimental" );

		return new ABYSpeedrunHelper();
	}

	MODLOADER_EXPORT const TCHAR* GetModAutoUpdateURL()
	{
		return "https://github.com/InfiniteC0re/OpenBarnyard/releases/download/speedrunhelper/update.json";
	}

	MODLOADER_EXPORT const TCHAR* GetModName()
	{
		return "BYSpeedrunHelper";
	}

	MODLOADER_EXPORT TUINT32 GetModVersion()
	{
		return TVERSION( 1, 0 );
	}
}
