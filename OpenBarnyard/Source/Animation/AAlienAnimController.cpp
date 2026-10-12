#include "pch.h"
#include "AAlienAnimController.h"
#include "AAnimatableObject.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/AAlienSteer.h"
#include "UnitMovement/Steering/ASteerKinematics3D.h"
#include "UnitMovement/Steering/ASteerTraits.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00585540, CreateObject 00585740, CreateObjectInPlace 005857a0
TDEFINE_CLASS( AAlienAnimController );

const TPString8* AAlienAnimController::ms_apRunAnims[ NUM_DIRECTIONS ];

TPSTRING8_DECLARE( milk_run_n );
TPSTRING8_DECLARE( milk_run_ne );
TPSTRING8_DECLARE( milk_run_e );
TPSTRING8_DECLARE( milk_run_se );
TPSTRING8_DECLARE( milk_run_s );
TPSTRING8_DECLARE( milk_run_sw );
TPSTRING8_DECLARE( milk_run_w );
TPSTRING8_DECLARE( milk_run_nw );

// $Barnyard: FUNCTION 00585560
AAlienAnimController::AAlienAnimController()
{
	ms_apRunAnims[ 0 ] = &TPS8( milk_run_n );
	ms_apRunAnims[ 1 ] = &TPS8( milk_run_ne );
	ms_apRunAnims[ 2 ] = &TPS8( milk_run_e );
	ms_apRunAnims[ 3 ] = &TPS8( milk_run_se );
	ms_apRunAnims[ 4 ] = &TPS8( milk_run_s );
	ms_apRunAnims[ 5 ] = &TPS8( milk_run_sw );
	ms_apRunAnims[ 6 ] = &TPS8( milk_run_w );
	ms_apRunAnims[ 7 ] = &TPS8( milk_run_nw );

	m_bRolling     = TFALSE;
	m_bWasStrafing = TFALSE;
	m_bAnimEnabled = TTRUE;
}

// $Barnyard: FUNCTION 00585800
// $Barnyard: FUNCTION 005855d0
AAlienAnimController::~AAlienAnimController()
{
}

TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( squirt );
TPSTRING8_DECLARE( milk_combatroll_l );
TPSTRING8_DECLARE( milk_combatroll_r );

// $Barnyard: FUNCTION 005858b0
void AAlienAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	AUnit*                    pUnit          = TSTATICCAST( AUnit, m_pOwner );
	AAlienSteer*              pSteer         = TSTATICCAST( AAlienSteer, pUnit->GetSteer() );
	AToshiAnimationInterface& rAnimInterface = GetAnimInterface();
	const TPString8*          pAnimName      = &TPS8( idle_lp );
	TFLOAT                    fSpeed         = pSteer->GetKinematics()->GetSpeed();

	TBOOL bWasStrafing = m_bWasStrafing;

	if ( bWasStrafing != pSteer->IsStrafingActive() )
	{
		// Go back to idle when the strafing ends
		if ( bWasStrafing && m_bAnimEnabled && !m_pAnimatableObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
			m_pAnimatableObject->PlayAnimation( TPS8( idle_lp ) );

		m_bWasStrafing = pSteer->IsStrafingActive();
	}

	if ( !m_bWasStrafing || ( fSpeed < 0.01f && !pSteer->IsTurning() && !pSteer->IsRolling() ) )
	{
		// Not strafing, stop the milk running animations
		// NOTE: squirt is checked twice, so one of the milk_run or combat roll animations has to play
		if ( fSpeed < 0.01f && m_bAnimEnabled &&
		     !m_pAnimatableObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) &&
		     !m_pAnimatableObject->IsAnimPlaying( TPS8( squirt ), 0.0f ) &&
		     ( m_pAnimatableObject->IsAnimPlaying( TPS8( squirt ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_run_n ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_run_ne ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_run_e ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_run_se ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_run_s ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_run_sw ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_run_w ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_run_nw ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_combatroll_l ), 0.0f ) ||
		       m_pAnimatableObject->IsAnimPlaying( TPS8( milk_combatroll_r ), 0.0f ) ) )
		{
			m_pAnimatableObject->PlayAnimation( TPS8( idle_lp ) );
		}

		ACharacterAnimController::OnUpdate( a_fDeltaTime );
		return;
	}

	TVector4 vecFacing   = *pSteer->GetKinematics()->GetFacingDir();
	TVector4 vecVelocity = *pSteer->GetDesiredVelocity();
	TVector4 vecPosition = *pSteer->GetPosition();
	TINT     iDirection  = GetDirectionIndex( vecFacing, vecVelocity );

	TBOOL bUpdateAnimation;

	if ( !pSteer->IsRolling() )
	{
		if ( fSpeed < 0.01f && pSteer->IsTurning() )
		{
			// Turning on the spot
			pAnimName = ( pSteer->GetFacingAngle() <= 0.03f ) ? &TPS8( idle_lp ) : &TPS8( walk_lp );
		}
		else
		{
			pAnimName = ms_apRunAnims[ iDirection ];
		}

		bUpdateAnimation = m_bAnimEnabled;
	}
	else if ( !m_bRolling )
	{
		// Start a combat roll to the side, the character faces the side it rolls to
		if ( iDirection == 2 )
		{
			vecFacing = vecVelocity;
			vecFacing.AsVector3().RotateY( -TMath::HALF_PI );
			m_pRollAnim = &TPS8( milk_combatroll_r );
		}
		else if ( iDirection == 6 )
		{
			vecFacing = vecVelocity;
			vecFacing.AsVector3().RotateY( TMath::HALF_PI );
			m_pRollAnim = &TPS8( milk_combatroll_l );
		}
		else
		{
			ACharacterAnimController::OnUpdate( a_fDeltaTime );
			return;
		}

		bUpdateAnimation = m_bAnimEnabled;
	}
	else
	{
		bUpdateAnimation = m_bAnimEnabled;

		if ( !m_pAnimatableObject->IsAnimationPlanned( TPS8( milk_combatroll_r ) ) &&
		     !m_pAnimatableObject->IsAnimationPlanned( TPS8( milk_combatroll_l ) ) &&
		     !m_pAnimatableObject->IsAnimPlaying( TPS8( milk_combatroll_r ), 0.0f ) &&
		     !m_pAnimatableObject->IsAnimPlaying( TPS8( milk_combatroll_l ), 0.0f ) &&
		     !m_bAnimEnabled )
		{
			// The roll is over
			m_bRolling = TFALSE;

			AToshiAnimationRef* pAnimRef = rAnimInterface.GetAnim();

			if ( pAnimRef )
				rAnimInterface.SetAnimSpeed( pAnimRef, 1.0f );

			pSteer->StopRoll();
		}
	}

	if ( bUpdateAnimation )
	{
		if ( pSteer->IsRolling() )
		{
			if ( m_pAnimatableObject->IsAnimPlaying( *pAnimName, 0.0f ) || m_bRolling )
			{
				AToshiAnimationRef* pRollRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), *m_pRollAnim );

				if ( pRollRef && rAnimInterface.IsAnimPlaying( *m_pRollAnim, TFALSE ) == TTRUE )
					rAnimInterface.SetAnimSpeed( pRollRef, 0.97f );

				if ( m_pAnimatableObject->IsAnimationFinished( *m_pRollAnim ) )
				{
					m_bRolling = TFALSE;

					AToshiAnimationRef* pAnimRef = rAnimInterface.GetAnim();

					if ( pAnimRef )
						rAnimInterface.SetAnimSpeed( pAnimRef, 1.0f );

					pSteer->StopRoll();
				}
			}
			else
			{
				m_bRolling = TTRUE;
				m_pAnimatableObject->PlayAnimation( *m_pRollAnim );
			}
		}
		else
		{
			// Play the movement animation at the speed of the steer
			const ASteerTraits* pSteerTraits   = pUnit->GetSteerTraits();
			TFLOAT              fAnimSpeed     = pSteer->GetMeasuredSpeed();
			TFLOAT              fAnimatedSpeed = pSteerTraits->GetAnimatedSpeed( pSteer->GetMovementType() );
			ASteerKinematics*   pKinematics    = pSteer->GetKinematics();

			// Moving backwards
			if ( pKinematics && pKinematics->IsA( &TGetClass( ASteerKinematics3D ) ) && pKinematics->GetDesiredSpeed() < 0.0f )
				fAnimSpeed = fAnimSpeed * -1.0f;

			fAnimSpeed = fAnimSpeed / fAnimatedSpeed;

			if ( !rAnimInterface.IsAnimPlaying( *pAnimName, TFALSE ) )
				rAnimInterface.PlayAnim( *pAnimName, -1.0f, TFALSE );

			AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), *pAnimName );

			if ( pAnimRef && rAnimInterface.IsAnimPlaying( *pAnimName, TFALSE ) == TTRUE )
			{
				// Don't let the run animation play too fast
				if ( pSteerTraits->m_fMaxAesRunSpeedRatio > 0.0f && fAnimSpeed > pSteerTraits->m_fMaxAesRunSpeedRatio )
					fAnimSpeed = pSteerTraits->m_fMaxAesRunSpeedRatio;

				if ( !pSteer->IsTurning() )
					rAnimInterface.SetAnimSpeed( pAnimRef, fAnimSpeed );
			}
		}
	}

	m_pAnimatableObject->GetModelInstance()->SetTransform( vecPosition, vecFacing );
}

// $Barnyard: FUNCTION 00585820
void AAlienAnimController::SetAnimEnabled( TBOOL a_bValue )
{
	if ( !m_bAnimEnabled )
	{
		m_bAnimEnabled = a_bValue;
		return;
	}

	if ( !a_bValue )
	{
		if ( !m_pAnimatableObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
			m_pAnimatableObject->PlayAnimation( TPS8( idle_lp ) );

		if ( m_bRolling )
		{
			// Stop the combat roll
			m_bRolling = TFALSE;

			AToshiAnimationRef* pAnimRef = GetAnimInterface().GetAnim();

			if ( pAnimRef )
				GetAnimInterface().SetAnimSpeed( pAnimRef, 1.0f );

			TSTATICCAST( AAlienSteer, TSTATICCAST( AUnit, m_pOwner )->GetSteer() )->StopRoll();
			m_bAnimEnabled = TFALSE;
			return;
		}
	}

	m_bAnimEnabled = a_bValue;
}

// $Barnyard: FUNCTION 005855e0
TINT AAlienAnimController::GetDirectionIndex( const TVector4& a_rcFacing, const TVector4& a_rcMovement )
{
	static TFLOAT s_fAngleToIndex = 2.546479f; // 8 / PI

	TVector4 vecCross;
	vecCross.CrossProduct( a_rcFacing, a_rcMovement );

	TFLOAT fAngle = a_rcFacing.GetAngleBetweenVector( a_rcMovement );

	if ( vecCross.y < 0.0f )
		fAngle = 6.2831855f - fAngle;

	TINT iIndex = TMath::FloorToInt( ( s_fAngleToIndex * fAngle + 1.0f ) * 0.5f );

	if ( iIndex > 7 )
		iIndex = 0;

	return iIndex;
}
