#include "pch.h"
#include "AAnticGameOverState.h"
#include "Sound/APositionalSoundSet.h"
#include "Sound/AMusicManager.h"
#include "SoundBank/music.h"
#include "Helpers/AGameLoader.h"
#include "AGameStateController.h"
#include "AAnticScoreState.h"
#include "AAnticTrophyState.h"
#include "GUI/AFadeManager.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUISystem.h"
#include "Sound/ASoundManager.h"
#include "Tasks/ARootTask.h"
#include "MiniGame/AMiniGameManager.h"
#include "MiniGame/ACowTippingMiniGameState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AAnticGameOverState );

TPSTRING8_DECLARE( DialogInputMap );
TPSTRING8_DECLARE( bkg_End_wood );

// $Barnyard: FUNCTION 00445330
AAnticGameOverState::AAnticGameOverState( AMiniGameState::S1* a_pResults, TBOOL a_bRemoveWhenDone )
    : m_pResults( a_pResults )
    , m_bUnk( TFALSE )
    , m_bRemoveWhenDone( a_bRemoveWhenDone )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail leaves these uninitialised until OnInsertion/OnFadeOver
	m_eState    = STATE_FADING_OUT;
	m_pMiniGame = TNULL;
	m_fVolume   = 1.0f;
#endif
}

// $Barnyard: FUNCTION 00445280
// $Barnyard: FUNCTION 004453a0
AAnticGameOverState::~AAnticGameOverState()
{
}

// $Barnyard: FUNCTION 00445310
TBOOL AAnticGameOverState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	// Swallow START so the start menu can't be opened over the game over screen
	if ( a_eInputCommand == AInputCommand_Start )
		return TTRUE;

	return TFALSE;
}

// $Barnyard: FUNCTION 004453c0
TBOOL AAnticGameOverState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_eState == STATE_FADING_OUT )
	{
		// Fade the sound out together with the screen
		m_fVolume -= a_fDeltaTime + a_fDeltaTime;

		if ( m_fVolume < 0.0f )
			m_fVolume = 0.0f;

		ASoundManager::GetSingleton()->SetGlobalVolume( m_fVolume );
		return AGameState::OnUpdate( a_fDeltaTime );
	}

	if ( m_eState == STATE_FADED_OUT )
	{
		if ( m_pMiniGame )
		{
			m_pMiniGame->GetGUIElement().Hide();

			// The cow tipping showdown (variant 2) continues the minigame
			if ( m_pOwnerState && m_pOwnerState->IsA( &TGetClass( ACowTippingMiniGameState ) ) && AMiniGameManager::GetSingleton()->GetMiniGameVariant() == 2 )
			{
				if ( !m_pResults->bWon && m_pResults->oPlayerList[ 1 ].bUnk1 != TTRUE )
				{
					m_pMiniGame->SetActive( TTRUE );
					m_pMiniGame->OnPreStart();
					m_pMiniGame->PlayLoseSequence();
					m_eState = STATE_SHOW_RESULTS;
					return AGameState::OnUpdate( a_fDeltaTime );
				}

				m_pMiniGame->SetActive( TTRUE );
				m_pMiniGame->OnPreStart();
				m_pMiniGame->PlayWinSequence();
			}
		}

		m_eState = STATE_SHOW_RESULTS;
		return AGameState::OnUpdate( a_fDeltaTime );
	}

	if ( m_eState == STATE_SHOW_RESULTS )
	{
		if ( m_pMiniGame )
			m_pMiniGame->GetGUIElement().Hide();

		m_eState = STATE_RESULTS;

		AGameState* pResultsState = TNULL;

		if ( m_pResults->bFlag5 == TTRUE )
		{
			// The trophy of the story minigames
			pResultsState = new AAnticTrophyState( AAnticTrophyState::GetResult( m_pResults ) );
		}
		else
		{
			pResultsState = new AAnticScoreState( m_pResults );
		}

		// NOTE: AddChildState doesn't accept TNULL, retail would crash here when the allocation failed
		if ( pResultsState )
			AddChildState( pResultsState );

		TBOOL bResult = AGameState::OnUpdate( a_fDeltaTime );

		if ( m_pMiniGame )
			m_pMiniGame->OnResultsShown();

		return bResult;
	}

	// STATE_RESULTS
	if ( m_ChildStates.Size() == 0 )
	{
		// The results state has gone
		if ( m_bRemoveWhenDone == TTRUE )
		{
			Remove();
			return TTRUE;
		}

		// Minigames that own the game over state continue by themselves
		// NOTE: some of these classes may not be reconstructed (linked) yet, so they're looked up by name
		static const TCHAR* s_aContinuingMiniGames[] = {
			"AChickenFireMinigameState",
			"AWhackARaccMiniGameState",
			"ACowTippingMiniGameState",
			"ABarnYardPoolMiniGameState",
			"ACarChaseMiniGameState",
			"AChasingChicksMiniGame",
			"APrecisionSquirt",
		};

		if ( m_pOwnerState )
		{
			for ( TINT i = 0; i < TARRAYSIZE( s_aContinuingMiniGames ); i++ )
			{
				const TClass* pClass = TClass::Find( s_aContinuingMiniGames[ i ] );

				if ( pClass && m_pOwnerState->IsA( pClass ) )
				{
					AMiniGameState* pMiniGame = TSTATICCAST( AMiniGameState, m_pOwnerState );

					// NOTE: deletes this state
					Remove();

					pMiniGame->SetActive( TTRUE );
					pMiniGame->OnPreStart();
					pMiniGame->Pause();
					return TTRUE;
				}
			}
		}

		// Restart the minigame (shows the instruction screen again)
		AGameLoader::RestartMiniGame( TTRUE );
		return TTRUE;
	}

	// Play the results music once the game over jingle has finished
	if ( m_eState != STATE_FADING_OUT )
	{
		AMusicManager* pMusicManager = AMusicManager::GetSingleton();

		if ( pMusicManager->IsMusicOver() == TTRUE && pMusicManager->GetForcedSound() != soundbank::MUSIC_MUD )
		{
			pMusicManager->SetForcedSound( soundbank::MUSIC_MUD );
			pMusicManager->ForceUpdate();
		}
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00445a10
void AAnticGameOverState::OnInsertion()
{
	AGameStateController::ms_bPausedByMenu = TTRUE;
	m_HUDParams.SetFlags( 0 );
	m_eState = STATE_FADING_OUT;

	AFade* pFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 0.5f );

	m_oFadeOverListener.Disconnect();
	m_oFadeOverListener.Connect(
	    pFade->GetFadeOverEmitter(),
	    this,
	    []( AAnticGameOverState* a_pState, AFade* a_pFade, TBOOL* a_pThrowFadeOver ) {
		    return a_pState->OnFadeOver( a_pFade, a_pThrowFadeOver );
	    },
	    0
	);

	m_fVolume = 1.0f;
	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 004457a0
void AAnticGameOverState::OnRemoval()
{
	AGameStateController::ms_bPausedByMenu = TFALSE;

	if ( m_pMiniGame )
	{
		m_pMiniGame->GetGUIElement().Show();
		m_pMiniGame->GetHUDParams().SetFlags( AHudElement::MASK_MINIGAME_DEFAULT );
	}

	AGUISystem::GetSingleton()->SetPicture( TPString8() );
	AGUISystem::GetSingleton()->ResetPicture();
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );

	// Stop the results music
	AMusicManager::GetSingleton()->ClearForcedSound( TFALSE );

	// Resume the minigame sounds
	APositionalSoundSet::GetSingleton()->SetPaused( TFALSE );
	ASoundManager::GetSingleton()->RestoreAllCuesVolume();

	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 004459f0
// $Barnyard: FUNCTION 00445880
TBOOL AAnticGameOverState::OnFadeOver( AFade* a_pFade, TBOOL* a_pThrowFadeOver )
{
	if ( *a_pThrowFadeOver )
		return TFALSE;

	// Pause the minigame sounds
	APositionalSoundSet::GetSingleton()->SetPaused( TTRUE );

	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );
	m_oFadeOverListener.Disconnect();
	SetInputMap( TPS8( DialogInputMap ) );
	m_eState = STATE_FADED_OUT;

	m_pMiniGame = TNULL;

	// Find the minigame state on the stack
	AMiniGameManager* pMiniGameManager = AMiniGameManager::GetSingleton();
	TPString8         strClassName;
	pMiniGameManager->GetStateClassName( strClassName, pMiniGameManager->GetNextMiniGameIndex() );
	m_pMiniGame = TSTATICCAST( AMiniGameState, AGameStateController::GetSingleton()->FindGameStateOnStack( strClassName ) );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail doesn't check it
	if ( m_pMiniGame )
#endif
		m_pMiniGame->GetHUDParams().SetFlags( 0 );

	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );

	// The game over jingle
	AMusicManager::GetSingleton()->ForceUpdate();
	AMusicManager::GetSingleton()->SetForcedSound( ( m_pResults->bWon == TTRUE ) ? soundbank::MUSIC_MISSION_COMPLETE : soundbank::MUSIC_MISSION_FAILURE );

	AGUISystem::GetSingleton()->SetPicture( TPS8( bkg_End_wood ) );
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );

	return TTRUE;
}
