#include "pch.h"
#include "ARunTimer.h"
#include "AGUISpeedometer.h"
#include "ASplitsServer.h"
#include "AModSettings.h"
#include "AUIManager.h"

#include "Assets/AAssetLoader.h"
#include "GameInterface/AGameStateController.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "MiniGame/ABikeRaceMicroGame.h"
#include "MiniGame/AGolfMinigameState.h"
#include "Mod/ImGui/AImGui.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AInteractionManager.h"

#include <Toshi/TScheduler.h>
#include <Toshi/TSystem.h>
#include <Math/TRandom.h>
#include <Input/TInputDeviceKeyboard.h>
#include <T2Locale/T2Locale.h>
#include <File/TTRB.h>
#include <GUI/T2GUIFontManager.h>
#include <ToshiTools/T2CommandLine.h>

#include <Platform/DX8/TRenderContext_DX8.h>

TOSHI_NAMESPACE_USING

AGUI2TextBox* g_pExperimentalModeText = TNULL;

const T2CommandLine* g_pCommandLine;
TBOOL                g_bIsExperimentalMode = TFALSE;
TBOOL                g_bIsFunCategory      = TFALSE;

static TBOOL s_bRunFinished = TFALSE;

static void OnQuestProgressAdvanced( const ModEvent::AQuestManager::ProgressAdvanced& a_rEvent, void* a_pUserData )
{
	TTRACE( "Split! Old State: (%d, %d); Currect State: (%d, %d)\n", a_rEvent.iOldAct, a_rEvent.iOldTier, a_rEvent.iAct, a_rEvent.iTier );
	AUIManager::GetSingleton()->GetTimer().Split();
}

static void OnCowTippingEndGame( const ModEvent::ACowTippingMiniGameState::EndGame& a_rEvent, void* a_pUserData )
{
	// Detect game end, only if we are not on the normal cow tipping level
	if ( AGameLoader::GetCurrentTerrain() != AGameLoader::Terrain_CowTipping )
	{
		AUIManager::GetSingleton()->GetTimer().End();
		s_bRunFinished = TTRUE;
	}
}

// Replaces the text of the end popup once the run is finished
MEMBER_HOOK( &T2Locale::GetString, T2Locale, T2Locale_GetString, T2LocalisedString, TINT a_iNumString )
{
	if ( a_iNumString == 588 && s_bRunFinished )
	{
		s_bRunFinished = TFALSE;
		return const_cast<T2LocalisedString>( L"Nice run!!!" );
	}

	return CallOriginal( a_iNumString );
}

HOOK( &AInteractionManager::Candidates::GetRandomWeight, AInteractionManager_Candidates_GetRandomWeight, TFLOAT, TRandom* a_pRandom, TFLOAT a_fMaxWeight )
{
	// Always drop items instead of coins when kicking destructible objects
	return 0.0f;
}

static void OnNewGame( const ModEvent::AGameLoader::NewGame& a_rEvent, void* a_pUserData )
{
	AUIManager::GetSingleton()->GetTimer().Start();
}

static void OnAGUI2PostRender( const ModEvent::AGUI2::PostRender& a_rEvent, void* a_pUserData )
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

static const TClass* s_AGolfMiniGameState       = TNULL;
static const TClass* s_AVeggiePatchDefenderGame = TNULL;
static const TClass* s_AChasingChicksMiniGame   = TNULL;
static const TClass* s_AChickenCoopDefender     = TNULL;
static const TClass* s_ASleepCycleState         = TNULL;
static const TClass* s_AAnticGameOverState      = TNULL;
static const TClass* s_ABikeRaceMicroGame       = TNULL;

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
	if ( g_oSettings.bForceSkin && APlayerProfileManager::IsSingletonCreated() )
	{
		APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();

		pProfileManager->GetMainProfile()->SetCostume( g_oSettings.eCowSkin );
		pProfileManager->SetMainGender( g_oSettings.bIsMale ? APlayerProfile::GENDER_MALE : APlayerProfile::GENDER_FEMALE );
	}
}

Toshi::TRandom g_oRandom;

static void OnLoadScreenStateChanged( const ModEvent::ALoadScreen::StateChanged& a_rEvent, void* a_pUserData )
{
	AUIManager::GetSingleton()->GetTimer().SetIsLoadingScreen( a_rEvent.bLoading );
}

HOOK( &AAssetLoader::LoadPlayerCharacter, AAssetLoader_LoadPlayerCharacter, void, const PBPropertyValue* a_pProperty )
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
	CallOriginal( a_pProperty );
}

// NOTE: BYModCore hooked every int read from the save buffer to put the skin back, this does it once the profile is loaded
MEMBER_HOOK( &APlayerProfileManager::LoadMainProfile, APlayerProfileManager, APlayerProfileManager_LoadMainProfile, TBOOL, AHackedQuickSave::Buffer* a_pBuffer )
{
	TBOOL bResult = CallOriginal( a_pBuffer );

	if ( g_oSettings.bForceSkin )
		GetMainProfile()->SetCostume( g_oSettings.eCowSkin );

	return bResult;
}

//-----------------------------------------------------------------------------
// Fun% category
//-----------------------------------------------------------------------------

// Don't waste stamina in main game
MEMBER_HOOK( &APlayerProfileManager::UpdateMainProfile, APlayerProfileManager, APlayerProfileManager_UpdateMainProfile, TBOOL, TFLOAT a_fDeltaTime )
{
	APlayerSimProfile* pProfile = GetMainProfile();
	pProfile->SetUpdateStamina( TFALSE );
	pProfile->SetStamina( pProfile->GetMaxStamina() );

	return CallOriginal( a_fDeltaTime );
}

// NOTE: BYModCore patched three instructions of AGameStateController for these, the hooks below do the same
MEMBER_HOOK( &AGameStateController::UpdateTransition, AGameStateController, AGameStateController_UpdateTransition, TBOOL, TFLOAT a_flDeltaTime )
{
	// Remove the screen fade out: the overlay gets fully black in a single frame
	const TFLOAT flTransitionDeltaTime = HASANYFLAG( GetFlags(), FLAGS_AWAITING_TRANSITION ) ? 1.0f : a_flDeltaTime;

	if ( CallOriginal( flTransitionDeltaTime ) )
		return TTRUE;

	// OnUpdate only skips the update of the current state while waiting for a transition, not while
	// the input is ignored or the screen fades in (the flag check in OnUpdate checked FLAGS_AWAITING_TRANSITION only)
	const FLAGS eFlags = GetFlags();

	if ( HASANYFLAG( eFlags, FLAGS_IGNORE_INPUT | FLAGS_FADING_IN ) && !HASANYFLAG( eFlags, FLAGS_AWAITING_TRANSITION ) )
		GetCurrentState()->OnUpdate( a_flDeltaTime );

	return TFALSE;
}

MEMBER_HOOK( &AGameStateController::ProcessInput, AGameStateController, AGameStateController_ProcessInput, TBOOL, const TInputInterface::InputEvent* a_pEvent )
{
	// Remove the input ignore flag check
	const TBOOL bIgnoreInput = HASANYFLAG( GetFlags(), FLAGS_IGNORE_INPUT );
	SetIgnoreInput( TFALSE );

	TBOOL bResult = CallOriginal( a_pEvent );

	SetIgnoreInput( bIgnoreInput );
	return bResult;
}

class ABYSpeedrunHelper : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		SUBSCRIBE_EVENT( AGameLoader::NewGame, OnNewGame );
		SUBSCRIBE_EVENT( AGUI2::Created, OnAGUI2Created );
		SUBSCRIBE_EVENT( AGUI2::PostRender, OnAGUI2PostRender );
		SUBSCRIBE_EVENT( AQuestManager::ProgressAdvanced, OnQuestProgressAdvanced );
		SUBSCRIBE_EVENT( ACowTippingMiniGameState::EndGame, OnCowTippingEndGame );
		SUBSCRIBE_EVENT( ALoadScreen::StateChanged, OnLoadScreenStateChanged );

		g_oSettings.Load();

		InstallHook<T2Locale_GetString>();
		InstallHook<AAssetLoader_LoadPlayerCharacter>();
		InstallHook<APlayerProfileManager_LoadMainProfile>();

		if ( g_bIsFunCategory )
		{
			// Remove screen fade out and the input ignore flag of the game state controller
			InstallHook<AGameStateController_UpdateTransition>();
			InstallHook<AGameStateController_ProcessInput>();

			// For Fun% category reduce number of laps in the bike race mini game
			// NOTE: the tracks are const, so they need the protection of their page changed
			ABikeRaceMicroGame::TrackInfo* pTracks = const_cast<ABikeRaceMicroGame::TrackInfo*>( ABikeRaceMicroGame::ms_aTracks );

			DWORD dwOldProtection;
			VirtualProtect( pTracks, sizeof( ABikeRaceMicroGame::ms_aTracks ), PAGE_READWRITE, &dwOldProtection );

			for ( TINT i = 0; i < ABikeRaceMicroGame::NUM_TRACKS; i++ )
				pTracks[ i ].iNumLaps = 1;

			VirtualProtect( pTracks, sizeof( ABikeRaceMicroGame::ms_aTracks ), dwOldProtection, &dwOldProtection );

			// Derandomizer
			InstallHook<AInteractionManager_Candidates_GetRandomWeight>();

			// Don't waste stamina in main game
			InstallHook<APlayerProfileManager_UpdateMainProfile>();

			// Load overrides with minigames settings
			m_oOverridesTRB.Load( TString8::VarArgs( "%sBYSpeedrunHelper\\Fun\\Overrides.trb", AModLoader::GetModsDirectory() ) );
		}

		// TODO: ACollisionInspector (-experimental) isn't ported yet
		ASplitsServer::CreateSingleton();

		return TTRUE;
	}

	void OnAllModsLoaded() override
	{
		s_AGolfMiniGameState       = TClass::Find( "AGolfMinigameState" );
		s_AVeggiePatchDefenderGame = TClass::Find( "AVeggiePatchDefenderGame" );
		s_AChasingChicksMiniGame   = TClass::Find( "AChasingChicksMiniGame" );
		s_AChickenCoopDefender     = TClass::Find( "AChickenCoopDefender" );
		s_ASleepCycleState         = TClass::Find( "ASleepCycleState" );
		s_AAnticGameOverState      = TClass::Find( "AAnticGameOverState" );
		s_ABikeRaceMicroGame       = TClass::Find( "ABikeRaceMicroGame" );
	}

	TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override
	{
		if ( !AGameStateController::IsSingletonCreated() )
			return TTRUE;

		AGameStateController* pGameStateController = AGameStateController::GetSingleton();

		// Apply minigame specific settings
		if ( g_oGolfMiniGameSettings.bChangeLevel )
		{
			AGameState* pCurrentState = pGameStateController->GetCurrentState();

			if ( pGameStateController->GetNumStates() > 1 && pCurrentState->GetClass() == s_AGolfMiniGameState )
			{
				AGolfMinigameState* pGolfState = TSTATICCAST( AGolfMinigameState, pCurrentState );

				g_oGolfMiniGameSettings.bChangeLevel = TFALSE;

				// Set the first level if tried to load previous while on the first one
				pGolfState->SetCurrentHoleIndex( TMath::Max( g_oGolfMiniGameSettings.iLevel, -1 ) );

				// Ask the game to load next level
				pGolfState->LoadHole();
			}
		}

		// Fun% mode
		if ( g_bIsFunCategory )
		{
			// Adjust speed of the game
			TBOOL  bSpeedUp = TFALSE;
			TFLOAT fSpeed   = 1.0f;

			if ( pGameStateController->GetNumStates() > 1 )
			{
				const TClass* pStateClass = pGameStateController->GetCurrentState()->GetClass();

				if ( pStateClass == s_AChickenCoopDefender || pStateClass == s_AChasingChicksMiniGame )
				{
					bSpeedUp = TTRUE;
					fSpeed   = 2.0f;
				}
				else if ( pStateClass == s_AVeggiePatchDefenderGame )
				{
					bSpeedUp = TTRUE;
					fSpeed   = 2.0f;
				}
				else if ( pStateClass == s_AAnticGameOverState )
				{
					bSpeedUp = TTRUE;
					fSpeed   = 4.0f;
				}
				else if ( pStateClass == s_ASleepCycleState )
				{
					bSpeedUp = TTRUE;
					fSpeed   = 10.0f;
				}

				g_oSystemManager.GetScheduler()->SetDebugDeltaTimeMult( bSpeedUp, fSpeed );
			}
		}

		// Experimental mode
		if ( g_bIsExperimentalMode && TInputInterface::IsSingletonCreated() )
		{
			// Simulate lag by pressing Q key
			TInputDeviceKeyboard* pKeyboard = TSTATICCAST( TInputDeviceKeyboard, TInputInterface::GetSingleton()->GetDeviceByIndex( &TGetClass( TInputDeviceKeyboard ), 0 ) );

			if ( pKeyboard )
			{
				TBOOL        bIsDown    = pKeyboard->IsDown( TInputDeviceKeyboard::KEY_Q );
				static TBOOL s_bWasDown = TFALSE;

				g_oSystemManager.GetScheduler()->SetDebugDeltaTime( !s_bWasDown && bIsDown, 0.5f );
				s_bWasDown = bIsDown;
			}
		}

		if ( AUIManager::IsSingletonCreated() )
			AUIManager::GetSingleton()->Update( a_fDeltaTime );

		return TTRUE;
	}

	void OnUnload() override
	{
		UNSUBSCRIBE_EVENT( AGameLoader::NewGame, OnNewGame );
		UNSUBSCRIBE_EVENT( AGUI2::Created, OnAGUI2Created );
		UNSUBSCRIBE_EVENT( AGUI2::PostRender, OnAGUI2PostRender );
		UNSUBSCRIBE_EVENT( AQuestManager::ProgressAdvanced, OnQuestProgressAdvanced );
		UNSUBSCRIBE_EVENT( ACowTippingMiniGameState::EndGame, OnCowTippingEndGame );
		UNSUBSCRIBE_EVENT( ALoadScreen::StateChanged, OnLoadScreenStateChanged );

		if ( AUIManager::IsSingletonCreated() )
			AUIManager::GetSingleton()->Destroy();
	}

	static void OnAGUI2Created( const ModEvent::AGUI2::Created& a_rEvent, void* a_pUserData )
	{
		AUIManager::CreateSingleton()->Create();

		if ( g_bIsExperimentalMode )
		{
			TFLOAT fWidth, fHeight;
			AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

			auto pFont              = T2GUIFontManager::FindFont( "Rekord18" );
			g_pExperimentalModeText = new AGUI2TextBox();

			g_pExperimentalModeText->Create( pFont, 400.0f );
			g_pExperimentalModeText->SetText( L"Experimental mode!" );
			g_pExperimentalModeText->SetColour( TCOLOR( 255, 0, 0 ) );
			g_pExperimentalModeText->SetTransform( 0.0f, -fHeight / 2 + 32.0f );
			g_pExperimentalModeText->SetAlpha( 1.0f );
			g_pExperimentalModeText->SetTextAlign( T2GUIFont::TextAlign_Center );
			g_pExperimentalModeText->SetInFront();
			g_pExperimentalModeText->SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
		}
	}

	void OnImGuiRender( AImGui* a_pImGui ) override
	{
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
		if ( AGameStateController::IsSingletonCreated() )
		{
			// NOTE: the current state is the ImGui window itself, the minigame is under it
			AGameState* pCurrentState = AGameStateController::GetSingleton()->GetPreviousState();

			if ( pCurrentState )
			{
				const TClass* pCurrentStateClass = pCurrentState->GetClass();
				TBOOL         bDrawnUI           = TFALSE;

				if ( pCurrentStateClass == s_AGolfMiniGameState )
				{
					ImGui::Text( "Golf Hole Selector" );

					AGolfMinigameState* pGolfState    = TSTATICCAST( AGolfMinigameState, pCurrentState );
					const TINT          iCurrentLevel = pGolfState->GetCurrentHoleIndex();

					auto fnLoadLevel = [ a_pImGui ]( TINT iLevel ) {
						g_oGolfMiniGameSettings.bChangeLevel = TTRUE;
						g_oGolfMiniGameSettings.iLevel       = iLevel;
						a_pImGui->Toggle();
					};

					if ( ImGui::Button( "Prev Level" ) ) fnLoadLevel( iCurrentLevel - 2 );
					ImGui::SameLine();
					if ( ImGui::Button( "Restart Level" ) ) fnLoadLevel( iCurrentLevel - 1 );
					ImGui::SameLine();
					if ( ImGui::Button( "Next Level" ) ) fnLoadLevel( iCurrentLevel );

					static TINT s_iHoleSelection = 1;
					ImGui::SliderInt( "##Hole", &s_iHoleSelection, 1, pGolfState->GetCourse()->iNumHoles );
					ImGui::SameLine();
					if ( ImGui::Button( "Start Hole" ) ) fnLoadLevel( s_iHoleSelection - 2 );

					bDrawnUI = TTRUE;
				}
				else if ( m_pBikeRace )
				{
					ImGui::Text( "Bike Race" );
					ImGui::Checkbox( "Debug", &g_oSettings.bShowBikeRaceDebug );
					ImGui::Checkbox( "Show waypoints", &g_oSettings.bShowWaypoints );
					ImGui::Checkbox( "Hide all except max reachable", &g_oSettings.bShowOnlyMaxReachableWaypoint );
					ImGui::Checkbox( "Show reachable waypoints", &g_oSettings.bShowReachableWaypoints );
					ImGui::Checkbox( "Show unreachable waypoints", &g_oSettings.bShowUnreachableWaypoints );
					ImGui::Checkbox( "Show reached waypoints", &g_oSettings.bShowReachedWaypoints );

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

	TBOOL IsOverlayVisible() override
	{
		m_pBikeRace = TNULL;

		if ( !AGameStateController::IsSingletonCreated() )
			return TFALSE;

		auto fnCheckGameState = [ this ]( AGameState* a_pGameState ) {
			if ( a_pGameState && a_pGameState->GetClass() == s_ABikeRaceMicroGame )
				m_pBikeRace = TSTATICCAST( ABikeRaceMicroGame, a_pGameState );

			return m_pBikeRace != TNULL;
		};

		AGameStateController* pGameStateController = AGameStateController::GetSingleton();

		// Check both current and previous states
		if ( pGameStateController->GetNumStates() > 1 && !fnCheckGameState( pGameStateController->GetCurrentState() ) )
			fnCheckGameState( pGameStateController->GetPreviousState() );

		return m_pBikeRace != TNULL;
	}

	void OnImGuiRenderOverlay( AImGui* a_pImGui ) override
	{
		if ( !m_pBikeRace || !g_oSettings.bShowBikeRaceDebug ) return;

		ImGui::SetNextWindowPos( ImVec2( 16.0f, 64.0f ) );
		ImGui::Begin(
		    "Bike Race Debug",
		    TNULL,
		    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize
		);
		{
			const ABikeRaceMicroGame::Biker& rPlayer = m_pBikeRace->GetBiker( 0 );

			const TVector4* pWaypointPositions           = m_pBikeRace->GetWaypoints();
			const TINT      iCurrentWaypointControlIndex = rPlayer.iSegment;
			const TINT      iCurrentWaypointIndex        = rPlayer.iWaypoint;
			const TINT      iNumWaypoints                = m_pBikeRace->GetNumWaypoints();

			const TINT iMaxReachableWP = TMath::Min( ( iCurrentWaypointControlIndex + 1 ) * ABikeRaceMicroGame::SEGMENT_SIZE, iNumWaypoints );

			// Print on screen info
			ImGui::Text( "Waypoint: %d / %d", iCurrentWaypointIndex, iNumWaypoints );
			ImGui::Text( "Reachable: %d", iMaxReachableWP );
			ImGui::Text( "Control: %d", iCurrentWaypointControlIndex );

			// Draw the reachable waypoint
			ARenderer* pRenderer = ARenderer::GetSingleton();

			if ( g_oSettings.bShowWaypoints && pRenderer && pRenderer->GetMainViewport() )
			{
				TRenderContextD3D* pRenderContext = TRenderContextD3D::Upcast( pRenderer->GetMainViewport()->GetRenderContext() );

				const TMatrix44& matProj = pRenderContext->GetProjectionMatrix();
				const TMatrix44& matWVM  = pRenderContext->GetWorldViewMatrix();

				ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
				draw_list->PushClipRectFullScreen();

				auto& viewportParams = pRenderContext->GetViewportParameters();

				auto fnDrawWorldPoint = [ & ]( const TVector4& a_vWorldPoint, TFLOAT a_flMinRadius, TFLOAT a_flMaxRadius, TUINT32 a_uiColor, TUINT32 a_uiFillColor, const char* a_pchText ) {
					TVector4 vReachableWP_View;
					TMatrix44::TransformVector( vReachableWP_View, matWVM, a_vWorldPoint );

					TVector4 vReachableWP_Screen;
					TMatrix44::TransformVector( vReachableWP_Screen, matProj, vReachableWP_View );

					if ( vReachableWP_Screen.w > 0.0f )
					{
						// Point is visible, can draw
						vReachableWP_Screen.x /= vReachableWP_Screen.w;
						vReachableWP_Screen.y /= vReachableWP_Screen.w;

						// Normalize coords
						vReachableWP_Screen.x = ( vReachableWP_Screen.x + 1.0f ) * 0.5f;
						vReachableWP_Screen.y = 1.0f - ( vReachableWP_Screen.y + 1.0f ) * 0.5f;

						TFLOAT flDistanceFactor = 1.0f - TMath::Min( vReachableWP_Screen.w, 100.0f ) / 100.0f;

						const TFLOAT flRadius = TMath::Min( TMath::Max( a_flMaxRadius * flDistanceFactor, a_flMinRadius ), a_flMaxRadius );

						ImVec2 vScreenPos( vReachableWP_Screen.x * viewportParams.fWidth, vReachableWP_Screen.y * viewportParams.fHeight );

						if ( a_uiFillColor != 0 )
						{
							draw_list->AddCircleFilled(
							    vScreenPos,
							    flRadius,
							    a_uiFillColor,
							    0
							);
						}

						// Draw an outlined circle
						draw_list->AddCircle(
						    vScreenPos,
						    flRadius,
						    a_uiColor,
						    0,
						    2.0f
						);

						if ( a_pchText )
						{
							ImVec2 vTextSize = ImGui::CalcTextSize( a_pchText );

							draw_list->AddText( ImVec2( vScreenPos.x - vTextSize.x * 0.5f, vScreenPos.y + flRadius ), a_uiColor, a_pchText );
						}
					}
				};

				auto fnGetReachableColor = []( TUINT8 a_uiAlpha ) {
					return IM_COL32( 68, 180, 68, a_uiAlpha );
				};

				auto fnGetUnreachableColor = []( TUINT8 a_uiAlpha ) {
					return IM_COL32( 180, 68, 68, a_uiAlpha );
				};

				auto fnGetReachedColor = []( TUINT8 a_uiAlpha ) {
					return IM_COL32( 180, 180, 180, a_uiAlpha );
				};

				TCHAR szWaypointName[ 16 ];

				if ( g_oSettings.bShowOnlyMaxReachableWaypoint )
				{
					// Show the max reachable
					T2String8::Format( szWaypointName, sizeof( szWaypointName ), "wp_%.3i", iMaxReachableWP );

					// Calculate terrain height for the point
					TVector4 vecWorldPos = pWaypointPositions[ iMaxReachableWP - 1 ];
					ATerrainInterface::GetSingleton()->SetToGroundHeightBottom( vecWorldPos, 0 );

					fnDrawWorldPoint( pWaypointPositions[ iMaxReachableWP - 1 ], 10.0f, 35.0f, IM_COL32( 255, 255, 255, 255 ), fnGetReachableColor( 255 ), szWaypointName );
				}
				else
				{
					for ( TINT i = 0; i < iNumWaypoints; i++ )
					{
						const TBOOL bIsReachable = ( i - iCurrentWaypointControlIndex * ABikeRaceMicroGame::SEGMENT_SIZE ) < ABikeRaceMicroGame::SEGMENT_SIZE;
						const TBOOL bIsReached   = i < iCurrentWaypointIndex;

						if ( bIsReachable && !g_oSettings.bShowReachableWaypoints ) continue;
						if ( !bIsReachable && !g_oSettings.bShowUnreachableWaypoints ) continue;
						if ( bIsReached && !g_oSettings.bShowReachedWaypoints ) continue;

						T2String8::Format( szWaypointName, sizeof( szWaypointName ), "wp_%.3i", i + 1 );

						// Calculate terrain height for the point
						TVector4 vecWorldPos = pWaypointPositions[ i ];
						ATerrainInterface::GetSingleton()->SetToGroundHeightBottom( vecWorldPos, 0 );

						fnDrawWorldPoint( vecWorldPos, 10.0f, 25.0f, IM_COL32( 255, 255, 255, 255 ), bIsReachable ? ( bIsReached ? fnGetReachedColor( 255 ) : fnGetReachableColor( 255 ) ) : fnGetUnreachableColor( 255 ), szWaypointName );
					}
				}

				draw_list->PopClipRect();
			}

			ImGui::End();
		}
	}

	TBOOL HasSettingsUI() override
	{
		return TTRUE;
	}

private:
	TTRB m_oOverridesTRB;

	ABikeRaceMicroGame* m_pBikeRace = TNULL;
};

extern "C"
{
	MOD_EXPORT AModInstance* CreateModInstance( const T2CommandLine* a_pCommandLine )
	{
		g_pCommandLine        = a_pCommandLine;
		g_bIsExperimentalMode = g_pCommandLine->HasParameter( "-experimental" );
		g_bIsFunCategory      = g_pCommandLine->HasParameter( "-fun" );

		return new ABYSpeedrunHelper();
	}

	MOD_EXPORT const TCHAR* GetModName()
	{
		return "BYSpeedrunHelper";
	}

	MOD_EXPORT TUINT32 GetModVersion()
	{
		return TVERSION( 2, 0 );
	}
}
