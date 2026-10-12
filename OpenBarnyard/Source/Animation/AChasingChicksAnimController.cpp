#include "pch.h"
#include "AChasingChicksAnimController.h"
#include "AChasingChicksAnimControllerType.h"
#include "ACharacterAnimController.h"
#include "AAnimatableObject.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "UnitMovement/Steering/ASteerKinematics3D.h"
#include "Units/AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005896b0, CreateObject 00589ec0, CreateObjectInPlace 00589f40
TDEFINE_CLASS( AChasingChicksAnimController );

AChasingChicksAnimController::AChasingChicksAnimController()
    : m_bPlayingCustomAnim( TFALSE )
    , m_pAnimationSet( TNULL )
{
}

// $Barnyard: FUNCTION 00589fb0
// $Barnyard: FUNCTION 00589840
AChasingChicksAnimController::~AChasingChicksAnimController()
{
}

// $Barnyard: FUNCTION 00589fd0
TBOOL AChasingChicksAnimController::OnCreate()
{
	SetAnimationSet( TNULL );

	if ( TSTATICCAST( AChasingChicksAnimControllerType, m_pControllerType )->IsUserRender() )
		m_pAnimatableObject->GetModelInstance()->GetSceneObject()->GetInstance()->SetPreRenderCallback( ACharacterAnimController::UserRenderCallback );

	return TTRUE;
}

TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( cc_chick_held );

// $Barnyard: FUNCTION 0058a030
void AChasingChicksAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bPlayingCustomAnim )
	{
		AToshiAnimationInterface& rAnimInterface = m_pAnimatableObject->GetToshiAnimInterface();

		// Give the control back once a movement animation plays again
		if ( rAnimInterface.IsAnimPlaying( TPS8( idle_lp ), TFALSE ) ||
		     rAnimInterface.IsAnimPlaying( TPS8( walk_lp ), TFALSE ) ||
		     rAnimInterface.IsAnimPlaying( TPS8( run_lp ), TFALSE ) )
		{
			m_bPlayingCustomAnim = TFALSE;
		}
	}

	UpdateTransform( a_fDeltaTime );

	if ( !m_bPlayingCustomAnim )
		UpdateMovementAnimation( a_fDeltaTime );
}

// NOTE: folded with ACharacterAnimController::Unknown3 (005896d0)
TBOOL AChasingChicksAnimController::Unknown3( void* )
{
	return m_bPlayingCustomAnim;
}

// NOTE: folded with ACharacterAnimController::IsAnimationAllowed (00588c30)
TBOOL AChasingChicksAnimController::IsAnimationAllowed( ANamedAnimation* a_pNamedAnimation )
{
	if ( !m_bPlayingCustomAnim )
		m_bPlayingCustomAnim = TTRUE;

	return TTRUE;
}

// $Barnyard: FUNCTION 005898b0
void AChasingChicksAnimController::UpdateTransform( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = TSTATICCAST( AUnit, m_pOwner )->GetSteer();

	TVector4 vecPosition;
	pSteer->CopyPosition( vecPosition );

	TVector4 vecDirection;
	pSteer->CopyDirection( vecDirection );

	TFLOAT fPitch;
	TFLOAT fRoll;

	if ( pSteer->GetTilt( &fPitch, &fRoll ) )
		m_pAnimatableObject->GetModelInstance()->SetTransformTilted( vecPosition, vecDirection, fPitch, fRoll );
	else
		m_pAnimatableObject->GetModelInstance()->SetTransform( vecPosition, vecDirection );
}

// $Barnyard: FUNCTION 005899b0
void AChasingChicksAnimController::UpdateMovementAnimation( TFLOAT a_fDeltaTime )
{
	AUnit*              pUnit        = TSTATICCAST( AUnit, m_pOwner );
	const ASteerTraits* pSteerTraits = pUnit->GetSteerTraits();
	ASteer*             pSteer       = pUnit->GetSteer();

	TFLOAT           fSpeed         = pSteer->GetMeasuredSpeed();
	ASteer::Movement eMovement      = pSteer->GetMovementType();
	TFLOAT           fAnimatedSpeed = pSteerTraits->GetAnimatedSpeed( eMovement );

	ASteerKinematics* pKinematics = pSteer->GetKinematics();

	// Moving backwards
	if ( pKinematics && pKinematics->IsA( &TGetClass( ASteerKinematics3D ) ) && pKinematics->GetDesiredSpeed() < 0.0f )
		fSpeed *= -1.0f;

	TFLOAT fAnimSpeed = fSpeed / fAnimatedSpeed;

	TPString8 strRun  = TPS8( run_lp );
	TPString8 strWalk = TPS8( walk_lp );
	TPString8 strIdle = TPS8( idle_lp );

	AAnimatableObject*        pAnimatableObject = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject();
	AToshiAnimationInterface& rAnimInterface    = pAnimatableObject->GetToshiAnimInterface();
	AAnimatableObject*        pHeldChick        = pAnimatableObject->FindAttachedObjectOfType( TPS8( cc_chick_held ) );

	AToshiAnimationRef* pAnimRef = TNULL;

	if ( pHeldChick )
	{
		// The chick in the hands plays the same animation
		AToshiAnimationInterface& rHeldInterface = pHeldChick->GetToshiAnimInterface();
		AToshiAnimationRef*       pHeldAnimRef;

		if ( eMovement == ASteer::MOVEMENT_STOPPED )
		{
			pAnimRef     = rAnimInterface.PlayAnim( strIdle, -1.0f, TFALSE );
			pHeldAnimRef = rHeldInterface.PlayAnim( strIdle, -1.0f, TFALSE );

			if ( pAnimRef && pHeldAnimRef )
			{
				rHeldInterface.SetAnimTime( pHeldAnimRef, rAnimInterface.GetAnimTime( pAnimRef ) );
				rHeldInterface.SetAnimWeight( pHeldAnimRef, rAnimInterface.GetAnimWeight( pAnimRef ) );
			}

			return;
		}

		if ( eMovement == ASteer::MOVEMENT_WALK )
		{
			pAnimRef     = rAnimInterface.PlayAnim( strWalk, -1.0f, TFALSE );
			pHeldAnimRef = rHeldInterface.PlayAnim( strWalk, -1.0f, TFALSE );

			if ( !pAnimRef )
				return;

			rAnimInterface.SetAnimSpeed( pAnimRef, fAnimSpeed );
		}
		else if ( eMovement == ASteer::MOVEMENT_RUN )
		{
			pAnimRef     = rAnimInterface.PlayAnim( strRun, -1.0f, TFALSE );
			pHeldAnimRef = rHeldInterface.PlayAnim( strRun, -1.0f, TFALSE );

			if ( !pAnimRef )
				return;

			fAnimSpeed *= 0.5f;

			if ( 0.0f < pSteerTraits->m_fMaxAesRunSpeedRatio && pSteerTraits->m_fMaxAesRunSpeedRatio < fAnimSpeed )
				fAnimSpeed = pSteerTraits->m_fMaxAesRunSpeedRatio;

			rAnimInterface.SetAnimSpeed( pAnimRef, fAnimSpeed );
		}
		else
		{
			return;
		}

		if ( pHeldAnimRef )
			rHeldInterface.SetAnimSpeed( pHeldAnimRef, fAnimSpeed );

		return;
	}

	switch ( eMovement )
	{
		case ASteer::MOVEMENT_STOPPED:
			rAnimInterface.PlayAnim( strIdle, -1.0f, TFALSE );
			return;
		case ASteer::MOVEMENT_WALK:
			pAnimRef = rAnimInterface.PlayAnim( strWalk, -1.0f, TFALSE );

			if ( !pAnimRef )
				return;

			break;
		case ASteer::MOVEMENT_RUN:
			pAnimRef = rAnimInterface.PlayAnim( strRun, -1.0f, TFALSE );

			if ( !pAnimRef )
				return;

			fAnimSpeed *= 0.5f;

			if ( 0.0f < pSteerTraits->m_fMaxAesRunSpeedRatio && pSteerTraits->m_fMaxAesRunSpeedRatio < fAnimSpeed )
				fAnimSpeed = pSteerTraits->m_fMaxAesRunSpeedRatio;

			break;
		case ASteer::MOVEMENT_SPRINT:
			// NOTE: sprinting plays the run animation
			pAnimRef = rAnimInterface.PlayAnim( strRun, -1.0f, TFALSE );

			if ( !pAnimRef )
				return;

			if ( 0.0f < pSteerTraits->m_fMaxAesRunSpeedRatio && pSteerTraits->m_fMaxAesRunSpeedRatio < fAnimSpeed )
				fAnimSpeed = pSteerTraits->m_fMaxAesRunSpeedRatio;

			break;
		default:
			return;
	}

	rAnimInterface.SetAnimSpeed( pAnimRef, fAnimSpeed );
}

TPSTRING8_DECLARE( default );

// $Barnyard: FUNCTION 00589db0
void AChasingChicksAnimController::SetAnimationSet( const TPString8* a_pName )
{
	AAnimatableObjectType* pObjectType = m_pAnimatableObject->GetObjectType();

	if ( a_pName )
		m_pAnimationSet = pObjectType->FindAnimationSet( *a_pName );
	else
		m_pAnimationSet = pObjectType->FindAnimationSetOrDefault( TPS8( default ) );
}
