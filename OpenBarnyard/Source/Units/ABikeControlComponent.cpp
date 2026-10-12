#include "pch.h"
#include "ABikeControlComponent.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "AUnitManager.h"
#include "AObjectTraits.h"
#include "AInteractionComponent.h"
#include "AInventoryGroupComponent.h"
#include "ACarryableUnitHelper.h"
#include "Helpers/ABikeManager.h"
#include "GameInterface/AGameStateController.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( ASimState );

// NOTE: GetClass 0058b490, CreateObject 0058b4c0, CreateObjectInPlace 0058b560
TDEFINE_CLASS( ABikeControlComponent );

ABikeControlComponent::ABikeControlComponent()
{
	// NOTE: inlined into CreateObject/CreateObjectInPlace in retail
	m_vecInput.x   = 0.0f;
	m_vecInput.y   = 0.0f;
	m_vecInput.z   = 0.0f;
	m_vecInput.w   = 0.0f;
	m_bHorn        = TFALSE;
	m_bUnk2D       = TFALSE;
	m_bTrickLeft   = TFALSE;
	m_bTrickRight  = TFALSE;
	m_bTrick       = TFALSE;
	m_iUnk34       = 0;
	m_bBoost       = TFALSE;
	m_iUnk3C       = 0;
	m_fPickUpTimer = 5.0f;
}

// $Barnyard: FUNCTION 0058b5f0
// $Barnyard: FUNCTION 0058b4a0
ABikeControlComponent::~ABikeControlComponent()
{
}

// $Barnyard: FUNCTION 0058ba70
void ABikeControlComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, GetOwner()->GetSteer() );

#ifdef BARNYARD_COMMUNITY_PATCH
	if ( ms_bDevInput )
	{
		m_vecInput.x += ms_fDevSteer;
		m_vecInput.z = ms_fDevThrottle;
	}
#endif

	if ( m_bBoost == TTRUE && m_vecInput.z > 0.0f )
		m_vecInput.z = 2.0f;

	TMath::Clip( m_vecInput.x, -1.0f, 1.0f );
	TMath::Clip( m_vecInput.y, -1.0f, 1.0f );

	pSteer->SetDesiredVelocity( m_vecInput );

	m_vecInput   = TVector4::VEC_ZERO;
	m_vecInput.w = 0.0f;

	if ( m_bHorn )
	{
		pSteer->PlayHorn();
		m_bHorn = TFALSE;
	}

	if ( m_bTrick == TTRUE )
	{
		if ( m_bTrickLeft == TTRUE )
		{
			if ( m_bTrickRight == TTRUE )
				pSteer->SetTrick( TTRUE, 2 );
			else
				pSteer->SetTrick( TTRUE, 0 );
		}
		else if ( m_bTrickRight == TTRUE )
		{
			pSteer->SetTrick( TTRUE, 1 );
		}
	}
	else
	{
		pSteer->SetTrick( TFALSE, 0 );
	}

	UpdatePickUps( a_fDeltaTime );
	m_bTrick = TFALSE;
}

// $Barnyard: FUNCTION 0058b610
TBOOL ABikeControlComponent::ProcessCommand( AInputCommand a_eCommand, const TInputInterface::InputEvent* a_pEvent )
{
	// NOTE: for the digital moves retail calls 006c3cf0, the event's StartRepeat with zero times
	switch ( a_eCommand )
	{
		case AInputCommand_Leave:
			ABikeManager::GetSingleton()->SetWantsToGetOff( TTRUE );
			return TTRUE;
		case AInputCommand_Move:
			m_vecInput.x += a_pEvent->GetMagnitudeFloat( 0 );
			m_vecInput.y -= a_pEvent->GetMagnitudeFloat( 1 );
			return TTRUE;
		case AInputCommand_Left:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vecInput.x -= 1.0f;
			return TTRUE;
		case AInputCommand_Right:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vecInput.x += 1.0f;
			return TTRUE;
		case AInputCommand_Forward:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vecInput.y += 1.0f;
			return TTRUE;
		case AInputCommand_Backward:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vecInput.y -= 1.0f;
			return TTRUE;
		case AInputCommand_Jump:
			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				a_pEvent->StartRepeat( 0.0f, 0.0f );
				m_vecInput.w += 1.0f;
			}
			return TTRUE;
		case AInputCommand_Brake:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vecInput.z = -1.0f;
			return TTRUE;
		case AInputCommand_Accelerator:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vecInput.z = 1.0f;
			return TTRUE;
		case AInputCommand_Bell:
			m_bHorn = TTRUE;
			return TTRUE;
		case AInputCommand_Trick1:
			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				a_pEvent->StartRepeat( 0.0f, 0.0f );
				m_bBoost     = TTRUE;
				m_vecInput.z = 1.0f;
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT )
			{
				m_vecInput.z = 1.0f;
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				m_bBoost = TFALSE;
			}
			return TTRUE;
		case AInputCommand_Trick3:
			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				if ( TSTATICCAST( ABikeSteer, GetOwner()->GetSteer() )->IsAirborne() )
				{
					m_bTrickLeft = TTRUE;
					a_pEvent->StartRepeat( 0.0f, 0.0f );
					m_bTrick = TTRUE;
				}
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT )
			{
				m_bTrickLeft = TTRUE;
				m_bTrick     = TTRUE;
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				m_bTrickLeft = TFALSE;
				m_bTrick     = TFALSE;
			}
			return TTRUE;
		case AInputCommand_Trick4:
			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				if ( TSTATICCAST( ABikeSteer, GetOwner()->GetSteer() )->IsAirborne() )
				{
					m_bTrickRight = TTRUE;
					m_bTrick      = TTRUE;
					a_pEvent->StartRepeat( 0.0f, 0.0f );
				}
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT )
			{
				m_bTrickRight = TTRUE;
				m_bTrick      = TTRUE;
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				m_bTrickRight = TFALSE;
				m_bTrick      = TFALSE;
			}
			return TTRUE;
		default:
			return TFALSE;
	}
}

// $Barnyard: FUNCTION 0058b4b0
void ABikeControlComponent::OnDisable()
{
	m_bBoost = TFALSE;
	AControlComponent::OnDisable();
}

// $Barnyard: FUNCTION 0058b8f0
void ABikeControlComponent::UpdatePickUps( TFLOAT a_fDeltaTime )
{
	m_fPickUpTimer -= a_fDeltaTime;

	if ( m_fPickUpTimer >= 0.0f )
		return;

	// Picks up the items around the bike that are picked up automatically
	AUnit*                  pOwner = GetOwner();
	TINT                    iNumUnits;
	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( *pOwner->GetSteer()->GetPosition(), 1.7f, &iNumUnits, TFALSE, 420, TFALSE, TNULL );

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

		if ( pOwner->GetSteer()->GetClass()->IsA( &TGetClass( ABikeSteer ) ) )
			bPickUp = pOwner->GetSteer()->CanStartAction();

		if ( bPickUp )
		{
			AInventoryGroupComponent::CollectUnit( pUnit, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );
			m_fPickUpTimer = 0.25f;
		}
	}

	// NOTE: retail always overwrites the 0.25s above
	m_fPickUpTimer = 0.1f;
}
