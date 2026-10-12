#include "pch.h"
#include "APlayerControlComponent.h"
#include "MiniGame/ACoinPickupMicroGame.h"
#include "MiniGame/AMiniGameManager.h"
#include "MiniGame/AShakeTreeMicroGame.h"
#include <Toshi/TScheduler.h>
#include "AUnitPlayer.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInventoryState2.h"
#include "AInteractionComponent.h"
#include "AInteractionManager.h"
#include "AInteractionSequence.h"
#include "AInteraction.h"
#include "ASpeechActivity.h"
#include "AObjectTraits.h"
#include "AUnitType.h"
#include "Player/APlayerManager.h"
#include "Helpers/AGameLoader.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "HUD/AHudManager.h"
#include "HUD/AInteractionHudElement.h"
#include "Helpers/AHintManager.h"
#include "ACharacterUnit.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "AQuestActivity.h"
#include "AInventoryGroupComponent.h"
#include "AInventoryItem.h"
#include "APlayerInventoryCallbacks.h"
#include "ACarryableUnit.h"
#include "ACarryableUnitHelper.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/ACharacterAnimController.h"
#include "Animation/AAlienAnimController.h"
#include "AUnitBehaviour.h"
#include "AUnitManager.h"
#include "AUnitNameManager.h"
#include "Projectiles/AAimedSquirtProjectileLauncher.h"
#include "GameInterface/AAnimalConversationState.h"
#include "Helpers/AConversationManager.h"
#include "Quests/AQuestManager.h"
#include "AUnitFacialComponent.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Helpers/ATreeManager.h"
#include "Helpers/ATreasureManager.h"
#include "Helpers/ARegrowthManager.h"
#include "Helpers/AGateManager.h"
#include "Units/AUnitFactory.h"
#include "Units/ACharacterUnitCarrable.h"
#include "UnitMovement/Steering/ABallSteer2.h"
#include "FX/AAniEffectManager.h"
#include "FX/AWaitAniEffect.h"
#include "FX/AFadeAniEffect.h"
#include "FX/ADieAniEffect.h"
#include "ARandom.h"
#include "GameInterface/ASimState.h"
#include "Helpers/ALadderBeansGame.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( bikekey );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( AInventoryState2 );
TPSTRING8_DECLARE( trigger_hint );
TPSTRING8_DECLARE( bike );
TPSTRING8_DECLARE( fixpipe );
TPSTRING8_DECLARE( putaway );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( Equip );
TPSTRING8_DECLARE( sunglasses );
TPSTRING8_DECLARE( Hand );
TPSTRING8_DECLARE( BackPack );
TPSTRING8_DECLARE( FlowerPack );
TPSTRING8_DECLARE( Knapsack );
TPSTRING8_DECLARE( nb_bull );
TPSTRING8_DECLARE( milkweed_seed );
TPSTRING8_DECLARE( milkweed );
TPSTRING8_DECLARE( goldrush_token );
TPSTRING8_DECLARE( playerinstance );
TPSTRING8_DECLARE( trigger_gate );

// NOTE: CreateObject 00591660 / CreateObjectInPlace 005916c0
// NOTE: GetClass 0058e500, CreateObject 00591660, CreateObjectInPlace 005916c0
TDEFINE_CLASS( APlayerControlComponent );

// NOTE: inlined into the constructors of the derived classes
AUnitActionReceiver::AUnitActionReceiver()
{
	// Add to the tail of the global animation event list
	AAnimationBreakpointUnitAction::Listener& rHead = AAnimationBreakpointUnitAction::GetListeners();

	m_oListener.pCaller   = this;
	m_oListener.pCallback = UnitActionCallback;
	m_oListener.pNext     = &rHead;
	m_oListener.pPrev     = rHead.pPrev;
	rHead.pPrev->pNext    = &m_oListener;
	rHead.pPrev           = &m_oListener;
}

// $Barnyard: FUNCTION 004ca690
// $Barnyard: FUNCTION 004ca6f0
AUnitActionReceiver::~AUnitActionReceiver()
{
	if ( m_oListener.pNext != &m_oListener )
	{
		m_oListener.pPrev->pNext = m_oListener.pNext;
		m_oListener.pNext->pPrev = m_oListener.pPrev;
		m_oListener.pNext        = &m_oListener;
		m_oListener.pPrev        = &m_oListener;
	}

	m_oListener.pCaller   = TNULL;
	m_oListener.pCallback = TNULL;
}

// $Barnyard: FUNCTION 00539780
void __stdcall AUnitActionReceiver::UnitActionCallback( void* a_pCaller, AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	TSTATICCAST( AUnitActionReceiver, a_pCaller )->OnUnitAction( a_pEvent );
}

// $Barnyard: FUNCTION 005913d0
APlayerControlComponent::APlayerControlComponent()
    : m_pAutoArea( TNULL )
    , m_pArea( TNULL )
    , m_pContextArea( TNULL )
    , m_pTargetUnit( TNULL )
    , m_pConversationUnit( TNULL )
    , m_fContextTime( 0.0f )
    , m_fTargetDistanceSq( 0.0f )
    , m_pLeak( TNULL )
    , m_pInteraction( TNULL )
    , m_fBushTime( 5.0f )
    , m_bContextEnabled( TTRUE )
    , m_bMoved( TTRUE )
    , m_bAreaInteraction( TFALSE )
    , m_bTargetCanTalk( TFALSE )
    , m_wszOverrideText( TNULL )
    , m_wszContextText( TNULL )
    , m_vLastPosition( TVector4::VEC_ZERO )
    , m_bLookAt( TTRUE )
    , m_bInventoryDisabled( TFALSE )
    , m_bUnitInteractions( TTRUE )
    , m_fUseItemCooldown( 1.0f )
{
	m_pInventoryCallbacks = APlayerInventoryCallbacks::Create();
}

// $Barnyard: FUNCTION 00591500
// $Barnyard: FUNCTION 00591700
// $Barnyard: FUNCTION 005914f0
APlayerControlComponent::~APlayerControlComponent()
{
	if ( m_pInventoryCallbacks )
		delete m_pInventoryCallbacks;
}

// $Barnyard: FUNCTION 00591720
void APlayerControlComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	TBOOL bPaused = TTRUE;

	if ( !( AGameStateController::GetSingleton()->GetFlags() & ( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_LEAVE_IMMEDIATELY ) ) && !( AGameStateController::GetSingleton()->GetFlags() & AGameStateController::FLAGS_UNK10 ) )
	{
		// The player can't move once a minigame is about to start
		if ( AMiniGameManager::GetSingleton()->GetNextMiniGameName().IsEmpty() )
			bPaused = TFALSE;
	}

	if ( bPaused )
		return;

	m_fUseItemCooldown -= a_fDeltaTime;

	if ( m_fUseItemCooldown < 0.0f )
		m_fUseItemCooldown = 0.0f;

	AUnit* pOwner = GetOwner();

	if ( pOwner )
		UpdateInventoryGroups( pOwner );

	if ( IsActionAllowed( pOwner ) )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		if ( ms_bDevStick )
		{
			// Dev: feed the fake left stick like AInputCommand_Move events would
			TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_MOVED, ms_vDevStick.x, ms_vDevStick.y );
			ASimplePlayerControlComponent::ProcessCommand( AInputCommand_Move, &oEvent );
		}
#endif

		ASimplePlayerControlComponent::UpdateAI( a_fDeltaTime );
	}

	TVector4 vPosition = *pOwner->GetSteer()->GetPosition();
	m_bMoved           = a_fDeltaTime * 0.0000010000001f < TVector4::DistanceSq( vPosition, m_vLastPosition );
	m_vLastPosition    = vPosition;

	UpdateCurrentArea( a_fDeltaTime );

	if ( GetOwner()->FindComponent( &TGetClass( AInteractionComponent ) ) )
		UpdateContextTimer( a_fDeltaTime );

#ifdef BARNYARD_COMMUNITY_PATCH
	if ( ms_bDevInteract )
	{
		// Dev: press the action button like AInputCommand_Interact would
		ms_bDevInteract = TFALSE;
		TINFO( "[devtalk] interact, target %s, conversation unit %s, context text set: %d\n", m_pTargetUnit ? m_pTargetUnit->GetType()->GetName().GetString() : "none", m_pConversationUnit ? m_pConversationUnit->GetType()->GetName().GetString() : "none", m_wszContextText != TNULL );

		TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
		ProcessCommand( AInputCommand_Interact, &oEvent );
	}
#endif

	UpdateBushes( a_fDeltaTime );
	UpdateLookAt( pOwner );
}

// $Barnyard: FUNCTION 0058ff80
TBOOL APlayerControlComponent::ProcessCommand( AInputCommand a_eCommand, const TInputInterface::InputEvent* a_pEvent )
{
	AUnit* pOwner = GetOwner();

	if ( !pOwner )
		return TFALSE;

	if ( !GetOwner()->GetSteer()->CanStartAction() )
		return TFALSE;

	// No commands while the time step of the scheduler is multiplied: ASleepCycleState fast-forwards the night with it
	// (TScheduler::SetDebugDeltaTimeMult, 0x6bca00), the player can't act while sleeping
	if ( g_oSystemManager.GetScheduler()->IsDebugDeltaTimeMultUsed() == TTRUE )
		return TFALSE;

	switch ( a_eCommand )
	{
		case AInputCommand_Left:
		case AInputCommand_Right:
		case AInputCommand_Forward:
		case AInputCommand_Backward:
		case AInputCommand_Jump:
			if ( !IsActionAllowed( GetOwner() ) )
				return TFALSE;

			break;
		case AInputCommand_SuperSpeed:
			// Can't do it while carrying something heavy or the detector
			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				AUnit* pCarried = TSTATICCAST( AAnimatedUnit, pOwner )->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

				if ( pCarried )
				{
					AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pCarried->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

					if ( pTraits && ( pTraits->GetFlags() & ( AObjectTraits::FLAGS_HEAVY | AObjectTraits::FLAGS_DETECTOR ) ) )
						return TTRUE;
				}
			}

			break;
		case AInputCommand_Shoot:
			if ( !IsActionAllowed( GetOwner() ) )
				return TTRUE;

			if ( !CanUseHand() )
				return TTRUE;

			break;
		case AInputCommand_ShootAlt:
			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN && IsInteracting() )
				return TTRUE;

			break;
		case AInputCommand_Switch:
		{
			ASteer* pSteer = pOwner->GetSteer();

			if ( ( pSteer->IsA( &TGetClass( APlayerSteer ) ) && TSTATICCAST( APlayerSteer, pSteer )->GetState() == APlayerSteer::STATE_CLIMBING ) || !CanUseHand() )
				return TTRUE;

			if ( !IsActionAllowed( GetOwner() ) )
				return TFALSE;

			break;
		}
		default:
			break;
	}

	if ( ASimplePlayerControlComponent::ProcessCommand( a_eCommand, a_pEvent ) )
		return TTRUE;

	if ( a_eCommand == AInputCommand_Interact )
	{
		// The context action
		AUnit* pCarried = TSTATICCAST( AAnimatedUnit, pOwner )->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

		if ( !AGameLoader::IsBarnExploreMiniGame() )
		{
			AProjectileLauncher* pLauncher = TSTATICCAST( AUnitPlayer, pOwner )->GetActiveLauncher();

			if ( pLauncher && pLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
				return TTRUE;
		}

		if ( IsInteracting() == TTRUE )
		{
			// Skip the speech the player listens to
			AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, GetOwner()->FindComponent( &TGetClass( AInteractionComponent ) ) );
			ASpeechActivity*       pSpeechActivity       = TNULL;

			if ( pInteractionComponent->GetSpeechActivity( &pSpeechActivity ) )
				ASpeechActivity::SkipSpeech( pInteractionComponent->GetSequence() );

			return TTRUE;
		}

		// Talk to the unit
		AInteractionSequence* pConversation = FindUnitInteraction( m_pTargetUnit, TFALSE );

		if ( pConversation )
		{
			AGameStateController::GetSingleton()->PushState( new AAnimalConversationState( m_pTargetUnit, pConversation ) );
			return TTRUE;
		}

		if ( DropHeavyItem() == TTRUE )
			return TTRUE;

		if ( StartUnitInteraction( m_pTargetUnit, "playerobject" ) )
			return TTRUE;

		if ( FixLeak() == TTRUE )
		{
			TSTATICCAST( AAnimatedUnit, GetOwner() )->GetAnimatableObject()->PlayAnimation( TPS8( fixpipe ) );
			return TTRUE;
		}

		if ( GetMilkingInteraction() || GetPresentInteraction() )
			return TTRUE;

		if ( GetGateInteraction() && !pCarried )
			return TTRUE;

		if ( GetRegrowthInteraction() || SelectConversationTarget() == TTRUE )
			return TTRUE;

		if ( m_bAreaInteraction == TTRUE && GetAreaInteraction() && !pCarried )
		{
			m_pContextArea     = TNULL;
			m_bAreaInteraction = TFALSE;
			return TTRUE;
		}

		return TTRUE;
	}

	if ( a_eCommand == AInputCommand_Inventory )
	{
		if ( !AGameLoader::IsBarnExploreMiniGame() && IsActionAllowed( GetOwner() ) && IsInteractionAllowed( GetOwner() ) && !m_bInventoryDisabled )
		{
			AAnimatableObject*        pAnimatableObject = TSTATICCAST( AAnimatedUnit, pOwner )->GetAnimatableObject();
			AProjectileLauncher*      pLauncher         = TSTATICCAST( AUnitPlayer, pOwner )->GetActiveLauncher();
			AInventoryGroupComponent* pInventory        = TNULL;

			if ( IsCarryingTool() && !pAnimatableObject->IsAnimationPlanned( TPS8( putaway ) ) && !pAnimatableObject->IsAnimPlaying( TPS8( putaway ), 0.0f ) &&
			     ( pInventory = TSTATICCAST( AInventoryGroupComponent, pOwner->FindComponent( &TGetClass( AInventoryGroupComponent ) ) ),
			       pInventory->GetCategory( TPString8( "Hand" ) )->GetNumItems() == 0 ) )
			{
				// Take the sunglasses off
				if ( !pLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) && pOwner->GetSteer()->IsA( &TGetClass( APlayerSteer ) ) )
				{
					APlayerSteer::STATE eState = TSTATICCAST( APlayerSteer, pOwner->GetSteer() )->GetState();

					if ( eState == APlayerSteer::STATE_STILL || eState == APlayerSteer::STATE_WALKING )
					{
						AInventoryGroupComponent* pGroup     = TSTATICCAST( AInventoryGroupComponent, GetOwner()->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
						AInventoryGroupComponent* pPlayerInv = TSTATICCAST( AInventoryGroupComponent, AInventoryComponentInterface::GetInventory( GetOwner() ) );

						pGroup->SwapItemCategory( pPlayerInv->GetCategory( TPS8( Equip ) )->FindItemByType( TPS8( sunglasses ) ), 1 );
						return TTRUE;
					}
				}
			}
			else if ( !pLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) && m_fUseItemCooldown <= 0.0f )
			{
				UseHandItem();
			}
		}

		return TTRUE;
	}

	if ( a_eCommand == 79 )
		return UseLauncher();

	return TFALSE;
}

// $Barnyard: FUNCTION 0058e560
void APlayerControlComponent::OnEnable()
{
	if ( !( AGameStateController::GetSingleton()->GetFlags() & ( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_LEAVE_IMMEDIATELY ) ) )
	{
		ASimplePlayerControlComponent::OnEnable();

		if ( m_bContextEnabled )
		{
			AInteractionHudElement* pInteractionHud = AHudManager::GetSingleton()->FindElement<AInteractionHudElement>();
			pInteractionHud->SetText( TNULL, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
			pInteractionHud->SetForceHide( TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 0058e8b0
void APlayerControlComponent::OnDisable()
{
	ASimplePlayerControlComponent::OnDisable();

	if ( !( GetOwner()->GetFlags() & AUnit::FLAGS_DYING ) )
	{
		AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, GetOwner()->FindComponent( &TGetClass( AUnitFacialComponent ) ) );

		if ( pFacial )
			pFacial->StopLook();
	}

	AInteractionHudElement* pInteractionHud = AHudManager::GetSingleton()->FindElement<AInteractionHudElement>();
	pInteractionHud->SetText( TNULL, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
	pInteractionHud->SetForceHide( TTRUE );
}

// $Barnyard: FUNCTION 00591370
void APlayerControlComponent::OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	TRef<AUnitProxy> pProxy     = a_pEvent->GetUnitProxy();
	AUnit*           pEventUnit = pProxy.Get() ? pProxy.Get()->GetUnit() : TNULL;

	if ( pEventUnit == GetOwner() && a_pEvent->eAction == AAnimationBreakpointUnitAction::ACTION_PUTDOWN )
		PutDownHandItem( a_pEvent );
}

// $Barnyard: FUNCTION 0058e510
TBOOL APlayerControlComponent::IsCarryingTool()
{
	// Whether the owner (an AUnitPlayer) wears the sunglasses
	AUnit* pOwner = GetOwner();

	if ( pOwner->IsA( &TGetClass( AUnitPlayer ) ) )
	{
		AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, AInventoryComponentInterface::GetInventory( pOwner ) );

		if ( pInventory )
		{
			AInventoryComponent* pEquip = pInventory->GetCategory( TPS8( Equip ) );

			if ( pEquip && pEquip->FindItemByType( TPS8( sunglasses ) ) )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0058e5c0
void APlayerControlComponent::UpdateInventoryGroups( AUnit* a_pOwner )
{
	// Makes the inventory groups of the owner use m_pInventoryCallbacks
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, a_pOwner->FindComponentExact( &TGetClass( AInventoryGroupComponent ) ) );

	if ( pInventory )
	{
		pInventory->GetCategory( TPS8( Hand ) )->SetCallbacks( m_pInventoryCallbacks );
		pInventory->GetCategory( TPS8( Equip ) )->SetCallbacks( m_pInventoryCallbacks );
		pInventory->GetCategory( TPS8( BackPack ) )->SetCallbacks( m_pInventoryCallbacks );
		pInventory->GetCategory( TPS8( Knapsack ) )->SetCallbacks( m_pInventoryCallbacks );
		pInventory->GetCategory( TPS8( FlowerPack ) )->SetCallbacks( m_pInventoryCallbacks );
	}
}

// $Barnyard: FUNCTION 0058e660
TBOOL APlayerControlComponent::IsInteractionAllowed( AUnit* a_pOwner )
{
	// TTRUE unless the owner's AInteractionComponent is busy
	AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, a_pOwner->FindComponent( &TGetClass( AInteractionComponent ) ) );

	return !pInteractionComponent || !pInteractionComponent->IsInSequence();
}

// $Barnyard: FUNCTION 0058e690
AInteractionSequence* APlayerControlComponent::FindUnitInteraction( AUnit* a_pUnit, TBOOL a_bAnyState )
{
	if ( !m_bUnitInteractions )
		return TNULL;

	if ( a_pUnit && a_pUnit->CanInteract() && AAnimalConversationState::CanConverse( a_pUnit ) )
	{
		AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, a_pUnit->FindComponent( &TGetClass( AUnitBehaviour ) ) );

		if ( !pBehaviour || pBehaviour->IsBehaviourStopped() != TTRUE )
		{
			AUnit*                 pOwner                = GetOwner();
			AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, pOwner->FindComponent( &TGetClass( AInteractionComponent ) ) );

			if ( !pOwner->HasInteraction() && !pInteractionComponent->IsInteractionsDisabled() )
			{
				// Only while walking around
				APlayerSteer*  pSteer = TSTATICCAST( APlayerSteer, pOwner->GetSteer() );
				AInteriorArea* pArea  = pSteer->GetCurrentArea();

				if ( a_bAnyState || ( pArea && pSteer->GetState() == APlayerSteer::STATE_WALKING ) )
					return AAnimalConversationState::StartConversation( a_pUnit );
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0058e750
TBOOL APlayerControlComponent::IsInteracting()
{
	// Whether the owner's AInteractionComponent runs an interaction
	AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, GetOwner()->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pInteractionComponent )
		return pInteractionComponent->IsInSequence();

	return TFALSE;
}

// $Barnyard: FUNCTION 0058e770
AInteractionSequence* APlayerControlComponent::GetConversationSequence()
{
	// The sequence m_pConversationUnit talks in (ASpeechActivity)
	AUnit* pUnit = m_pConversationUnit;

	if ( pUnit && !pUnit->CanInteract() )
	{
		AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pInteractionComponent )
		{
			AInteractionSequence* pSequence = pInteractionComponent->GetSequence();

			if ( pSequence && pSequence->GetCurrentActivity()->IsA( &TGetClass( ASpeechActivity ) ) )
				return pSequence;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0058e7c0
TBOOL APlayerControlComponent::SelectConversationTarget()
{
	AInteractionSequence* pSequence = GetConversationSequence();

	if ( !pSequence )
		return TFALSE;

	AConversationManager::GetSingleton()->SetTargetSequence( pSequence->GetId() );
	return TTRUE;
}

// $Barnyard: FUNCTION 0058e7e0
TBOOL APlayerControlComponent::UseLauncher()
{
	// Uses the item the walking player carries in the hand
	AUnit* pPlayerUnit = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pPlayerUnit->GetSteer()->IsA( &TGetClass( APlayerSteer ) ) )
		return TFALSE;

	// NOTE: retail checks the steer class of the first player but reads the state of the owner's steer
	if ( TSTATICCAST( APlayerSteer, GetOwner()->GetSteer() )->GetState() != APlayerSteer::STATE_WALKING )
		return TFALSE;

	AUnit* pCarried = TSTATICCAST( AAnimatedUnit, pPlayerUnit )->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

	if ( !pCarried || !pCarried->IsA( &TGetClass( ACarryableUnit ) ) )
		return TFALSE;

	return pCarried->GetCarryableHelper()->Use();
}

// $Barnyard: FUNCTION 0058e860
TBOOL APlayerControlComponent::CanUseHand()
{
	if ( AGameLoader::IsBarnExploreMiniGame() )
		return TFALSE;

	if ( AGameLoader::IsMainGameLoaded() )
	{
		// Only the first player in the sim
		AUnit* pPlayerUnit = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		if ( GetOwner() == pPlayerUnit )
			return IsCarryingTool();

		return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0058e910
TBOOL APlayerControlComponent::IsActionAllowed( AUnit* a_pOwner )
{
	// TTRUE unless the current interaction pauses the player
	AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, a_pOwner->FindComponent( &TGetClass( AInteractionComponent ) ) );

	return !pInteractionComponent ||
	    !pInteractionComponent->IsInSequence() ||
	    !( pInteractionComponent->GetSequence()->GetInteraction()->GetFlags() & AInteraction::FLAGS_PAUSE_PLAYER_STEERING );
}

// $Barnyard: FUNCTION 0058e950
void APlayerControlComponent::UpdateLookAt( AUnit* a_pOwner )
{
	if ( !m_bLookAt )
		return;

	AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, a_pOwner->FindComponent( &TGetClass( AUnitFacialComponent ) ) );

	if ( !pFacial || AQuestManager::GetSingleton()->IsTimerPaused() )
		return;

	ASteer* pSteer = a_pOwner->GetSteer();

	if ( pSteer->IsInteracting() || ( pSteer->IsA( &TGetClass( APlayerSteer ) ) && TSTATICCAST( APlayerSteer, pSteer )->GetState() != APlayerSteer::STATE_WALKING ) )
	{
		pFacial->StopLook();
		return;
	}

	ACameraHelper* pCameraHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	if ( pCameraHelper != ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) )
		return;

	AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraHelper );

	if ( pGTAHelper->GetIdleTime() < 1.0f )
	{
		// The player turns the camera, look where it looks
		TVector4 vecTarget = ACameraManager::GetSingleton()->GetCurrentCamera()->GetOrientation();

		const TVector4* pPos = pSteer->GetPosition();
		vecTarget.x += pPos->x;
		vecTarget.y += pPos->y;
		vecTarget.z += pPos->z;
		vecTarget.y -= pGTAHelper->GetHeightOffset();

		pFacial->LookAtPosition( vecTarget, 7.0f );
		return;
	}

	if ( GetIdleTime() <= 8.0f )
	{
		pFacial->StopLook();
		return;
	}

	// Idle for a while, look at the units around
	ASteer* pOwnerSteer = GetOwner()->GetSteer();

	if ( pOwnerSteer && TSTATICCAST( APlayerSteer, pOwnerSteer )->GetState() == APlayerSteer::STATE_CLIMBING )
		return;

	if ( !pFacial->IsLooking() )
		pFacial->SetLookTargetRandom( -1.0f );
}

// $Barnyard: FUNCTION 0058eac0
TBOOL APlayerControlComponent::ReleaseInteraction( AInteractionSequence** a_ppSequence )
{
	// Clears *a_ppSequence once it's finished, TFALSE while it still runs
	AInteractionSequence* pSequence = *a_ppSequence;

	if ( !pSequence )
		return TTRUE;

	if ( pSequence->GetFlags() & AInteractionSequence::FLAGS_ENDED )
	{
		AInteractionManager::GetSingleton()->EndSequence( pSequence, TFALSE );
		*a_ppSequence = TNULL;
		return TTRUE;
	}

	if ( !pSequence->GetInteraction() || ( pSequence->GetFlags() & AInteractionSequence::FLAGS_INTERACTION_DONE ) )
	{
		*a_ppSequence = TNULL;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0058eb10
TBOOL APlayerControlComponent::ReleaseInteraction()
{
	AInteractionSequence* pSequence = m_pInteraction;

	if ( !pSequence )
		return TTRUE;

	if ( pSequence->GetFlags() & AInteractionSequence::FLAGS_ENDED )
	{
		AInteractionManager::GetSingleton()->EndSequence( pSequence, TFALSE );
		m_pInteraction = TNULL;
		return TTRUE;
	}

	if ( !pSequence->GetInteraction() || ( pSequence->GetFlags() & AInteractionSequence::FLAGS_INTERACTION_DONE ) )
	{
		m_pInteraction = TNULL;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0058eb60
void APlayerControlComponent::UpdateBushes( TFLOAT a_fDeltaTime )
{
	m_fBushTime -= a_fDeltaTime;

	if ( !m_bMoved || m_fBushTime >= 0.0f )
		return;

	// Picks up the items around the player that are picked up automatically
	AUnit*                  pPlayer = GetOwner();
	TINT                    iNumUnits;
	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( *pPlayer->GetSteer()->GetPosition(), 1.7f, &iNumUnits, TFALSE, 420, TFALSE, TNULL );

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit->IsDying() )
			continue;

		// Not while it's flying
		ASteer* pSteer = pUnit->GetSteer();

		if ( pSteer && pSteer->GetClass()->IsA( &TGetClass( AThrowItemSteer ) ) && !TSTATICCAST( AThrowItemSteer, pSteer )->IsStopped() )
			continue;

		// Not while it's busy
		if ( pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) &&
		     TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) )->GetSequence() )
			continue;

		AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pUnit->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

		if ( !pTraits || !pTraits->IsAutoPickup() )
			continue;

		TBOOL bPickUp = TTRUE;

		if ( pUnit->GetCarryableHelper() )
		{
			bPickUp = pUnit->GetCarryableHelper()->IsDelayOver();

			if ( !bPickUp )
				continue;
		}

		if ( pPlayer->GetSteer()->GetClass()->IsA( &TGetClass( APlayerSteer ) ) )
			bPickUp = pPlayer->GetSteer()->CanStartAction();

		if ( bPickUp )
		{
			AInventoryGroupComponent::CollectUnit( pUnit, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );
			m_fBushTime = 0.25f;
		}
	}

	// NOTE: retail always overwrites the 0.25s above
	m_fBushTime = 0.1f;
}

// $Barnyard: FUNCTION 0058ed00
TBOOL APlayerControlComponent::IsAreaInReach( AInteriorArea* a_pArea )
{
	if ( !a_pArea )
		return TFALSE;

	ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, GetOwner()->GetSteer() );

	// NOTE: retail calls this without using the result
	pSteer->GetCurrentNetwork();

	TVector4 vecPosition;
	pSteer->CopyPosition( vecPosition );

	if ( !HASANYFLAG( a_pArea->m_eFlags, AInteriorArea::FLAGS_HEIGHT_FROM_TERRAIN ) )
	{
		// The steer has to be between the area's height and its base height (the Y axis points down)
		TVector4 vecAreaPosition;
		a_pArea->GetPosition( vecAreaPosition );

		TFLOAT fTop    = vecAreaPosition.y + 0.25f;
		TFLOAT fBottom = -a_pArea->m_fBaseHeight - 0.25f;

		if ( fBottom < fTop )
		{
			if ( vecPosition.y > fTop || vecPosition.y < fBottom )
				return TFALSE;
		}
		else
		{
			if ( vecPosition.y < fTop || vecPosition.y > fBottom )
				return TFALSE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0058edb0
void* APlayerControlComponent::GetAreaInteraction()
{
	AUnit* pOwner = GetOwner();

	if ( !m_pContextArea || m_pContextArea->m_strName->IsEmpty() || pOwner->HasInteraction() )
		return TNULL;

	TPString8 strName = *m_pContextArea->m_strName;
	AUnit*    pUnit   = GetOwner();

	if ( HASANYFLAG( m_pContextArea->m_eFlags, AInteriorArea::FLAGS_SPECIAL_GROUP_AREA ) && HASANYFLAG( m_pContextArea->m_eFlags, AInteriorArea::FLAGS_RUNTIME_SPECIAL ) )
	{
		// The milkweed patches of the ladder beans game: plant a seed in an empty patch
		ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

		if ( pSimState && pSimState->GetLadderBeansGame() )
		{
			if ( !pSimState->GetLadderBeansGame()->IsPlanted( ALadderBeansGame::FindLadder( strName.GetString() ) ) )
			{
				AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pUnit->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

				if ( pInventory->FindItemByType( TPS8( milkweed_seed ) ) )
					return AInteractionManager::GetSingleton()->AssignInteractionForced( pUnit, TPS8( milkweed ) );

				// "You need a milkweed seed"
				m_wszContextText = ALocaleManager::Interface()->GetString( 347 );
			}
		}
	}

	if ( HASANYFLAG( m_pContextArea->m_eFlags, AInteriorArea::FLAGS_RUNTIME_SPECIAL ) )
		return TNULL;

	return AInteractionManager::GetSingleton()->AssignInteractionForced( pUnit, strName );
}

// $Barnyard: FUNCTION 0058efa0
void* APlayerControlComponent::GetRegrowthInteraction()
{
	// Nothing to pick up while collecting the coins
	if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass()->IsA( &TGetClass( ACoinPickupMicroGame ) ) )
		return TNULL;

	AUnit*            pOwner = GetOwner();
	ASteer*           pSteer = pOwner->GetSteer();
	TVector2          vecDirection( pSteer->GetDirection()->x, pSteer->GetDirection()->z );
	ARegrowthManager* pRegrowth = ARegrowthManager::GetSingleton();

	// NOTE: retail leaves the type uninitialised if there's no object
	TINT iType = -1;

	// The fruit, vegetable, milk... in front of the player
	m_pRegrowth       = pRegrowth->FindObjectInFront( *pSteer->GetPosition(), vecDirection, ARegrowthManager::TypeMask::NonTrees(), iType, 0.9f, 0.8f );
	m_iGateOrRegrowth = iType;

	if ( !m_pRegrowth )
		return TNULL;

	AUnit* pUnit = pOwner;

	if ( !AInteractionComponent::IsUnitSuitableForInteraction( pOwner, TTRUE ) )
		return TNULL;

	return AInteractionManager::GetSingleton()->AssignInteractionToUnits( &pUnit, 1, TPS8( playerinstance ), AInteractionManager::ASSIGNFLAGS_FIRST_IS_INITIATOR | AInteractionManager::ASSIGNFLAGS_FORCED );
}

// The item in the "Hand" inventory group of a_pOwner, unless the player is busy with something else
// NOTE: inlined into GetHandItem and HasFlowersForBen
static AInventoryItem* GetHandInventoryItem( AUnit* a_pOwner )
{
	if ( !a_pOwner )
		return TNULL;

	ASteer* pSteer = a_pOwner->GetSteer();

	if ( pSteer->IsA( &TGetClass( APlayerSteer ) ) && TSTATICCAST( APlayerSteer, pSteer )->GetState() != APlayerSteer::STATE_WALKING )
		return TNULL;

	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, a_pOwner )->GetAnimatableObject();

	if ( pAnimatableObject->IsAnimationPlanned( TPS8( putaway ) ) || pAnimatableObject->IsAnimPlaying( TPS8( putaway ), 0.0f ) )
		return TNULL;

	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, a_pOwner->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	if ( !pInventory )
		return TNULL;

	AInventoryComponent* pHand = pInventory->GetCategory( TPString8( "Hand" ) );

	if ( pHand->GetNumItems() <= 0 )
		return TNULL;

	return pHand->GetItems().Begin();
}

// $Barnyard: FUNCTION 0058f0b0
TBOOL APlayerControlComponent::HasFlowersForBen()
{
	// Quest "Daisy Flowers For Ben"
	AInventoryItem* pItem = GetHandInventoryItem( GetOwner() );

	if ( !pItem || !pItem->GetUnit() || !pItem->GetUnit()->GetCarryableHelper() )
		return TFALSE;

	AObjectTraits* pTraits = pItem->GetUnit()->GetCarryableHelper()->GetObjectTraits();

	if ( AQuestManager::GetSingleton()->FindAssignedQuest( TPString8( "Daisy Flowers For Ben" ), 1 ) )
		return TFALSE;

	return pTraits->m_strInventoryForStore == TPS8( FlowerPack ) || pTraits->m_strInventoryForStore == TPS8( Knapsack );
}

// $Barnyard: FUNCTION 0058f260
AUnit* APlayerControlComponent::GetHandItem()
{
	// The carried unit in the "Hand" inventory group that can be put down
	AInventoryItem* pItem = GetHandInventoryItem( GetOwner() );

	if ( pItem && pItem->GetUnit() )
	{
		ACarryableUnitHelper* pHelper = pItem->GetUnit()->GetCarryableHelper();

		if ( pHelper )
		{
			AObjectTraits::UseTrigger* pWhenUse = pHelper->GetObjectTraits()->m_pWhenUse;

			if ( pWhenUse && ( pWhenUse->m_eFlags & AObjectTraits::UseTrigger::FLAGS_DROP ) )
				return pItem->GetUnit();
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0058f3c0
AInteractionSequence* APlayerControlComponent::StartUnitInteraction( AUnit* a_pUnit, const TCHAR* a_szName )
{
	if ( !a_pUnit )
		return TNULL;

	AUnit*                 pOwner                = GetOwner();
	AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( !pInteractionComponent || pInteractionComponent->IsAllowConversation() == TTRUE )
		return TNULL;

	AInteractionSequence* pSequence = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) )->GetSequence();

	if ( !pSequence || pSequence->GetInteraction() )
	{
		// Start a new sequence with both units
		AUnit* apUnits[ 2 ] = { a_pUnit, pOwner };

		if ( AInteractionComponent::IsUnitSuitableForInteraction( apUnits[ 0 ], TTRUE ) && AInteractionComponent::IsUnitSuitableForInteraction( apUnits[ 1 ], TTRUE ) )
		{
			return AInteractionManager::GetSingleton()->AssignInteractionToUnits(
			    apUnits, 2, TPString8( a_szName ), AInteractionManager::ASSIGNFLAGS_FIRST_IS_INITIATOR | AInteractionManager::ASSIGNFLAGS_FORCED
			);
		}
	}
	else
	{
		// The unit is in a sequence that's between two interactions, join it
		pSequence->AddUnit( pOwner );
		pSequence->FinishInteraction();

		pSequence->SetFlags( pSequence->GetFlags() | AInteractionSequence::FLAGS_PERSISTENT );
		TBOOL bPicked = AInteractionManager::GetSingleton()->PickInteraction( pSequence, TPString8() );
		pSequence->SetFlags( pSequence->GetFlags() & ~AInteractionSequence::FLAGS_PERSISTENT );

		if ( bPicked == TTRUE )
			return pSequence;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0058f530
void* APlayerControlComponent::GetGateInteraction()
{
	// NOTE: the name is wrong, it shakes the trees (fruits) and kicks the crates (ARegrowthManager)
	if ( m_pTargetUnit )
		return TNULL;

	ASteer* pSteer = GetOwner()->GetSteer();

	if ( pSteer->IsA( &TGetClass( APlayerSteer ) ) )
	{
		APlayerSteer* pPlayerSteer = TSTATICCAST( APlayerSteer, GetOwner()->GetSteer() );
		AUnit*        pOwner       = GetOwner();

		if ( pPlayerSteer->GetState() != APlayerSteer::STATE_WALKING )
			return TNULL;

		if ( pOwner->HasInteraction() == TTRUE )
			return TNULL;

		// NOTE: retail checks the current state, but AShakeTreeMicroGame is added as a child of it, so this never matches
		if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass()->IsA( &TGetClass( AShakeTreeMicroGame ) ) )
			return TNULL;

		if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass()->IsA( &TGetClass( ACoinPickupMicroGame ) ) )
			return TNULL;
	}

	m_iGateOrRegrowth = -1;

	TVector2          vecDirection( pSteer->GetDirection()->x, pSteer->GetDirection()->z );
	ARegrowthManager* pRegrowth = ARegrowthManager::GetSingleton();

	// A tree that has grown fruits
	ATreeManager::TreeInstance* pTree = ATreeManager::GetSingleton()->FindTreeInFront( *pSteer->GetPosition(), vecDirection, 2.5f );
	m_pGate                           = pTree;

	if ( pTree )
	{
		TINT iType = pRegrowth->FindTypeOfGroup( pTree->uiRegrowthGroup );

		if ( !ARegrowthManager::HasTypeFlag( iType, ARegrowthManager::TYPEFLAG_FRUIT ) ||
		     TSTATICCAST( ATreeManager::TreeInstance, m_pGate )->uiRegrowthGroup == 0xff ||
		     !pRegrowth->FindRandomObjectAtStage( TSTATICCAST( ATreeManager::TreeInstance, m_pGate )->uiRegrowthGroup, 10000, iType ) )
		{
			m_pGate = TNULL;
		}
	}

	AUnit* pOwner = GetOwner();

	if ( m_pGate )
	{
		m_iGateOrRegrowth = pRegrowth->FindTypeOfGroup( TSTATICCAST( ATreeManager::TreeInstance, m_pGate )->uiRegrowthGroup );
		return AInteractionManager::GetSingleton()->AssignInteractionForced( pOwner, pRegrowth->GetInteraction( m_iGateOrRegrowth ) );
	}

	// A crate
	TINT iType = -1;

	ARegrowthManager::GrowthObject* pCrate = pRegrowth->FindObjectInFront( *pSteer->GetPosition(), vecDirection, ARegrowthManager::TypeMask::Crates(), iType, 0.9f, 0.8f );
	m_iGateOrRegrowth                      = iType;

	if ( !pCrate )
		return TNULL;

	return AInteractionManager::GetSingleton()->AssignInteractionForced( pOwner, pRegrowth->GetInteraction( iType ) );
}

// $Barnyard: FUNCTION 0058f750
TBOOL APlayerControlComponent::UseHandItem()
{
	// NOTE: retail returns whatever was left in al, the callers don't use it
	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, GetOwner() );

	if ( !pPlayer )
		return TFALSE;

	ASteer* pSteer = pPlayer->GetSteer();

	if ( pSteer->GetClass()->IsA( &TGetClass( APlayerSteer ) ) && TSTATICCAST( APlayerSteer, pSteer )->GetState() != APlayerSteer::STATE_WALKING )
		return TFALSE;

	if ( pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( putaway ) ) || pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( putaway ), 0.0f ) )
		return TFALSE;

	if ( AInventoryGroupComponent::PutAwayHandItem() )
		return TFALSE;

	// Nothing in the hand, opens the inventory
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	if ( pInventory->GetCategory( TPS8D( "Hand" ) )->GetNumItems() != 0 )
		return TFALSE;

	if ( AGameLoader::IsBarnExploreMiniGame() )
		return TFALSE;

	AGameStateController* pController     = AGameStateController::GetSingleton();
	AInventoryState2*     pInventoryState = TSTATICCAST( AInventoryState2, pController->GetCurrentState()->FindChildState( &TGetClass( AInventoryState2 ) ) );

	if ( !pInventoryState )
	{
		pController->AddChildStateByName( TPS8( AInventoryState2 ) );
		pInventoryState = TSTATICCAST( AInventoryState2, pController->GetCurrentState()->FindChildState( &TGetClass( AInventoryState2 ) ) );
	}

	pInventoryState->Open();

	return TFALSE;
}

// $Barnyard: FUNCTION 0058f8a0
void APlayerControlComponent::PutDownHandItem( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	AUnit* pHandItem = GetHandItem();

	if ( !pHandItem )
		return;

	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, GetOwner() );
	TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) )->RemoveUnit( pHandItem );

	// The item is put down in front of the player
	// A carried character gets its steer back once it lies still
	TClass* pPrevSteerClass = TNULL;

	if ( pHandItem->IsA( &TGetClass( ACharacterUnitCarrable ) ) )
		pPrevSteerClass = const_cast<TClass*>( pHandItem->GetSteer()->GetClass() );

	pHandItem->ChangeSteer( &TGetClass( ABallSteer2 ) );

	TVector4 vecItemPosition;
	TSTATICCAST( AAnimatedUnit, pHandItem )->GetAnimatableObject()->GetModelInstance()->GetPosition( vecItemPosition );
	pHandItem->GetSteer()->SetPosition( vecItemPosition );

	// It's thrown away from the player a little
	const TVector4& rcPlayerPosition = *pPlayer->GetSteer()->GetPosition();

	TVector4 vecVelocity( vecItemPosition.x - rcPlayerPosition.x, 0.0f, vecItemPosition.z - rcPlayerPosition.z, vecItemPosition.w );
	vecVelocity.Normalise();
	vecVelocity.Multiply( 1.3f );
	vecVelocity.y = -0.25f;

	TVector4 vecPlayerVelocity = pPlayer->GetSteer()->GetVelocity();
	vecVelocity.x += vecPlayerVelocity.x;
	vecVelocity.y += vecPlayerVelocity.y;
	vecVelocity.z += vecPlayerVelocity.z;
	pHandItem->GetSteer()->SetVelocity( vecVelocity );

	ABallSteer2* pBallSteer = TSTATICCAST( ABallSteer2, pHandItem->GetSteer() );
	pBallSteer->SetDamping( 1.5f );

	TVector4 vecOffset;
	vecOffset.Multiply( TVector4::VEC_POSY, pHandItem->GetSteer()->GetRadius() );

	if ( pHandItem->IsA( &TGetClass( ACharacterUnitCarrable ) ) )
	{
		pBallSteer->SetBounceDistanceFactor( 0.0f );
		pBallSteer->SetNextSteerClass( pPrevSteerClass );
	}

	pBallSteer->SetOffset( vecOffset );

	AVisibleUnit* pVisibleItem = TSTATICCAST( AVisibleUnit, pHandItem );
	pVisibleItem->SetVisible( TTRUE );
	pVisibleItem->GetModelInstance()->SetSkeletonUpdating( TTRUE );
	pVisibleItem->GetModelInstance()->SetOpacity( 1.0f );

	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pHandItem->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( pTraits && pTraits->m_pWhenThrow && !pTraits->m_pWhenThrow->m_bDestroy )
	{
		// Stays, a bit above where it was
		TVector4 vecPosition;
		pHandItem->GetSteer()->CopyPosition( vecPosition );
		vecPosition.y -= 0.3f;
		pHandItem->GetSteer()->SetPosition( vecPosition );
	}
	else
	{
		// Waits, fades out and dies
		AWaitAniEffect* pWait = AAniEffectManager::GetSingleton()->CreateEffect<AWaitAniEffect>();
		AFadeAniEffect* pFade = AAniEffectManager::GetSingleton()->CreateEffect<AFadeAniEffect>();
		ADieAniEffect*  pDie  = AAniEffectManager::GetSingleton()->CreateEffect<ADieAniEffect>();

		pDie->SetUnit( pHandItem );
		pFade->SetUnit( pHandItem );
		pFade->SetSpeed( 0.5f );
		pFade->SetNextEffect( pDie );
		pWait->SetTime( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 15.0f, 20.0f ) );
		pWait->SetNextEffect( pFade );
	}

	// The player goes back to idle with the animations of its controller
	AAnimatableObject* pAnimObject = pPlayer->GetAnimatableObject();
	AAnimController*   pController = pAnimObject->FindAnimController( &TGetClass( ACharacterAnimController ) );

	if ( !pController )
		pController = pAnimObject->FindAnimController( &TGetClass( AAlienAnimController ) );

	// NOTE: retail calls ACharacterAnimController::GetAnimationSet on both kinds of controllers
	if ( pController && TSTATICCAST( ACharacterAnimController, pController )->GetAnimationSet() )
		pAnimObject->GetToshiAnimInterface().SetAnimationSet( TSTATICCAST( ACharacterAnimController, pController )->GetAnimationSet() );

	pPlayer->GetAnimatableObject()->KillAllAnimations();
	pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
}

// $Barnyard: FUNCTION 0058fcf0
TBOOL APlayerControlComponent::DropHeavyItem()
{
	AUnit* pHandItem = GetHandItem();

	return pHandItem && StartUnitInteraction( pHandItem, "ondrop_heavy" );
}

// $Barnyard: FUNCTION 0058fd20
TBOOL APlayerControlComponent::FixLeak()
{
	if ( m_pTargetUnit || !m_pLeak )
		return TFALSE;

	ATreasureManager::GetSingleton()->RemoveTreasure( TSTATICCAST( ATreasureManager::Treasure, m_pLeak ) );
	m_pLeak = TNULL;

	ATreasureManager* pTreasureManager = ATreasureManager::GetSingleton();

	if ( pTreasureManager->CountType( ATreasureManager::TYPE_LEAK ) == 0 )
	{
		// All the leaks of the sabotage quest are fixed
		if ( pTreasureManager->GetSabotageStage() == ATreasureManager::SABOTAGE_FIRST )
		{
			AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( TPString8( "allleaksfixed" ) );
			return TTRUE;
		}

		if ( pTreasureManager->GetSabotageStage() == ATreasureManager::SABOTAGE_SECOND )
			AInteractionManager::GetSingleton()->AssignInteractionToAllUnits( TPString8( "allleaksfixed2" ) );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0058fdd0
void APlayerControlComponent::StartAutoInteraction( AInteriorArea* a_pArea )
{
	// The "auto" areas start the interaction they are named after
	if ( a_pArea && !a_pArea->m_strName->IsEmpty() )
		AInteractionManager::GetSingleton()->AssignInteractionForced( GetOwner(), *a_pArea->m_strName );
}

// $Barnyard: FUNCTION 0058fe80
void* APlayerControlComponent::GetMilkingInteraction()
{
	// NOTE: it starts the "trigger_hint" interaction when the player faces one of Pip's hint notes (AHintManager)
	AUnit* pOwner = GetOwner();
	m_bFacingHint = AHintManager::GetSingleton()->IsFacingHint( pOwner );

	if ( !m_bFacingHint )
		return TNULL;

	if ( !AInteractionComponent::IsUnitSuitableForInteraction( pOwner, TTRUE ) )
		return TNULL;

	return AInteractionManager::GetSingleton()->AssignInteractionToUnits( &pOwner, 1, TPS8( trigger_hint ), AInteractionManager::ASSIGNFLAGS_FIRST_IS_INITIATOR | AInteractionManager::ASSIGNFLAGS_FORCED );
}

// $Barnyard: FUNCTION 0058ff00
void* APlayerControlComponent::GetPresentInteraction()
{
	// NOTE: the name is wrong, it opens the padlock of the gate the player stands next to (AGateManager)
	if ( !AGateManager::IsSingletonCreated() )
		return TNULL;

	AUnit* pOwner = GetOwner();

	if ( !AGateManager::GetSingleton()->GetNearbyPadlock() )
		return TNULL;

	if ( !AInteractionComponent::IsUnitSuitableForInteraction( pOwner, TTRUE ) )
		return TNULL;

	return AInteractionManager::GetSingleton()->AssignInteractionToUnits( &pOwner, 1, TPS8( trigger_gate ), AInteractionManager::ASSIGNFLAGS_FIRST_IS_INITIATOR | AInteractionManager::ASSIGNFLAGS_FORCED );
}

// $Barnyard: FUNCTION 005905b0
void APlayerControlComponent::UpdateContextText()
{
	// Shows the text of the context action in the AInteractionHudElement
	AInteractionHudElement* pInteractionHud = AHudManager::GetSingleton()->FindElement<AInteractionHudElement>();
	TINT                    iInputMap       = TINT( TREINTERPRETCAST( TUINTPTR, AGameStateController::GetSingleton()->GetCurrentState()->GetInputMap() ) );

	if ( m_bContextEnabled )
	{
		if ( m_wszOverrideText && *m_wszOverrideText )
		{
			pInteractionHud->SetForceHide( TFALSE );
			pInteractionHud->SetText( m_wszOverrideText, 2, iInputMap, 53, 0, 0xFFFFFFFF, -1.0f );
			return;
		}

		AInteractionComponent* pOwnInteractionComponent = TSTATICCAST( AInteractionComponent, GetOwner()->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( !pOwnInteractionComponent || !pOwnInteractionComponent->IsInSequence() )
		{
			TBOOL bHasHandItem = GetHandItem() != TNULL;

			if ( GetConversationSequence() && !bHasHandItem )
			{
				// Talk to the unit
				ALocale::FormatTags( m_wszText, 128, 190, PPropertyValue( const_cast<TWCHAR*>( m_pConversationUnit->GetScreenName() ), TFALSE ) );
				m_wszContextText = m_wszText;
			}
			else if ( m_pTargetUnit )
			{
				if ( m_pTargetUnit->CanInteract() && AAnimalConversationState::CanConverse( m_pTargetUnit ) )
				{
					ALocale::FormatTags( m_wszText, 128, 193, PPropertyValue( const_cast<TWCHAR*>( m_pTargetUnit->GetScreenName() ), TFALSE ) );

					if ( !m_wszContextText )
						m_wszContextText = m_wszText;
				}
				else if ( !bHasHandItem && !m_pTargetUnit->IsA( &TGetClass( ACharacterUnit ) ) && AInteractionComponent::IsUnitSuitableForInteraction( m_pTargetUnit, TTRUE ) )
				{
					if ( m_pTargetUnit->GetType()->GetName() == TPS8( bike ) )
					{
						AInventoryGroupComponent* pInventory  = TSTATICCAST( AInventoryGroupComponent, GetOwner()->FindComponentExact( &TGetClass( AInventoryGroupComponent ) ) );
						TBOOL                     bHasBikeKey = pInventory && pInventory->FindItemByType( TPS8( bikekey ) );

						if ( !bHasBikeKey )
						{
							m_wszContextText = ALocaleManager::Interface()->GetString( 3349 );
						}
						else
						{
							ALocale::FormatTags( m_wszText, 128, 203, PPropertyValue( const_cast<TWCHAR*>( m_pTargetUnit->GetScreenName() ), TFALSE ) );
							m_wszContextText = m_wszText;
						}
					}
					else
					{
						AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, m_pTargetUnit->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

						if ( !pTraits || !pTraits->IsAutoPickup() )
						{
							// Some units have their own text (0x00728a28)
							static const struct
							{
								TINT iScreenNameId;
								TINT iTextId;
							} s_aUnitTexts[] = {
								{ 0x172, 798 },
								{ 0x183, 800 },
								{ 0x186, 802 },
								{ 0x187, 804 },
								{ 0x178, 806 },
								{ 0x17a, 808 },
								{ 0x185, 810 },
								{ 0x174, 812 },
								{ 0x175, 814 },
								{ 0x180, 816 },
								{ 0x184, 818 },
								{ 0x189, 820 },
							};

							TBOOL bFound = TFALSE;

							for ( TINT i = 0; i < TARRAYSIZE( s_aUnitTexts ); i++ )
							{
								if ( s_aUnitTexts[ i ].iScreenNameId == m_pTargetUnit->GetType()->GetScreenNameId() )
								{
									if ( s_aUnitTexts[ i ].iTextId > -1 )
									{
										TStringManager::String16Copy( m_wszText, ALocaleManager::Interface()->GetString( s_aUnitTexts[ i ].iTextId ) );
										m_wszContextText = m_wszText;
										bFound           = TTRUE;
									}

									break;
								}
							}

							if ( !bFound )
							{
								ALocale::FormatTags( m_wszText, 128, 194, PPropertyValue( const_cast<TWCHAR*>( m_pTargetUnit->GetScreenName() ), TFALSE ) );
								m_wszContextText = m_wszText;
							}
						}
					}
				}
			}

			if ( !m_wszContextText && HasFlowersForBen() )
			{
				ALocale::FormatTags( m_wszText, 128, 169 );
				m_wszContextText = m_wszText;
			}

			if ( m_wszContextText )
			{
				pInteractionHud->SetForceHide( TFALSE );
				pInteractionHud->SetText( m_wszContextText, 2, iInputMap, 53, 0, 0xFFFFFFFF, -1.0f );
				return;
			}
		}
	}

	pInteractionHud->SetText( TNULL, 2, 0, -1, 0, 0xFFFFFFFF, -1.0f );
	pInteractionHud->SetForceHide( TTRUE );
}

// Normalises the vector on the XZ plane (y = 0), (0, 0, 1) if it's zero
// NOTE: a TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// $Barnyard: FUNCTION 00590950
void APlayerControlComponent::FindNearestTarget()
{
	static TFLOAT s_fMinCos = TMath::Cos( TMath::HALF_PI ); // 0x0078a2a0
	static TFLOAT s_fRange  = 2.5f;                         // 0x0077510c

	m_bTargetCanTalk = TFALSE;

	AUnit*   pOwner = GetOwner();
	TVector4 vecPosition;
	pOwner->GetSteer()->CopyPosition( vecPosition );
	TVector4 vecDirection = *pOwner->GetSteer()->GetDirection();

	m_fTargetDistanceSq = s_fRange * s_fRange;

	TINT                    iNumUnits;
	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( vecPosition, s_fRange, &iNumUnits, TFALSE, 420, TFALSE, TNULL );

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit*   pUnit           = pResults[ i ].pUnit;
		TVector4 vecUnitPosition = *pUnit->GetSteer()->GetPosition();

		if ( pUnit == pOwner )
			continue;

		TVector4 vecToUnit( vecUnitPosition.x - vecPosition.x, vecUnitPosition.y - vecPosition.y, vecUnitPosition.z - vecPosition.z, vecUnitPosition.w );

		// Not too far below or above (the Y axis points down)
		if ( vecToUnit.y >= 1.0f || vecToUnit.y <= -2.0f )
			continue;

		// In front of the player
		NormaliseXZ( vecToUnit );

		if ( s_fMinCos >= TVector4::DotProduct3( vecDirection, vecToUnit ) )
			continue;

		TFLOAT fDistanceSq = TVector4::DistanceSq( vecPosition, vecUnitPosition );

		if ( !pUnit->CanInteract() )
		{
			// The unit is busy, the player can listen if it's talking
			AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, pUnit->FindComponent( &TGetClass( AInteractionComponent ) ) );

			if ( pInteractionComponent && pInteractionComponent->IsAllowConversation() )
			{
				AInteractionSequence* pSequence = pInteractionComponent->GetSequence();

				if ( pSequence )
				{
					AInteractionActivity* pActivity = pSequence->GetCurrentActivity();

					if ( pActivity && pActivity->IsA( &TGetClass( ASpeechActivity ) ) && !( TSTATICCAST( ASpeechActivity, pActivity )->GetFlags() & ASpeechActivity::FLAGS_FORCE_VISIBLE ) && fDistanceSq < m_fTargetDistanceSq )
					{
						m_pConversationUnit = pUnit;
						m_pTargetUnit       = TNULL;
						m_fTargetDistanceSq = fDistanceSq;
					}
				}
			}

			continue;
		}

		if ( fDistanceSq >= m_fTargetDistanceSq )
			continue;

		if ( pUnit->GetCarryableHelper() )
		{
			// Something to pick up
			m_pInteraction = StartUnitInteraction( pUnit, "playerobject" );

			if ( m_pInteraction )
			{
				m_pInteraction->SetFlags( m_pInteraction->GetFlags() | AInteractionSequence::FLAGS_ENDED );
				ReleaseInteraction();

				m_pTargetUnit       = pUnit;
				m_pConversationUnit = TNULL;
				m_fTargetDistanceSq = fDistanceSq;
			}

			continue;
		}

		if ( !m_pContextArea )
			continue;

		// Somebody to talk to, see if the conversation can be started
		m_pInteraction = FindUnitInteraction( pUnit, TFALSE );

		if ( !m_pInteraction )
			continue;

		m_pInteraction->SetFlags( m_pInteraction->GetFlags() | AInteractionSequence::FLAGS_ENDED );

		AInteraction* pInteraction = m_pInteraction->GetInteraction();
		TBOOL         bCanTalk     = TTRUE;

		if ( AQuestManager::GetSingleton()->GetNumAssignedQuests() > 0 && pInteraction->GetNumActivities() > 0 )
		{
			// The player can't take another quest while doing one
			for ( TINT j = 0; j < pInteraction->GetNumActivities(); j++ )
			{
				AInteractionActivity* pActivity = pInteraction->GetActivity( j );

				if ( pActivity && pActivity->IsA( &TGetClass( AQuestActivity ) ) && TSTATICCAST( AQuestActivity, pActivity )->IsAssigningQuest() )
				{
					bCanTalk = TFALSE;
					break;
				}
			}
		}

		if ( ( pInteraction->GetFlags() & AInteraction::FLAGS_MISSION_CONVERSATION ) && pUnit->GetType()->GetName() != TPS8( nb_bull ) )
		{
			// The unit has nothing to say about a quest of the current tier that isn't available
			AQuestManager::QuestTier* pTier = AQuestManager::GetSingleton()->GetCurrentQuestTier();

			for ( TINT j = 0; j < pTier->uiNumQuests; j++ )
			{
				AQuest* pQuest = AQuestManager::GetSingleton()->GetQuest( pTier->auiQuests[ j ] );

				if ( AQuestManager::GetSingleton()->IsQuestUnavailable( pQuest ) )
				{
					if ( pQuest->HasUnit( AUnitNameManager::GetSingleton()->GetUnitName( pUnit->GetId() ) ) )
					{
						bCanTalk = TFALSE;
						break;
					}
				}
			}
		}

		ReleaseInteraction();

		if ( bCanTalk )
		{
			const TWCHAR* wszHudText = GetHudText( pInteraction );

			if ( wszHudText )
				m_wszContextText = wszHudText;

			m_pTargetUnit       = pUnit;
			m_pConversationUnit = TNULL;
			m_fTargetDistanceSq = fDistanceSq;
			m_bTargetCanTalk    = TTRUE;
		}
	}
}

// $Barnyard: FUNCTION 0058e4e0
const TWCHAR* APlayerControlComponent::GetHudText( AInteraction* a_pInteraction )
{
	TINT iHudText = a_pInteraction->GetHudText().uiIndex;

	if ( iHudText == -1 )
		return TNULL;

	return ALocaleManager::Interface()->GetString( iHudText );
}

// $Barnyard: FUNCTION 00590d90
void APlayerControlComponent::UpdateContext()
{
	m_pTargetUnit       = TNULL;
	m_pConversationUnit = TNULL;

	// The interaction found last time is only a probe
	if ( m_pInteraction )
	{
		if ( m_pInteraction->GetFlags() & AInteractionSequence::FLAGS_ENDED )
		{
			AInteractionManager::GetSingleton()->EndSequence( m_pInteraction, TFALSE );
			m_pInteraction = TNULL;
		}
		else if ( !m_pInteraction->GetInteraction() || ( m_pInteraction->GetFlags() & AInteractionSequence::FLAGS_INTERACTION_DONE ) )
		{
			m_pInteraction = TNULL;
		}
	}

	m_pRegrowth      = TNULL;
	m_pGate          = TNULL;
	m_wszContextText = TNULL;
	m_pLeak          = TNULL;

	AUnit*   pOwner   = GetOwner();
	AUnit*   pCarried = TSTATICCAST( AAnimatedUnit, pOwner )->GetAttachedUnit( TPS8( cb_rlocator ), -1 );
	ASteer*  pSteer   = pOwner->GetSteer();
	TVector2 vecDirection2D( pSteer->GetDirection()->x, pSteer->GetDirection()->z );
	TBOOL    bGate = TFALSE;

	FindNearestTarget();

	if ( !m_pInteraction )
	{
		if ( !GetHandItem() )
		{
			if ( !m_pInteraction )
			{
				// The leak in front of the player ("Fix Leak")
				m_pLeak = ATreasureManager::GetSingleton()->FindTreasureInFront( *pSteer->GetPosition(), vecDirection2D, 1.2f, ATreasureManager::TYPE_LEAK );

				if ( m_pLeak )
				{
					m_wszContextText = ALocaleManager::Interface()->GetString( 197 );
				}
				else if ( m_wszContextText == ALocaleManager::Interface()->GetString( 197 ) )
				{
					m_wszContextText = TNULL;
				}
			}

			m_pInteraction = TSTATICCAST( AInteractionSequence, GetMilkingInteraction() );

			if ( m_pInteraction )
			{
				m_wszContextText = GetHudText( m_pInteraction->GetInteraction() );
				m_pInteraction->SetFlags( m_pInteraction->GetFlags() | AInteractionSequence::FLAGS_ENDED );
				ReleaseInteraction();
			}

			m_pInteraction = TSTATICCAST( AInteractionSequence, GetPresentInteraction() );

			if ( m_pInteraction )
			{
				m_wszContextText = GetHudText( m_pInteraction->GetInteraction() );
				m_pInteraction->SetFlags( m_pInteraction->GetFlags() | AInteractionSequence::FLAGS_ENDED );
				ReleaseInteraction();
			}

			if ( !pCarried )
			{
				m_pInteraction = TSTATICCAST( AInteractionSequence, GetGateInteraction() );

				if ( m_pInteraction )
				{
					bGate            = TTRUE;
					m_wszContextText = GetHudText( m_pInteraction->GetInteraction() );
					m_pInteraction->SetFlags( m_pInteraction->GetFlags() | AInteractionSequence::FLAGS_ENDED );
					ReleaseInteraction();

					if ( ARegrowthManager::HasTypeFlag( m_iGateOrRegrowth, ARegrowthManager::TYPEFLAG_CRATE ) )
					{
						// The crates have their own text (0x00728a88)
						static const struct
						{
							TINT iScreenNameId;
							TINT iTextId;
						} s_aCrateTexts[] = {
							{ 419, 822 },
							{ 420, 824 },
							{ 421, 826 },
							{ 422, 828 },
							{ 423, 830 },
							{ 425, 832 },
							{ 426, 834 },
							{ 428, 836 },
							{ 433, 838 },
							{ 436, 840 },
							{ 440, 842 },
							{ 429, 824 },
							{ 427, 828 },
							{ 435, 828 },
							{ 437, 828 },
							{ 438, 828 },
						};

						TINT  iScreenNameId = ARegrowthManager::GetSingleton()->GetScreenNameId( m_iGateOrRegrowth );
						TBOOL bFound        = TFALSE;

						for ( TINT i = 0; i < TARRAYSIZE( s_aCrateTexts ); i++ )
						{
							if ( s_aCrateTexts[ i ].iScreenNameId == iScreenNameId )
							{
								if ( s_aCrateTexts[ i ].iTextId > -1 )
								{
									TStringManager::String16Copy( m_wszText, ALocaleManager::Interface()->GetString( s_aCrateTexts[ i ].iTextId ), 128 );
									m_wszContextText = m_wszText;
									bFound           = TTRUE;
								}

								break;
							}
						}

						if ( !bFound )
						{
							TWCHAR wszName[ 128 ];
							ALocale::FormatTags( wszName, 128, iScreenNameId );
							ALocale::FormatTags( m_wszText, 128, 192, PPropertyValue( wszName, TFALSE ) );
							m_wszContextText = m_wszText;
						}
					}
					else
					{
						// "Shake"
						m_wszContextText = ALocaleManager::Interface()->GetString( 191 );
					}
				}
			}
		}

		if ( !m_pInteraction && !m_wszContextText )
		{
			m_pInteraction = TSTATICCAST( AInteractionSequence, GetRegrowthInteraction() );

			if ( m_pInteraction )
			{
				m_wszContextText = GetHudText( m_pInteraction->GetInteraction() );
				m_pInteraction->SetFlags( m_pInteraction->GetFlags() | AInteractionSequence::FLAGS_ENDED );
				ReleaseInteraction();

				if ( !m_wszContextText )
				{
					// The name of the unit the object gives
					TPString8 strUnitType = ARegrowthManager::GetSingleton()->GetUnitType( m_iGateOrRegrowth );

					if ( !strUnitType.IsEmpty() )
					{
						AUnitType* pUnitType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( strUnitType );

						// Some units have their own text (0x00728a28)
						static const struct
						{
							TINT iScreenNameId;
							TINT iTextId;
						} s_aUnitTexts[] = {
							{ 0x172, 798 },
							{ 0x183, 800 },
							{ 0x186, 802 },
							{ 0x187, 804 },
							{ 0x178, 806 },
							{ 0x17a, 808 },
							{ 0x185, 810 },
							{ 0x174, 812 },
							{ 0x175, 814 },
							{ 0x180, 816 },
							{ 0x184, 818 },
							{ 0x189, 820 },
						};

						TBOOL bFound = TFALSE;

						for ( TINT i = 0; i < TARRAYSIZE( s_aUnitTexts ); i++ )
						{
							if ( s_aUnitTexts[ i ].iScreenNameId == pUnitType->GetScreenNameId() )
							{
								if ( s_aUnitTexts[ i ].iTextId > -1 )
								{
									TStringManager::String16Copy( m_wszText, ALocaleManager::Interface()->GetString( s_aUnitTexts[ i ].iTextId ), 128 );
									m_wszContextText = m_wszText;
									bFound           = TTRUE;
								}

								break;
							}
						}

						if ( !bFound )
						{
							ALocale::FormatTags( m_wszText, 128, 194, PPropertyValue( const_cast<TWCHAR*>( pUnitType->GetScreenName() ), TFALSE ) );
							m_wszContextText = m_wszText;
						}
					}
				}
			}
			else
			{
				m_bAreaInteraction = TFALSE;
			}
		}
	}

	if ( pCarried )
	{
		// The milkweed seeds and the coins can only be put down in their areas
		TINT iAct  = AQuestManager::GetSingleton()->GetAct();
		TINT iTier = AQuestManager::GetSingleton()->GetTier();
		TINT iQuestAct, iQuestTier;

		AQuestManager::GetSingleton()->FindQuestTier( "Daisy Flowers For Ben", &iQuestAct, &iQuestTier );

		if ( iAct == iQuestAct && iTier == iQuestTier )
			pCarried = TNULL;

		AQuestManager::GetSingleton()->FindQuestTier( "Daisy Happy Birthday", &iQuestAct, &iQuestTier );

		if ( iAct == iQuestAct && iTier == iQuestTier )
		{
			pCarried = TNULL;
		}
		else if ( pCarried )
		{
			const TPString8& rcType = pCarried->GetType()->GetName();

			if ( rcType == TPS8( milkweed_seed ) )
			{
				if ( m_pArea && TStringManager::String8FindString( m_pArea->m_strName->GetString(), "mweed" ) )
					pCarried = TNULL;
			}
			else if ( rcType == TPS8( goldrush_token ) )
			{
				if ( m_pArea && TStringManager::String8FindString( m_pArea->m_strName->GetString(), "cointrail" ) )
					pCarried = TNULL;
			}
		}
	}

	if ( !m_pInteraction && !bGate )
	{
		m_pContextArea = m_pArea;

		if ( m_pContextArea && !pCarried )
		{
			m_pInteraction = TSTATICCAST( AInteractionSequence, GetAreaInteraction() );

			if ( m_pInteraction )
			{
				m_wszContextText = GetHudText( m_pInteraction->GetInteraction() );
				m_pInteraction->SetFlags( m_pInteraction->GetFlags() | AInteractionSequence::FLAGS_ENDED );
				ReleaseInteraction();
				m_bAreaInteraction = TTRUE;
			}
			else
			{
				m_bAreaInteraction = TFALSE;
			}
		}
	}
}

// $Barnyard: FUNCTION 00591530
void APlayerControlComponent::UpdateContextTimer( TFLOAT a_fDeltaTime )
{
	m_fContextTime += a_fDeltaTime;

	if ( m_fContextTime >= 0.2f || m_pArea != m_pContextArea )
	{
		UpdateContext();
		UpdateContextText();
		m_fContextTime = 0.0f;
	}
}

// $Barnyard: FUNCTION 00591570
void APlayerControlComponent::UpdateCurrentArea( TFLOAT a_fDeltaTime )
{
	if ( m_bMoved )
	{
		ASteer*        pSteer = GetOwner()->GetSteer();
		AInteriorArea* pArea  = TSTATICCAST( ACommonSteer, pSteer )->GetCurrentArea();

		// The barn explore mini game checks the height too
		if ( !AGameLoader::IsBarnExploreMiniGame() || IsAreaInReach( pArea ) )
		{
			if ( m_pArea != pArea )
				m_pArea = pArea;
		}
		else
		{
			m_pArea = TNULL;
		}

		AInteriorNetwork* pAutoNetwork = AInteriorNetworkManager::GetSingleton()->FindNetworkContaining( "auto" );

		if ( pAutoNetwork )
		{
			AInteriorArea* pAutoArea = TREINTERPRETCAST( AInteriorArea*, TSTATICCAST( ACommonSteer, pSteer )->GetAreaInNetwork( TREINTERPRETCAST( TINT, pAutoNetwork ) ) );

			if ( IsAreaInReach( pAutoArea ) )
			{
				if ( m_pAutoArea != pAutoArea )
				{
					m_pAutoArea = pAutoArea;
					StartAutoInteraction( pAutoArea );
				}
			}
			else
			{
				m_pAutoArea = TNULL;
			}
		}
	}

	// Climb the ladders right away (unless carrying something)
	if ( m_pArea && HASANYFLAG( m_pArea->m_eFlags, AInteriorArea::FLAGS_SPECIAL_GROUP_AREA ) && !HASANYFLAG( m_pArea->m_eFlags, AInteriorArea::FLAGS_RUNTIME_SPECIAL ) )
	{
		AAnimatedUnit* pOwner = TSTATICCAST( AAnimatedUnit, GetOwner() );

		if ( !pOwner->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) && pOwner->GetSteer() )
			TSTATICCAST( APlayerSteer, pOwner->GetSteer() )->SetState( APlayerSteer::STATE_CLIMBING );
	}
}
