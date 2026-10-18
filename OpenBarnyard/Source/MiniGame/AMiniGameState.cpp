#include "pch.h"
#include "Helpers/ABikeManager.h"
#include "Sound/APositionalSoundSet.h"
#include "AMiniGameState.h"
#include "Sound/AMusicManager.h"
#include "Helpers/AHighScoreManager.h"
#include "AMiniGameManager.h"
#include "FX/AParticleEffectManager.h"
#include "Input/AInputHandler.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAnticGameOverState.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "HUD/AHudManager.h"
#include "Tasks/ARootTask.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "GUI/AGUI2.h"
#include "Movie/AMoviePlayer.h"
#include "Quests/AQuestManager.h"
#include "Helpers/ASpeechDialogManager.h"
#include "Units/ANPCBehaviourManager.h"
#include "Units/AAnimalPopulationManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Units/ASSeqController.h"
#include "FX/AAniEffectManager.h"
#include "FX/ASwirlTrackHudAniEffect.h"
#include "GUI/AGUI2ModelRendererScreen.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObject.h"
#include "Physics/ACollisionObjectSimple.h"
#include "Locale/ALocaleManager.h"
#include "Render/ADebugOverlay.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AMiniGameState );

TPSTRING8_DECLARE( charity_everet );
TPSTRING8_DECLARE( charity_daisy );
TPSTRING8_DECLARE( idle );

#ifdef BARNYARD_COMMUNITY_PATCH
// NOTE: retail never sets m_bShowPlayerFloats to TRUE (every write stores FALSE), so the
// "P1".."P4"/"Com" markers above the players are dead code on PC. "-playerfloats" turns
// them back on for the minigames that don't explicitly disable them (Statues does).
static TBOOL ShouldShowPlayerFloats()
{
	return TStringManager::String8FindString( GetCommandLineA(), "-playerfloats" ) != TNULL;
}
#endif

// $Barnyard: FUNCTION 004697e0
AMiniGameState::S1::S1()
{
}

// $Barnyard: FUNCTION 0046bed0
AMiniGameState::AMiniGameState()
{
	m_iMusicCue               = -1;
	m_bWon                    = TFALSE;
	m_bShowPlayerFloats       = TFALSE;
	m_bIsActive               = TFALSE;
	m_bStartHudWasHidden      = TFALSE;
	m_bCountUp                = TFALSE;
	m_pCharityEveret          = TNULL;
	m_pCharityDaisy           = TNULL;
	m_pCharityEveretCollision = TNULL;
	m_pCharityDaisyCollision  = TNULL;
	m_bStarted                = TFALSE;
	m_pPlayerAi               = new AMinigamePlayerAi();
	m_bUnk1                   = TFALSE;
	m_fTimeLimit              = 0.0f;
	m_iMusicCue               = -1;

#ifdef BARNYARD_COMMUNITY_PATCH
	m_bShowPlayerFloats = ShouldShowPlayerFloats();
#endif
}

// $Barnyard: FUNCTION 0046c010
AMiniGameState::AMiniGameState( AMinigamePlayerAi* a_pPlayerAi )
{
	m_bShowPlayerFloats       = TFALSE;
	m_bIsActive               = TFALSE;
	m_bStartHudWasHidden      = TFALSE;
	m_bCountUp                = TFALSE;
	m_pCharityEveret          = TNULL;
	m_pCharityDaisy           = TNULL;
	m_pCharityEveretCollision = TNULL;
	m_pCharityDaisyCollision  = TNULL;
	m_bUnk1                   = TFALSE;
	m_pPlayerAi               = a_pPlayerAi;
	m_fTimeLimit              = 0.0f;
	m_iMusicCue               = -1;

#ifdef BARNYARD_COMMUNITY_PATCH
	m_bShowPlayerFloats = ShouldShowPlayerFloats();
#endif
}

// $Barnyard: FUNCTION 0046c430
// $Barnyard: FUNCTION 0046c120
AMiniGameState::~AMiniGameState()
{
	if ( m_pPlayerAi )
		delete m_pPlayerAi;
}

// $Barnyard: FUNCTION 0046b3f0
TBOOL AMiniGameState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand < AInputCommand_Quit || a_eInputCommand > AInputCommand_Start )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( AInputHandler::GetSingleton()->IsValidDevice( a_pInputEvent->GetSource() ) )
	{
		// Open the start menu unless the game is over
		if ( !FindChildState( &TGetClass( AAnticGameOverState ) ) )
			AGameStateController::GetSingleton()->StartMenuIfPossible();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0046bb60
void AMiniGameState::OnStart()
{
	m_bIsActive = TTRUE;
	OnPreStart();
	Pause();

	AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	m_oStartHudElement.Setup( pPlayerManager->GetCurrentPlayerSet()->GetHumanPlayerCount(), pPlayerManager->GetCurrentPlayerIterator() );
	m_bStarted           = TFALSE;
	m_bStartHudWasHidden = TFALSE;

	TINT iNumPlayers = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetTotalPlayerCount();

	if ( m_bShowPlayerFloats == TTRUE )
	{
		for ( TINT i = 0; i < iNumPlayers; i++ )
			m_aPlayerFloats[ i ].Show();

		for ( TINT i = iNumPlayers; i < MAX_PLAYERS; i++ )
			m_aPlayerFloats[ i ].Hide();
	}
}

// $Barnyard: FUNCTION 0046b930
TBOOL AMiniGameState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_oTimeLeftHudElement.IsForceHidden() )
	{
		TFLOAT fTime;

		if ( m_bCountUp == TTRUE )
		{
			fTime = ( m_fElapsedTime < 0.0f ) ? 0.0f : m_fElapsedTime;
		}
		else
		{
			TFLOAT fTimeLeft = m_fTimeLimit - m_fElapsedTime;
			fTime            = ( 0.0f <= fTimeLeft ) ? fTimeLeft : 0.0f;
		}

		m_oTimeLeftHudElement.SetTime( fTime );
	}

	UpdateAlways( a_fDeltaTime );

	if ( !m_oStartHudElement.IsForceHidden() )
	{
		// The start HUD is still visible
		UpdatePreStart( a_fDeltaTime );
		m_bStartHudWasHidden = m_oStartHudElement.IsForceHidden();
		return AGameState::OnUpdate( a_fDeltaTime );
	}

	if ( !m_bStartHudWasHidden )
	{
		// The start HUD has just gone
		TINT iNumPlayers = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetTotalPlayerCount();

		if ( m_bShowPlayerFloats == TTRUE )
		{
			for ( TINT i = 0; i < iNumPlayers; i++ )
				m_aPlayerFloats[ i ].SetEnabled( TTRUE );
		}

		m_bStarted = TTRUE;
		m_GUIElement.Show();
		OnStartHudFinished();
	}

	m_bStartHudWasHidden = m_oStartHudElement.IsForceHidden();

	if ( !ARootTask::GetSingleton()->IsPaused() )
	{
		m_fElapsedTime += a_fDeltaTime;
		m_pPlayerAi->Update( a_fDeltaTime );
		UpdateGame( a_fDeltaTime );
	}

	ADEBUGLINE( LINE_STATE, ADebugOverlay::INFO_COLOUR, "Time = %.3f", m_fElapsedTime );

	if ( IsGameOver() || ( 0.0f < m_fTimeLimit && m_fTimeLimit < m_fElapsedTime ) )
		UpdateGameOver( a_fDeltaTime );

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0046b460
void AMiniGameState::OnInsertion()
{
	m_HUDParams.SetFlags( AHudElement::MASK_MINIGAME_DEFAULT );
	m_oPickUpHudElement.m_bAutoFade = TFALSE;

	AHudManager* pHudManager = AHudManager::GetSingleton();
	pHudManager->AddElement( &m_oStartHudElement, TFALSE, TTRUE, TFALSE );
	pHudManager->AddElement( &m_oTimeLeftHudElement, TFALSE, TTRUE, TFALSE );
	pHudManager->AddElement( &m_oPickUpHudElement, TFALSE, TTRUE, TFALSE );

	m_oTimeLeftHudElement.SetForceHide( TTRUE );
	m_oPickUpHudElement.SetForceHide( TTRUE );
	m_fElapsedTime = 0.0f;
	m_pPlayerAi->Reset();

	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetMiniGamePickups( 0 );

	// Type of the start HUD (countdown variant) of the minigame
	switch ( AMiniGameManager::GetSingleton()->GetNextMiniGame()->GetStartHudKind() )
	{
		case 0:
		case 3:
		case 5:
			m_oStartHudElement.SetType( AStartMiniGameHudElement::TYPE_COUNTDOWN );
			break;
		case 1:
		case 4:
		case 6:
			m_oStartHudElement.SetType( AStartMiniGameHudElement::TYPE_READY_SILENT );
			break;
		case 2:
			m_oStartHudElement.SetType( AStartMiniGameHudElement::TYPE_GO );
			break;
		case 7:
			m_oStartHudElement.SetType( AStartMiniGameHudElement::TYPE_PLAYER_COUNTDOWN );
			break;
		case 8:
			m_oStartHudElement.SetType( AStartMiniGameHudElement::TYPE_PLAYER_READY );
			break;
	}

	APlayerManager::GetSingleton()->ResetPlayerIterator();
	m_bWon = TFALSE;

	if ( m_bShowPlayerFloats == TTRUE )
	{
		APlayerSet* pPlayerSet  = APlayerManager::GetSingleton()->GetCurrentPlayerSet();
		TINT        iNumPlayers = pPlayerSet->GetTotalPlayerCount();

		for ( TINT i = 0; i < iNumPlayers; i++ )
		{
			AUnit* pUnit = pPlayerSet->GetPlayerUnit( i );

			if ( pPlayerSet->IsPlayerSlotUsedByHuman( i ) )
				m_aPlayerFloats[ i ].SetPlayer( i, TTRUE, pUnit );
#ifdef BARNYARD_COMMUNITY_PATCH
			// Retail leaves the computer floats unbound, but OnStart shows them anyway
			// (their Tick would dereference a NULL unit), so bind them as "Com"
			else
				m_aPlayerFloats[ i ].SetPlayer( i, TFALSE, pUnit );
#endif

			m_aPlayerFloats[ i ].Hide();
		}
	}

	AHighScoreManager::GetSingleton()->ClearLastRecord();
	m_bSubmitHighScore = TTRUE;

	// Pause the tasks that shouldn't run during a minigame.
	// NOTE: some of these managers may not exist, so check them before use
	if ( AMoviePlayer::IsSingletonCreated() )
		AMoviePlayer::GetSingleton()->Activate( TFALSE );

	ASpeechDialogManager::GetSingleton()->Activate( TFALSE );

	AQuestManager::GetSingleton()->Activate( TFALSE );

	ANPCBehaviourManager::GetSingleton()->Activate( TFALSE );
	AAnimalPopulationManager::GetSingleton()->Activate( TFALSE );

	if ( ASimAnimModelHelperManager::IsSingletonCreated() )
		ASimAnimModelHelperManager::GetSingleton()->Activate( TFALSE );

	ABikeManager::GetSingleton()->Activate( TFALSE );

	if ( ASSeqController::IsSingletonCreated() )
		ASSeqController::GetSingleton()->Activate( TFALSE );

	if ( !AAniEffectManager::IsSingletonCreated() )
		new AAniEffectManager();

	if ( !AGUI2ModelRendererScreen::GetSingleton() )
		new AGUI2ModelRendererScreen();

	m_oResults.bWon    = TFALSE;
	m_oResults.iResult = -1;

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();

	m_oResults.iMiniGameVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0046b740
void AMiniGameState::OnRemoval()
{
	AMiniGameManager::GetSingleton()->StoreResults( &m_oResults );

	OnDetachHUD();

	if ( m_oResults.bWon == TFALSE )
		APlayerProfileManager::GetSingleton()->GetMainProfile()->SetMiniGamePickups( 0 );

	if ( m_iMusicCue != -1 )
		ASoundManager::GetSingleton()->StopCue( m_iMusicCue );

	// Resume the tasks paused by OnInsertion.
	// NOTE: some of these managers may not exist, so check them before use
	if ( AMoviePlayer::IsSingletonCreated() )
		AMoviePlayer::GetSingleton()->Activate( TTRUE );

	ASpeechDialogManager::GetSingleton()->Activate( TTRUE );

	AQuestManager::GetSingleton()->Activate( TTRUE );

	ANPCBehaviourManager::GetSingleton()->Activate( TTRUE );
	AAnimalPopulationManager::GetSingleton()->Activate( TTRUE );

	if ( ASimAnimModelHelperManager::IsSingletonCreated() )
		ASimAnimModelHelperManager::GetSingleton()->Activate( TTRUE );

	ABikeManager::GetSingleton()->Activate( TTRUE );

	if ( ASSeqController::IsSingletonCreated() )
		ASSeqController::GetSingleton()->Activate( TTRUE );

	if ( AGUI2ModelRendererScreen::GetSingleton() )
		delete AGUI2ModelRendererScreen::GetSingleton();

	if ( AAniEffectManager::IsSingletonCreated() )
		delete AAniEffectManager::GetSingleton();

	AParticleEffectManager::GetSingleton()->DetachAllEffects();

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0046b830
void AMiniGameState::OnActivate()
{
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );

	TUINT16 uiControllerFlags = AGameStateController::GetSingleton()->GetFlags();
	TBOOL   bTransitioning;

	if ( ( uiControllerFlags & ( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_LEAVE_IMMEDIATELY ) ) == 0 && ( uiControllerFlags & AGameStateController::FLAGS_UNK10 ) == 0 )
	{
		bTransitioning = TFALSE;

		if ( m_bStarted == TTRUE )
			m_GUIElement.Show();
		else
			m_GUIElement.Hide();
	}
	else
	{
		m_HUDParams.SetFlags( 0 );
		bTransitioning = TTRUE;
		m_GUIElement.Hide();
	}

	m_oTimeLeftHudElement.Setup( m_fTimeLimit );

	if ( !bTransitioning )
	{
		if ( m_iMusicCue != -1 )
			ASoundManager::GetSingleton()->PauseCue( m_iMusicCue, TFALSE );

		APositionalSoundSet::GetSingleton()->SetPaused( TFALSE );
	}

	if ( m_bIsActive )
	{
		m_bIsActive = TFALSE;
		OnPause();
	}

	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 0046b8f0
void AMiniGameState::OnDeactivate()
{
	m_GUIElement.Hide();

	if ( m_iMusicCue != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_iMusicCue, TTRUE );

	APositionalSoundSet::GetSingleton()->SetPaused( TTRUE );

	AGameState::OnDeactivate();
}

void AMiniGameState::OnObjectCollected( AAnimatableObject* a_pObject )
{
	// DOES_NOTHING
}

// $Barnyard: FUNCTION 0046b3a0
void AMiniGameState::Pause()
{
	m_bIsActive = TFALSE;
	OnPause();
}

// $Barnyard: FUNCTION 0046bae0
void AMiniGameState::OnDetachHUD()
{
	m_oStartHudElement.Detach();
	m_oTimeLeftHudElement.Detach();
	m_oPickUpHudElement.Detach();
}

// $Barnyard: FUNCTION 0046b710
void AMiniGameState::OnStartHudFinished()
{
	AHighScoreManager::GetSingleton()->ClearLastRecord();
	m_GUIElement.Show();
	m_bSubmitHighScore = TTRUE;
}

// $Barnyard: FUNCTION 0046bb20
void AMiniGameState::OnSetupPlayers()
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	TINT            iNumPlayers    = pPlayerManager->GetCurrentPlayerSet()->GetTotalPlayerCount();

	m_oStartHudElement.Setup( iNumPlayers, pPlayerManager->GetNextPlayer() );
	m_bStartHudWasHidden = TFALSE;
	m_bStarted           = TFALSE;
}

// $Barnyard: FUNCTION 0046bc20
void AMiniGameState::CreateCharityActors( TBOOL a_bWithCollision )
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	if ( !m_pCharityEveret )
	{
		m_pCharityEveret = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( charity_everet ) ), TNULL, 0 );
		m_pCharityEveret->PlayAnimation( TPS8( idle ) );

		if ( a_bWithCollision == TTRUE )
		{
			m_pCharityEveretCollision = new ACollisionObjectSimple();
			m_pCharityEveretCollision->CreateActive( TVector4::VEC_ZERO, 0.75f, 2.0f, 0.0f, 0, TNULL );
		}
	}

	if ( !m_pCharityDaisy )
	{
		m_pCharityDaisy = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( charity_daisy ) ), TNULL, 0 );
		m_pCharityDaisy->PlayAnimation( TPS8( idle ) );

		if ( a_bWithCollision == TTRUE )
		{
			m_pCharityDaisyCollision = new ACollisionObjectSimple();
			m_pCharityDaisyCollision->CreateActive( TVector4::VEC_ZERO, 0.5f, 2.0f, 0.0f, 0, TNULL );
		}
	}
}

// $Barnyard: FUNCTION 0046bd80
void AMiniGameState::DestroyCharityActors()
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	if ( m_pCharityEveret )
	{
		pAnimObjManager->DestroyAnimatableObject( m_pCharityEveret );
		m_pCharityEveret = TNULL;
	}

	if ( m_pCharityDaisy )
	{
		pAnimObjManager->DestroyAnimatableObject( m_pCharityDaisy );
		m_pCharityDaisy = TNULL;
	}

	if ( m_pCharityEveretCollision )
	{
		delete m_pCharityEveretCollision;

#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: retail doesn't clear the pointer, so calling this twice deletes the object twice
		m_pCharityEveretCollision = TNULL;
#endif
	}

	if ( m_pCharityDaisyCollision )
	{
		delete m_pCharityDaisyCollision;

#ifdef BARNYARD_COMMUNITY_PATCH
		m_pCharityDaisyCollision = TNULL;
#endif
	}
}

// $Barnyard: FUNCTION 0046b3b0
void AMiniGameState::SetCharityActorsSkeletonUpdating( TBOOL a_bUpdating )
{
	if ( m_pCharityEveret )
		m_pCharityEveret->SetSkeletonUpdating( a_bUpdating, TTRUE );

	if ( m_pCharityDaisy )
		m_pCharityDaisy->SetSkeletonUpdating( a_bUpdating, TTRUE );
}

// $Barnyard: FUNCTION 0046c330
void AMiniGameState::PlaceCharityActors( const Toshi::TVector4& a_rcEveretPos, const Toshi::TVector4& a_rcDaisyPos, const Toshi::TVector4& a_rcEveretDir, Toshi::TVector4 a_vDaisyDir )
{
	if ( m_pCharityEveret )
	{
		AModelInstanceRef pModelInstance = m_pCharityEveret->GetModelInstance();
		pModelInstance->SetTransform( a_rcEveretPos, a_rcEveretDir );

		if ( m_pCharityEveretCollision )
			m_pCharityEveretCollision->UpdateHash( *TREINTERPRETCAST( const TVector3*, &a_rcEveretPos ) );
	}

	if ( m_pCharityDaisy )
	{
		AModelInstanceRef pModelInstance = m_pCharityDaisy->GetModelInstance();
		pModelInstance->SetTransform( a_rcDaisyPos, a_vDaisyDir );

		if ( m_pCharityDaisyCollision )
			m_pCharityDaisyCollision->UpdateHash( *TREINTERPRETCAST( const TVector3*, &a_rcDaisyPos ) );
	}
}

// $Barnyard: FUNCTION 0046c210
void AMiniGameState::StartCollectEffect( AAnimatableObject* a_pObject, TINT a_iCount, TFLOAT a_fSpeed )
{
	AAniEffectManager* pAniEffectManager = AAniEffectManager::GetSingleton();

	if ( pAniEffectManager && pAniEffectManager->ReserveEffects( 2 ) == TTRUE )
	{
		a_pObject->SetOpacity( 1.0f, TTRUE );
		a_pObject->SetAnimControllersDisabled( TTRUE );

		ASwirlTrackHudAniEffect* pSwirlEffect     = pAniEffectManager->CreateEffect<ASwirlTrackHudAniEffect>();
		AInventoryAniEffect*     pInventoryEffect = pAniEffectManager->CreateEffect<AInventoryAniEffect>();
		pInventoryEffect->SetObject( a_pObject, a_iCount, this );

		TQuaternion qRotation = TQuaternion::IDENTITY;
		qRotation.SetFromEulerRollPitchYaw( TMath::PI / 2, 0.0f, 0.0f );

		// Throw the object into the icon of the pick up counter
		TVector2 vScreenPos;
		m_oPickUpHudElement.GetIconScreenPosition( vScreenPos );

		pSwirlEffect->Setup( a_pObject, vScreenPos, qRotation, 12.0f, 15.0f, 0.0f, 0.5f );
		pSwirlEffect->SetSpeed( a_fSpeed );
		pSwirlEffect->SetNextEffect( pInventoryEffect );
		pSwirlEffect->SetActive( TTRUE );

		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );
	}
}

// $Barnyard: FUNCTION 0046dbf0
void AMiniGameState::S1::ComputeTotal( TINT a_iPlayer )
{
	PlayerInfo& rInfo = oPlayerList[ a_iPlayer ];

	rInfo.wszTotalName = ALocaleManager::Interface()->GetString( 0x2c3 );
	rInfo.iTotal       = 0;

	for ( TINT i = 0; i < rInfo.vecRows.Size(); i++ )
		rInfo.iTotal += rInfo.vecRows[ i ].iScore;

	if ( rInfo.iTotal < 0 )
		rInfo.iTotal = 0;
}
