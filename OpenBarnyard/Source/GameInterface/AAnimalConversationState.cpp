#include "pch.h"
#include "AAnimalConversationState.h"
#include "AGameStateController.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Cameras/ASpeechCameraHelper.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/ASpeechDialogManager.h"
#include "Input/AInputMapManager.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "Units/AInteraction.h"
#include "Units/AInteractionComponent.h"
#include "Units/AInteractionManager.h"
#include "Units/ASpeechActivity.h"
#include "Units/AWalkActivity.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "GameInterface/ASimState.h"

#include <Input/TInputDeviceController.h>
#include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0042fda0
TDEFINE_CLASS_NORUNTIME( AAnimalConversationState );

TPSTRING8_DECLARE( player );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 00430cb0
AAnimalConversationState::AAnimalConversationState( AUnit* a_pUnit, AInteractionSequence* a_pSequence )
    : m_pStartSequence( a_pSequence )
    , m_oSequence( TNULL )
    , m_bReady( TFALSE )
    , m_fStartDelay( 0.0f )
    , m_fStartTime( 0.0f )
    , m_bStartPending( TFALSE )
    , m_fCameraYaw( TVector2::VEC_ZERO.x )
    , m_fCameraPitch( TVector2::VEC_ZERO.y )
    , m_iNumChoices( 0 )
    , m_iPickedChoice( -1 )
    , m_fInputDelay( 0.0f )
{
	m_pUnitProxy = a_pUnit->GetSelfProxy();
	m_strPlayer  = TPString8( "player" );
	m_pUnit      = a_pUnit;

	// Remember the steering mode of the unit to restore it later
	ASteer* pSteer = m_pUnitProxy->GetUnit()->GetSteer();

	if ( pSteer->IsA( &TGetClass( AInteriorSteer ) ) )
		m_eUnitSteerMode = TSTATICCAST( AInteriorSteer, pSteer )->GetMode();
}

// $Barnyard: FUNCTION 00430760
// $Barnyard: FUNCTION 00430ed0
AAnimalConversationState::~AAnimalConversationState()
{
}

// $Barnyard: FUNCTION 0042fdb0
TBOOL AAnimalConversationState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	TINT  iDoodad = a_pInputEvent->GetDoodad();
	TBOOL bResult = TFALSE;

	// Escape and Start do nothing during a conversation
	if ( iDoodad == TInputDeviceKeyboard::KEY_ESCAPE || iDoodad == TInputDeviceController::BUTTON_START )
		bResult = TTRUE;

	if ( iDoodad == TInputDeviceController::STICK_RIGHT )
	{
		// The right stick turns the camera
		m_fCameraYaw -= a_pInputEvent->GetMagnitudeFloat( 0 );
		m_fCameraPitch += a_pInputEvent->GetMagnitudeFloat( 1 );
		return TTRUE;
	}

	if ( !bResult )
		return AGameState::ProcessInput( a_pInputEvent );

	return bResult;
}

// $Barnyard: FUNCTION 00430110
TBOOL AAnimalConversationState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand == AInputCommand_OK )
		SkipPlayerSpeech();
	else if ( a_eInputCommand == AInputCommand_Cancel || a_eInputCommand == AInputCommand_Close || a_eInputCommand == AInputCommand_Back )
		return TTRUE;

	TUINT8 uiDirection = 0;

	switch ( a_eInputCommand )
	{
		case AInputCommand_OK:
		case AInputCommand_Interact:
		case AInputCommand_Shoot:
		{
			ASpeechDialogManager::Choice* pChoice = ASpeechDialogManager::GetSingleton()->GetChoice();

			if ( pChoice->eState == ASpeechDialogManager::Choice::STATE_SHOWN )
			{
				m_iPickedChoice = pChoice->GetSelection();
				ASpeechDialogManager::GetSingleton()->CloseChoice();
			}

			return TFALSE;
		}
		case AInputCommand_Move:
		{
			if ( m_fInputDelay > 0.0f )
				break;

			TFLOAT fMagnitude = a_pInputEvent->GetMagnitudeFloat( 1 );

			if ( fMagnitude < -0.5f )
				uiDirection = 1;
			else if ( fMagnitude > 0.5f )
				uiDirection = 2;

			break;
		}
		case AInputCommand_Up:
		case AInputCommand_Forward:
			if ( m_fInputDelay <= 0.0f )
				uiDirection = 1;

			break;
		case AInputCommand_Down:
		case AInputCommand_Backward:
			if ( m_fInputDelay <= 0.0f )
				uiDirection = 2;

			break;
	}

	if ( uiDirection & 3 )
	{
		// Move the selection of the choice dialog
		ASpeechDialogManager::Choice* pChoice = ASpeechDialogManager::GetSingleton()->GetChoice();

		if ( pChoice->eState == ASpeechDialogManager::Choice::STATE_SHOWN )
		{
			TINT iSelection = pChoice->GetSelection() + ( ( uiDirection & 1 ) ? -1 : 1 );
			TMath::Clip( iSelection, 0, m_iNumChoices - 1 );
			pChoice->SetSelection( iSelection );
			m_fInputDelay = 0.2f;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00430f40
TBOOL AAnimalConversationState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_iPickedChoice >= 0 && ASpeechDialogManager::GetSingleton()->GetChoice()->eState == ASpeechDialogManager::Choice::STATE_NONE )
	{
		// The choice dialog is closed, start the picked interaction
		AAnimalConversationEvent oEvent( AAnimalConversationEvent::TYPE_CHOICE, m_iPickedChoice );
		m_iPickedChoice = -1;
		OnConversationEvent( oEvent );
	}

	if ( m_fInputDelay > 0.0f )
		m_fInputDelay -= a_fDeltaTime;

	if ( !m_bReady )
		return AGameState::OnUpdate( a_fDeltaTime );

	ACameraHelper* pCameraHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	if ( pCameraHelper && pCameraHelper->GetClass() == &TGetClass( AGTAStyleCameraHelper ) )
	{
		// Turn the camera with the right stick
		AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraHelper );
		pGTAHelper->SetYaw( a_fDeltaTime * m_fCameraYaw * -100.0f + pGTAHelper->GetYaw() );
		pGTAHelper->SetPitch( a_fDeltaTime * m_fCameraPitch + pGTAHelper->GetPitch() );

		m_fCameraYaw   = TVector2::VEC_ZERO.x;
		m_fCameraPitch = TVector2::VEC_ZERO.y;
	}

	if ( m_eState == STATE_DONE )
	{
		if ( !m_strMiniGame.IsEmpty() )
		{
			AGameLoader::SetTransitionToMiniGame( m_strMiniGame, TTRUE );
			return TTRUE;
		}

		Remove();
		return TTRUE;
	}

	if ( m_bStartPending == TTRUE )
	{
		m_fStartTime += a_fDeltaTime;

		if ( m_fStartTime >= m_fStartDelay )
		{
			m_bStartPending = TFALSE;
			m_fStartTime    = 0.0f;
			m_oSequence.Validate();
			m_eState = ( AInteractionManager::GetSingleton()->StartInteraction( m_oSequence.pSequence, m_iInteraction ) != TTRUE ) ? STATE_DONE : STATE_PLAYING;
		}
	}

	m_oSequence.Validate();
	AInteractionSequence* pSequence = m_oSequence.pSequence;

	if ( !pSequence || m_eState == STATE_DONE )
	{
		m_eState = STATE_DONE;
		return TTRUE;
	}

	if ( pSequence->GetFlags() & AInteractionSequence::FLAGS_PERSISTENT )
	{
		if ( m_eState == STATE_PLAYING )
		{
			if ( pSequence->IsInteractionOver() )
			{
				PickNextInteraction();
				return AGameState::OnUpdate( a_fDeltaTime );
			}
		}
		else if ( m_eState == STATE_STARTED && pSequence->IsInteractionOver() )
		{
			AInteraction* pInteraction = pSequence->GetInteraction();

			if ( pInteraction && ( pInteraction->GetFlags() & AInteraction::FLAGS_DIALOGUE_NEXT ) )
			{
				PickNextInteraction();
				return AGameState::OnUpdate( a_fDeltaTime );
			}

			m_eState = STATE_CHOOSING;
			FindChoices( m_oSequence );
		}
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004304a0
// $BarnyardWiiJP: FUNCTION 8013ad6c
void AAnimalConversationState::OnInsertion()
{
#ifdef BARNYARD_LATEST
	// Latest version: the player is back at the Pig with the apple pie (see AGameLoader::IsPigApplePieReturn)
	if ( AGameLoader::IsPigApplePieReturn() )
	{
		AGameLoader::SetPigApplePieReturn( TFALSE );
		AGameStateController::GetSingleton()->ShowOverlay( TTRUE, TFALSE, 1.0f, TFALSE, TNULL );

		// NOTE: the Wii doesn't check for ASimState
		if ( AGameState* pSimState = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) )
			pSimState->SetOverlay( AGameState::OVERLAY_1 );
	}
#endif

	AGameState* pParentState = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) );

	if ( pParentState )
	{
		// Hide the target reticles of the milk squirt
		TSTATICCAST( ASimState, pParentState )->GetMilkReticles().SetActive( TFALSE );
	}
	else
	{
		pParentState = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "ABarnExploreMinigameState" ) );
	}

	m_HUDParams.SetFlags( pParentState->GetHUDMask() );
	ACameraManager::GetSingleton()->SetSplitscreenCameraHelper( 1 );
	m_pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSelfProxy();

	if ( m_pStartSequence )
	{
		m_oSequence = m_pStartSequence;
		m_eState    = STATE_STARTED;
	}
	else
	{
		m_eState = STATE_DONE;
	}

	AGameState::OnInsertion();
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( DialogInputMap ) );
}

// $Barnyard: FUNCTION 004308e0
void AAnimalConversationState::OnRemoval()
{
	ACameraHelper* pCameraHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	if ( pCameraHelper && pCameraHelper->IsA( &TGetClass( ASpeechCameraHelper ) ) )
		TSTATICCAST( ASpeechCameraHelper, pCameraHelper )->ShowHiddenUnit();

	m_oActivityListener.Disconnect();

	m_oSequence.Validate();
	AInteractionSequence* pSequence = m_oSequence.pSequence;

	if ( pSequence )
	{
		pSequence->SetFlags( pSequence->GetFlags() & ~AInteractionSequence::FLAGS_PERSISTENT );

		TINT iFinishLatch = AInteractionManager::GetSingleton()->GetFinishLatch();
		pSequence->GetLatches().auiValues[ iFinishLatch >> 5 ] |= 1 << ( iFinishLatch & 31 );

		AUnit* pPlayer = m_pPlayer ? m_pPlayer->GetUnit() : TNULL;

		if ( pPlayer )
			TSTATICCAST( AInteractionComponent, pPlayer->FindComponent( &TGetClass( AInteractionComponent ) ) )->LeaveSequence( TFALSE );

		for ( auto it = pSequence->GetComponents().Begin(); it != pSequence->GetComponents().End(); it++ )
			TSTATICCAST( AInteractionComponent, it->GetOwner()->FindComponent( &TGetClass( AInteractionComponent ) ) )->RecordInteraction();
	}
	else
	{
		AUnit* pUnit = m_pUnitProxy ? m_pUnitProxy->GetUnit() : TNULL;

		if ( pUnit && pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) )
			TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) )->RecordInteraction();
	}

	m_oSequence = TNULL;
	m_eState    = STATE_DONE;

	AUnit* pPlayer = m_pPlayer ? m_pPlayer->GetUnit() : TNULL;

	if ( pPlayer )
	{
		TSTATICCAST( APlayerSteer, pPlayer->GetSteer() )->SetState( APlayerSteer::STATE_WALKING );

		AUnit* pUnit = m_pUnitProxy ? m_pUnitProxy->GetUnit() : TNULL;

		if ( pUnit && pUnit->GetSteer()->IsA( &TGetClass( AInteriorSteer ) ) )
			TSTATICCAST( AInteriorSteer, pUnit->GetSteer() )->SetMode( AInteriorSteer::MODE( m_eUnitSteerMode ) );
	}

	m_pPlayer = TNULL;
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00430ef0
void AAnimalConversationState::OnActivate()
{
	m_oActivityListener.Connect( this, OnActivityEventCallback );
	m_iNumChoices   = 0;
	m_iPickedChoice = -1;
	m_fInputDelay   = 0.0f;

	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 00430ae0
void AAnimalConversationState::OnDeactivate()
{
	m_oActivityListener.Disconnect();
	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 004306f0
TBOOL AAnimalConversationState::PickNextInteraction()
{
	m_oSequence.Validate();
	AInteractionSequence* pSequence = m_oSequence.pSequence;

	m_iNumChoices   = 0;
	m_iPickedChoice = -1;
	m_fInputDelay   = 0.0f;

	if ( AInteractionManager::GetSingleton()->PickInteraction( pSequence, TPString8() ) == TTRUE )
	{
		m_eState = STATE_STARTED;
		return TTRUE;
	}

	// The conversation is over
	pSequence->SetFlags( pSequence->GetFlags() & ~AInteractionSequence::FLAGS_PERSISTENT );
	m_eState = STATE_STARTED;
	return TFALSE;
}

// Sorts the candidates by their display order and name
// $Barnyard: FUNCTION 00430000
static TINT CompareCandidates( TINT a_iInteraction1, TINT a_iInteraction2 )
{
	AInteraction* pInteraction1 = AInteractionManager::GetSingleton()->GetInteraction( a_iInteraction1 );
	AInteraction* pInteraction2 = AInteractionManager::GetSingleton()->GetInteraction( a_iInteraction2 );

	if ( pInteraction1->GetDisplayOrder() != pInteraction2->GetDisplayOrder() )
		return pInteraction1->GetDisplayOrder() - pInteraction2->GetDisplayOrder();

	return TStringManager::String8Compare( pInteraction1->GetUnk4().GetString(), pInteraction2->GetUnk4().GetString() );
}

// Binary searches the position after the elements equal to a_iValue (a_rFound tells if there are any)
// NOTE: a method of the sorted vector in the original (returns an iterator)
// $Barnyard: FUNCTION 004302f0
static TINT FindCandidateInsertPosition( const TINT* a_pSorted, TINT a_iNumSorted, TINT a_iValue, TBOOL& a_rFound )
{
	TINT iLow  = 0;
	TINT iHigh = a_iNumSorted - 1;

	a_rFound = TFALSE;

	while ( iLow <= iHigh )
	{
		TINT iMid     = ( iLow + iHigh ) / 2;
		TINT iCompare = CompareCandidates( a_iValue, a_pSorted[ iMid ] );

		if ( iCompare < 0 )
		{
			iHigh = iMid - 1;
		}
		else if ( iCompare > 0 )
		{
			iLow = iMid + 1;
		}
		else
		{
			TINT iPos = iMid + 1;

			while ( iPos <= iHigh && CompareCandidates( a_iValue, a_pSorted[ iPos ] ) == 0 )
				iPos++;

			a_rFound = TTRUE;
			return iPos;
		}
	}

	return iLow;
}

// $Barnyard: FUNCTION 00430b20
TBOOL AAnimalConversationState::FindChoices( AInteractionSequenceRef a_oSequence )
{
	a_oSequence.Validate();

	AInteractionManager::Candidates* pCandidates = AInteractionManager::GetSingleton()->FindCandidates( a_oSequence.pSequence, TFALSE, TPString8() );
	TBOOL                            bEmpty      = pCandidates->IsEmpty();

	m_iNumCandidates = 0;

	if ( bEmpty )
	{
		m_eState = STATE_DONE;
		return TFALSE;
	}

	// Pick up to MAX_NUM_CHOICES candidates and sort them
	// NOTE: retail inserts them into a sorted vector (FindCandidateInsertPosition finds the position after the equal ones, SortedArray::Insert inserts)
	TINT aiSorted[ MAX_NUM_CHOICES ];
	TINT iNumSorted = 0;

	for ( TINT i = MAX_NUM_CHOICES; i != 0; i-- )
	{
		if ( !pCandidates->IsEmpty() )
		{
			AInteractionManager::Candidates::Candidate* pCandidate   = pCandidates->Pick();
			TINT                                        iInteraction = pCandidate->iInteraction;
			pCandidates->Remove( pCandidate );

			TBOOL bFound;
			TINT  iLow = FindCandidateInsertPosition( aiSorted, iNumSorted, iInteraction, bFound );

			for ( TINT j = iNumSorted; j > iLow; j-- )
				aiSorted[ j ] = aiSorted[ j - 1 ];

			aiSorted[ iLow ] = iInteraction;
			iNumSorted++;
		}
	}

	// NOTE: the texts would be shown by the choice dialog, retail never uses them
	TWCHAR* awszTexts[ MAX_NUM_CHOICES ];

	for ( TINT i = 0; i < iNumSorted; i++ )
	{
		m_aiCandidates[ i ] = aiSorted[ i ];
		awszTexts[ i ]      = AInteractionManager::GetSingleton()->GetInteraction( aiSorted[ i ] )->GetActivityText();
		m_iNumCandidates++;
	}

	// Start the first one
	m_oSequence.Validate();
	m_iInteraction  = m_aiCandidates[ 0 ];
	m_bStartPending = TTRUE;
	m_fStartTime    = 0.0f;

	return TTRUE;
}

// $Barnyard: FUNCTION 0042ff40
void AAnimalConversationState::OnConversationEvent( const AAnimalConversationEvent& a_rcEvent )
{
	if ( a_rcEvent.GetType() == AAnimalConversationEvent::TYPE_CHOICE )
	{
		TINT iChoice = a_rcEvent.GetChoice();
		m_oSequence.Validate();
		m_iInteraction  = m_aiCandidates[ iChoice ];
		m_bStartPending = TTRUE;
		m_fStartTime    = 0.0f;
	}
}

// $Barnyard: FUNCTION 0042fe40
void AAnimalConversationState::OnActivityEvent( const AInteractionActivityEvent& a_rcEvent )
{
	AInteraction* pInteraction = a_rcEvent.GetSequence()->GetInteraction();

	if ( !pInteraction )
		return;

	// The "player" interaction is over before it got further than its first activity
	if ( a_rcEvent.GetType() == AInteractionActivityEvent::TYPE_SEQUENCE_FINISH && pInteraction->GetTrigger() == TPS8( player ) && a_rcEvent.GetSequence()->GetActivityState().iActivity == 0 )
	{
		Remove();
		return;
	}

	if ( a_rcEvent.GetType() == AInteractionActivityEvent::TYPE_FINISHED && pInteraction->GetTrigger() == TPS8( player ) && a_rcEvent.GetSequence()->GetActivityState().iActivity == 0 )
	{
		// The first activity of the "player" interaction is over (the units walked to each other)
		AInteractionActivity* pActivity = pInteraction->GetActivity( 0 );

		if ( pActivity->IsA( &TGetClass( AWalkActivity ) ) == TTRUE )
		{
			m_fWalkDistance = TSTATICCAST( AWalkActivity, pActivity )->GetMaxRadius() * 2;

			AUnit* pPlayer = m_pPlayer ? m_pPlayer->GetUnit() : TNULL;
			TSTATICCAST( APlayerSteer, pPlayer->GetSteer() )->SetState( APlayerSteer::STATE_STILL );

			AUnit* pUnit = m_pUnitProxy ? m_pUnitProxy->GetUnit() : TNULL;

			if ( pUnit->GetSteer()->IsA( &TGetClass( AInteriorSteer ) ) )
				TSTATICCAST( AInteriorSteer, pUnit->GetSteer() )->SetMode( AInteriorSteer::MODE( 1 ) );
		}

		m_bReady = TTRUE;
	}
}

// $Barnyard: FUNCTION 004302d0
void AAnimalConversationState::OnActivityEventCallback( void* a_pCaller, const AInteractionActivityEvent& a_rcEvent )
{
	TSTATICCAST( AAnimalConversationState, a_pCaller )->OnActivityEvent( a_rcEvent );
}

// $Barnyard: FUNCTION 0042ff80
void AAnimalConversationState::SkipPlayerSpeech()
{
	AUnit*                 pPlayer               = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
	AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, pPlayer->FindComponent( &TGetClass( AInteractionComponent ) ) );
	ASpeechActivity*       pSpeechActivity       = TNULL;

	if ( pInteractionComponent->GetSpeechActivity( &pSpeechActivity ) )
		ASpeechActivity::SkipSpeech( pInteractionComponent->GetSequence() );
}

// $Barnyard: FUNCTION 0042ffd0
TBOOL AAnimalConversationState::CanConverse( AUnit* a_pUnit )
{
	AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	return pInteractionComponent && pInteractionComponent->IsAllowConversation();
}

// $Barnyard: FUNCTION 00430630
AInteractionSequence* AAnimalConversationState::StartConversation( AUnit* a_pUnit )
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( a_pUnit->CanInteract() != TTRUE )
		return TNULL;

	AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( !pInteractionComponent || pInteractionComponent->IsAllowConversation() != TTRUE )
		return TNULL;

	// Stop what the unit is doing
	AInteractionSequence* pSequence = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) )->GetSequence();

	if ( pSequence )
		AInteractionManager::GetSingleton()->EndSequence( pSequence, TFALSE );

	AUnit* apUnits[ 2 ] = { a_pUnit, pPlayer };

	return AInteractionManager::GetSingleton()->AssignInteractionToUnits(
	    apUnits, 2, TPS8( player ), AInteractionManager::ASSIGNFLAGS_FIRST_IS_INITIATOR | AInteractionManager::ASSIGNFLAGS_PERSISTENT | AInteractionManager::ASSIGNFLAGS_FORCED
	);
}
