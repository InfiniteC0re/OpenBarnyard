#include "pch.h"
#include "ACharacterMovementComponent.h"
#include "AGeneralUnitComponentManager.h"
#include "AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 0054f4d0 / CreateObjectInPlace 0054f540
TDEFINE_CLASS( ACharacterMovementComponent, 0, 2 );

// $Barnyard: FUNCTION 0054f340
// $Barnyard: FUNCTION 0054f220
ACharacterMovementComponent::~ACharacterMovementComponent()
{
}

// NOTE: folded getter of AGeneralUnitComponentManager::ms_pSingleton (005d2300)
AUnitComponentManager* ACharacterMovementComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingleton();
}

// $Barnyard: FUNCTION 0054f240
void ACharacterMovementComponent::SetSpeed( APPLY a_eApply, TFLOAT a_fSpeed, SPEED a_eSpeed )
{
	AUnit*              pUnit   = GetOwner();
	const ASteerTraits* pTraits = pUnit->GetSteerTraits();

	if ( a_eSpeed != SPEED_RUN )
		m_fRunTimer = -1.0f;

	TFLOAT fModifier = ( pTraits->GetLogicalSpeed( a_eSpeed ) * a_fSpeed ) / pTraits->m_fLogicalRunSpeed;

	if ( fModifier >= 0.0f )
	{
		pUnit->GetSteer()->SetSpeedModifier( fModifier );
	}

	if ( a_eApply == APPLY_IF_MOVING )
	{
		if ( pUnit->GetSteer()->GetDesiredSpeed() > TMath::TFLOAT_EPSILON )
			pUnit->GetSteer()->SetToCurrentFullDesiredSpeed();
	}
	else if ( a_eApply == APPLY_IMMEDIATELY )
	{
		ASteer* pSteer = pUnit->GetSteer();
		pSteer->SetSpeed( pSteer->GetSpeedFromModifier( pSteer->GetSpeedModifier() ) );
	}
}

// $Barnyard: FUNCTION 0054f470
void ACharacterMovementComponent::SetWalkingSpeed( APPLY a_eApply, TFLOAT a_fSpeed )
{
	SetSpeed( a_eApply, a_fSpeed, SPEED_WALK );
}

// $Barnyard: FUNCTION 0054f490
void ACharacterMovementComponent::SetRunningSpeed( APPLY a_eApply, TFLOAT a_fSpeed )
{
	SetSpeed( a_eApply, a_fSpeed, SPEED_RUN );
}

// $Barnyard: FUNCTION 0054f4b0
void ACharacterMovementComponent::ResetSpeed( APPLY a_eApply )
{
	SetSpeed( a_eApply, GetWalkSpeed(), SPEED_WALK );
}

// $Barnyard: FUNCTION 0054f360
void ACharacterMovementComponent::SetDesireWalk()
{
	// NOTE: SetSpeed is inlined here in retail
	SetSpeed( APPLY_IF_MOVING, GetWalkSpeed(), SPEED_WALK );
}

// $Barnyard: FUNCTION 0054f3e0
void ACharacterMovementComponent::SetDesireRun( TFLOAT a_fTime )
{
	m_fRunTimer = a_fTime;

	// NOTE: SetSpeed is inlined here in retail
	SetSpeed( APPLY_IF_MOVING, GetRunSpeed(), SPEED_RUN );
}

// $Barnyard: FUNCTION 0054f460
void ACharacterMovementComponent::SetDesireRun()
{
	SetDesireRun( TMath::MAXFLOAT );
}
