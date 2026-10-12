#include "pch.h"
#include "ABikeCrashAnimController.h"
#include "AAnimatableObject.h"
#include "Units/AUnit.h"
#include "AModel.h"
#include "ANamedAnimationSet.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "Physics/APhysicsBike.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00586ac0, CreateObject 00586d00, CreateObjectInPlace 00586d70
TDEFINE_CLASS( ABikeCrashAnimController );

TPSTRING8_DECLARE( bk_base_idle );
TPSTRING8_DECLARE( bk_front_wheel_spin );
TPSTRING8_DECLARE( bk_rear_wheel_spin );
TPSTRING8_DECLARE( bk_front_wheel_turn_right );
TPSTRING8_DECLARE( bk_front_wheel_turn_left );
TPSTRING8_DECLARE( bk_flapping );
TPSTRING8_DECLARE( bk_flapping_left );
TPSTRING8_DECLARE( bk_flapping_right );

// 0x00789d44, a wheel spin animation is one turn of the wheel
// NOTE: initialised at startup (0070b1e0) in the original
static const TFLOAT WHEEL_SPIN_PER_TURN = TMath::TWO_PI;

// 0x00789e08, steering angle of the full wheel turn animation (initialised at startup, 0070b1d0)
static constexpr TFLOAT MAX_WHEEL_TURN_ANGLE = 0.6981317f;

// 0x007750c8
static constexpr TFLOAT WHEEL_TURN_WEIGHT_SCALE = 1.0f;

// 0x007750d0, offset of the model from the steer position
static constexpr TVector4 MODEL_OFFSET = TVector4( 0.0f, -0.05f, -0.12f, 1.0f );

ABikeCrashAnimController::ABikeCrashAnimController()
    : m_fWheelTurn( 0.0f )
{
}

// $Barnyard: FUNCTION 00586e80
// $Barnyard: FUNCTION 00586ad0
ABikeCrashAnimController::~ABikeCrashAnimController()
{
}

// NOTE: folded with ADartThrowAnimController::OnCreate (0058a350)
TBOOL ABikeCrashAnimController::OnCreate()
{
	AAnimController::OnCreate();
	return TTRUE;
}

// $Barnyard: FUNCTION 005888d0
void ABikeCrashAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	AUnit* pUnit = TSTATICCAST( AUnit, m_pOwner );

	if ( !pUnit || !pUnit->GetSteer() )
		return;

	UpdateBikeAnimations( a_fDeltaTime );
	UpdateTransform( a_fDeltaTime );

	if ( pUnit->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
		TSTATICCAST( ABikeSteer, pUnit->GetSteer() )->GetPhysics()->VTable0x1C();
}

// $Barnyard: FUNCTION 00588220
void ABikeCrashAnimController::UpdateBikeAnimations( TFLOAT a_fDeltaTime )
{
	ASteer* pUnitSteer = TSTATICCAST( AUnit, m_pOwner )->GetSteer();

	if ( !pUnitSteer->IsA( &TGetClass( ABikeSteer ) ) )
		return;

	AToshiAnimationInterface& rAnims   = m_pAnimatableObject->GetToshiAnimInterface();
	ABikeSteer*               pSteer   = TSTATICCAST( ABikeSteer, pUnitSteer );
	APhysicsBike*             pPhysics = pSteer->GetPhysics();

	if ( !rAnims.GetAnimationSet()->GetNamedAnimation( TPS8( bk_base_idle ) ) )
		return;

	if ( !rAnims.IsAnimPlaying( TPS8( bk_base_idle ), TFALSE ) )
		rAnims.PlayAnim( TPS8( bk_base_idle ), -1.0f, TFALSE );

	// No more steering and flapping once the rider has been thrown off
	if ( pPhysics->IsDriverless() == TTRUE )
	{
		rAnims.StopAnim( TPS8( bk_front_wheel_turn_right ) );
		rAnims.StopAnim( TPS8( bk_front_wheel_turn_left ) );
		rAnims.StopAnim( TPS8( bk_flapping ) );
		rAnims.StopAnim( TPS8( bk_flapping_left ) );
		rAnims.StopAnim( TPS8( bk_flapping_right ) );
		return;
	}

	// Wheels
	if ( AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_front_wheel_spin ) ) )
		rAnims.SetAnimSpeed( rAnims.FindAnimationRef( TPS8( bk_front_wheel_spin ) ), pPhysics->m_aWheels[ 0 ].m_fAngularVelocity / WHEEL_SPIN_PER_TURN );
	else
		rAnims.PlayAnim( TPS8( bk_front_wheel_spin ), -1.0f, TFALSE );

	if ( AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_rear_wheel_spin ) ) )
		rAnims.SetAnimSpeed( rAnims.FindAnimationRef( TPS8( bk_rear_wheel_spin ) ), pPhysics->m_aWheels[ 1 ].m_fAngularVelocity / WHEEL_SPIN_PER_TURN );
	else
		rAnims.PlayAnim( TPS8( bk_rear_wheel_spin ), -1.0f, TFALSE );

	// Steering
	m_fWheelTurn = TMath::LERP( m_fWheelTurn, pPhysics->m_fSteeringAngle, 0.75f );

	if ( AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_front_wheel_turn_right ) ) )
	{
		TFLOAT fWeight = 0.0f;

		if ( m_fWheelTurn < 0.0f )
			fWeight = TMath::Abs( m_fWheelTurn ) * ( TMath::Abs( m_fWheelTurn ) / MAX_WHEEL_TURN_ANGLE ) * WHEEL_TURN_WEIGHT_SCALE;

		rAnims.SetAnimWeight( rAnims.FindAnimationRef( TPS8( bk_front_wheel_turn_right ) ), fWeight );
	}
	else
	{
		rAnims.PlayAnim( TPS8( bk_front_wheel_turn_right ), -1.0f, TFALSE );
	}

	if ( AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_front_wheel_turn_left ) ) )
	{
		TFLOAT fWeight = 0.0f;

		if ( m_fWheelTurn > 0.0f )
			fWeight = TMath::Abs( m_fWheelTurn ) * ( TMath::Abs( m_fWheelTurn ) / MAX_WHEEL_TURN_ANGLE ) * WHEEL_TURN_WEIGHT_SCALE;

		rAnims.SetAnimWeight( rAnims.FindAnimationRef( TPS8( bk_front_wheel_turn_left ) ), fWeight );
	}
	else
	{
		rAnims.PlayAnim( TPS8( bk_front_wheel_turn_left ), -1.0f, TFALSE );
	}

	// Flapping
	if ( rAnims.GetAnimationSet()->GetNamedAnimation( TPS8( bk_flapping ) ) )
	{
		TFLOAT fWeight = pSteer->GetSpeed() / pPhysics->m_oParams.fMaxBoostSpeed;
		TMath::Clip( fWeight, 0.0f, 1.0f );

		if ( !AToshiAnimationInterface::GetAnimationRef( rAnims.GetOverlayAnimRefs(), TPS8( bk_flapping ) ) )
		{
			rAnims.PlayAnim( TPS8( bk_flapping ), fWeight, TFALSE );
			rAnims.StopAnim( TPS8( bk_flapping_left ) );
			rAnims.StopAnim( TPS8( bk_flapping_right ) );
			return;
		}

		rAnims.SetAnimWeight( rAnims.FindAnimationRef( TPS8( bk_flapping ) ), fWeight );
	}
}

// $Barnyard: FUNCTION 005885c0
void ABikeCrashAnimController::UpdateTransform( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = TSTATICCAST( AUnit, m_pOwner )->GetSteer();

	if ( pSteer->IsA( &TGetClass( ABikeSteer ) ) )
	{
		TMatrix44 matTransform;
		matTransform.SetFromQuaternion( TSTATICCAST( ABikeSteer, pSteer )->GetPhysics()->GetBody().m_qOrientation );
		matTransform.GetTranslation() = *pSteer->GetPosition();

		TVector4 vOffset;
		TMatrix44::RotateVector( vOffset, matTransform, MODEL_OFFSET );

		TVector4& rTranslation = matTransform.GetTranslation();
		rTranslation.x         = rTranslation.x + vOffset.x;
		rTranslation.y         = rTranslation.y + vOffset.y;
		rTranslation.z         = rTranslation.z + vOffset.z;

		matTransform.RotateY( TMath::PI );
		matTransform.RotateX( 1.5707964f );

		m_pAnimatableObject->GetModelInstance()->SetTransform( matTransform );
		return;
	}

	TVector4 vecPosition;
	pSteer->CopyPosition( vecPosition );

	TVector4 vecDirection;
	pSteer->CopyDirection( vecDirection );

	// The default up vector can't be used when facing straight up or down
	if ( vecDirection.y > 0.999f || vecDirection.y < -0.999f )
		m_pAnimatableObject->GetModelInstance()->SetTransformKeepRight( vecPosition, vecDirection );
	else
		m_pAnimatableObject->GetModelInstance()->SetTransform( vecPosition, vecDirection );
}
