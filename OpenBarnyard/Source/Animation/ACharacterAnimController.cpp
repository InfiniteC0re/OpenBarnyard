#include "pch.h"
#include "ACharacterAnimController.h"
#include "ACharacterAnimControllerType.h"
#include "AAnimatableObject.h"
#include "AAnimatableObjectManager.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ASteerKinematics.h"
#include "UnitMovement/Steering/ASteerTraits.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00588940, CreateObject 005894f0, CreateObjectInPlace 00589570
TDEFINE_CLASS( ACharacterAnimController );

// $Barnyard: FUNCTION 00588d20
ACharacterAnimController::ACharacterAnimController()
    : m_bPlayingCustomAnim( TFALSE )
    , m_pAnimationSet( TNULL )
{
}

// $Barnyard: FUNCTION 005895e0
// $Barnyard: FUNCTION 00588d40
ACharacterAnimController::~ACharacterAnimController()
{
}

// $Barnyard: FUNCTION 00589600
TBOOL ACharacterAnimController::OnCreate()
{
	SetAnimationSet( TNULL );

	if ( TSTATICCAST( ACharacterAnimControllerType, m_pControllerType )->IsUserRender() )
		m_pAnimatableObject->GetModelInstance()->GetSceneObject()->GetInstance()->SetPreRenderCallback( UserRenderCallback );

	return TTRUE;
}

// $Barnyard: FUNCTION 00589660
void ACharacterAnimController::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Give the control back once a movement animation plays again
	if ( m_bPlayingCustomAnim && IsMovementAnimPlaying() )
		m_bPlayingCustomAnim = TFALSE;

	UpdateTransform( a_fDeltaTime );

	if ( !m_bPlayingCustomAnim )
		UpdateMovementAnimation( a_fDeltaTime );
}

// $Barnyard: FUNCTION 005896d0
TBOOL ACharacterAnimController::Unknown3( void* )
{
	return m_bPlayingCustomAnim;
}

// $Barnyard: FUNCTION 00588c30
TBOOL ACharacterAnimController::IsAnimationAllowed( ANamedAnimation* a_pNamedAnimation )
{
	if ( !m_bPlayingCustomAnim )
		m_bPlayingCustomAnim = TTRUE;

	return TTRUE;
}

TPSTRING8_DECLARE( idle_hold_l );
TPSTRING8_DECLARE( walk_hold_l );
TPSTRING8_DECLARE( idle_hold_s );
TPSTRING8_DECLARE( walk_hold_s );
TPSTRING8_DECLARE( run_hold_s );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( sprint_lp );
TPSTRING8_DECLARE( ladder_climb );
TPSTRING8_DECLARE( ladder_getoff_start );
TPSTRING8_DECLARE( ladder_getoff_end );
TPSTRING8_DECLARE( ladder_idle );
TPSTRING8_DECLARE( sidestep_left );
TPSTRING8_DECLARE( sidestep_right );
TPSTRING8_DECLARE( pickup );
TPSTRING8_DECLARE( putaway );
TPSTRING8_DECLARE( eat );
TPSTRING8_DECLARE( throw );
TPSTRING8_DECLARE( milk_run_w );
TPSTRING8_DECLARE( milk_run_e );
TPSTRING8_DECLARE( milk_run_n );
TPSTRING8_DECLARE( milk_run_ne );
TPSTRING8_DECLARE( milk_run_se );
TPSTRING8_DECLARE( milk_run_s );
TPSTRING8_DECLARE( milk_run_sw );
TPSTRING8_DECLARE( milk_run_nw );
TPSTRING8_DECLARE( jump_stat_land );
TPSTRING8_DECLARE( squirt );

// $Barnyard: FUNCTION 005889a0
TBOOL ACharacterAnimController::IsMovementAnimPlaying()
{
	AToshiAnimationInterface& rAnimInterface = m_pAnimatableObject->GetToshiAnimInterface();

	if ( rAnimInterface.IsAnimPlaying( TPS8( idle_hold_l ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( walk_hold_l ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( idle_hold_s ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( walk_hold_s ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( run_hold_s ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( idle_lp ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( walk_lp ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( run_lp ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( sprint_lp ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( ladder_climb ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( ladder_getoff_start ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( ladder_getoff_end ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( ladder_idle ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( sidestep_left ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( sidestep_right ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( pickup ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( putaway ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( eat ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( throw ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( milk_run_w ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( milk_run_e ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( milk_run_n ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( milk_run_ne ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( milk_run_se ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( milk_run_s ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( milk_run_sw ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( milk_run_nw ), TFALSE ) ||
	     rAnimInterface.IsAnimPlaying( TPS8( jump_stat_land ), TFALSE ) )
	{
		return TTRUE;
	}

	// Squirting while moving
	return rAnimInterface.IsAnimPlaying( TPS8( squirt ), TFALSE ) &&
	    TSTATICCAST( AUnit, m_pOwner )->GetSteer()->GetMovementType() != ASteer::MOVEMENT_STOPPED;
}

// $Barnyard: FUNCTION 00588db0
void ACharacterAnimController::UpdateTransform( TFLOAT a_fDeltaTime )
{
	if ( !m_pOwner || m_pAnimatableObject->GetParentObject() )
		return;

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

TPSTRING8_DECLARE( default );

// $Barnyard: FUNCTION 005893e0
void ACharacterAnimController::SetAnimationSet( const TPString8* a_pName )
{
	AAnimatableObjectType* pObjectType = m_pAnimatableObject->GetObjectType();

	if ( a_pName )
		m_pAnimationSet = pObjectType->FindAnimationSet( *a_pName );
	else
		m_pAnimationSet = pObjectType->FindAnimationSetOrDefault( TPS8( default ) );
}

// $Barnyard: FUNCTION 00588ed0
void ACharacterAnimController::UpdateMovementAnimation( TFLOAT a_fDeltaTime )
{
	AUnit*              pUnit        = TSTATICCAST( AUnit, m_pOwner );
	const ASteerTraits* pSteerTraits = pUnit->GetSteerTraits();
	ASteer*             pSteer       = pUnit->GetSteer();

	TFLOAT           fSpeed         = pSteer->GetMeasuredSpeed();
	ASteer::Movement eMovement      = pSteer->GetMovementType();
	TFLOAT           fAnimatedSpeed = pSteerTraits->GetAnimatedSpeed( eMovement );

	ASteerKinematics* pKinematics = pSteer->GetKinematics();

	if ( pKinematics )
	{
		// NOTE: ASteerKinematics3D isn't reconstructed yet, so its TClass is looked up by name
		static const TClass* s_pKinematics3DClass = TClass::Find( "ASteerKinematics3D", &TGetClass( ASteerKinematics ) );

		// Moving backwards
		if ( s_pKinematics3DClass && pKinematics->IsA( s_pKinematics3DClass ) && pKinematics->GetDesiredSpeed() < 0.0f )
			fSpeed *= -1.0f;
	}

	TFLOAT fAnimSpeed = fSpeed / fAnimatedSpeed;

	TPString8 strSprint = TPS8( sprint_lp );
	TPString8 strRun    = TPS8( run_lp );
	TPString8 strWalk   = TPS8( walk_lp );
	TPString8 strIdle   = TPS8( idle_lp );

	AToshiAnimationInterface& rAnimInterface = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetToshiAnimInterface();

	ANamedAnimation* pIdleAnim   = rAnimInterface.GetAnimationSet()->GetNamedAnimation( strIdle );
	ANamedAnimation* pWalkAnim   = rAnimInterface.GetAnimationSet()->GetNamedAnimation( strWalk );
	ANamedAnimation* pRunAnim    = rAnimInterface.GetAnimationSet()->GetNamedAnimation( strRun );
	ANamedAnimation* pSprintAnim = rAnimInterface.GetAnimationSet()->GetNamedAnimation( strSprint );

	if ( !pRunAnim && eMovement == ASteer::MOVEMENT_RUN )
	{
		// NOTE: retail reads an uninitialised (zero) global here
		eMovement = ASteer::MOVEMENT_STOPPED;
	}

	// Some steers play the animations themselves
	if ( pSteer->UpdateAnimation( &rAnimInterface, *TREINTERPRETCAST( void**, &fSpeed ) ) || !pIdleAnim )
		return;

	if ( !pWalkAnim )
	{
		if ( !rAnimInterface.IsAnimPlaying( strIdle, TFALSE ) )
			rAnimInterface.PlayAnim( strIdle, -1.0f, TFALSE );

		return;
	}

	if ( pWalkAnim->IsOverlay() )
	{
		// Walking is an overlay on top of the idle animation
		rAnimInterface.PlayAnim( strIdle, -1.0f, TFALSE );

		if ( eMovement == ASteer::MOVEMENT_STOPPED )
		{
			rAnimInterface.StopAnim( strWalk );
		}
		else
		{
			AToshiAnimationRef* pAnimRef = rAnimInterface.PlayAnim( strWalk, -1.0f, TFALSE );

			if ( pAnimRef )
				rAnimInterface.SetAnimSpeed( pAnimRef, fAnimSpeed );
		}

		return;
	}

	// Fall back to the slower animations if there's no run or sprint animation
	if ( !pRunAnim ) strRun = strWalk;
	if ( !pSprintAnim ) strSprint = strRun;

	const TPString8*    pAnimName = TNULL;
	AToshiAnimationRef* pAnimRef  = TNULL;

	switch ( eMovement )
	{
		case ASteer::MOVEMENT_STOPPED:
			if ( !rAnimInterface.IsAnimPlaying( strIdle, TTRUE ) )
				rAnimInterface.PlayAnim( strIdle, -1.0f, TFALSE );

			return;
		case ASteer::MOVEMENT_WALK:
			pAnimName = &strWalk;
			break;
		case ASteer::MOVEMENT_RUN:
			pAnimName = &strRun;
			break;
		case ASteer::MOVEMENT_SPRINT:
			pAnimName = &strSprint;
			break;
		default:
			return;
	}

	pAnimRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), *pAnimName );

	if ( !pAnimRef )
	{
		if ( rAnimInterface.IsAnimPlaying( *pAnimName, TFALSE ) )
			return;

		pAnimRef = rAnimInterface.PlayAnim( *pAnimName, -1.0f, TFALSE );

		if ( !pAnimRef )
			return;
	}

	if ( eMovement == ASteer::MOVEMENT_RUN )
	{
		// Don't let the run animation play too fast
		if ( pSteerTraits->m_fMaxAesRunSpeedRatio > 0.0f && fAnimSpeed > pSteerTraits->m_fMaxAesRunSpeedRatio )
			fAnimSpeed = pSteerTraits->m_fMaxAesRunSpeedRatio;
	}

	rAnimInterface.SetAnimSpeed( pAnimRef, fAnimSpeed );
}

// $Barnyard: FUNCTION 005897c0
void ACharacterAnimController::UserRenderCallback( TModelInstance* a_pInstance )
{
	TSkeletonInstance* pSkeletonInstance = a_pInstance->GetSkeletonInstance();
	TModel*            pModel            = a_pInstance->GetModel();

	// NOTE: retail UpdateState has no frame check
	pSkeletonInstance->UpdateState( TTRUE );
	TRenderInterface::GetSingleton()->GetCurrentContext()->SetSkeletonInstance( pSkeletonInstance );

	TINT iFrame = TRenderInterface::GetSingleton()->GetFrameCount();

	if ( !AAnimatableObjectManager::GetSingleton()->IsActive() )
		iFrame = 0;

	// Render a single mesh each frame, different instances render different meshes
	TModelLOD& rLOD        = pModel->GetLOD( a_pInstance->GetLOD() );
	TINT       iMeshToDraw = ( TINT( TREINTERPRETCAST( TUINTPTR, pSkeletonInstance ) ) + iFrame ) % rLOD.iNumMeshes;

	for ( TINT i = 0; i < rLOD.iNumMeshes; i++ )
	{
		if ( i == iMeshToDraw )
			rLOD.ppMeshes[ i ]->Render();
	}
}
