#include "pch.h"
#include "ABikeAnimController.h"
#include "AAnimatableObject.h"
#include "AAnimatableObjectManager.h"
#include "AModel.h"
#include "ANamedAnimationSet.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "Physics/APhysicsBike.h"

#include <Render/TAnimation.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005869c0, CreateObject 00586ba0, CreateObjectInPlace 00586c00
TDEFINE_CLASS( ABikeAnimController );

TPSTRING8_DECLARE( bk_ftdn );
TPSTRING8_DECLARE( bk_cst );
TPSTRING8_DECLARE( bk_slow );
TPSTRING8_DECLARE( bk_fast );
TPSTRING8_DECLARE( bk_slide_left );
TPSTRING8_DECLARE( bk_slide_right );
TPSTRING8_DECLARE( bk_base_idle );
TPSTRING8_DECLARE( bk_front_wheel_spin );
TPSTRING8_DECLARE( bk_rear_wheel_spin );
TPSTRING8_DECLARE( bk_front_wheel_turn_right );
TPSTRING8_DECLARE( bk_front_wheel_turn_left );
TPSTRING8_DECLARE( bk_pedal_spin );
TPSTRING8_DECLARE( bk_wheelie_pedal );
TPSTRING8_DECLARE( bk_wheelie );
TPSTRING8_DECLARE( bk_stoppie );
TPSTRING8_DECLARE( bk_flapping );
TPSTRING8_DECLARE( bk_flapping_left );
TPSTRING8_DECLARE( bk_flapping_right );
TPSTRING8_DECLARE( bk_simple_crash );
TPSTRING8_DECLARE( bk_reverse );
TPSTRING8_DECLARE( bk_jump_prep );
TPSTRING8_DECLARE( bk_jump_start );
TPSTRING8_DECLARE( bk_jump_mid );
TPSTRING8_DECLARE( bk_jump_end );
TPSTRING8_DECLARE( bk_trick_left_start );
TPSTRING8_DECLARE( bk_trick_left_loop );
TPSTRING8_DECLARE( bk_trick_left_end );
TPSTRING8_DECLARE( bk_trick_centre_start );
TPSTRING8_DECLARE( bk_trick_centre_loop );
TPSTRING8_DECLARE( bk_trick_centre_end );
TPSTRING8_DECLARE( bk_trick_right_start );
TPSTRING8_DECLARE( bk_trick_right_loop );
TPSTRING8_DECLARE( bk_trick_right_end );
TPSTRING8_DECLARE( ch_bikeaxl_1 );
TPSTRING8_DECLARE( body_loc );

// 0x00789d44, a wheel spin animation is one turn of the wheel
// NOTE: initialised at startup (0070b1e0) in the original
static const TFLOAT WHEEL_SPIN_PER_TURN = TMath::TWO_PI;

// 0x00789e08, steering angle of the full wheel turn animation
static constexpr TFLOAT MAX_WHEEL_TURN_ANGLE = 0.6981317f;

// 0x007750c8
static constexpr TFLOAT WHEEL_TURN_WEIGHT_SCALE = 1.0f;

// 0x007750cc, time the bike has to stand still before the rider puts a foot down
static constexpr TFLOAT FOOT_DOWN_DELAY = 0.25f;

// 0x007750d0, offset of the model from the steer position
static constexpr TVector4 MODEL_OFFSET = TVector4( 0.0f, -0.05f, -0.12f, 1.0f );

// 0x007750e4, the rider falls off the bike only below this speed
static constexpr TFLOAT CRASH_MAX_SPEED = 2.0f;

// 0x007750e0
static constexpr TFLOAT CRASH_MIN_IMPULSE = 70.0f;

// Does the animation set of the interface have the animation?
// NOTE: AAnimatableObject::HasAnimation (0057c850) in the original
static TBOOL HasAnimation( AToshiAnimationInterface& a_rAnimInterface, const TPString8& a_rcName )
{
	return a_rAnimInterface.GetAnimationSet()->GetNamedAnimation( a_rcName ) != TNULL;
}

// $Barnyard: FUNCTION 005869d0
ABikeAnimController::ABikeAnimController()
    : m_vecLastPosition( TVector4::VEC_ZERO )
    , m_fStoppedTime( 0.25f )
    , m_bPlayingCustomAnim( TFALSE )
    , m_bRestoreFastProgress( TFALSE )
    , m_fSlowProgress( 0.0f )
    , m_fWheelTurn( 0.0f )
{
}

// $Barnyard: FUNCTION 00586e00
// $Barnyard: FUNCTION 00586a20
ABikeAnimController::~ABikeAnimController()
{
}

// $Barnyard: FUNCTION 00586e20
TBOOL ABikeAnimController::OnCreate()
{
	m_bRestoreFastProgress = TFALSE;
	m_vecLastPosition      = TVector4::VEC_ZERO;

	return TTRUE;
}

// $Barnyard: FUNCTION 00588850
void ABikeAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	AUnit* pUnit = TSTATICCAST( AUnit, m_pOwner );

	if ( pUnit->IsDying() )
		return;

	// Give the control back once a riding animation plays again
	if ( m_bPlayingCustomAnim && IsMovementAnimPlaying() )
		m_bPlayingCustomAnim = TFALSE;

	UpdateTransform( a_fDeltaTime );

	if ( !m_bPlayingCustomAnim )
	{
		UpdateRiderAnimation( a_fDeltaTime );
		UpdateFlappingAnimation( a_fDeltaTime );
		UpdateBikeAnimations( a_fDeltaTime );
	}

	m_vecLastPosition = *pUnit->GetSteer()->GetPosition();
}

// $Barnyard: FUNCTION 00586a90
TBOOL ABikeAnimController::Unknown3( void* )
{
	return m_bPlayingCustomAnim;
}

// $Barnyard: FUNCTION 00586aa0
TBOOL ABikeAnimController::IsAnimationAllowed( ANamedAnimation* a_pNamedAnimation )
{
	if ( !m_bPlayingCustomAnim )
		m_bPlayingCustomAnim = TTRUE;

	return TTRUE;
}

// $Barnyard: FUNCTION 00586a30
TBOOL ABikeAnimController::IsMovementAnimPlaying()
{
	AToshiAnimationInterface& rAnimInterface = m_pAnimatableObject->GetToshiAnimInterface();

	return rAnimInterface.IsAnimPlaying( TPS8( bk_ftdn ), TFALSE ) ||
	    rAnimInterface.IsAnimPlaying( TPS8( bk_cst ), TFALSE ) ||
	    rAnimInterface.IsAnimPlaying( TPS8( bk_slow ), TFALSE ) ||
	    rAnimInterface.IsAnimPlaying( TPS8( bk_fast ), TFALSE );
}

// $Barnyard: FUNCTION 00586ea0
void ABikeAnimController::UpdateTransform( TFLOAT a_fDeltaTime )
{
	ABikeSteer*   pSteer   = TSTATICCAST( ABikeSteer, TSTATICCAST( AUnit, m_pOwner )->GetSteer() );
	APhysicsBike* pPhysics = pSteer->GetPhysics();

	TMatrix44 matTransform;
	matTransform.SetFromQuaternion( pPhysics->GetBody().m_qOrientation );
	matTransform.GetTranslation() = *pSteer->GetPosition();

	AToshiAnimationInterface& rAnimInterface = m_pAnimatableObject->GetToshiAnimInterface();

	ANamedAnimation* pSlideLeft  = rAnimInterface.GetAnimationSet()->GetNamedAnimation( TPS8( bk_slide_left ) );
	ANamedAnimation* pSlideRight = rAnimInterface.GetAnimationSet()->GetNamedAnimation( TPS8( bk_slide_right ) );

	TSkeletonInstance* pSkeletonInstance = m_pAnimatableObject->GetModelInstance()->GetSkeletonInstance();

	TAnimation* pSlideLeftAnim  = pSkeletonInstance->GetAnimation( pSlideLeft->GetSequenceId() );
	TAnimation* pSlideRightAnim = pSkeletonInstance->GetAnimation( pSlideRight->GetSequenceId() );

	TFLOAT fSlideWeight;
	TBOOL  bSliding = TTRUE;

	if ( pSlideLeftAnim )
		fSlideWeight = pSlideLeftAnim->GetWeight();
	else if ( pSlideRightAnim )
		fSlideWeight = pSlideRightAnim->GetWeight();
	else
		bSliding = TFALSE;

	// Straighten the bike up while the rider slides
	if ( bSliding && fSlideWeight > 0.0f )
	{
		TVector4& rRight   = matTransform.AsBasisVector4( BASISVECTOR_RIGHT );
		TVector4& rUp      = matTransform.AsBasisVector4( BASISVECTOR_UP );
		TVector4& rForward = matTransform.AsBasisVector4( BASISVECTOR_FORWARD );

		TVector4 vUp = TVector4::VEC_POSY;
		vUp.Lerp4( rUp, vUp, fSlideWeight );

		rRight.CrossProduct( vUp, rForward );
		rRight.w = 0.0f;
		rRight.Normalise();
		rUp.CrossProduct( rForward, rRight );
		rUp.Normalise();
	}

	TVector4 vOffset;
	TMatrix44::RotateVector( vOffset, matTransform, MODEL_OFFSET );

	TVector4& rTranslation = matTransform.GetTranslation();
	rTranslation.x         = rTranslation.x + vOffset.x;
	rTranslation.y         = rTranslation.y + vOffset.y;
	rTranslation.z         = rTranslation.z + vOffset.z;

	matTransform.RotateY( TMath::PI );
	matTransform.RotateX( 1.5707964f );

	m_pAnimatableObject->GetModelInstance()->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 00587180
void ABikeAnimController::UpdateBikeAnimations( TFLOAT a_fDeltaTime )
{
	// Models with the bike built in have no separate bike to animate
	if ( m_pAnimatableObject->GetModelInstance()->GetBoneID( TPS8( ch_bikeaxl_1 ) ) != -1 )
		return;

	AAnimatableObject* pBike = AAnimatableObjectManager::GetSingleton()->GetAttachedObjectByBoneName( m_pAnimatableObject, TPS8( body_loc ), 0 );

	if ( !pBike )
		return;

	AToshiAnimationInterface& rBikeAnims  = pBike->GetToshiAnimInterface();
	AToshiAnimationInterface& rRiderAnims = m_pAnimatableObject->GetToshiAnimInterface();
	ABikeSteer*               pSteer      = TSTATICCAST( ABikeSteer, TSTATICCAST( AUnit, m_pOwner )->GetSteer() );

	if ( !HasAnimation( rBikeAnims, TPS8( bk_base_idle ) ) )
		return;

	if ( !rBikeAnims.IsAnimPlaying( TPS8( bk_base_idle ), TFALSE ) )
		rBikeAnims.PlayAnim( TPS8( bk_base_idle ), -1.0f, TFALSE );

	// Wheels
	if ( AToshiAnimationInterface::GetAnimationRef( rBikeAnims.GetOverlayAnimRefs(), TPS8( bk_front_wheel_spin ) ) )
		rBikeAnims.SetAnimSpeed( rBikeAnims.FindAnimationRef( TPS8( bk_front_wheel_spin ) ), pSteer->GetPhysics()->m_aWheels[ 0 ].m_fAngularVelocity / WHEEL_SPIN_PER_TURN );
	else
		rBikeAnims.PlayAnim( TPS8( bk_front_wheel_spin ), -1.0f, TFALSE );

	if ( AToshiAnimationInterface::GetAnimationRef( rBikeAnims.GetOverlayAnimRefs(), TPS8( bk_rear_wheel_spin ) ) )
		rBikeAnims.SetAnimSpeed( rBikeAnims.FindAnimationRef( TPS8( bk_rear_wheel_spin ) ), pSteer->GetPhysics()->m_aWheels[ 1 ].m_fAngularVelocity / WHEEL_SPIN_PER_TURN );
	else
		rBikeAnims.PlayAnim( TPS8( bk_rear_wheel_spin ), -1.0f, TFALSE );

	// Steering
	m_fWheelTurn = TMath::LERP( m_fWheelTurn, pSteer->GetPhysics()->m_fSteeringAngle, 0.75f );

	if ( AToshiAnimationInterface::GetAnimationRef( rBikeAnims.GetOverlayAnimRefs(), TPS8( bk_front_wheel_turn_right ) ) )
	{
		TFLOAT fWeight = 0.0f;

		if ( m_fWheelTurn < 0.0f )
			fWeight = TMath::Abs( m_fWheelTurn ) * ( TMath::Abs( m_fWheelTurn ) / MAX_WHEEL_TURN_ANGLE ) * WHEEL_TURN_WEIGHT_SCALE;

		rBikeAnims.SetAnimWeight( rBikeAnims.FindAnimationRef( TPS8( bk_front_wheel_turn_right ) ), fWeight );
	}
	else
	{
		rBikeAnims.PlayAnim( TPS8( bk_front_wheel_turn_right ), -1.0f, TFALSE );
	}

	if ( AToshiAnimationInterface::GetAnimationRef( rBikeAnims.GetOverlayAnimRefs(), TPS8( bk_front_wheel_turn_left ) ) )
	{
		TFLOAT fWeight = 0.0f;

		if ( m_fWheelTurn > 0.0f )
			fWeight = TMath::Abs( m_fWheelTurn ) * ( TMath::Abs( m_fWheelTurn ) / MAX_WHEEL_TURN_ANGLE ) * WHEEL_TURN_WEIGHT_SCALE;

		rBikeAnims.SetAnimWeight( rBikeAnims.FindAnimationRef( TPS8( bk_front_wheel_turn_left ) ), fWeight );
	}
	else
	{
		rBikeAnims.PlayAnim( TPS8( bk_front_wheel_turn_left ), -1.0f, TFALSE );
	}

	// Pedals follow the pedalling animation of the rider
	TFLOAT fPedalSpeed    = 0.0f;
	TFLOAT fPedalProgress = 0.0f;

	if ( !AToshiAnimationInterface::GetAnimationRef( rBikeAnims.GetOverlayAnimRefs(), TPS8( bk_pedal_spin ) ) )
	{
		rBikeAnims.PlayAnim( TPS8( bk_pedal_spin ), -1.0f, TFALSE );
	}
	else
	{
		AToshiAnimationRef* pRiderRef = TNULL;

		if ( AToshiAnimationInterface::GetAnimationRef( rRiderAnims.GetAnimRefs(), TPS8( bk_slow ) ) )
			pRiderRef = rRiderAnims.FindAnimationRef( TPS8( bk_slow ) );
		else if ( AToshiAnimationInterface::GetAnimationRef( rRiderAnims.GetAnimRefs(), TPS8( bk_fast ) ) )
			pRiderRef = rRiderAnims.FindAnimationRef( TPS8( bk_fast ) );
		else if ( AToshiAnimationInterface::GetAnimationRef( rRiderAnims.GetAnimRefs(), TPS8( bk_wheelie_pedal ) ) )
			pRiderRef = rRiderAnims.FindAnimationRef( TPS8( bk_wheelie_pedal ) );

		if ( pRiderRef )
		{
			fPedalSpeed    = rRiderAnims.GetAnimSpeed( pRiderRef );
			fPedalProgress = rRiderAnims.GetAnimProgress( pRiderRef );
		}
		else
		{
			fPedalSpeed    = 0.0f;
			fPedalProgress = 0.0f;
		}
	}

	AToshiAnimationRef* pPedalRef = AToshiAnimationInterface::GetAnimationRef( rBikeAnims.GetAnimRefs(), TPS8( bk_pedal_spin ) );

	if ( pPedalRef || ( pPedalRef = AToshiAnimationInterface::GetAnimationRef( rBikeAnims.GetOverlayAnimRefs(), TPS8( bk_pedal_spin ) ) ) )
	{
		rBikeAnims.SetAnimSpeed( pPedalRef, fPedalSpeed );
		rBikeAnims.SetAnimProgress( pPedalRef, fPedalProgress );
	}

	// Flapping
	if ( HasAnimation( rBikeAnims, TPS8( bk_flapping ) ) )
	{
		TFLOAT fWeight = pSteer->GetSpeed() / pSteer->GetPhysics()->m_oParams.fMaxBoostSpeed;
		TMath::Clip( fWeight, 0.0f, 1.0f );

		if ( AToshiAnimationInterface::GetAnimationRef( rBikeAnims.GetOverlayAnimRefs(), TPS8( bk_flapping ) ) )
		{
			rBikeAnims.SetAnimWeight( rBikeAnims.FindAnimationRef( TPS8( bk_flapping ) ), fWeight );
		}
		else
		{
			rBikeAnims.PlayAnim( TPS8( bk_flapping ), fWeight, TFALSE );
			rBikeAnims.StopAnim( TPS8( bk_flapping_left ) );
			rBikeAnims.StopAnim( TPS8( bk_flapping_right ) );
		}
	}
}

// $Barnyard: FUNCTION 00587640
void ABikeAnimController::UpdateFlappingAnimation( TFLOAT a_fDeltaTime )
{
	AToshiAnimationInterface& rAnims = m_pAnimatableObject->GetToshiAnimInterface();

	if ( !HasAnimation( rAnims, TPS8( bk_flapping ) ) )
		return;

	ABikeSteer* pSteer = TSTATICCAST( ABikeSteer, TSTATICCAST( AUnit, m_pOwner )->GetSteer() );

	// The rider only flaps while coasting
	if ( !rAnims.IsAnimPlaying( TPS8( bk_cst ), TFALSE ) )
	{
		rAnims.StopAnim( TPS8( bk_flapping ) );
		rAnims.StopAnim( TPS8( bk_flapping_left ) );
		rAnims.StopAnim( TPS8( bk_flapping_right ) );
		return;
	}

	APhysicsBike*       pPhysics = pSteer->GetPhysics();
	AToshiAnimationRef* pRef;
	TFLOAT              fWeight;

	if ( TMath::Abs( pPhysics->m_fSteering ) >= 0.05f )
	{
		if ( pPhysics->m_fSteering <= 0.05f )
		{
			fWeight = pSteer->GetSpeed() / pPhysics->m_oParams.fMaxBoostSpeed;
			TMath::Clip( fWeight, 0.0f, 1.0f );

			if ( !AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_flapping_right ) ) )
			{
				rAnims.PlayAnim( TPS8( bk_flapping_right ), fWeight, TFALSE );
				rAnims.StopAnim( TPS8( bk_flapping ) );
				rAnims.StopAnim( TPS8( bk_flapping_left ) );
				return;
			}

			pRef = rAnims.FindAnimationRef( TPS8( bk_flapping_right ) );
		}
		else
		{
			fWeight = pSteer->GetSpeed() / pPhysics->m_oParams.fMaxBoostSpeed;
			TMath::Clip( fWeight, 0.0f, 1.0f );

			if ( !AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_flapping_left ) ) )
			{
				rAnims.PlayAnim( TPS8( bk_flapping_left ), fWeight, TFALSE );
				rAnims.StopAnim( TPS8( bk_flapping ) );
				rAnims.StopAnim( TPS8( bk_flapping_right ) );
				return;
			}

			pRef = rAnims.FindAnimationRef( TPS8( bk_flapping_left ) );
		}
	}
	else
	{
		fWeight = pSteer->GetSpeed() / pPhysics->m_oParams.fMaxBoostSpeed;
		TMath::Clip( fWeight, 0.0f, 1.0f );

		if ( !AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_flapping ) ) )
		{
			rAnims.PlayAnim( TPS8( bk_flapping ), fWeight, TFALSE );
			rAnims.StopAnim( TPS8( bk_flapping_left ) );
			rAnims.StopAnim( TPS8( bk_flapping_right ) );
			return;
		}

		pRef = rAnims.FindAnimationRef( TPS8( bk_flapping ) );
	}

	rAnims.SetAnimWeight( pRef, fWeight );
}

// $Barnyard: FUNCTION 00587880
void ABikeAnimController::UpdateRiderAnimation( TFLOAT a_fDeltaTime )
{
	AUnit*                    pUnit    = TSTATICCAST( AUnit, m_pOwner );
	ABikeSteer*               pSteer   = TSTATICCAST( ABikeSteer, pUnit->GetSteer() );
	AToshiAnimationInterface& rAnims   = m_pAnimatableObject->GetToshiAnimInterface();
	APhysicsBike*             pPhysics = pSteer->GetPhysics();

	TFLOAT fSpeed     = pSteer->GetSpeed();
	TFLOAT fAnimSpeed = fSpeed / pUnit->GetSteerTraits()->GetAnimatedSpeed( ASteer::MOVEMENT_WALK ) * 0.14f;

	if ( AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_simple_crash ) ) ||
	     AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_simple_crash ) ) )
	{
		return;
	}

	// Tricks
	if ( HasAnimation( rAnims, TPS8( bk_trick_left_start ) ) )
	{
		const TPString8* pTrickStart;
		const TPString8* pTrickLoop;
		const TPString8* pTrickEnd;

		if ( pSteer->GetTrickDirection() == 1 )
		{
			pTrickStart = &TPS8( bk_trick_left_start );
			pTrickLoop  = &TPS8( bk_trick_left_loop );
			pTrickEnd   = &TPS8( bk_trick_left_end );
		}
		else if ( pSteer->GetTrickDirection() == 2 )
		{
			pTrickStart = &TPS8( bk_trick_centre_start );
			pTrickLoop  = &TPS8( bk_trick_centre_loop );
			pTrickEnd   = &TPS8( bk_trick_centre_end );
		}
		else
		{
			pTrickStart = &TPS8( bk_trick_right_start );
			pTrickLoop  = &TPS8( bk_trick_right_loop );
			pTrickEnd   = &TPS8( bk_trick_right_end );
		}

		switch ( pSteer->GetTrickState() )
		{
			case ABikeSteer::TRICKSTATE_NONE:
				if ( pSteer->IsTrickHeld() == TTRUE && pSteer->IsAirborne() )
				{
					rAnims.PlayAnim( *pTrickStart, -1.0f, TFALSE );
					pSteer->SetTrickState( ABikeSteer::TRICKSTATE_START );
					return;
				}

				break;
			case ABikeSteer::TRICKSTATE_START:
			{
				AToshiAnimationRef* pStartRef = rAnims.FindAnimationRef( *pTrickStart );

				if ( !pStartRef || rAnims.IsAnimFinishing( pStartRef, a_fDeltaTime ) == TTRUE )
				{
					rAnims.PlayAnim( *pTrickLoop, -1.0f, TFALSE );
					pSteer->SetTrickState( ABikeSteer::TRICKSTATE_LOOP );
				}

				return;
			}
			case ABikeSteer::TRICKSTATE_LOOP:
				if ( !pSteer->IsTrickHeld() )
				{
					rAnims.PlayAnim( *pTrickEnd, -1.0f, TFALSE );
					pSteer->SetTrickState( ABikeSteer::TRICKSTATE_END );
				}

				return;
			case ABikeSteer::TRICKSTATE_END:
				if ( pPhysics->m_iNumWheelsOnGround > 0 )
				{
					rAnims.PlayAnim( TPS8( bk_jump_end ), -1.0f, TFALSE );
					pSteer->SetTrickState( ABikeSteer::TRICKSTATE_NONE );
					return;
				}

				if ( rAnims.FindAnimationRef( *pTrickEnd ) )
					return;

				rAnims.PlayAnim( TPS8( bk_jump_mid ), -1.0f, TFALSE );
				pSteer->SetTrickState( ABikeSteer::TRICKSTATE_NONE );
				break;
		}
	}

	// Jumps
	AToshiAnimationRef* pJumpPrep = AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_jump_prep ) );
	if ( !pJumpPrep ) pJumpPrep = AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_jump_prep ) );

	AToshiAnimationRef* pJumpStart = AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_jump_start ) );
	if ( !pJumpStart ) pJumpStart = AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_jump_start ) );

	AToshiAnimationRef* pJumpMid = AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_jump_mid ) );
	if ( !pJumpMid ) pJumpMid = AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_jump_mid ) );

	AToshiAnimationRef* pJumpEnd = AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_jump_end ) );
	if ( !pJumpEnd ) pJumpEnd = AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_jump_end ) );

	if ( pJumpPrep || pJumpStart || pJumpEnd )
		return;

	if ( pJumpMid )
	{
		if ( pPhysics->m_iNumWheelsOnGround > 0 )
			rAnims.PlayAnim( TPS8( bk_jump_end ), -1.0f, TFALSE );

		return;
	}

	if ( pSteer->IsJumping() == TTRUE )
	{
		rAnims.PlayAnim( TPS8( bk_jump_start ), -1.0f, TFALSE );
		return;
	}

	// Foot down when standing still
	if ( TMath::Abs( pSteer->GetSpeed() ) < 0.1f && ( pSteer->WasAirborne() || pPhysics->m_bFrozen ) && !pPhysics->IsWheelie() )
	{
		m_fStoppedTime += a_fDeltaTime;

		if ( m_fStoppedTime > FOOT_DOWN_DELAY && !rAnims.IsAnimPlaying( TPS8( bk_ftdn ), TFALSE ) )
			rAnims.PlayAnim( TPS8( bk_ftdn ), -1.0f, TFALSE );

		return;
	}

	m_fStoppedTime = 0.0f;

	// Is the rider pedalling?
	TBOOL bCoasting = TTRUE;

	if ( !pSteer->WasAirborne() )
	{
		if ( pPhysics->m_oGearBox.iGear != 0 )
		{
			if ( pPhysics->m_fBrake <= 0.0f && pPhysics->m_fThrottle > 0.0f )
				bCoasting = TFALSE;
		}
		else
		{
			if ( pPhysics->m_fThrottle <= 0.0f && pPhysics->m_fBrake > 0.0f )
				bCoasting = TFALSE;
		}
	}

	TBOOL bWheelie      = TFALSE;
	TBOOL bPedalWheelie = TFALSE;

	if ( pPhysics->IsWheelie() )
	{
		if ( pPhysics->m_oGearBox.iGear != 0 && pPhysics->m_fThrottle > 0.0f && fSpeed > 0.0f )
			bPedalWheelie = TTRUE;
		else
			bWheelie = TTRUE;
	}

	// Fall off the bike when hitting something head on
	TBOOL bCrash = TFALSE;

	if ( HasAnimation( rAnims, TPS8( bk_simple_crash ) ) && pPhysics->m_iNumContacts > 0 )
	{
		for ( TINT i = 0; i < pPhysics->m_iNumContacts; i++ )
		{
			APhysicsVehicleContact* pContact = pPhysics->GetContact( i );

			// NOTE: the original normalises a copy of the relative velocity and doesn't use it
			TVector4 vRelativeVelocity = pContact->vRelativeVelocity;
			vRelativeVelocity.Normalise();

			TFLOAT fDot = TVector4::DotProduct3( pContact->vNormal, pPhysics->GetBody().GetTransform().AsBasisVector4( BASISVECTOR_FORWARD ) );

			if ( pPhysics->GetBody().GetVelocity().Magnitude() < CRASH_MAX_SPEED && fDot < -0.85000002f && pContact->fImpulse > CRASH_MIN_IMPULSE )
			{
				bCrash = TTRUE;
				break;
			}
		}
	}

	if ( pSteer->IsSkidding() == TTRUE )
	{
		if ( !AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_slide_left ) ) &&
		     !AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_slide_right ) ) )
		{
			TFLOAT fTurn = TVector4::DotProduct3( pPhysics->GetBody().m_vAngularVelocity, pPhysics->GetBody().GetTransform().AsBasisVector4( BASISVECTOR_UP ) );

			TPString8 strSlide = TPS8( bk_slide_left );

			if ( fTurn > 0.0f )
				strSlide = TPS8( bk_slide_right );

			rAnims.PlayAnim( strSlide, -1.0f, TFALSE );
		}

		return;
	}

	if ( pPhysics->IsStoppie() == TTRUE )
	{
		if ( !AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_stoppie ) ) )
			rAnims.PlayAnim( TPS8( bk_stoppie ), -1.0f, TFALSE );

		return;
	}

	if ( bWheelie == TTRUE )
	{
		if ( pPhysics->m_fThrottle > 0.0f && AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_wheelie_pedal ) ) )
			rAnims.SetAnimSpeed( AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_wheelie_pedal ) ), 0.0f );
		else if ( !AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_wheelie ) ) )
			rAnims.PlayAnim( TPS8( bk_wheelie ), -1.0f, TFALSE );

		return;
	}

	if ( bPedalWheelie == TTRUE )
	{
		AToshiAnimationRef* pRef = AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_wheelie_pedal ) );

		if ( !pRef )
		{
			rAnims.PlayAnim( TPS8( bk_wheelie_pedal ), -1.0f, TFALSE );
			return;
		}

		rAnims.SetAnimSpeed( pRef, fAnimSpeed );
		return;
	}

	if ( bCrash == TTRUE )
	{
		rAnims.PlayAnim( TPS8( bk_simple_crash ), -1.0f, TFALSE );
		pSteer->PlayCrashSound();
		return;
	}

	if ( bCoasting == TTRUE )
	{
		if ( !AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_cst ) ) )
			rAnims.PlayAnim( TPS8( bk_cst ), -1.0f, TFALSE );

		return;
	}

	if ( fSpeed < 0.0f )
	{
		if ( HasAnimation( rAnims, TPS8( bk_reverse ) ) && !AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_reverse ) ) )
			rAnims.PlayAnim( TPS8( bk_reverse ), -1.0f, TFALSE );

		return;
	}

	// Pedalling, fast while boosting
	if ( pPhysics->m_fThrottle <= 1.0f )
	{
		AToshiAnimationRef* pRef = AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_slow ) );

		if ( !pRef )
		{
			pRef = rAnims.PlayAnim( TPS8( bk_slow ), -1.0f, TFALSE );

			if ( !pRef )
				return;
		}

		rAnims.SetAnimSpeed( pRef, fAnimSpeed );
		return;
	}

	AToshiAnimationRef* pFastRef = AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_fast ) );

	if ( !pFastRef )
	{
		// Continue the pedalling where bk_slow is
		AToshiAnimationRef* pSlowRef = AToshiAnimationInterface::GetAnimationRef( rAnims.GetAnimRefs(), TPS8( bk_slow ) );

		if ( pSlowRef )
		{
			m_bRestoreFastProgress = TTRUE;
			m_fSlowProgress        = rAnims.GetAnimProgress( pSlowRef );
		}

		pFastRef = rAnims.PlayAnim( TPS8( bk_fast ), -1.0f, TFALSE );

		if ( !pFastRef )
			return;
	}

	TFLOAT fFastSpeed = ( fSpeed - pPhysics->m_oParams.fMaxSpeed ) / ( pPhysics->m_oParams.fMaxBoostSpeed - pPhysics->m_oParams.fMaxSpeed );
	TMath::Clip( fFastSpeed, 0.0f, 1.0f );
	fFastSpeed = fFastSpeed * 1.4000001f;

	rAnims.SetAnimSpeed( pFastRef, fFastSpeed );

	if ( m_bRestoreFastProgress == TTRUE )
	{
		rAnims.SetAnimProgress( pFastRef, m_fSlowProgress );
		m_bRestoreFastProgress = TFALSE;
	}
}
