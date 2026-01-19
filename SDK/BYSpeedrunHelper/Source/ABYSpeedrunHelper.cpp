#include "pch.h"
#include "ARunTimer.h"
#include "AGUISpeedometer.h"
#include "ACollisionInspector.h"
#include "ASplitsServer.h"
#include "AModSettings.h"
#include "AUIManager.h"

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
#include <Math/TRandom.h>
#include <Input/TInputDeviceKeyboard.h>
#include <T2Locale/T2Locale.h>
#include <File/TFile.h>
#include <ToshiTools/T2CommandLine.h>

TOSHI_NAMESPACE_USING

AGUI2TextBox* g_pExperimentalModeText = TNULL;

const T2CommandLine* g_pCommandLine;
TBOOL                g_bIsExperimentalMode = TFALSE;
TBOOL                g_bIsFunCategory      = TFALSE;

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
		AUIManager::GetSingleton()->GetTimer().Split();
	}
}

MEMBER_HOOK( 0x006be7b0, T2Locale, T2Locale_GetString, const TWCHAR*, TINT a_iNumString )
{
	// Detect game end
	if ( a_iNumString == 588 )
	{
		if ( ABYardTerrainManager::ms_eCurrentLevel != ABYardTerrainManager::Terrain_CowTipping )
		{
			static const TClass* pCowTippingGameClass = TClass::Find( "ACowTippingMiniGameState", &TGetClass( THookedObject ) );
			AGameState*          pCurrentGameState    = AGameStateController::GetSingleton()->GetCurrentState();

			if ( pCurrentGameState->IsExactly( pCowTippingGameClass ) )
			{
				AUIManager::GetSingleton()->GetTimer().End();
				return L"Nice run!!!";
			}
		}
	}

	return CallOriginal( a_iNumString );
}

void NewGameStarted()
{
	AUIManager::GetSingleton()->GetTimer().Start();
}

void AGUI2_MainPostRenderCallback()
{
	if ( AUIManager::IsSingletonCreated() )
		AUIManager::GetSingleton()->Render();

	if ( g_pExperimentalModeText )
	{
		g_pExperimentalModeText->PreRender();
		g_pExperimentalModeText->Render();
		g_pExperimentalModeText->PostRender();
	}
}

struct GolfMiniGameSettings
{

	TBOOL bChangeLevel = TFALSE;
	TINT  iLevel       = 1;
	TINT  iTargetLevel = -1;

} g_oGolfMiniGameSettings;

struct AGolfMinigameState : public AGameState
{};

AImGUI*     g_pImGui                   = TNULL;
static auto s_AGolfMiniGameState       = TClass::Find( "AGolfMinigameState", &THookedObject::ms_oClass );
static auto s_AVeggiePatchDefenderGame = TClass::Find( "AVeggiePatchDefenderGame", &THookedObject::ms_oClass );
static auto s_AChickenCoopDefender     = TClass::Find( "AChickenCoopDefender", &THookedObject::ms_oClass );
static auto s_ASleepCycleState         = TClass::Find( "ASleepCycleState", &THookedObject::ms_oClass );

static const char* COW_SKIN_LIST[] = {
	"American",
	"Angus",
	"Beefmaster",
	"Spaniard",
	"Longhorn",
	"Brahman",
	"Holsten"
};

void UpdateCowCustomization()
{
	void* pPlayerProfileManager = *(void**)0x007b493c;

	if ( g_oSettings.bForceSkin && pPlayerProfileManager )
	{
		*(CowSkin*)( TUINTPTR( pPlayerProfileManager ) + 0x48 ) = g_oSettings.eCowSkin;
		*(TINT*)( TUINTPTR( pPlayerProfileManager ) + 0x10 )    = g_oSettings.bIsMale ? 0 : 1;
	}
}

class ALoadScreen
{};

Toshi::TRandom g_oRandom;

MEMBER_HOOK( 0x0042b160, ALoadScreen, ALoadScreen_StartLoading, void, TINT a_iUnused, TBOOL a_bRender )
{
	AUIManager::GetSingleton()->GetTimer().SetIsLoadingScreen( TTRUE );
	CallOriginal( a_iUnused, a_bRender );
}

MEMBER_HOOK( 0x0042b100, ALoadScreen, ALoadScreen_SetLoadingState, void, TBOOL a_bIsLoading, TBOOL a_bRender )
{
	AUIManager::GetSingleton()->GetTimer().SetIsLoadingScreen( a_bIsLoading );

	CallOriginal( a_bIsLoading, a_bRender );
}

MEMBER_HOOK( 0x0042b260, ALoadScreen, ALoadScreen_StopLoading, void )
{
	AUIManager::GetSingleton()->GetTimer().SetIsLoadingScreen( TFALSE );
	CallOriginal();
}

struct AAssetLoader
{};

HOOK( 0x00424a20, AAssetLoader_LoadPlayerCharacter, void, void* a_pProperties )
{
	// Update cow skin
	if ( g_oSettings.bForceSkin && g_oSettings.bRandomSkin )
	{
		g_oSettings.eCowSkin = (CowSkin)g_oRandom.GetInt( 0, CowSkin_NumOf );

		if ( g_oSettings.eCowSkin < 0 )
			g_oSettings.eCowSkin = CowSkin_American;

		if ( g_oSettings.eCowSkin >= CowSkin_NumOf )
			g_oSettings.eCowSkin = CowSkin_Holsten;
	}

	UpdateCowCustomization();
	CallOriginal( a_pProperties );
}

class ASimProfile
{};

MEMBER_HOOK( 0x0062e2e0, ASimProfile, ASimProfile_Unknown, void, TINT& a_rOut )
{
	CallOriginal( a_rOut );

	void* pPlayerProfileManager = *(void**)0x007b493c;
	if ( g_oSettings.bForceSkin && pPlayerProfileManager )
		*(CowSkin*)( TUINTPTR( pPlayerProfileManager ) + 0x48 ) = g_oSettings.eCowSkin;
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
			InstallHook<ALoadScreen_SetLoadingState>();
			InstallHook<ALoadScreen_StopLoading>();
			InstallHook<AAssetLoader_LoadPlayerCharacter>();
			InstallHook<ASimProfile_Unknown>();

			ACollisionInspector::CreateSingleton();
			ASplitsServer::CreateSingleton();

			return TTRUE;
		}

		return TFALSE;
	}

	TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override
	{
		AGameStateController* pGameStateController = AGameStateController::GetSingleton();

		// Apply minigame specific settings
		if ( g_oGolfMiniGameSettings.bChangeLevel )
		{
			AGameState* pCurrentState = pGameStateController->GetCurrentState();

			if ( pGameStateController->GetNumStates() >= 1 && pCurrentState )
			{
				const TClass* pCurrentStateClass = pCurrentState->GetClass();

				// Golf minigame - change level
				if ( g_oGolfMiniGameSettings.bChangeLevel && pCurrentStateClass == s_AGolfMiniGameState )
				{
					TINT& rCurrentLevel = *(TINT*)( TUINTPTR( pCurrentState ) + 0x41D8 );

					g_oGolfMiniGameSettings.bChangeLevel = TFALSE;
					rCurrentLevel                        = g_oGolfMiniGameSettings.iLevel;

					// Set the first level if tried to load previous while on the first one
					if ( rCurrentLevel < -1 )
						rCurrentLevel = -1;

					// Ask the game to load next level
					CALL_THIS( 0x004754f0, AGolfMinigameState*, void, TSTATICCAST( AGolfMinigameState, pCurrentState ) );
				}
			}
		}

		// Fun% mode
		if ( g_bIsFunCategory )
		{
			// Adjust speed of the game
			TBOOL  bSpeedUp = TFALSE;
			TFLOAT fSpeed   = 1.0f;

			AGameStateController* pGameStateController = AGameStateController::GetSingleton();
			if ( pGameStateController->GetNumStates() >= 1 )
			{
				TSystemManager* pSystemManager = (TSystemManager*)0x007ce640;
				const TClass*   pStateClass    = pGameStateController->GetCurrentState()->GetClass();

				if ( pStateClass == s_AChickenCoopDefender )
				{
					bSpeedUp = TTRUE;
					fSpeed   = 2.0f;
				}
				else if ( pStateClass == s_AVeggiePatchDefenderGame )
				{
					bSpeedUp = TTRUE;
					fSpeed   = 2.0f;
				}
				else if ( pStateClass == s_ASleepCycleState )
				{
					bSpeedUp = TTRUE;
					fSpeed   = 8.0f;
				}

				pSystemManager->GetScheduler()->SetDebugDeltaTimeMult( bSpeedUp, fSpeed );
			}
		}

		// Experimental mode
		if ( g_bIsExperimentalMode )
		{
			// Simulate lag by pressing Z key
			TInputInterface* pInputInterface = *(TInputInterface**)( 0x007cec34 );

			if ( pInputInterface )
			{
				static const TClass*  pKeyboardClass = TClass::Find( "TInputDeviceKeyboard", &TGetClass( THookedObject ) );
				TInputDeviceKeyboard* pKeyboard      = (TInputDeviceKeyboard*)pInputInterface->GetDeviceByIndex( pKeyboardClass, 0 );
				TSystemManager*       pSystemManager = (TSystemManager*)0x007ce640;

				TBOOL        bIsDown    = pKeyboard->IsDown( TInputDeviceKeyboard::KEY_Q );
				static TBOOL s_bWasDown = TFALSE;

				pSystemManager->GetScheduler()->SetDebugDeltaTime( !s_bWasDown && bIsDown, 0.5f );
				s_bWasDown = bIsDown;
			}
		}

		if ( AUIManager::IsSingletonCreated() )
			AUIManager::GetSingleton()->Update( a_fDeltaTime );

		return TTRUE;
	}

	void OnUnload() override
	{
		if ( AUIManager::IsSingletonCreated() )
			AUIManager::GetSingleton()->Destroy();
	}

	void OnRenderInterfaceReady( Toshi::TRenderD3DInterface* a_pRenderInterface ) override
	{
		TRenderInterface::SetSingletonExplicit(
		    THookedRenderD3DInterface::GetSingleton()
		);
	}

	void OnAGUI2Ready() override
	{
		AUIManager::CreateSingleton()->Create();

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

	void OnImGuiRender( AImGUI* a_pImGui ) override
	{
		// Update pointer to the AImGui interface
		g_pImGui = a_pImGui;

		TBOOL bChangedSettings = TFALSE;

		// Render cow customization UI
		ImGui::Text( "Cow Customization" );
		{
			bChangedSettings |= ImGui::Checkbox( "Change Skin", &g_oSettings.bForceSkin );
			bChangedSettings |= ImGui::Checkbox( "Randomize", &g_oSettings.bRandomSkin );

			if ( ImGui::BeginCombo( "##CowSkin", COW_SKIN_LIST[ g_oSettings.eCowSkin ] ) )
			{
				for ( TUINT i = 0; i < TARRAYSIZE( COW_SKIN_LIST ); i++ )
				{
					TBOOL bSelected = ( i == g_oSettings.eCowSkin );
					if ( ImGui::Selectable( COW_SKIN_LIST[ i ], &bSelected ) && bSelected )
					{
						g_oSettings.eCowSkin = (CowSkin)i;
						bChangedSettings     = TTRUE;
					}
				}

				ImGui::EndCombo();
			}

			ImGui::SameLine();
			bChangedSettings |= ImGui::Checkbox( "Male", &g_oSettings.bIsMale );

			ImGui::Separator();
		}

		// Render minigame specific UI
		if ( AGameStateController* pGameStateController = AGameStateController::GetSingleton() )
		{
			AGameState* pCurrentState = pGameStateController->GetPreviousState();

			if ( pGameStateController->GetNumStates() >= 2 && pCurrentState )
			{
				const TClass* pCurrentStateClass = pCurrentState->GetClass();
				TBOOL         bDrawnUI           = TFALSE;

				if ( pCurrentStateClass == s_AGolfMiniGameState )
				{
					ImGui::Text( "Golf Hole Selector" );

					TINT& rCurrentLevel = *(TINT*)( TUINTPTR( pCurrentState ) + 0x41D8 );

					auto fnLoadLevel = []( TINT iLevel ) {
						g_oGolfMiniGameSettings.bChangeLevel = TTRUE;
						g_oGolfMiniGameSettings.iLevel       = iLevel;
						g_pImGui->Toggle();
					};

					if ( ImGui::Button( "Prev Level" ) ) fnLoadLevel( rCurrentLevel - 2 );
					ImGui::SameLine();
					if ( ImGui::Button( "Restart Level" ) ) fnLoadLevel( rCurrentLevel - 1 );
					ImGui::SameLine();
					if ( ImGui::Button( "Next Level" ) ) fnLoadLevel( rCurrentLevel );

					TINT* pLevels  = *(TINT**)( TUINTPTR( pCurrentState ) + 0x4108 );
					TINT  iMaxHole = *(TINT*)( TUINTPTR( pLevels ) + 0x90 );

					static TINT s_iHoleSelection = 1;
					ImGui::SliderInt( "##Hole", &s_iHoleSelection, 1, iMaxHole );
					ImGui::SameLine();
					if ( ImGui::Button( "Start Hole" ) ) fnLoadLevel( s_iHoleSelection - 2 );

					bDrawnUI = TTRUE;
				}

				if ( bDrawnUI )
					ImGui::Separator();
			}
		}

		bChangedSettings |= ImGui::Checkbox( "Show LRT Timer", &g_oSettings.bShowLRTTimer );
		bChangedSettings |= ImGui::Checkbox( "Show RTA Timer", &g_oSettings.bShowRTATimer );
		bChangedSettings |= ImGui::Checkbox( "Show Speedometer", &g_oSettings.bShowSpeedometer );

		ImGui::PushItemWidth( 300.0f );
		bChangedSettings |= ImGui::ColorPicker4( "##HUD Color", (float*)&g_oSettings.vecHUDColor, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar );

		if ( g_bIsExperimentalMode )
		{
			ImGui::Checkbox( "Show Collision", &ACollisionInspector::GetSingleton()->IsCollisionVisible() );

			if ( ImGui::Button( "Restart Timer" ) ) AUIManager::GetSingleton()->GetTimer().Start();
		}

		if ( bChangedSettings )
			g_oSettings.Apply();

		ImGui::Separator();

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
		g_bIsFunCategory      = g_pCommandLine->HasParameter( "-fun" );

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
