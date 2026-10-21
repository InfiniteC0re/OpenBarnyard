#include "pch.h"
#include "AGameStateController.h"
#include "ASleepCycleChooseState.h"
#include "Tasks/ARootTask.h"
#include "GUI/AGUI2.h"
#include "AppBoot.h"
#include "ACheatActivator.h"
#include "ALoadScreen.h"
#include "Cameras/ACameraManager.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "Helpers/AGameLoader.h"
#include "HUD/AHudManager.h"
#include "GUI/AFadeManager.h"
#include "AStartMenuState.h"
#include "AHackedQuickSave.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"
#include "SaveLoadSKU.h"
#include "Helpers/APlayerProgress.h"
#include "Quests/AQuestManager.h"
#include "ADebugMenuState.h"
#include "GUI/AGUISystem.h"
#include "Mod/ImGui/AImGui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00428b70, CreateObject 004296e0, CreateObjectInPlace 00429740
TDEFINE_CLASS_INIT( AGameStateController );

// DAT_0078178c
AFunctionTable AGameStateController::ms_oFunctionTable( &TGetClass( AGameStateController ), TNULL );

// The natives of the table are called on the controller
// $Barnyard: FUNCTION 00428ba0
static TObject* __stdcall AGameStateControllerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return AGameStateController::GetSingleton();
}

// $Barnyard: FUNCTION 0042a9d0
void AGameStateController::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AGameStateControllerFunctionFallback ) );

	// NOTE: 00428150 is the compiler's vcall thunk of the virtual TTask::Reset
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AGameStateController>::Create( "Reset", &AGameStateController::Reset ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AGameStateController, const TPString8&>::Create( "ResetStackForSingleState", &AGameStateController::RecreateMiniGameState ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AGameStateController, TPString8>::Create( "CreateAndPushNamedState", &AGameStateController::CreateAndPushNamedState ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AGameStateController>::Create( "PopCurrentState", &AGameStateController::PopCurrentGameState ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AGameStateController, TPString8>::Create( "ReturnToNamedState", &AGameStateController::ReturnToNamedState ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AGameStateController, const TPString8&>::Create( "CreateAndTransferToNamedState", &AGameStateController::CreateAndTransferToNamedState ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AGameStateController, const TPString8&>::Create( "CreateAndAddChildState", &AGameStateController::AddChildStateByName ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AGameStateController, const TPString8&>::Create( "IsStateOnStack", &AGameStateController::IsStateOnStack ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AGameStateController, TPString8>::Create( "StartGameState", &AGameStateController::StartGameState ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AGameStateController>::Create( "FinishGameState", &AGameStateController::FinishGameState ) );
	ms_oFunctionTable.AddFunction( AMethodPointer3Args<AGameStateController, TBOOL, TBOOL, TBOOL>::Create( "SaveCurrentGameState", &AGameStateController::SaveCurrentGameState ) );
}

// $Barnyard: FUNCTION 00428b80
void AGameStateController::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00429b70
void AGameStateController::StartGameState( TPString8 a_strClassName )
{
	AGameState* pCurrentState = GetCurrentState();

	if ( pCurrentState )
	{
		pCurrentState->GetHUDParams().SetFlags( 0 );
		AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	}

	UpdateScreenOverlay();
	AGameLoader::SetTransitionToGameState( a_strClassName, TTRUE );
}

// $Barnyard: FUNCTION 00429520
void AGameStateController::FinishGameState()
{
	AGameState* pCurrentState = GetCurrentState();

	if ( pCurrentState )
	{
		pCurrentState->GetHUDParams().SetFlags( 0 );
		AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	}

	UpdateScreenOverlay();
	AHackedQuickSave::GetSingleton()->SaveTime();
	AHackedQuickSave::GetSingleton()->SaveProfile();
	AGameLoader::SetTransitionToMainGame( TFALSE );
}

// $Barnyard: FUNCTION 0042a7f0
void AGameStateController::CreateAndPushNamedState( TPString8 a_strClassName )
{
	const TClass* pClass = TClass::Find( a_strClassName.GetString8().GetString(), &TGetClass( AGameState ) );

	if ( pClass )
		PushState( TSTATICCAST( AGameState, pClass->CreateObject() ) );
}

// $Barnyard: FUNCTION 0042a110
void AGameStateController::CreateAndTransferToNamedState( const TPString8& a_rcClassName )
{
	const TClass* pClass = TClass::Find( a_rcClassName.GetString8().GetString(), &TGetClass( AGameState ) );

	if ( pClass )
		ReplaceState( TSTATICCAST( AGameState, pClass->CreateObject() ) );
}

// $Barnyard: FUNCTION 0042a890
void AGameStateController::ReturnToNamedState( TPString8 a_strClassName )
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
	{
		AGameState* pState = *it;

		if ( TPString8( pState->GetClass()->GetName() ) == a_strClassName )
		{
			while ( GetCurrentState() != pState )
				PopState( GetCurrentState() );

			return;
		}
	}
}

// $Barnyard: FUNCTION 004298f0
TBOOL AGameStateController::IsStateOnStack( const TPString8& a_rcClassName )
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
	{
		if ( TPString8( ( *it )->GetClass()->GetName() ) == a_rcClassName )
			return TTRUE;
	}

	return TFALSE;
}

TPSTRING8_DECLARE( AAnimalConversationState );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( ABarnExploreMinigameState );

// $Barnyard: FUNCTION 00429250
AGameStateController::AGameStateController()
    : m_UnkVector( m_VectorData, TARRAYSIZE( m_VectorData ), 0 )
{
	// Incomplete
	m_eFlagsRaw     = 0;
	m_flOverlayGoal = 0.0f;
}

// $Barnyard: FUNCTION 004292f0
// $Barnyard: FUNCTION 00429780
AGameStateController::~AGameStateController()
{
	TASSERT( m_oStates.Size() == 0 );
}

// $Barnyard: FUNCTION 00429e90
TBOOL AGameStateController::OnCreate()
{
	InsertGameState( new AGameState );

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	m_oOverlay.SetDimensions( fWidth, fHeight );
	m_oOverlay.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oOverlay.SetInFront();
	m_oOverlay.Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 0042a770
TBOOL AGameStateController::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_eFlagsRaw & FLAGS_LEAVE_IMMEDIATELY )
	{
		m_eFlagsRaw &= ~FLAGS_LEAVE_IMMEDIATELY;

		if ( HASANYFLAG( m_eFlagsRaw, FLAGS_RESTART_MINIGAME ) )
		{
			AGameLoader::RestartMiniGame( TFALSE );
			return TTRUE;
		}
		else
		{
			AGameLoader::ReturnToFrontEnd( TNULL, TTRUE, TFALSE );
			return TTRUE;
		}
	}

	if ( m_oStates.Size() > 1 )
	{
		// Update transition state first
		if ( UpdateTransition( a_fDeltaTime ) )
			return TTRUE;

		// If no transition and input is allowed, update gamestate
		if ( !HASANYFLAG( m_eFlagsRaw, FLAGS_IGNORE_INPUT | FLAGS_AWAITING_TRANSITION | FLAGS_FADING_IN ) )
			( *m_oStates.Back() )->OnUpdate( a_fDeltaTime );
	}

	UpdateScreenOverlay();
	return TTRUE;
}

// $Barnyard: FUNCTION 00429380
void AGameStateController::OnDestroy()
{
	TASSERT( m_oStates.Size() == 1 );

	auto pGameState = *m_oStates.Back();
	m_oStates.PopBack();

	pGameState->Destroy();
}

// $Barnyard: FUNCTION 00429b00
void AGameStateController::InsertGameState( AGameState* a_pGameState )
{
	TGlobalEmitter<AGameStateControllerEvent>::Throw(
	    AGameStateControllerEvent( a_pGameState, AGameStateControllerEvent::Type_GameStateBeingAdded )
	);

	m_oStates.PushBack( a_pGameState );
	a_pGameState->OnInsertion();
	a_pGameState->Activate();
}

// $Barnyard: FUNCTION 0042a090
void AGameStateController::ReplaceState( AGameState* a_pGameState )
{
	if ( m_oStates.Size() > 1 )
	{
		auto pCurrentGameState = *m_oStates.Back();
		m_oStates.PopBack();

		pCurrentGameState->Destroy();
	}

	InsertGameState( a_pGameState );
}

// $Barnyard: FUNCTION 00429f50
void AGameStateController::PushState( AGameState* a_pGameState )
{
	if ( m_oStates.Size() > 1 )
	{
		// NOTE: AGameState::Deactivate inlined; the original reads the top of the stack again after each call
		( *m_oStates.Back() )->OnDeactivate();

		TGlobalEmitter<AGameStateControllerEvent>::Throw(
		    AGameStateControllerEvent( *m_oStates.Back(), AGameStateControllerEvent::Type_GameStateDeactivated )
		);

		( *m_oStates.Back() )->OnSuspend();
	}

	InsertGameState( a_pGameState );
}

// $Barnyard: FUNCTION 00429fb0
void AGameStateController::PopState( AGameState* a_pGameState )
{
	auto pCurrentGameState = *m_oStates.Back();

	TASSERT( a_pGameState == pCurrentGameState );
	m_oStates.PopBack();

	pCurrentGameState->Destroy();

	if ( m_oStates.Size() == 1 && ARootTask::GetSingleton()->IsGameSystemCreated() )
	{
		g_oTheApp.Destroy();
		return;
	}

	if ( m_oStates.Size() > 1 )
	{
		// NOTE: AGameState::Activate inlined; the original reads the top of the stack again after each call.
		// The resumed state can change the stack in OnActivate (AEndGameState returns to the front end, which
		// destroys every state), so the activation event is thrown for the state on top afterwards.
		( *m_oStates.Back() )->OnResume( a_pGameState );
		( *m_oStates.Back() )->OnActivate();

		TGlobalEmitter<AGameStateControllerEvent>::Throw(
		    AGameStateControllerEvent( *m_oStates.Back(), AGameStateControllerEvent::Type_GameStateActivated )
		);
	}
}

// $Barnyard: FUNCTION 0042a870
void AGameStateController::PopCurrentGameState()
{
	TASSERT( m_oStates.Size() > 1 );

	if ( m_oStates.Size() > 1 )
	{
		PopState( *m_oStates.Back() );
	}
}

// $Barnyard: FUNCTION 00429490
void AGameStateController::UpdateScreenOverlay()
{
	TBOOL bAddOverlay = TFALSE;

	if ( m_oStates.Size() > 1 && ( *m_oStates.Back() )->GetOverlay() != AGameState::OVERLAY_1 )
	{
		OverlayData* pOverlay = GetOverlayParams( ( *m_oStates.Back() )->GetOverlay() );
		m_oOverlay.SetColour( TCOLOR4( pOverlay->uiColorR, pOverlay->uiColorG, pOverlay->uiColorB, pOverlay->uiColorA ) );

		bAddOverlay = TTRUE;
	}

	m_oOverlay.RemoveSelf();

	if ( bAddOverlay )
	{
		AGUI2::GetRootElement()->AddChildTail( m_oOverlay );
		m_oOverlay.Show();
	}
	else
	{
		m_oOverlay.Hide();
	}
}

// $Barnyard: FUNCTION 0042a1e0
void AGameStateController::RecreateMiniGameState( const TPString8& a_rcClassName )
{
	m_eFlagsRaw = 0;
	ResetStack();

	const TClass* pClass = TClass::Find( a_rcClassName.GetString8().GetString(), &TGetClass( AGameState ) );

	if ( pClass )
		InsertGameState( TSTATICCAST( AGameState, pClass->CreateObject() ) );
}

// $Barnyard: FUNCTION 0042a150
void AGameStateController::ResetStack()
{
	UpdateScreenOverlay();
	m_eFlagsRaw = 0;

	TBOOL bIsFirst = TTRUE;

	while ( 1 < m_oStates.Size() )
	{
		auto pGameState = *m_oStates.Back();
		m_oStates.PopBack();

		pGameState->Destroy( bIsFirst );
		bIsFirst = TFALSE;
	}
}

// $Barnyard: FUNCTION 00429da0
void AGameStateController::ResetOverlays()
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
		( *it )->SetOverlay( AGameState::OVERLAY_1 );
}

// $Barnyard: FUNCTION 00429580
void AGameStateController::SetFlags( FLAGS a_eFlags )
{
	m_eFlagsRaw |= a_eFlags;
	m_flSoundVolume = 1.0f;

	if ( m_eFlagsRaw & FLAGS_KEEP_OVERLAY_ALPHA )
	{
		m_eFlagsRaw &= ~FLAGS_KEEP_OVERLAY_ALPHA;
		m_flOverlayAlpha = m_flOverlayGoal;
	}
	else
	{
		m_flOverlayGoal  = 0.0f;
		m_flOverlayAlpha = ( a_eFlags == FLAGS_FADING_IN ) ? 1.0f : 0.0f;
	}

	OverlayData oData;
	oData.uiColorA = TUINT8( m_flOverlayAlpha * 255.0f );
	oData.uiColorR = 0;
	oData.uiColorG = 0;
	oData.uiColorB = 0;
	SetOverlayParams( AGameState::OVERLAY_3, oData );

	( *m_oStates.Back() )->SetOverlay( AGameState::OVERLAY_3 );
	UpdateScreenOverlay();
}

// $Barnyard: FUNCTION 004299f0
AGameState* AGameStateController::FindGameStateOnStack( const TPString8& a_rcClassName )
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
	{
		if ( TPString8( ( *it )->GetClass()->GetName() ) == a_rcClassName )
			return *it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00429450
void AGameStateController::AddChildStateByName( const TPString8& a_rcClassName )
{
	const TClass* pClass = TClass::Find( a_rcClassName.GetString(), &TGetClass( AGameState ) );

	if ( pClass )
		GetCurrentState()->AddChildState( TSTATICCAST( AGameState, pClass->CreateObject() ) );
}

// $Barnyard: FUNCTION 006034b0
TBOOL AGameStateController::IsBusy()
{
	return ( GetSingleton()->m_eFlagsRaw & ( FLAGS_AWAITING_TRANSITION | FLAGS_FADING_IN ) ) ||
	    ( AFadeManager::IsSingletonCreated() && AFadeManager::GetSingleton()->HasAnyFadesInProgress( TFALSE ) );
}

// $Barnyard: FUNCTION 004297a0
void AGameStateController::RenderStates( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
		( *it )->OnRender( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00429810
void AGameStateController::PostRenderStates( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
		( *it )->OnPostRender( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00429880
void AGameStateController::RenderStatesOverlay( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
		( *it )->OnRenderOverlay( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004280b0
void AGameStateController::SetOverlayParams( AGameState::OVERLAY a_eOverlay, const OverlayData& a_rcParams )
{
	TASSERT( a_eOverlay < AGameState::OVERLAY_NUMOF );
	ms_aOverlays[ a_eOverlay ] = a_rcParams;
}

// $Barnyard: FUNCTION 004280a0
AGameStateController::OverlayData* AGameStateController::GetOverlayParams( AGameState::OVERLAY a_eOverlay )
{
	TASSERT( a_eOverlay < AGameState::OVERLAY_NUMOF );
	return &ms_aOverlays[ a_eOverlay ];
}

// $Barnyard: FUNCTION 0042a320
TBOOL AGameStateController::UpdateTransition( TFLOAT a_flDeltaTime )
{
	TASSERT( m_oStates.Size() >= 1 );
	AGameState* pCurrentState = *m_oStates.Back();

	if ( m_eFlagsRaw & FLAGS_AWAITING_TRANSITION )
	{
		// Transition is planned
		m_flOverlayAlpha = a_flDeltaTime + a_flDeltaTime + m_flOverlayAlpha;

		// Update volume if needed
		if ( m_eFlagsRaw & FLAGS_FADE_SOUND )
		{
			m_flSoundVolume = m_flSoundVolume - ( a_flDeltaTime + a_flDeltaTime );
			ASoundManager::GetSingleton()->SetGlobalVolume( m_flSoundVolume );
		}

		// Check if the transition is over
		if ( m_flOverlayAlpha > 1.0 )
		{
			// Screen is black, can change game state
			m_eFlagsRaw &= ~( FLAGS_AWAITING_TRANSITION | FLAGS_FADE_SOUND );
			m_flOverlayAlpha = 1.0;
			UpdateScreenOverlay();
			pCurrentState->m_eOverlayColorIndex = 0;

			if ( m_eFlags.bIgnoreInput )
			{
				// Input is ignored
				pCurrentState->m_HUDParams.SetFlags( 0 );

				AHudManager::GetSingleton()->OnUpdate( a_flDeltaTime );

				m_eFlags.bIgnoreInput = TFALSE;

				if ( m_eFlagsRaw & FLAGS_RETURN_TO_FRONTEND )
				{
					m_eFlagsRaw &= ~FLAGS_RETURN_TO_FRONTEND;
					AGameLoader::ReturnToFrontEnd( TNULL, TTRUE, TTRUE );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_TRANSITION_TO_MINIGAME )
				{
					m_eFlagsRaw &= ~FLAGS_TRANSITION_TO_MINIGAME;
					AGameLoader::SetTransitionToMiniGame( AGameLoader::GetNextMiniGame(), TTRUE );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_LEAVE_SCRIPTED_SEQUENCE )
				{
					m_eFlagsRaw &= ~FLAGS_LEAVE_SCRIPTED_SEQUENCE;
					AGameLoader::LeaveScriptedSequence( TTRUE );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_LOAD_SAVED_GAME )
				{
					m_eFlagsRaw &= ~FLAGS_LOAD_SAVED_GAME;
					AGameLoader::LoadSavedGame( TTRUE );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_REMOVE_STATE )
				{
					m_eFlagsRaw &= ~FLAGS_REMOVE_STATE;
					pCurrentState->Remove();
					ShowOverlay( TTRUE, TFALSE, 1.0f, TFALSE );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_RESTART_MINIGAME )
				{
					m_eFlagsRaw &= ~FLAGS_RESTART_MINIGAME;
					( *m_oStates.Back() )->m_GUIElement.Hide();

					AGameLoader::RestartMiniGame( TTRUE );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				pCurrentState->m_GUIElement.Hide();
				AGameLoader::LeaveMiniGame( TTRUE );

				m_eFlags.UNK10 = TFALSE;
				return TTRUE;
			}
			else
			{
				// Input is not ignored
				if ( m_eFlagsRaw & FLAGS_START_NEW_GAME )
				{
					m_eFlagsRaw &= ~FLAGS_START_NEW_GAME;
					AGameLoader::StartNewGame();

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}
			}
		}

		UpdateOverlayAlpha();
	}
	else
	{
		// No transition
		if ( m_eFlagsRaw & FLAGS_FADING_IN )
		{
			m_flOverlayAlpha = m_flOverlayAlpha - ( a_flDeltaTime + a_flDeltaTime );

			if ( m_flOverlayAlpha < 0.0 )
			{
				m_eFlagsRaw &= ~FLAGS_FADING_IN;
				m_flOverlayAlpha = 0.0;
				UpdateScreenOverlay();
				pCurrentState->m_eOverlayColorIndex = 0;

				// The act 3 tier 16 save screen fades in over the sim, which hides its overlay too
				if ( AQuestManager::IsSingletonCreated() && AQuestManager::GetSingleton()->GetAct() == 3 && AQuestManager::GetSingleton()->GetTier() == 16 &&
				     pCurrentState->GetClass()->IsA( &TGetClass( SaveLoadSKU ) ) == TTRUE )
				{
					AGameState* pSimState = FindGameStateOnStack( TPS8( ASimState ) );

					if ( pSimState )
						pSimState->m_eOverlayColorIndex = 0;
				}
			}

			UpdateOverlayAlpha();
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0042a230
void AGameStateController::SaveCurrentGameState( TBOOL a_bUnused, TBOOL a_bUnk, TBOOL a_bDontSave )
{
	if ( a_bDontSave || a_bUnk || AQuestManager::GetSingleton()->GetNumAssignedQuests() == 0 )
	{
		SaveLoadSKU* pSaveState = new SaveLoadSKU( APlayerProgress::GetSingleton(), TFALSE, TNULL );
		pSaveState->StartSave( a_bUnk, a_bDontSave, TTRUE );
		PushState( pSaveState );
	}
	else
	{
		// Quests are assigned, the game can't be saved now
		ASleepCycleChooseState* pChooseState = new ASleepCycleChooseState();
		pChooseState->SetCannotSave( TTRUE );
		PushState( pChooseState );
	}
}

// $Barnyard: FUNCTION 0042a660
void AGameStateController::StartMenuIfPossible()
{
	if ( AFadeManager::IsSingletonCreated() && AFadeManager::GetSingleton()->HasAnyFadesInProgress( TFALSE ) )
		return;

	if ( HASANYFLAG( m_eFlagsRaw, FLAGS_AWAITING_TRANSITION | FLAGS_FADING_IN ) )
		return;

	PushState( new AStartMenuState() );
}

// $Barnyard: FUNCTION 00429c00
// $BarnyardWiiJP: FUNCTION 80130f04
#ifdef BARNYARD_LATEST
void AGameStateController::ShowOverlay( TBOOL a_bShow, TBOOL a_bResetHUD, TFLOAT a_fAlpha, TBOOL a_bCurrentState, const TCHAR* a_szStateClass )
#else
void AGameStateController::ShowOverlay( TBOOL a_bShow, TBOOL a_bResetHUD, TFLOAT a_fAlpha, TBOOL a_bCurrentState )
#endif
{
	if ( a_bResetHUD && GetCurrentState() )
	{
		GetCurrentState()->m_HUDParams.SetFlags( 0 );
		AHudManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	}

	AGameState* pGameState;

	if ( a_bCurrentState )
	{
		pGameState = GetCurrentState();
	}
#ifdef BARNYARD_LATEST
	else if ( a_szStateClass )
	{
		pGameState = FindGameStateOnStack( TPString8( a_szStateClass ) );
	}
#endif
	else
	{
		pGameState = FindGameStateOnStack( TPS8( AAnimalConversationState ) );

		if ( !pGameState )
			pGameState = FindGameStateOnStack( TPS8( ASimState ) );

		if ( !pGameState )
			pGameState = FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) );
	}

	if ( !pGameState )
		return;

	if ( a_bShow )
	{
		pGameState->SetOverlay( AGameState::OVERLAY_3 );

		OverlayData oData;
		oData.uiColorA = TUINT8( a_fAlpha * 255.0f );
		oData.uiColorR = 0;
		oData.uiColorG = 0;
		oData.uiColorB = 0;
		SetOverlayParams( AGameState::OVERLAY_3, oData );
	}
	else
	{
		pGameState->SetOverlay( AGameState::OVERLAY_1 );
	}

	UpdateScreenOverlay();
}

// $Barnyard: FUNCTION 004293d0
TBOOL AGameStateController::ProcessInput( const TInputInterface::InputEvent* a_pEvent )
{
#if defined( BARNYARD_COMMUNITY_PATCH )
	// The mod settings overlay takes the input before the game sees it
	if ( AImGui::IsSingletonCreated() && AImGui::GetSingleton()->ProcessInput( a_pEvent ) )
		return TTRUE;
#endif

	if ( m_eFlags.bIgnoreInput ) return TTRUE;

	if ( m_oStates.Size() <= 1 )
	{
		return TFALSE;
	}

	auto pGameState = *m_oStates.Back();

	if ( ACheatActivator::IsSingletonCreated() )
	{
		ACheatActivator* pCheatActivator = ACheatActivator::GetSingleton();

		if ( pCheatActivator->IsCheatKeyDown() )
		{
			if ( pCheatActivator->ProcessCheatCodeInput( a_pEvent ) )
				return TTRUE;
		}
		else
		{
			pCheatActivator->ResetCheatCodeInput();
		}
	}

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	// NOTE: development: the DebugMenu command opens and closes the debug menu
	if ( ADebugMenuState::ProcessDebugMenuInput( pGameState, a_pEvent ) )
		return TTRUE;
#endif

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: Barnyard route added on top: the TGUI screen of AGUISystem gets the input while it shows a widget
	if ( AGUISystem::IsSingletonCreated() && AGUISystem::GetSingleton()->NotifyTGUIInputEvent( a_pEvent ) )
		return TTRUE;
#endif

	if ( pGameState->SendInputCommands( a_pEvent ) )
	{
		return TTRUE;
	}

	return pGameState->ProcessInput( a_pEvent );
}
