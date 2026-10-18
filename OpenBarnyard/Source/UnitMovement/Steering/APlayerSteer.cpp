#include "pch.h"
#include "Helpers/ABikeManager.h"
#include "APlayerSteer.h"
#include "Units/AUnitFacialComponent.h"
#include "AThrowItemSteer.h"
#include "Units/AAnimatedUnit.h"

#include <Toshi/TScheduler.h>
#include "ASteerKinematics3D.h"
#include "AKinematicsPool.h"
#include "Units/AKinematicsObstruction.h"
#include "Units/AKinematicsObstructionManager.h"
#include "ASteerTraits.h"
#include "Units/AUnit.h"
#include "Units/AVisibleUnit.h"
#include "Units/AObjectTraits.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AUnitType.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectType.h"
#include "Animation/AToshiAnimationInterface.h"
#include "Animation/ACharacterAnimController.h"
#include "Animation/AAlienAnimController.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "Render/ARenderer.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/barnyard.h"
#include "SoundBank/am_f.h"
#include "SoundBank/am_m.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitProxy.h"
#include "Units/APlayerControlComponent.h"
#include "Cameras/ACameraManager.h"
#include "AOptions.h"
#include "GameInterface/AGameStateController.h"
#include "MiniGame/ATreasueHuntMicroGame.h"
#include "Helpers/AGameLoader.h"
#include "Player/APlayerManager.h"
#include "Render/ADebugOverlay.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Input/AInputHandler.h"
#include "Player/APlayerProfileManager.h"
#include "Physics/ACollisionModelInstance.h"
#include "Animation/AModel.h"
#include "GUI/AFade.h"
#include "GUI/AFadeManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/APhysics.h"
#include "Physics/ACollisionManager.h"
#include "FX/AParticleEffectManager.h"
#include "FX/ASplashManager.h"
#include "ARandom.h"
#include "GameInterface/ASimState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( jump_run_up );
TPSTRING8_DECLARE( jump_stat_up );
TPSTRING8_DECLARE( jump_run_land );
TPSTRING8_DECLARE( jump_stat_land );
TPSTRING8_DECLARE( jump_run_fall );
TPSTRING8_DECLARE( jump_stat_fall );
TPSTRING8_DECLARE( barnyard_centre );
TPSTRING8_DECLARE( throw );
TPSTRING8_DECLARE( eat );
TPSTRING8_DECLARE( wave );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( putaway );
TPSTRING8_DECLARE( getiteminv );
TPSTRING8_DECLARE( pickup );
TPSTRING8_DECLARE( putdown );
TPSTRING8_DECLARE( defeat );
TPSTRING8_DECLARE( yawn );
TPSTRING8_DECLARE( fixpipe );
TPSTRING8_DECLARE( coinspawner );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( Hand );
TPSTRING8_DECLARE( detector );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( carrylarge );
TPSTRING8_DECLARE( carrysmall );
TPSTRING8_DECLARE( ladder_climb );
TPSTRING8_DECLARE( ladder_geton );
TPSTRING8_DECLARE( ladder_getoff_start );
TPSTRING8_DECLARE( ladder_getoff_end );
TPSTRING8_DECLARE( c_neck );
TPSTRING8_DECLARE( water_respawn_goal );
TPSTRING8_DECLARE( AGopherShopGame );
TPSTRING8_DECLARE( ACookingMiniGame );

// NOTE: GetClass 00567420, CreateObject 0056c6a0, CreateObjectInPlace 0056c700
TDEFINE_CLASS( APlayerSteer );

// NOTE: retail has a plain table of __thiscall function pointers here
APlayerSteer::t_StateUpdate APlayerSteer::ms_aStateUpdates[ STATE_NUMOF ] = {
	&APlayerSteer::UpdateNone,    // STATE_UNKNOWN
	&APlayerSteer::UpdateWalking, // STATE_WALKING
	&APlayerSteer::UpdateStill,   // STATE_STILL
	&APlayerSteer::UpdateJumping, // STATE_JUMPING
	&APlayerSteer::UpdateFalling, // STATE_FALLING
	&APlayerSteer::UpdateNone,    // STATE_CLIMBING, updated by LadderAnimEndCallback instead
	&APlayerSteer::UpdateThrow,   // STATE_THROW
};

const TCHAR* APlayerSteer::ms_aStateNames[ STATE_NUMOF ] = {
	"STATE_UNKNOWN",
	"STATE_WALKING",
	"STATE_STILL",
	"STATE_JUMPING",
	"STATE_FALLING",
	"STATE_CLIMBING",
	"STATE_THROW",
};

const TCHAR* APlayerSteer::ms_aJumpStateNames[ JUMPSTATE_NUMOF ] = {
	"JUMPSTATE_NONE",
	"JUMPSTATE_END",
	"JUMPSTATE_FALL",
	"JUMPSTATE_START",
	"JUMPSTATE_UP",
	"JUMPSTATE_APEX",
	"JUMPSTATE_DOWN",
	"JUMPSTATE_DOWN_END",
};

const TCHAR* APlayerSteer::ms_aClimbStateNames[ CLIMBSTATE_NUMOF ] = {
	"CLIMBSTATE_NONE",
	"CLIMBSTATE_SEEK",
	"CLIMBSTATE_ATTACH",
	"CLIMBSTATE_MOVE",
	"CLIMBSTATE_UP",
	"CLIMBSTATE_DOWN",
	"CLIMBSTATE_OFFLOAD_UP",
	"CLIMBSTATE_OFFLOAD_DOWN",
	"CLIMBSTATE_FALL",
};

TFLOAT APlayerSteer::ms_fUpdateInterval = 0.0f;

// DAT_00774330, AInteriorNetworkPath::m_fRadius of every new steer
static TFLOAT s_fPathRadius = 0.25f;

// The player steers always use ASteerKinematics3D
static ASteerKinematics3D* GetKinematics3D( ASteerKinematics* a_pKinematics )
{
	return TSTATICCAST( ASteerKinematics3D, a_pKinematics );
}

// $Barnyard: FUNCTION 0056b5e0
void APlayerSteer::AnimEventListener::Connect( APlayerSteer* a_pCaller, t_AnimEventCallback a_pCallback )
{
	pCallback = a_pCallback;
	pCaller   = a_pCaller;

	// Insert at the tail of the global animation event list (same layout as its nodes)
	AnimEventListener* pHead = TREINTERPRETCAST( AnimEventListener*, &AAnimationBreakpointUnitAction::GetListeners() );

	pNext               = pHead;
	pPrev               = pHead->pPrev;
	pHead->pPrev->pNext = this;
	pHead->pPrev        = this;
}

// $Barnyard: FUNCTION 0056a470
void APlayerSteer::AnimEventListener::Disconnect()
{
	if ( pNext != this )
	{
		pPrev->pNext = pNext;
		pNext->pPrev = pPrev;
		pNext        = this;
		pPrev        = this;
	}

	pCaller   = TNULL;
	pCallback = TNULL;
}

// $Barnyard: FUNCTION 0056b670
APlayerSteer::APlayerSteer()
    : m_pKinematics( TNULL )
    , m_fUpdateTime( 0.1f )
    , m_eActionState( STATE_WALKING )
    , m_eNextState( STATE_UNKNOWN )
    , m_eJumpState( JUMPSTATE_NONE )
    , m_eClimbState( CLIMBSTATE_NONE )
    , m_vLadderBase( TVector4::VEC_ZERO )
    , m_vLadderStart( TVector4::VEC_ZERO )
    , m_vLadderEnd( TVector4::VEC_ZERO )
    , m_vDesiredFacing( TVector4::VEC_POSX )
    , m_bGrounded( TTRUE )
    , m_vThrowHandPosition( TVector4::VEC_ZERO )
    , m_bSteppedUp( TFALSE )
    , m_bUnk569( TTRUE )
    , m_bSpeechDialogMode( TFALSE )
    , m_bTurning( TFALSE )
    , m_bRestoreAnimationSet( TTRUE )
    , m_bUnk584( TFALSE )
    , m_vLastPosition( TVector4::VEC_ZERO )
    , m_bIsInWater( TFALSE )
    , m_fWaterTime( 0.0f )
    , m_iIgnoreGroups( 0 )
    , m_pSinkFade( TNULL )
    , m_iWaterSplashAEffect( -1 )
    , m_iWaterSplashBEffect( -1 )
    , m_iWaterRippleEffect( -1 )
    , m_iMilkDripEffect( -1 )
    , m_iBushHitEffect( -1 )
    , m_iBushesCue( -1 )
{
	m_oPath.m_fRadius = s_fPathRadius;
	m_pGoal           = TNULL;

	// The particle effects aren't needed in the gopher shop and cooking mini games
	// NOTE: retail uses AGameStateController::IsStateOnStack (AGameStateController, TTRUE if a state of the class is on the stack)
	AGameStateController* pGameStateController = AGameStateController::GetSingleton();
	TBOOL                 bNoEffects           = pGameStateController->FindGameStateOnStack( TPS8( AGopherShopGame ) ) || pGameStateController->FindGameStateOnStack( TPS8( ACookingMiniGame ) );

	if ( !AGameLoader::IsBarnExploreMiniGame() && !bNoEffects )
	{
		m_iWaterSplashAEffect = AParticleEffectManager::FindEffect( "watersplash2a" );
		m_iWaterSplashBEffect = AParticleEffectManager::FindEffect( "watersplash2b" );
		m_iWaterRippleEffect  = AParticleEffectManager::FindEffect( "waterripple" );
		m_iMilkDripEffect     = AParticleEffectManager::FindEffect( "milkdrip" );
		m_iBushHitEffect      = AParticleEffectManager::FindEffect( "bushhit" );
	}
}

// $Barnyard: FUNCTION 0056b940
// $Barnyard: FUNCTION 0056a4e0
APlayerSteer::~APlayerSteer()
{
	if ( m_pNetwork )
	{
		// m_pNetwork is the AInteriorNetwork the steer is connected to
		TSTATICCAST( AInteriorNetwork, m_pNetwork )->DisconnectUnit( &m_oPath, &GetParentUnit() );
		m_pNetwork = TNULL;
	}

	if ( m_pKinematics )
		AKinematicsPool::GetSingleton()->DestroyKinematics( m_pKinematics );

	AKinematicsObstructionManager::GetSingleton()->RemoveObstruction( TSTATICCAST( AKinematicsObstruction, GetParentUnit().GetObstruction() ) );

	if ( m_eActionState == STATE_CLIMBING )
		m_oLadderAnimListener.Disconnect();

	m_pSinkFade = TNULL;

	if ( m_oSinkFadeListener.IsLinked() )
		m_oSinkFadeListener.Disconnect();
}

// $Barnyard: FUNCTION 00569370
ASteer::Movement APlayerSteer::GetMovementType()
{
	TFLOAT         fSpeed  = GetMeasuredSpeed();
	AObjectTraits* pTraits = GetCarriedObjectTraits();

	if ( fSpeed < 0.05f )
		return MOVEMENT_STOPPED;

	if ( pTraits )
	{
		if ( pTraits->IsDetector() )
			return MOVEMENT_WALK;

		// Can't sprint while carrying something large
		if ( pTraits->IsHeavy() && ASteer::GetMovementType() == MOVEMENT_SPRINT )
			return MOVEMENT_RUN;
	}

	return ASteer::GetMovementType();
}

// $Barnyard: FUNCTION 00568e30
TBOOL APlayerSteer::UpdateAnimation( void* a_pAnimInterface, void* a_pDeltaTime )
{
	AToshiAnimationInterface* pAnimInterface = TSTATICCAST( AToshiAnimationInterface, a_pAnimInterface );
	TFLOAT                    fDeltaTime     = *TREINTERPRETCAST( TFLOAT*, &a_pDeltaTime );

	AAnimatedUnit*     pUnit             = TSTATICCAST( AAnimatedUnit, &GetParentUnit() );
	AAnimatableObject* pAnimatableObject = pUnit->GetAnimatableObject();
	AUnit*             pCarried          = TNULL;

	if ( pAnimatableObject->GetModelInstance()->GetBoneID( TPS8( cb_rlocator ) ) >= 0 )
		pCarried = pUnit->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

	AToshiAnimationInterface& rObjectAnimInterface = pAnimatableObject->GetToshiAnimInterface();

	if ( pCarried )
	{
		// Use the animation set matching the carried object
		AObjectTraits*         pTraits = TSTATICCAST( AObjectTraits, pCarried->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );
		AAnimatableObjectType* pType   = pAnimatableObject->GetObjectType();

		if ( pTraits && pTraits->IsDetector() )
		{
			ANamedAnimationSetRef pAnimationSet = pType->FindAnimationSetOrDefault( TPS8( detector ) );

			if ( rObjectAnimInterface.GetAnimationSet().Get() != pAnimationSet.Get() )
			{
				rObjectAnimInterface.SetAnimationSet( pAnimationSet );

				if ( rObjectAnimInterface.IsAnimPlaying( TPS8( idle_lp ), TFALSE ) )
					rObjectAnimInterface.StopAnim( TPS8( idle_lp ) );

				rObjectAnimInterface.PlayAnim( TPS8( idle_lp ), -1.0f, TFALSE );
			}
		}
		else
		{
			ANamedAnimationSetRef pAnimationSet = pType->FindAnimationSetOrDefault( ( pTraits && pTraits->IsHeavy() ) ? TPS8( carrylarge ) : TPS8( carrysmall ) );

			if ( rObjectAnimInterface.GetAnimationSet().Get() != pAnimationSet.Get() )
				rObjectAnimInterface.SetAnimationSet( pAnimationSet );
		}
	}
	else if ( m_bRestoreAnimationSet == TTRUE )
	{
		// Restore the animation set of the character controller
		ACharacterAnimController* pController = TSTATICCAST( ACharacterAnimController, pAnimatableObject->FindAnimController( &TGetClass( ACharacterAnimController ) ) );

		if ( !pController )
			pController = TSTATICCAST( ACharacterAnimController, pAnimatableObject->FindAnimController( &TGetClass( AAlienAnimController ) ) );

		if ( pController && pController->GetAnimationSet().Get() )
			rObjectAnimInterface.SetAnimationSet( pController->GetAnimationSet() );
	}

	// The climbing animations play with the climbing speed
	if ( m_eActionState != STATE_CLIMBING || m_eClimbState < CLIMBSTATE_MOVE || m_eClimbState > CLIMBSTATE_OFFLOAD_DOWN )
		return TFALSE;

	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( pAnimInterface->GetAnimRefs(), TPS8( ladder_climb ) );

	if ( !pAnimRef )
		pAnimRef = AToshiAnimationInterface::GetAnimationRef( pAnimInterface->GetOverlayAnimRefs(), TPS8( ladder_climb ) );

	if ( !pAnimRef )
		pAnimRef = pAnimInterface->FindAnimationRef( TPS8( ladder_getoff_start ) );

	if ( pAnimatableObject->IsAnimationPlanned( TPS8( ladder_getoff_start ) ) || pAnimInterface->IsAnimPlaying( TPS8( ladder_getoff_start ), TFALSE ) )
	{
		pAnimInterface->SetAnimSpeed( pAnimRef, fDeltaTime / m_pTraits->m_fAnimatedClimbSpeed );
	}
	else if ( pAnimRef )
	{
		pAnimInterface->SetAnimSpeed( pAnimRef, 0.0f );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0056a6b0
TBOOL APlayerSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ACommonSteer::CreateFrom( a_pOther, a_iLOD );

	m_pKinematics = AKinematicsPool::GetSingleton()->CreateKinematics( &TGetClass( ASteerKinematics3D ) );
	TBOOL bResult = m_pKinematics->CreateFrom( a_pOther );
	m_pKinematics->SetSteer( this );

	OnCreate();
	return bResult;
}

// $Barnyard: FUNCTION 0056a610
TBOOL APlayerSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ACommonSteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );

	m_pKinematics = AKinematicsPool::GetSingleton()->CreateKinematics( &TGetClass( ASteerKinematics3D ) );
	TBOOL bResult = m_pKinematics->Create( a_rcPosition, a_rcDirection, m_pTraits, 0.0f );
	m_pKinematics->SetSteer( this );

	OnCreate();

	AInteriorNetwork* pNetwork = GetNetwork();

	if ( pNetwork )
	{
		// Connect to the network
		m_pNetwork = pNetwork;

		if ( pNetwork->ConnectUnit( &m_oPath, &GetParentUnit(), *m_pKinematics->GetPosition() ) == AInteriorNetwork::CONNECT_FAILED )
			m_pNetwork = TNULL;
	}

	return bResult;
}

// $Barnyard: FUNCTION 0056b960
TBOOL APlayerSteer::Update( TFLOAT a_fDeltaTime )
{
	TFLOAT fDeltaTime = a_fDeltaTime;

#ifdef ADEBUGOVERLAY
	// Development: the lines of the player on the debug overlay
	if ( ADebugOverlay::IsPrinting() && APlayerManager::IsSingletonCreated() && &GetParentUnit() == APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) )
	{
		static TFLOAT s_fGroundTime = 0.0f;

		// Same limit as the jump check
		if ( m_pKinematics->GetAirTime() >= 0.2f )
			s_fGroundTime = 0.0f;
		else
			s_fGroundTime += a_fDeltaTime;

		ADEBUGLINE( LINE_PLAYER, ADebugOverlay::INFO_COLOUR, "GroundTime = [%f]", s_fGroundTime );

		if ( AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject() )
			pObject->DebugPrintAnimations();

		// The velocity the steer is asked for (the movement input turned into a direction and a speed)
		const TVector4* pFacing = m_pKinematics->GetDesiredFacing();
		TFLOAT          fSpeed  = m_pKinematics->GetDesiredSpeed();
		ADEBUGLINE( LINE_STEER, ADebugOverlay::STEER_COLOUR, "ManualSteer(%f,%f,%f)", pFacing->x * fSpeed, pFacing->y * fSpeed, pFacing->z * fSpeed );
	}
#endif

	if ( UpdateGoal() )
		return TTRUE;

	m_fUpdateTime += fDeltaTime;
	if ( m_fUpdateTime < ms_fUpdateInterval )
		return TTRUE;

	fDeltaTime    = m_fUpdateTime;
	m_fUpdateTime = 0.0f;

	GetKinematics3D( m_pKinematics )->SetGravityScale( 3.0f, fDeltaTime );
	m_bSteppedUp = TFALSE;

	if ( !CanStartAction() )
	{
		TVector4 vVelocity;
		CopyVelocity( vVelocity );
		vVelocity.x = 0.0f;
		vVelocity.z = 0.0f;

		SetVelocity( vVelocity );
		m_pKinematics->SetSpeed( 0.0f );
	}

	TVector4 vPrevPosition = *GetPosition();
	UpdateCarrying();

	if ( ( this->*ms_aStateUpdates[ m_eActionState ] )( fDeltaTime ) )
	{
		TVector4 vPosition = *m_pKinematics->GetPosition();

		// Integrate the movement
		m_pKinematics->MoveTo( fDeltaTime, ( m_pKinematics->GetDesiredSpeed() == 0.0f ) ? &vPosition : &vPrevPosition, &vPosition );
		UpdateCollision( fDeltaTime, &vPrevPosition, TFALSE, TFALSE, TTRUE );

		TVector4 vNewPosition = *GetPosition();
		m_pKinematics->ApplyGravity( fDeltaTime );
		UpdateCollision( fDeltaTime, &vNewPosition, TTRUE, TTRUE, TFALSE );

		UpdateCarrying();
		UpdatePath( fDeltaTime );
		UpdateWater( fDeltaTime );
		UpdateSinking();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00555e10
TBOOL APlayerSteer::Command_StandStill()
{
	return SetState( STATE_STILL );
}

// $Barnyard: FUNCTION 00555e20
TBOOL APlayerSteer::EndActionState()
{
	return EndState( m_eActionState );
}

// $Barnyard: FUNCTION 00555e40
void APlayerSteer::SetDesiredDirection( const TVector4& a_rcDirection )
{
	m_pKinematics->SetDesiredFacing( a_rcDirection );
}

// $Barnyard: FUNCTION 00555e50
void APlayerSteer::SetSpeechDialogMode( TBOOL a_bEnabled )
{
	m_bSpeechDialogMode = a_bEnabled;
}

// $Barnyard: FUNCTION 00567660
void APlayerSteer::SetDesiredSpeed( TFLOAT a_fSpeed )
{
	ASteer::SetDesiredSpeed( a_fSpeed );
}

// $Barnyard: FUNCTION 00567790
void APlayerSteer::SetInteracting( TINT a_iUnk )
{
	TBOOL bValue = TBOOL( a_iUnk );

	if ( IsInteracting() != bValue && bValue )
		GetKinematics3D( m_pKinematics )->SetTilt( 0.0f, 0.0f, TFALSE );

	ACommonSteer::SetInteracting( a_iUnk );
}

// $Barnyard: FUNCTION 00568160
TBOOL APlayerSteer::CanStartAction()
{
	AAnimatableObject* pAnimObj = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();

	// Can't do anything else while one of these is playing
	return !pAnimObj->IsAnimPlaying( TPS8( eat ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( wave ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( yuk ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( putaway ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( getiteminv ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( pickup ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( putdown ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( defeat ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( yawn ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( fixpipe ), 0.0f ) &&
	    !pAnimObj->IsAnimPlaying( TPS8( coinspawner ), 0.0f );
}

// $Barnyard: FUNCTION 00555e30
TBOOL APlayerSteer::GetTilt( void* a_pPitch, void* a_pRoll )
{
	return m_pKinematics->GetTilt( TSTATICCAST( TFLOAT, a_pPitch ), TSTATICCAST( TFLOAT, a_pRoll ) );
}

// $Barnyard: FUNCTION 00567760
void APlayerSteer::SetTilt( TFLOAT a_fPitch, TFLOAT a_fRoll, TBOOL a_bImmediate )
{
	if ( m_eActionState == STATE_WALKING )
		GetKinematics3D( m_pKinematics )->SetTilt( a_fPitch, a_fRoll, a_bImmediate );
	else if ( m_eActionState == STATE_CLIMBING )
		GetKinematics3D( m_pKinematics )->SetTilt( 0.0f, 0.0f, a_bImmediate );
}

// $Barnyard: FUNCTION 0056af10
TBOOL APlayerSteer::SetDesiredVelocity( const TVector4& a_rcVelocity )
{
	TVector4 vVelocity = a_rcVelocity;
	TFLOAT   fSpeedSq  = vVelocity.MagnitudeSq();

	// NOTE: retail compares the squared speed with the speeds
	if ( fSpeedSq >= m_pTraits->m_fLogicalRunSpeed )
	{
		AObjectTraits* pTraits = GetCarriedObjectTraits();

		if ( pTraits )
		{
			if ( pTraits->IsDetector() )
			{
				vVelocity.Divide( TMath::Sqrt( fSpeedSq ) );
				vVelocity.Multiply( m_pTraits->m_fLogicalWalkSpeed );
			}
			else if ( pTraits->IsHeavy() && fSpeedSq >= m_pTraits->m_fLogicalSprintSpeed )
			{
				vVelocity.Divide( TMath::Sqrt( fSpeedSq ) );
				vVelocity.Multiply( m_pTraits->m_fLogicalRunSpeed );
			}
		}
	}

	if ( m_bTurning )
	{
		if ( IsFacingDesired( 0.95f ) )
			m_bTurning = TFALSE;

		if ( m_bTurning )
			return TFALSE;
	}

	m_pKinematics->Halt();

	TVector4 vDirection = vVelocity;
	TFLOAT   fSpeed     = vVelocity.MagnitudeSq();

	// NOTE: TMath::Sqrt asserts on zero (retail just takes the square root of the squared length)
	fSpeed = ( fSpeed != 0.0f ) ? TMath::Sqrt( fSpeed ) : 0.0f;
	m_pKinematics->SetDesiredSpeed( fSpeed );

	if ( fSpeed > 0.05f )
	{
		vDirection.Divide( fSpeed );
		m_pKinematics->SetDesiredFacing( vDirection );
		m_vDesiredFacing = vDirection;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005675d0
TBOOL APlayerSteer::StopAndFace( const TVector4& a_rcFacing )
{
	const TVector4& rcFacing = a_rcFacing;

	if ( m_bTurning )
	{
		if ( IsFacingDesired( 0.95f ) )
			m_bTurning = TFALSE;

		if ( m_bTurning )
			return TFALSE;
	}

	m_pKinematics->Halt();
	m_pKinematics->SetDesiredSpeed( 0.0f );
	m_pKinematics->SetDesiredFacing( rcFacing );
	SetTilt( 0.0f, 0.0f, TFALSE );
	m_vDesiredFacing = rcFacing;

	return TTRUE;
}

// $Barnyard: FUNCTION 0056aeb0
TBOOL APlayerSteer::SetGoal( AInteriorGoal* a_pGoal )
{
	if ( m_pGoal != a_pGoal )
	{
		m_pGoal = a_pGoal;
		GetCurrentNetwork();

		TVector4 vPosition;
		m_pGoal->GetPosition( vPosition );
		vPosition.y -= 0.01f;

		m_pKinematics->SetPosition( vPosition );
		UpdateGoal();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005679e0
void* APlayerSteer::GetCurrentNetwork()
{
	if ( m_pNetwork )
		return m_pNetwork;

	return AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
}

// $Barnyard: FUNCTION 005676b0
AInteriorArea* APlayerSteer::GetCurrentArea()
{
	return TREINTERPRETCAST( AInteriorArea*, GetAreaInNetwork( TREINTERPRETCAST( TINT, GetCurrentNetwork() ) ) );
}

// $Barnyard: FUNCTION 00567670
TINT APlayerSteer::GetAreaInNetwork( TINT a_iUnk )
{
	AInteriorNetwork* pNetwork = TREINTERPRETCAST( AInteriorNetwork*, a_iUnk );

	if ( pNetwork )
	{
		TINT iArea = pNetwork->FindContainingArea( *m_pKinematics->GetPosition() );

		if ( iArea != -1 )
			return TREINTERPRETCAST( TINT, &pNetwork->m_pINetwork->m_pAreas[ iArea ] );
	}

	return 0;
}

// $Barnyard: FUNCTION 0056c770
TBOOL APlayerSteer::SetState( STATE a_eState )
{
#ifdef BARNYARD_LATEST
	// The latest version (802d6540) keeps the height of the camera while the player jumps (Follow Jumps camera option off)
	AGTAStyleCameraHelper* pGTACamera = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );

	if ( a_eState == STATE_JUMPING && m_eActionState != STATE_JUMPING )
	{
		if ( pGTACamera == ACameraManager::GetSingleton()->GetCurrentCameraHelper() && AOptions::IsJumpHeightHoldEnabled() )
			pGTACamera->HoldHeight();
	}
	else if ( a_eState != STATE_JUMPING && a_eState != STATE_FALLING )
	{
		if ( pGTACamera == ACameraManager::GetSingleton()->GetCurrentCameraHelper() )
			pGTACamera->ReleaseHeight();
	}
#endif

	if ( a_eState == m_eActionState )
	{
		// Jump again right after landing
		if ( a_eState == STATE_JUMPING && m_eActionState == STATE_JUMPING && m_eJumpState == JUMPSTATE_END && m_pKinematics->GetAirTime() < 0.01f )
		{
			m_eActionState = STATE_UNKNOWN;
			m_pKinematics->SetAirTime( 0 );
			return SetState( STATE_JUMPING );
		}

		return TFALSE;
	}

	// Can't jump or climb while carrying something large
	if ( a_eState == STATE_JUMPING || a_eState == STATE_CLIMBING )
	{
		AObjectTraits* pTraits = GetCarriedObjectTraits();

		if ( pTraits && pTraits->IsHeavy() )
			return TFALSE;
	}

	if ( m_eActionState >= STATE_JUMPING && m_eActionState <= STATE_FALLING )
		GetKinematics3D( m_pKinematics )->SetTilt( 0.0f, 0.0f, TFALSE );

	m_eNextState = STATE_UNKNOWN;

	switch ( a_eState )
	{
		case STATE_WALKING:
			m_eActionState = a_eState;
			ApplyGroundGravity();
			return TTRUE;
		case STATE_STILL:
			m_eActionState = a_eState;
			ApplyGroundGravity();
			m_pKinematics->SetSpeed( 0.0f );
			return TTRUE;
		case STATE_JUMPING:
			// GetAirTime is the time spent in the air
			if ( m_pKinematics->GetAirTime() < 0.2f && /*IsOnGround() &&*/ TFALSE == IsInWater() && JUMPSTATE_START > m_eJumpState ) //If have something to jump from
			{
				if ( GetState() == STATE_CLIMBING )
				{
					break; /*EndState(STATE_CLIMBING);*/
				}

				if ( GetParentUnit().IsA( &TGetClass( AAnimatedUnit ) ) )
				{
					AAnimatableObject* poPlayerAnimObj = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();
					TVALIDPTR( poPlayerAnimObj );

					// play jump start anim whether running or stationary, probably want a running jump start anim
					poPlayerAnimObj->KillAllAnimations();

					TBOOL            bPlayed   = TFALSE;
					ASteer::Movement eMoveType = GetMovementType();

					if ( eMoveType == ASteer::MOVEMENT_RUN || eMoveType == ASteer::MOVEMENT_SPRINT )
						bPlayed = poPlayerAnimObj->PlayAnimation( TPS8( jump_run_up ) );
					else
						bPlayed = poPlayerAnimObj->PlayAnimation( TPS8( jump_stat_up ) );

					if ( bPlayed )
					{
						m_eActionState = STATE_JUMPING;
						m_eJumpState   = JUMPSTATE_START;
						/*m_fJumpTime = 0;*/
					}

					break;
				}

				m_eActionState = a_eState;
			}
			break;
		case STATE_FALLING:
			m_eActionState = a_eState;

			// NOTE: never true here, the check is in retail
			if ( GetState() == STATE_CLIMBING )
				m_eClimbState = CLIMBSTATE_NONE;

			if ( m_eJumpState < JUMPSTATE_APEX )
				m_eJumpState = JUMPSTATE_FALL;

			break;
		case STATE_CLIMBING:
			if ( m_eClimbState > CLIMBSTATE_ATTACH || m_eClimbState == CLIMBSTATE_NONE )
			{
				AInteriorArea* pArea = GetCurrentArea();

				// Ladders are special group areas
				if ( !pArea || !HASANYFLAG( pArea->m_eFlags, AInteriorArea::FLAGS_SPECIAL_GROUP_AREA ) )
					return TFALSE;

				if ( m_pKinematics->GetSpeed() < 0.001f )
					return TFALSE;

				if ( m_pKinematics->GetDesiredSpeed() < 0.001f )
					return TFALSE;

				// The ladder goes from the area's position (y snapped to the rungs) to the second point of the area
				pArea->GetPosition( m_vLadderBase );
				m_vLadderBase.y = pArea->GetSpecialHeight();
				m_vLadderStart  = pArea->GetSpecialPoint( 0 );
				m_vLadderEnd    = pArea->GetSpecialPoint( 1 );

				TFLOAT fRungOffset = fmodf( TVector4::Distance( m_vLadderBase, m_vLadderEnd ), 0.934f ) - 0.467f;

				if ( fRungOffset < 0.0f )
					fRungOffset += 0.467f;

				m_vLadderBase.y -= fRungOffset;

				TVector4 vecToStart;
				vecToStart.Substract( m_vLadderStart, m_vLadderBase );

				TVector4 vecLadderFacing = vecToStart;
				vecLadderFacing.y        = 0.0f;
				vecLadderFacing.Normalise();

				// Has to face the ladder
				TVector4 vecDesiredFacing = *m_pKinematics->GetDesiredFacing();

				if ( TVector4::DotProduct3( vecLadderFacing, vecDesiredFacing ) < 0.5f )
					return TFALSE;

				// ...and be close to it
				TVector4 vecDirection = vecToStart;
				vecDirection.Normalise();

				TVector4 vecOffset;
				vecOffset.Multiply( vecDirection, 0.934f );
				vecOffset.Multiply( 2.0f );

				TVector4 vecSegmentEnd;
				vecSegmentEnd.Substract( m_vLadderStart, vecOffset );

				if ( TMath::DistancePointToSegmentSq( *GetPosition(), m_vLadderBase.AsVector3(), vecSegmentEnd.AsVector3() ) > 0.8f )
					return TFALSE;

				GetKinematics3D( m_pKinematics )->SetTilt( 0.0f, 0.0f, TTRUE );
				m_eActionState = a_eState;
				m_eClimbState  = CLIMBSTATE_SEEK;
				GetKinematics3D( m_pKinematics )->SetClimbing( TTRUE );

				// Climbing up the mweeds starts a bit further away
				const TCHAR* szAreaName = pArea->m_strName->GetString();

				if ( TStringManager::String8Length( szAreaName ) > 5 && TStringManager::String8Compare( szAreaName, "mweed", 5 ) == 0 )
				{
					TVector4 vecStartOffset = vecLadderFacing;
					vecStartOffset.Multiply( -0.2f );
					m_vLadderStart.AsVector3().Add( vecStartOffset.AsVector3() );
				}

				// Limit the camera to the half space in front of the ladder
				TFLOAT fLadderAngle = TMath::ATan2( vecLadderFacing.z, vecLadderFacing.x );
				TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) )->SetYawLimits( TTRUE, fLadderAngle - TMath::HALF_PI, fLadderAngle + TMath::HALF_PI );

				m_oLadderAnimListener.Connect( ARenderer::GetSingleton()->GetAnimationUpdateEndEmitter(), this, LadderAnimEndCallback, 0 );
				return TTRUE;
			}
			break;
		case STATE_THROW:
			if ( m_eActionState == STATE_WALKING )
			{
				m_eActionState = a_eState;
				TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject()->PlayAnimation( TPS8( throw ) );
				m_oAnimEventListener.Connect( this, AnimEventCallback );
				ASoundManager::GetSingleton()->PlayCue( soundbank::BARNYARD_APPLE_THROW );
				return TTRUE;
			}
			break;
		default:
			m_eActionState = a_eState;
			break;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0056a7c0
TBOOL APlayerSteer::EndState( STATE a_eState )
{
	if ( m_eActionState != a_eState )
		return TFALSE;

	m_oAnimEventListener.Disconnect();
	m_oLadderAnimListener.Disconnect();

	switch ( a_eState )
	{
		case STATE_WALKING:
			return TTRUE;
		case STATE_JUMPING:
			m_eNextState = STATE_FALLING;
			return TTRUE;
		case STATE_FALLING:
		case STATE_THROW:
			SetState( STATE_WALKING );
			return TTRUE;
		case STATE_CLIMBING:
			m_eClimbState = CLIMBSTATE_NONE;
			// Fallthrough
		default:
			SetState( STATE_WALKING );
			return TTRUE;
	}
}

// $Barnyard: FUNCTION 005678f0
TBOOL APlayerSteer::PlaceAtGoal( AInteriorGoal* a_pGoal, TBOOL a_bUseGoalFacing, TBOOL a_bSetGoal )
{
	if ( a_bSetGoal )
	{
		SetGoal( a_pGoal );
	}
	else
	{
		TVector4 vPosition;
		a_pGoal->GetPosition( vPosition );
		vPosition.y -= 0.01f;

		m_pKinematics->SetPosition( vPosition );
		m_vLastPosition = *GetPosition();
	}

	if ( a_bUseGoalFacing )
	{
		TVector4 vFacing = a_pGoal->m_vForward;

		if ( vFacing.MagnitudeSq() < 0.5f )
			vFacing = TVector4::VEC_NEGZ;

		// NOTE: retail returns the result of the kinematics' SetFacing (+0x8c)
		m_pKinematics->SetFacing( vFacing );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00567510
TBOOL APlayerSteer::PlaceAtNamedGoal( const TPString8& a_rcGoal, TBOOL a_bUseGoalFacing, TBOOL a_bSetGoal )
{
	return PlaceAtGoal( GetNetwork()->FindGoal( a_rcGoal ), a_bUseGoalFacing, a_bSetGoal );
}

// $Barnyard: FUNCTION 00568360
TFLOAT APlayerSteer::GetGroundHeight( const TVector4& a_rcPosition )
{
	static TFLOAT s_fRayRaise = 1.0f;

	// Offset of the rays to the sides of the steer
	TFLOAT   fRadius = GetRadius();
	TVector4 vecSideOffset;
	vecSideOffset.Multiply( *m_pKinematics->GetFacingDir(), fRadius );
	vecSideOffset.Multiply( 0.5f );
	vecSideOffset.AsVector3().RotateY( TMath::HALF_PI );

	ACollisionManager* pCollisionManager = ACollisionManager::GetSingleton();

	TVector4 vecRightStart( a_rcPosition.x + vecSideOffset.x, a_rcPosition.y - s_fRayRaise, a_rcPosition.z + vecSideOffset.z, a_rcPosition.w );
	TVector4 vecLeftStart( a_rcPosition.x - vecSideOffset.x, vecRightStart.y, a_rcPosition.z - vecSideOffset.z, a_rcPosition.w );

	ARayData oRightRay( vecRightStart, APhysics::g_vecGravityDirection, 40.0f );
	ARayData oLeftRay( vecLeftStart, APhysics::g_vecGravityDirection, 40.0f );

	TFLOAT fRightHeight = 40.0f;
	if ( pCollisionManager->CollideRay( &GetParentUnit(), oRightRay, fRightHeight, TCollisionCommon::TOSHICGROUPFLAG_NONE, ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_PLAYER ) )
		fRightHeight = fRightHeight + vecRightStart.y;
	else
		fRightHeight = 0.0f;

	TFLOAT fLeftHeight = 40.0f;
	if ( pCollisionManager->CollideRay( &GetParentUnit(), oLeftRay, fLeftHeight, TCollisionCommon::TOSHICGROUPFLAG_NONE, ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_PLAYER ) )
		fLeftHeight = fLeftHeight + vecLeftStart.y;
	else
		fLeftHeight = 0.0f;

	// The highest one (the Y axis points down)
	return ( fLeftHeight <= fRightHeight ) ? fLeftHeight : fRightHeight;
}

// $Barnyard: FUNCTION 00567720
TBOOL APlayerSteer::SetDesiredFacing( const TVector3& a_rcFacing )
{
	TVector4 vFacing( a_rcFacing.x, a_rcFacing.y, a_rcFacing.z, 1.0f );

	// NOTE: retail returns the result of the kinematics' SetDesiredFacing (+0x4c)
	m_pKinematics->SetDesiredFacing( vFacing );
	return TTRUE;
}

// $Barnyard: FUNCTION 00567540
TBOOL APlayerSteer::TurnTowards( const TVector4& a_rcFacing )
{
	if ( a_rcFacing.IsEqual( *GetDirection() ) )
	{
		m_bTurning = TFALSE;
		return TTRUE;
	}

	m_pKinematics->Halt();
	m_pKinematics->SetDesiredSpeed( 0.0f );
	m_pKinematics->SetDesiredFacing( a_rcFacing );
	SetTilt( 0.0f, 0.0f, TFALSE );
	m_vDesiredFacing = a_rcFacing;
	m_bTurning       = TTRUE;

	return TTRUE;
}

// $Barnyard: FUNCTION 005679f0
TBOOL APlayerSteer::IsAtGoal( const TPString8& a_rcGoal )
{
	return m_pGoal && *m_pGoal->m_strName == a_rcGoal;
}

// $Barnyard: FUNCTION 00568700
void APlayerSteer::DisableKinematicsObstructionStop( TBOOL a_bDisable )
{
	GetKinematics3D( m_pKinematics )->SetObstructionStop( a_bDisable );
}

// $Barnyard: FUNCTION 0056a880
TBOOL APlayerSteer::UpdateWalking( TFLOAT a_fDeltaTime )
{
	// NOTE: inlined in retail
	ApplyGroundGravity();

	UpdateKinematics( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 00567880
TBOOL APlayerSteer::UpdateStill( TFLOAT a_fDeltaTime )
{
	// NOTE: inlined in retail
	ApplyGroundGravity();

	const TVector4* pPosition = GetPosition();
	m_pKinematics->MoveTo( a_fDeltaTime, GetPosition(), pPosition );

	return TFALSE;
}

// $Barnyard: FUNCTION 0056a8e0
TBOOL APlayerSteer::UpdateJumping( TFLOAT a_fDeltaTime )
{
	UpdateKinematics( a_fDeltaTime );

	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();

	if ( m_eJumpState == JUMPSTATE_START )
	{
		// Take off
		m_eJumpState = JUMPSTATE_UP;
		m_pKinematics->SetAirTime( 0.0f );
		m_pKinematics->SetVerticalSpeed( -m_pTraits->m_fJumpPower );

		TFLOAT fRoll = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -0.08726646f, 0.08726646f );
		GetKinematics3D( m_pKinematics )->SetTilt( 0.0f, fRoll, TFALSE );
	}
	else
	{
		if ( m_eJumpState != JUMPSTATE_UP )
		{
			UpdateLanding( a_fDeltaTime );
			return TTRUE;
		}

		TFLOAT fAnimationTime;

		if ( pAnimatableObject->IsAnimPlaying( TPS8( jump_stat_up ), 0.0f ) )
		{
			fAnimationTime = pAnimatableObject->GetAnimationTime( TPS8( jump_stat_up ) );
		}
		else
		{
			if ( !pAnimatableObject->IsAnimPlaying( TPS8( jump_run_up ), 0.0f ) )
			{
				StartFallAnimation();
				return TTRUE;
			}

			fAnimationTime = pAnimatableObject->GetAnimationTime( TPS8( jump_run_up ) );
		}

		// Lean forward with the speed
		if ( fAnimationTime > 0.1f )
			GetKinematics3D( m_pKinematics )->SetPitch( m_pKinematics->GetSpeedRatio() * 0.08726646f );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0056aa00
TBOOL APlayerSteer::UpdateFalling( TFLOAT a_fDeltaTime )
{
	if ( m_pKinematics->GetAirTime() > 0.5f )
	{
		StartFallAnimation();
	}
	else if ( m_bGrounded && m_eJumpState < JUMPSTATE_APEX )
	{
		m_eJumpState = JUMPSTATE_NONE;
		SetState( STATE_WALKING );
	}

	UpdateKinematics( a_fDeltaTime );
	UpdateLanding( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 00568c80
TBOOL APlayerSteer::UpdateThrow( TFLOAT a_fDeltaTime )
{
	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();

	if ( !pAnimatableObject->IsAnimPlaying( TPS8( throw ), 0.0f ) )
	{
		EndState( STATE_THROW );
		return TTRUE;
	}

	// Remember where the hand is, the velocity of the thrown object comes from its movement
	AModelInstanceRef pModelInstance = pAnimatableObject->GetModelInstance();

	TMatrix44 matHand;
	pModelInstance->GetBoneTransform( pModelInstance->GetBoneIndex( TPS8( cb_rlocator ) ), matHand );

	m_vThrowHandPosition = matHand.GetTranslation();
	m_fThrowHandTime     = g_oSystemManager.GetScheduler()->GetTotalTime();

	return TFALSE;
}

// $Barnyard: FUNCTION 0056bb20
TBOOL APlayerSteer::UpdateClimbing( TFLOAT a_fDeltaTime )
{
	if ( m_eActionState != STATE_CLIMBING )
	{
		GetKinematics3D( m_pKinematics )->SetClimbing( TFALSE );
		m_oLadderAnimListener.Disconnect();
	}

	// Stop looking around
	AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, GetParentUnit().FindComponent( &TGetClass( AUnitFacialComponent ) ) );
	pFacial->StopLook();

	GetKinematics3D( m_pKinematics )->SetTilt( 0.0f, 0.0f, TTRUE );

	TFLOAT             fMinSpeed         = m_pTraits->m_fMinLogicalClimbSpeed;
	TFLOAT             fMaxSpeed         = m_pTraits->m_fMaxLogicalClimbSpeed;
	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();

	// Direction up the ladder
	TVector4 vecUp( m_vLadderStart.x - m_vLadderBase.x, m_vLadderStart.y - m_vLadderBase.y, m_vLadderStart.z - m_vLadderBase.z, m_vLadderStart.w );
	TFLOAT   fLadderLength = vecUp.AsVector3().Magnitude();
	vecUp.AsVector3().Divide( fLadderLength );

	// The facing on the ladder
	TVector4 vecFacing( m_vLadderEnd.x - m_vLadderStart.x, 0.0f, m_vLadderEnd.z - m_vLadderStart.z, m_vLadderEnd.w );
	vecFacing.Normalise();

	// NOTE: retail calls this without using the result
	m_pKinematics->GetDesiredFacing();
	TFLOAT fInput = TVector4::DotProduct3( vecFacing, m_vDesiredFacing );

	// The top of the climbable part
	TVector4 vecTopOffset;
	vecTopOffset.Multiply( vecUp, 1.167f );

	TVector4 vecTop( m_vLadderStart.x - vecTopOffset.x, m_vLadderStart.y - vecTopOffset.y, m_vLadderStart.z - vecTopOffset.z, m_vLadderStart.w );

	TFLOAT fProgress;
	TMath::DistancePointToLineSq( *GetPosition(), m_vLadderBase, vecTop, fProgress );

	TFLOAT   fClimbed = ( fLadderLength - 1.167f ) * fProgress;
	TVector4 vecPosition;

	switch ( m_eClimbState )
	{
		case CLIMBSTATE_SEEK:
		{
			if ( fInput <= 0.0f )
				return TFALSE;

			vecPosition.Set( m_vLadderBase );

			if ( fClimbed > 0.1f )
			{
				TVector4 vecOffset;
				vecOffset.Multiply( vecUp, fClimbed );
				vecPosition.AsVector3().Add( vecOffset.AsVector3() );
			}

			m_iUnk4E4 = 0;
			m_pKinematics->SetFacing( vecFacing );
			m_pKinematics->SetPosition( vecPosition );

			if ( fClimbed > 0.1f )
			{
				// Grab the ladder in the middle
				pAnimatableObject->PlayAnimation( TPS8( ladder_climb ) );
				m_eClimbState = CLIMBSTATE_MOVE;
			}
			else
			{
				m_eClimbState = CLIMBSTATE_ATTACH;
			}

			return TFALSE;
		}
		case CLIMBSTATE_ATTACH:
		{
			vecPosition = m_vLadderBase;

			m_iUnk4E4 = 0;
			m_pKinematics->SetFacing( vecFacing );
			m_pKinematics->SetPosition( vecPosition );

			if ( !pAnimatableObject->IsAnimPlaying( TPS8( ladder_geton ), 0.0f ) )
			{
				pAnimatableObject->PlayAnimation( TPS8( ladder_geton ) );
				m_eClimbState = CLIMBSTATE_MOVE;

				TVector4 vecBase;
				vecBase.Set( m_vLadderBase );
				SetPosition( vecBase );
			}

			m_oAnimEventListener.Connect( this, AnimEventCallback );
			UpdateKinematics( a_fDeltaTime );

			TFLOAT fSpeed    = m_pKinematics->GetSpeed() * fInput * 3.0f;
			TFLOAT fAbsSpeed = TMath::Abs( fSpeed );

			if ( fAbsSpeed < fMinSpeed )
				fSpeed = ( fSpeed < 0.0f ) ? -fMinSpeed : fMinSpeed;
			else if ( fAbsSpeed > fMaxSpeed )
				fSpeed = ( fSpeed < 0.0f ) ? -fMaxSpeed : fMaxSpeed;

			m_fClimbSpeed  = fSpeed;
			m_bActionEnded = TFALSE;
			return TFALSE;
		}
		case CLIMBSTATE_MOVE:
		case CLIMBSTATE_OFFLOAD_UP:
		{
			m_pKinematics->SetFacing( vecFacing );

			if ( !pAnimatableObject->IsAnimationFinished( TPS8( ladder_geton ) ) )
			{
				SetPosition( m_vLadderBase );
				return TFALSE;
			}

			if ( !pAnimatableObject->IsAnimPlaying( TPS8( ladder_climb ), 0.0f ) && m_eClimbState != CLIMBSTATE_OFFLOAD_UP )
			{
				pAnimatableObject->PlayAnimation( TPS8( ladder_climb ) );

				if ( 0.0f != m_pKinematics->GetSpeed() )
					return TFALSE;

				m_pKinematics->SetMeasuredSpeed( 0.0f );
				m_bActionEnded = TTRUE;

				AToshiAnimationRef* pClimbAnimRef = AToshiAnimationInterface::GetAnimationRef( pAnimatableObject->GetToshiAnimInterface().GetAnimRefs(), TPS8( ladder_climb ) );

				if ( !pClimbAnimRef )
					return TFALSE;

				// Restart the climb animation
				pAnimatableObject->SetAnimationTime( TPS8( ladder_climb ), 0.0f );
				pAnimatableObject->GetToshiAnimInterface().SetAnimWeight( pClimbAnimRef, 1.0f );
				return TFALSE;
			}

			// A step ends at every rung
			m_bActionEnded     = TFALSE;
			TFLOAT fRungOffset = fmodf( fClimbed, 0.467f );

			if ( fRungOffset < 0.05f || fRungOffset > 0.417f )
				m_bActionEnded = TTRUE;

			TBOOL bKeepSpeed = TFALSE;
			m_pKinematics->SetFacing( vecFacing );
			UpdateKinematics( a_fDeltaTime );

			TFLOAT fSpeed    = m_pKinematics->GetSpeed() * fInput * 3.0f;
			TFLOAT fAbsSpeed = TMath::Abs( fSpeed );

			if ( m_bActionEnded == TTRUE && 0.0f == m_pKinematics->GetSpeed() )
			{
				// Stop at the rung
				fSpeed = 0.0f;
				m_pKinematics->SetMeasuredSpeed( 0.0f );
			}
			else if ( fAbsSpeed >= fMinSpeed )
			{
				if ( fAbsSpeed > fMaxSpeed )
					fSpeed = ( fSpeed >= 0.0f ) ? fMaxSpeed : -fMaxSpeed;
			}
			else
			{
				// Finish the step with the last speed
				bKeepSpeed = TTRUE;

				if ( fSpeed == 0.0f )
					fSpeed = m_fClimbSpeed;
				else
					fSpeed = ( fSpeed >= 0.0f ) ? fMinSpeed : -fMinSpeed;
			}

			if ( m_eClimbState == CLIMBSTATE_OFFLOAD_UP )
			{
				bKeepSpeed = TTRUE;
				fSpeed     = m_fClimbSpeed;
			}

			TBOOL bGettingOff = TFALSE;

			if ( fSpeed == 0.0f )
			{
				TVector4 vecOffset;
				vecOffset.Multiply( vecUp, fClimbed );
				vecPosition.Add( m_vLadderBase, vecOffset );
				SetPosition( vecPosition );
			}
			else
			{
				TFLOAT fStep = fSpeed * a_fDeltaTime;
				fClimbed += fStep;

				if ( !bKeepSpeed && ( fSpeed < 0.0f || ( fMaxSpeed - fMinSpeed ) * 0.8f + fMinSpeed < fSpeed ) )
					m_fClimbSpeed = fSpeed;

				m_bActionEnded = TFALSE;

				if ( fProgress < 0.001f && fStep < 0.0f )
				{
					// Climbed down off the ladder
					m_eClimbState = CLIMBSTATE_FALL;

					vecPosition.Set( *m_pKinematics->GetFacingDir() );
					vecPosition.Multiply( -0.2f );
					vecPosition.y -= 0.01f;
					vecPosition.AsVector3().Add( m_vLadderBase.AsVector3() );
					SetPosition( vecPosition );
				}
				else
				{
					TVector4 vecOffset;
					vecOffset.Multiply( vecUp, fClimbed );
					vecPosition.Add( m_vLadderBase, vecOffset );
					SetPosition( vecPosition );

					if ( fProgress > 0.999f && fStep > 0.0f && pAnimatableObject->IsAnimPlaying( TPS8( ladder_getoff_end ), 0.0f ) == TTRUE )
					{
						// Reached the top
						m_eClimbState = CLIMBSTATE_OFFLOAD_DOWN;
						SetPosition3( m_vLadderStart.AsVector3() );
						return TFALSE;
					}

					if ( fProgress > 0.999f && fStep > 0.0f && !pAnimatableObject->IsAnimPlaying( TPS8( ladder_getoff_start ), 0.0f ) && m_eClimbState != CLIMBSTATE_OFFLOAD_UP )
					{
						// Start getting off at the top
						pAnimatableObject->PlayAnimation( TPS8( ladder_getoff_start ) );
						SetPosition3( vecTop.AsVector3() );
						m_eClimbState  = CLIMBSTATE_OFFLOAD_UP;
						m_fGetOffStart = fClimbed;
						bGettingOff    = TTRUE;
					}
					else if ( ( pAnimatableObject->IsAnimPlaying( TPS8( ladder_getoff_start ), 0.0f ) && pAnimatableObject->IsAnimationPlanned( TPS8( ladder_getoff_end ) ) ) || pAnimatableObject->IsAnimationPlanned( TPS8( ladder_getoff_start ) ) )
					{
						bGettingOff = TTRUE;
					}

					SetPosition( vecPosition );
					m_pKinematics->SetMeasuredSpeed( fSpeed );

					if ( bGettingOff )
						return TFALSE;
				}
			}

			// Sync the animations with the climbed distance
			if ( pAnimatableObject->IsAnimPlaying( TPS8( ladder_getoff_start ), 0.0f ) )
			{
				pAnimatableObject->SetAnimationTime( TPS8( ladder_getoff_start ), ( fClimbed - fSpeed * a_fDeltaTime * 0.5f - m_fGetOffStart ) * 0.92336106f * pAnimatableObject->GetAnimationDuration( TPS8( ladder_getoff_start ) ) );
				return TFALSE;
			}

			TFLOAT fClimbDuration = pAnimatableObject->GetAnimationDuration( TPS8( ladder_climb ) );
			TFLOAT fClimbTime     = fmodf( fClimbed - fSpeed * a_fDeltaTime * 0.5f, 0.934f ) * 1.0706638f;

			if ( fClimbTime < 0.0f )
				fClimbTime = 0.0f;

			pAnimatableObject->SetAnimationTime( TPS8( ladder_climb ), fClimbTime * fClimbDuration );
			return TFALSE;
		}
		case CLIMBSTATE_UP:
			m_pKinematics->SetFacing( vecFacing );
			return TFALSE;
		case CLIMBSTATE_DOWN:
			m_pKinematics->SetFacing( vecFacing );

			vecPosition = m_vLadderBase;
			SetPosition( vecPosition );
			DropToGround();
			return TFALSE;
		case CLIMBSTATE_OFFLOAD_DOWN:
		{
			m_oAnimEventListener.Disconnect();
			m_pKinematics->SetFacing( vecFacing );

			// How far the get off animation is
			TFLOAT fGetOffProgress = 1.0f;

			if ( pAnimatableObject->IsAnimPlaying( TPS8( ladder_getoff_end ), 0.0f ) )
			{
				fGetOffProgress = pAnimatableObject->GetAnimationProgress( TPS8( ladder_getoff_end ) );
			}

			TVector4 vecOffload;
			vecOffload.Substract( m_vLadderEnd, m_vLadderStart );

			TFLOAT fOffloadLength = vecOffload.AsVector3().Magnitude();
			vecOffload.AsVector3().Divide( fOffloadLength );

			m_pKinematics->SetMeasuredSpeed( 1.0f );

			TVector4 vecOffset;
			vecOffset.Multiply( vecOffload, fOffloadLength * fGetOffProgress );
			vecPosition.Add( m_vLadderStart, vecOffset );

			if ( fGetOffProgress < 0.999f )
			{
				SetPosition( vecPosition );
				return TFALSE;
			}

			SetPosition( m_vLadderEnd );

			// Off the ladder
			SetState( STATE_FALLING );
			TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) )->SetYawLimits( TFALSE, 0.0f, 0.0f );
			return TFALSE;
		}
		case CLIMBSTATE_FALL:
			m_oAnimEventListener.Disconnect();
			m_oLadderAnimListener.Disconnect();
			m_pKinematics->SetFacing( vecFacing );

			// Off the ladder
			SetState( STATE_FALLING );
			TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) )->SetYawLimits( TFALSE, 0.0f, 0.0f );
			return TFALSE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00568be0
void APlayerSteer::OnCreate()
{
	DropToGround();

	m_bSteppedUp        = TFALSE;
	m_bUnk569           = TTRUE;
	m_bSpeechDialogMode = TFALSE;

	m_pKinematics->SetFrozen( TFALSE );
	m_pKinematics->Unk_0x9c( TFALSE );
	m_pKinematics->SetSeekSlowdown( TFALSE );
	AKinematicsObstructionManager::GetSingleton()->AddObstruction( TSTATICCAST( AKinematicsObstruction, GetParentUnit().GetObstruction() ) );

	m_iIgnoreGroups = 0x30000;
	m_pSinkFade     = TNULL;
	m_vLastPosition = *GetPosition();
	m_iBushesCue    = -1;
}

// $Barnyard: FUNCTION 005676d0
void APlayerSteer::ApplyGroundGravity()
{
	TFLOAT fVerticalSpeed = m_pKinematics->GetVerticalSpeed();

	// Damp the vertical speed while on the ground
	if ( m_eActionState != STATE_JUMPING && m_bGrounded )
		m_pKinematics->SetVerticalSpeed( fVerticalSpeed * 0.5f );
}

// $Barnyard: FUNCTION 00568600
void APlayerSteer::DropToGround()
{
	TVector4 vPosition = *m_pKinematics->GetPosition();

	if ( ATerrainInterface::IsSingletonCreated() )
		vPosition.y = GetGroundHeight( vPosition );

	TVector4 vOffset;
	vOffset.Multiply( TVector4::VEC_NEGY, 0.2f );
	vPosition.Add( vOffset );

	m_pKinematics->SetPosition( vPosition );

	// Settle on the ground right away
	if ( TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject() && !m_pGoal )
		Update( 1.0f );
}

// $Barnyard: FUNCTION 00568d80
void APlayerSteer::PlaceAtCurrentGoal()
{
	GetCurrentNetwork();

	TVector4 vPosition( m_pGoal->m_fPositionX, -8.0f, m_pGoal->m_fPositionZ, 1.0f );
	m_oPath.m_iStartArea = m_pGoal->m_iAreaIndex;

	m_pGoal->GetPosition( vPosition );
	vPosition.y -= 0.01f;

	m_pKinematics->SetPosition( vPosition );
	m_pGoal = TNULL;

	DropToGround();
	m_vLastPosition = *GetPosition();
}

// $Barnyard: FUNCTION 0056ad50
TBOOL APlayerSteer::UpdateGoal()
{
	if ( !m_pGoal )
		return TFALSE;

	AUnit* pUnit = &GetParentUnit();

	if ( m_pGoal->m_iGroupId == -1 )
	{
		PlaceAtCurrentGoal();
		return TFALSE;
	}

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	ATerrainSection*   pSection = &pTerrain->GetVIS()->GetSections()[ m_pGoal->m_iGroupId ];

	if ( pSection->IsLODLoaded( ATerrainLODType_High ) && pTerrain->IsLoaded() )
	{
		// The terrain around the goal is there, place the player
		PlaceAtCurrentGoal();

		ACameraManager::GetSingleton()->SetSplitscreenCameraHelper( 1 );
		ACameraManager::GetSingleton()->GetCurrentCameraHelper()->SetTarget( pUnit->GetSelfProxy() );

		if ( m_eActionState == STATE_STILL )
			SetState( STATE_WALKING );

		return TFALSE;
	}

	if ( !pSection->IsLODLoading( ATerrainLODType_High ) )
	{
		// Load the terrain around the goal and wait for it
		SetState( STATE_STILL );

		ACameraManager::GetSingleton()->SetSplitscreenCameraHelper( 1 );
		ACameraManager::GetSingleton()->GetCurrentCameraHelper()->SetTarget( pUnit->GetSelfProxy() );

		pSection->ForceLoad( ATerrainLODType_High );
		pTerrain->WaitUntilLoaded();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005692a0
AObjectTraits* APlayerSteer::GetCarriedObjectTraits()
{
	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, &GetParentUnit() );

	// The carried object is attached to the right hand locator
	TINT iBone = pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneID( TPS8( cb_rlocator ) );

	if ( iBone >= 0 )
	{
		AUnit* pCarried = pUnit->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

		if ( pCarried && pCarried->GetType() )
			return TSTATICCAST( AObjectTraits, pCarried->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00568250
TBOOL APlayerSteer::StartFallAnimation()
{
	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();

	if ( m_eJumpState >= JUMPSTATE_APEX )
		return TFALSE;

	if ( !pAnimatableObject->IsAnimPlaying( TPS8( jump_stat_fall ), 0.0f ) && !pAnimatableObject->IsAnimPlaying( TPS8( jump_run_fall ), 0.0f ) )
	{
		Movement eMovement = GetMovementType();
		TBOOL    bPlayed;

		if ( eMovement == MOVEMENT_RUN || eMovement == MOVEMENT_SPRINT )
			bPlayed = pAnimatableObject->PlayAnimation( TPS8( jump_run_fall ) );
		else
			bPlayed = pAnimatableObject->PlayAnimation( TPS8( jump_stat_fall ) );

		if ( !bPlayed )
			return TFALSE;
	}

	// Lean back a bit (-5 degrees)
	GetKinematics3D( m_pKinematics )->SetTilt( -0.08726646f, 0.0f, TFALSE );
	m_eJumpState = JUMPSTATE_APEX;

	return TTRUE;
}

// $Barnyard: FUNCTION 005682e0
TBOOL APlayerSteer::UpdateLanding( TFLOAT a_fDeltaTime )
{
	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();

	if ( m_eJumpState == JUMPSTATE_END )
	{
		if ( m_pKinematics->GetMeasuredSpeed() < 0.1f )
			m_pKinematics->SetSpeed( 0.0f );

		// Walk once the landing animation is over
		if ( !pAnimatableObject->IsAnimPlaying( TPS8( jump_stat_land ), 0.0f ) && !pAnimatableObject->IsAnimPlaying( TPS8( jump_run_land ), 0.0f ) )
			SetState( STATE_WALKING );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0056a700
void APlayerSteer::UpdateKinematics( TFLOAT a_fDeltaTime )
{
	ASteerKinematics::UPDATEFLAGS uiFlags = ASteerKinematics::UPDATEFLAGS_NO_AVOID;

	if ( !m_bUnk569 || m_bSpeechDialogMode == TTRUE )
		uiFlags = ASteerKinematics::UPDATEFLAGS_NO_AVOID | ASteerKinematics::UPDATEFLAGS_NO_PUSH;

	AObjectTraits* pTraits = GetCarriedObjectTraits();

	if ( m_pKinematics->GetClass()->IsA( &TGetClass( ASteerKinematics3D ) ) )
		GetKinematics3D( m_pKinematics )->SetCarryingHeavy( pTraits && pTraits->IsHeavy() );

	ASteerKinematics::s_oObstructionLists.Add( GetNetwork() );

	AKinematicsObstruction* pOwnObstruction = TSTATICCAST( AKinematicsObstruction, GetParentUnit().GetObstruction() );
	m_pKinematics->Update( a_fDeltaTime, &ASteerKinematics::s_oObstructionLists, &pOwnObstruction, 1, uiFlags );

	ASteerKinematics::s_oObstructionLists.Clear();
}

// $Barnyard: FUNCTION 00567430
void APlayerSteer::UpdatePath( TFLOAT a_fDeltaTime )
{
	TVector4          vecPosition = *m_pKinematics->GetPosition();
	AInteriorNetwork* pNetwork    = GetNetwork();

	if ( pNetwork )
	{
		// Keep the path start in the area the steer is in
		TINT iArea = pNetwork->FindAreaAtPosition( *m_pKinematics->GetPosition(), m_oPath.m_iStartArea );

		if ( iArea != -1 )
		{
			TVector4 vecFacing       = *m_pKinematics->GetFacingDir();
			TVector4 vecPathPosition = vecPosition;

			pNetwork->SetPathStartArea( &m_oPath, iArea, vecPathPosition, vecFacing );

			// NOTE: the new position isn't used
			pNetwork->UpdateHardAvoidance( &m_oPath, vecPosition, TNULL, TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 005677d0
TVector4& APlayerSteer::GetPointInFront( TVector4& a_rOut )
{
	TVector4 vPosition = *GetPosition();

	TVector4 vOffset;
	vOffset.Multiply( *GetDirection(), 2.0f );

	a_rOut.x = vOffset.x + vPosition.x;
	a_rOut.y = vOffset.y + vPosition.y;
	a_rOut.z = vOffset.z + vPosition.z;
	a_rOut.w = vPosition.w;

	return a_rOut;
}

// $Barnyard: FUNCTION 005693e0
void APlayerSteer::UpdateCarrying()
{
	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, &GetParentUnit() );

	if ( pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneID( TPS8( cb_rlocator ) ) < 0 )
		return;

	AUnit* pCarried = pUnit->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

	if ( !pCarried )
		return;

	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pCarried->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	// Only the detector is kept away from the walls
	if ( !pTraits || !pTraits->IsDetector() )
		return;

	TVector4 vecFront;
	GetPointInFront( vecFront );

	TVector4 vecPosition = *GetPosition();
	vecFront.y           = vecPosition.y - 1.2f;

	// Three rays from the head to 2 metres in front of it: straight ahead and turned by 0.3 radians to each side
	TVector4 vecRayEnd( vecFront.x, vecFront.y - 0.6f, vecFront.z, vecFront.w );

	ACollisionObject* pCollisionObject = pUnit->GetCollisionObject();
	TFLOAT            fOldRadius       = pCollisionObject->GetObjectRadius();
	pCollisionObject->UpdateHash( 1.1f );

	TVector4 vecRayStart( vecPosition.x, vecPosition.y - 1.8f, vecPosition.z, vecPosition.w );
	ARayData oRay( vecRayStart, vecRayEnd );

	ARayData oRightRay = oRay;
	oRightRay.vDirection.AsVector3().RotateY( 0.3f );

	ARayData oLeftRay = oRay;
	oLeftRay.vDirection.AsVector3().RotateY( -0.3f );

	ACollisionManager* pCollisionManager = ACollisionManager::GetSingleton();
	TFLOAT             fRayLength        = oRay.fLength;

	TFLOAT fDistance      = fRayLength;
	TFLOAT fRightDistance = fRayLength;
	TFLOAT fLeftDistance  = fRayLength;

	if ( !pCollisionManager->CollideRay( pUnit, oRay, fDistance, m_iIgnoreGroups, ACollisionObject::TYPE_IGNORED ) &&
	     !pCollisionManager->CollideRay( pUnit, oRightRay, fRightDistance, m_iIgnoreGroups, ACollisionObject::TYPE_IGNORED ) &&
	     !pCollisionManager->CollideRay( pUnit, oLeftRay, fLeftDistance, m_iIgnoreGroups, ACollisionObject::TYPE_IGNORED ) )
	{
		pCollisionObject->UpdateHash( fOldRadius );
		return;
	}

	pCollisionObject->UpdateHash( fOldRadius );

	// The detector hits something, turn away from it alternating the sides with growing angles
	TVector4 vecFacing = *GetDirection();
	TBOOL    bPositive = TFALSE;

	TVector4 vecNewFacing = vecFacing;
	vecNewFacing.AsVector3().RotateY( 0.1f );

	TFLOAT fAngle = 0.1f;
	GetKinematics()->SetFacing( vecNewFacing );
	GetKinematics()->SetDesiredFacing( vecNewFacing );

	TVector4 vecRayDirection;
	vecRayDirection.Substract( vecRayEnd, vecPosition );

	TVector4 vecNewDirection = vecRayDirection;
	vecNewDirection.AsVector3().RotateY( 0.1f );
	vecNewDirection.Normalise();

	oRay.vDirection = vecNewDirection;

	oRightRay.vDirection = vecNewDirection;
	oRightRay.vDirection.AsVector3().RotateY( 0.3f );

	oLeftRay.vDirection = vecNewDirection;
	oLeftRay.vDirection.AsVector3().RotateY( -0.3f );

	fDistance      = fRayLength;
	fRightDistance = fRayLength;
	fLeftDistance  = fRayLength;

	for ( TINT i = 29;; )
	{
		if ( !pCollisionManager->CollideRay( pUnit, oRay, fDistance, m_iIgnoreGroups, ACollisionObject::TYPE_IGNORED ) &&
		     !pCollisionManager->CollideRay( pUnit, oRightRay, fRightDistance, m_iIgnoreGroups, ACollisionObject::TYPE_IGNORED ) &&
		     !pCollisionManager->CollideRay( pUnit, oLeftRay, fLeftDistance, m_iIgnoreGroups, ACollisionObject::TYPE_IGNORED ) )
			break;

		vecNewFacing    = vecFacing;
		vecNewDirection = vecRayDirection;

		TFLOAT fRotation;

		if ( bPositive )
		{
			fAngle += 0.1f;
			fRotation = fAngle;
		}
		else
		{
			fRotation = -fAngle;
		}

		vecNewFacing.AsVector3().RotateY( fRotation );
		vecNewDirection.AsVector3().RotateY( fRotation );

		GetKinematics()->SetFacing( vecNewFacing );
		GetKinematics()->SetDesiredFacing( vecNewFacing );

		bPositive = !bPositive;
		vecNewDirection.Normalise();

		oRay.vDirection = vecNewDirection;

		oRightRay.vDirection = vecNewDirection;
		oRightRay.vDirection.AsVector3().RotateY( 0.3f );

		oLeftRay.vDirection = vecNewDirection;
		oLeftRay.vDirection.AsVector3().RotateY( -0.3f );

		fDistance      = fRayLength;
		fRightDistance = fRayLength;
		fLeftDistance  = fRayLength;

		if ( --i <= 0 )
			break;
	}
}

// $Barnyard: FUNCTION 00569dc0
void APlayerSteer::UpdateWater( TFLOAT a_fDeltaTime )
{
	if ( m_vLastPosition.MagnitudeSq() < 0.0001f )
		return;

	if ( m_bIsInWater == TTRUE )
	{
		m_fWaterTime += a_fDeltaTime;
		return;
	}

	// Cast a ray from the last position to the current one
	TVector4 vecDirection;
	vecDirection.Substract( *GetPosition(), m_vLastPosition );
	TFLOAT fDistance = vecDirection.NormaliseMag();

	TVector4                  vecOrigin = m_vLastPosition;
	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideRay( vecOrigin, vecDirection, fDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE );

	TINT   iClosestHit      = -1;
	TFLOAT fClosestDistance = TMath::MAXFLOAT;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		if ( oHitList.aHits[ i ].fDistance < fClosestDistance )
		{
			iClosestHit      = i;
			fClosestDistance = oHitList.aHits[ i ].fDistance;
		}
	}

	if ( iClosestHit > -1 )
	{
		ACollisionModelRayHitList::Hit& rHit      = oHitList.aHits[ iClosestHit ];
		TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );

		// Fell into deep water
		if ( pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).eCollGroup == TCollisionCommon::TOSHICGROUP_DEEPWATER )
		{
			TVector4 vecOffset;
			vecOffset.Multiply( vecDirection, fClosestDistance );

			TVector4 vecSplash;
			vecSplash.Add( vecOrigin, vecOffset );

			m_bIsInWater = TTRUE;
			m_fWaterTime = 0.0f;

			ASoundManager::GetSingleton()->PlayCueAtPosition(
			    ( APlayerProfileManager::GetSingleton()->GetMainGender() == APlayerProfile::GENDER_FEMALE ) ? soundbank::AM_F_SPLASH : soundbank::AM_M_SPLASH,
			    vecSplash
			);

			AParticleEffectManager* pEffectManager = AParticleEffectManager::GetSingleton();
			pEffectManager->PlayEffect( m_iWaterSplashAEffect, vecSplash, TVector4::VEC_NEGY );
			pEffectManager->PlayEffect( m_iWaterSplashBEffect, vecSplash, TVector4::VEC_NEGY );
			pEffectManager->PlayEffect( m_iWaterRippleEffect, vecSplash, TVector4::VEC_NEGY );

			ASplashManager::GetSingleton()->Spawn( vecSplash, TVector4::VEC_NEGY, 1.0f, ASplashManager::SPLASHTYPE_WATER );

			AInputHandler::RumbleParams oRumble = { { 0.8f, 0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f }, 1 };
			AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );

			// Take the control away while sinking
			ASimplePlayerControlComponent* pControlComponent = TSTATICCAST( ASimplePlayerControlComponent, GetParentUnit().FindComponent( &TGetClass( APlayerControlComponent ) ) );

			if ( !pControlComponent )
				pControlComponent = TSTATICCAST( ASimplePlayerControlComponent, GetParentUnit().FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );

			if ( pControlComponent )
				pControlComponent->SetMovementEnabled( TFALSE );

			// Fade to black
			m_pSinkFade = AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 1.0f );
			m_oSinkFadeListener.Connect( m_pSinkFade->GetFadeOverEmitter(), this, SinkFadeOverCallback, 0 );
		}
	}

	m_vLastPosition = *GetPosition();
}

// $Barnyard: FUNCTION 0056a190
void APlayerSteer::UpdateSinking()
{
	if ( m_bIsInWater )
	{
		AUnitPlayer* pPlayer = TSTATICCAST( AUnitPlayer, &GetParentUnit() );

		if ( pPlayer->GetActiveLauncher() )
			pPlayer->SelectLauncher( TFALSE );

		ACameraManager::GetSingleton()->DetachCameraHelpers();
	}

	if ( m_bIsInWater && m_fWaterTime > 1.0f )
	{
		m_fWaterTime = 0.0f;
		m_bIsInWater = TFALSE;

		// Respawn at the closest water respawn goal
		AInteriorGoal* pGoal = GetNetwork()->FindClosestGoalOrFirst( TPS8( water_respawn_goal ), *GetPosition() );
		PlaceAtGoal( pGoal, TTRUE, TFALSE );
		DropToGround();

		m_eJumpState  = JUMPSTATE_END;
		m_eClimbState = CLIMBSTATE_NONE;
		SetState( STATE_WALKING );

		AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();
		pAnimatableObject->KillAllAnimations();
		pAnimatableObject->PlayAnimation( TPS8( yuk ) );
		pAnimatableObject->Update( 0.0f );

		// Bring the GTA style camera back, looking at the player from the water
		ACameraManager* pCameraManager = ACameraManager::GetSingleton();
		AUnit*          pUnit          = &GetParentUnit();

		pCameraManager->SetSplitscreenCameraHelper( 1 );
		pCameraManager->GetCurrentCameraHelper()->SetTarget( pUnit->GetSelfProxy() );

		AGTAStyleCameraHelper* pGTAHelper = TSTATICCAST( AGTAStyleCameraHelper, pCameraManager->GetSplitscreenCameraHelper( 1 ) );

		TVector4 vecOffset;
		vecOffset.Multiply( pGoal->m_vForward, 5.0f );

		TVector4 vecCameraPosition;
		vecCameraPosition.Add( *GetPosition(), vecOffset );
		vecCameraPosition.y -= pGTAHelper->GetHeightOffset();

		TVector4 vecCameraTarget = *GetPosition();
		vecCameraTarget.y -= pGTAHelper->GetHeightOffset();

		pGTAHelper->SetPosition( vecCameraPosition, vecCameraTarget, TTRUE );

		// Put the bikes back to the racks
		if ( AGameStateController::GetSingleton()->GetCurrentState()->IsA( &TGetClass( ASimState ) ) )
			ABikeManager::GetSingleton()->RespawnBikes( 1.0f / 30.0f, TTRUE );

		PlayMilkDrip();

		// End the treasure hunt
		AGameStateController* pController = AGameStateController::GetSingleton();

		if ( pController->GetCurrentState()->IsA( &TGetClass( ATreasueHuntMicroGame ) ) )
		{
			pController->PopCurrentGameState();
		}
		else if ( AGameState* pTreasureHunt = pController->GetCurrentState()->FindChildState( &TGetClass( ATreasueHuntMicroGame ) ) )
		{
			pTreasureHunt->Remove();
		}

		// Give the control back
		ASimplePlayerControlComponent* pControlComponent = TSTATICCAST( ASimplePlayerControlComponent, GetParentUnit().FindComponent( &TGetClass( APlayerControlComponent ) ) );

		if ( !pControlComponent )
		{
			pControlComponent = TSTATICCAST( ASimplePlayerControlComponent, GetParentUnit().FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );

			if ( !pControlComponent )
				return;
		}

		pControlComponent->SetEnabled( TTRUE );
		pControlComponent->SetMovementEnabled( TTRUE );
	}
}

// $Barnyard: FUNCTION 0056b090
TBOOL APlayerSteer::UpdateCollision( TFLOAT a_fDeltaTime, TVector4* a_pPosition, TBOOL a_bUnk1, TBOOL a_bUnk2, TBOOL a_bUnk3 )
{
	TVector4 vecTarget   = *m_pKinematics->GetPosition();
	TVector4 vecPosition = vecTarget;
	TVector4 vecFacing   = *m_pKinematics->GetFacingDir();
	TVector4 vecNormal   = TVector4::VEC_NEGY;
	TBOOL    bCollided   = TFALSE;

	if ( !vecTarget.IsEqual( *a_pPosition ) )
	{
		// Sweep the collision sphere from the previous position to the new one
		TFLOAT   fRadius = GetRadius();
		TVector4 vecCenter( a_pPosition->x, a_pPosition->y - fRadius, a_pPosition->z, a_pPosition->w );

		TVector4 vecDirection( vecTarget.x - a_pPosition->x, vecTarget.y - a_pPosition->y, vecTarget.z - a_pPosition->z, vecTarget.w );
		TFLOAT   fDistance = vecDirection.NormaliseMag();

		if ( 10.0f < fDistance )
		{
			// Don't move too far at once
			fDistance = 10.0f;

			vecTarget = vecDirection;
			vecTarget.Multiply( 10.0f );
			vecTarget.x = vecTarget.x + a_pPosition->x;
			vecTarget.y = vecTarget.y + a_pPosition->y;
			vecTarget.z = vecTarget.z + a_pPosition->z;
			m_pKinematics->SetPosition( vecTarget );
		}

		TBOOL             bUnk       = TFALSE;
		ACollisionObject* pHitObject = TNULL;
		TVector4          vecNewPosition;

		bCollided = !CollideAndSlide( vecCenter, vecDirection, fDistance, vecNewPosition, vecFacing, vecNormal, bUnk, a_bUnk1, &pHitObject, a_fDeltaTime );

		if ( bCollided )
		{
			vecPosition.x = vecNewPosition.x;
			vecPosition.y = vecNewPosition.y + fRadius;
			vecPosition.z = vecNewPosition.z;
			vecPosition.w = vecNewPosition.w;
			m_pKinematics->SetPosition( vecPosition );

			if ( ( !a_bUnk2 || !m_bUnk584 ) && !IsInteracting() )
				m_pKinematics->SetFacing( vecFacing );

			// Play the bush effect when running into a bush
			if ( pHitObject && ( pHitObject->GetType() & ACollisionObject::TYPE_BUSH ) && m_iBushHitEffect != -1 )
			{
				ASoundManager* pSoundManager = ASoundManager::GetSingleton();

				if ( ( m_pKinematics->GetSpeed() > 1.0f || m_pKinematics->GetVerticalSpeed() > 8.0f ) &&
				     ( m_iBushesCue == -1 || !pSoundManager->IsCueUsed( m_iBushesCue ) ) )
				{
					AParticleEffectManager::GetSingleton()->PlayEffect( m_iBushHitEffect, vecPosition, vecFacing );
					m_iBushesCue = pSoundManager->PlayCueAtPosition( soundbank::BARNYARD_BUSHES, vecPosition );
				}
			}
		}
	}

	// Stand on the ground if the steer sunk into it
	vecPosition = *m_pKinematics->GetPosition();

	TVector4 vecRayStart( vecPosition.x, vecPosition.y - 1.3f, vecPosition.z, vecPosition.w );
	TVector4 vecRayEnd( vecPosition.x, vecPosition.y + 0.2f, vecPosition.z, vecPosition.w );
	ARayData oRay( vecRayStart, vecRayEnd );

	TFLOAT fGroundDistance = 1.5f;

	if ( ACollisionManager::GetSingleton()->CollideRay( &GetParentUnit(), oRay, fGroundDistance, m_iIgnoreGroups, ACollisionObject::TYPE_UNIT ) )
	{
		TFLOAT fGroundHeight = fGroundDistance + vecRayStart.y;

		if ( fGroundHeight < vecPosition.y )
		{
			vecPosition.y = fGroundHeight;
			m_pKinematics->SetPosition( vecPosition );
			bCollided = TTRUE;
		}
	}

	if ( a_bUnk2 )
		UpdateGroundState( *a_pPosition, vecPosition, vecNormal, bCollided, a_fDeltaTime );

	if ( a_bUnk3 && bCollided && -0.8f < vecNormal.y )
	{
		// Hit a wall, try to step up on it if walking into it
		TVector4 vecWallNormal( vecNormal.x, 0.0f, vecNormal.z, vecNormal.w );
		vecWallNormal.Normalise();

		if ( TVector4::DotProduct3( vecWallNormal, *m_pKinematics->GetFacingDir() ) < 0.0f )
			TryStepUp( a_fDeltaTime, a_bUnk1 );
	}

	return bCollided;
}

// $Barnyard: FUNCTION 00568a10
TBOOL APlayerSteer::CollideAndSlide( const TVector4& a_rcCenter, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rOutPosition, TVector4& a_rFacing, TVector4& a_rNormal, TBOOL& a_rbUnk, TBOOL a_bSlopeSlowdown, ACollisionObject** a_ppHitObject, TFLOAT a_fDeltaTime )
{
	ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject();
	pCollisionObject->SetType( ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE );

	a_rbUnk   = TFALSE;
	a_rNormal = TVector4::VEC_NEGY;

	TINT  eIgnoreGroups  = m_iIgnoreGroups;
	TBOOL bSlopeSlowdown = a_bSlopeSlowdown;

	// NOTE: retail stores the step height in the stack slot of a_bSlopeSlowdown
	TFLOAT fStepHeight;

	if ( m_bUnk584 == TTRUE )
	{
		// NOTE: m_bUnk584 is only ever set to TFALSE (by the constructor), so retail never takes this path.
		// The final direction goes into a_rFacing and the hit object isn't output.
		TFLOAT fRadius = pCollisionObject->GetObjectRadius();

		return !ACollisionManager::GetSingleton()->SweepAndSlide( a_rcCenter, fRadius, pCollisionObject, a_rcDirection, a_rfDistance, 0.4f, 1.0f, fStepHeight, a_rOutPosition, a_rFacing, ACollisionObject::TYPE_IGNORED, TCollisionCommon::TOSHICGROUPFLAG( eIgnoreGroups ), bSlopeSlowdown, &a_rbUnk, -0.5f, 0.5f, -0.47f );
	}

	TFLOAT fRadius = pCollisionObject->GetObjectRadius();

	return !SweepAndSlide( a_rcCenter, fRadius, pCollisionObject, a_rcDirection, a_rfDistance, 0.4f, 1.0f, fStepHeight, a_rOutPosition, a_rNormal, a_ppHitObject, ACollisionObject::TYPE_UNIT, eIgnoreGroups, bSlopeSlowdown, &a_rbUnk, -0.5f, 0.55f, -0.47f );
}

// $Barnyard: FUNCTION 00567a20
TBOOL APlayerSteer::SweepAndSlide( const TVector4& a_rcCenter, TFLOAT a_fRadius, ACollisionObject* a_pObject, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, TVector4& a_rOutPosition, TVector4& a_rOutNormal, ACollisionObject** a_ppHitObject, TUINT a_eFlags, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bSlopeSlowdown, TBOOL* a_pUnk, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope, TFLOAT a_fUnused )
{
	a_rfStepHeight = -1.0f;

	if ( a_pUnk )
		*a_pUnk = TFALSE;

	ACollisionManager* pCollisionManager = ACollisionManager::GetSingleton();

	TBOOL bTouchingTerrain = ACollisionManager::IsSphereTouchingTerrain( a_rcCenter, a_fRadius, a_rcDirection, a_eFlags, TINT( TUINTPTR( a_pUnk ) ), a_eIgnoreGroups );

	ACollisionObject* pHitObject     = TNULL;
	TBOOL             bHit           = TFALSE;
	TINT              iNumIterations = 0;

	// Where the movement is going to end
	TVector4 vecMovement;
	vecMovement.Multiply( a_rcDirection, a_rfDistance );

	TVector4 vecTarget( vecMovement.x + a_rcCenter.x, vecMovement.y + a_rcCenter.y, vecMovement.z + a_rcCenter.z, a_rcCenter.w );
	TVector4 vecPosition  = a_rcCenter;
	a_rOutPosition        = vecTarget;
	TVector4 vecDirection = a_rcDirection;

	TFLOAT fUpperOffset = a_fRadius * 1.8f;
	TFLOAT fDistance    = a_rfDistance;
	TBOOL  bNormalSet   = TFALSE;
	TFLOAT fStepHeight  = a_rfStepHeight;

	while ( TTRUE )
	{
		TFLOAT fDistanceLeft  = fDistance;
		TFLOAT fUpperDistance = fDistance;

		TVector4 vecUpperPosition( vecPosition.x, vecPosition.y - fUpperOffset, vecPosition.z, vecPosition.w );

		TVector4 vecLowerNormal;
		TVector4 vecUpperNormal;
		TFLOAT   fPenetration;

		TUINT uiLowerHit = pCollisionManager->CollideSweptSphere( vecPosition, a_fRadius, a_pObject, vecDirection, fDistance, vecLowerNormal, a_fMaxStepHeight, fStepHeight, fPenetration, 0, a_eFlags, &pHitObject, a_eIgnoreGroups );

		if ( uiLowerHit != 0 )
		{
			if ( !bNormalSet )
			{
				a_rfStepHeight = fStepHeight;
				a_rOutNormal   = vecLowerNormal;
				bNormalSet     = TTRUE;
			}

			*a_ppHitObject = pHitObject;
		}

		TUINT uiUpperHit = pCollisionManager->CollideSweptSphere( vecUpperPosition, a_fRadius, a_pObject, vecDirection, fUpperDistance, vecUpperNormal, a_fMaxStepHeight, fStepHeight, fPenetration, 0, a_eFlags, &pHitObject, a_eIgnoreGroups );

		if ( uiLowerHit == 0 && uiUpperHit == 0 )
			break;

		bHit = TTRUE;

		// Use the closest hit
		TFLOAT          fHitDistance;
		const TVector4* pHitNormal;

		if ( uiLowerHit == 0 || ( uiUpperHit != 0 && fDistance > fUpperDistance ) )
		{
			fHitDistance = fUpperDistance;
			pHitNormal   = &vecUpperNormal;
		}
		else
		{
			fHitDistance = fDistance;
			pHitNormal   = &vecLowerNormal;
		}

		if ( fHitDistance >= 0.0f )
		{
			TVector4 vecOffset;
			vecOffset.Multiply( vecDirection, fHitDistance );

			a_rOutPosition.x = vecOffset.x + vecPosition.x;
			a_rOutPosition.y = vecOffset.y + vecPosition.y;
			a_rOutPosition.z = vecOffset.z + vecPosition.z;
			a_rOutPosition.w = vecPosition.w;
		}
		else
		{
			a_rOutPosition = vecPosition;
			fHitDistance   = 0.0f;
		}

		// Slide along the hit surface
		TFLOAT   fSlideDistance = 0.0f;
		TVector4 vecSlideDirection;
		TMath::SlideAlongPlane( vecDirection, *pHitNormal, a_fSpeedScale, fDistanceLeft, fHitDistance, fSlideDistance, vecSlideDirection, 0.1f, 0.004f, 1.0f );

		if ( a_bSlopeSlowdown && a_rcDirection.y > 0.9f )
			ACollisionManager::ScaleSpeedBySlope( fSlideDistance, vecSlideDirection, a_fMinSlope, a_fMaxSlope );

		vecPosition = a_rOutPosition;

		if ( fSlideDistance > 0.0f )
		{
			TVector4 vecSlide;
			vecSlide.Multiply( vecSlideDirection, fSlideDistance );

			vecTarget.x = vecSlide.x + a_rOutPosition.x;
			vecTarget.y = vecSlide.y + a_rOutPosition.y;
			vecTarget.z = vecSlide.z + a_rOutPosition.z;
			vecTarget.w = a_rOutPosition.w;
		}

		// Continue the movement towards the target
		vecDirection.Set( vecTarget.x - vecPosition.x, vecTarget.y - vecPosition.y, vecTarget.z - vecPosition.z, vecTarget.w );
		fDistance = vecDirection.MagnitudeSq();

		if ( fDistance == 0.0f )
		{
			vecDirection = TVector4::VEC_NEGY;
		}
		else
		{
			fDistance = TMath::Sqrt( fDistance );
			vecDirection.Multiply( 1.0f / fDistance );
		}

		if ( ++iNumIterations >= 15 || fDistance <= 0.0f )
			return bTouchingTerrain || bHit;
	}

	if ( bHit )
		a_rOutPosition = vecTarget;

	return bTouchingTerrain || bHit;
}

// $Barnyard: FUNCTION 00568730
void APlayerSteer::UpdateGroundState( const TVector4& a_rcPrevPosition, const TVector4& a_rcPosition, const TVector4& a_rcNormal, TBOOL a_bCollided, TFLOAT a_fDeltaTime )
{
	static TINT     s_iNumFallsOutOfWorld = 0;
	static TVector4 s_vecLastGroundPosition;

	TBOOL  bOnGround     = TFALSE;
	TFLOAT fFallDistance = a_rcPosition.y - a_rcPrevPosition.y;

	if ( a_bCollided )
		bOnGround = a_rcNormal.y < 0.0f;

	if ( m_bSteppedUp || bOnGround )
	{
		s_vecLastGroundPosition = a_rcPosition;
		s_iNumFallsOutOfWorld   = 0;

		if ( m_eJumpState != JUMPSTATE_START )
		{
			m_pKinematics->SetAirTime( 0.0f );
			m_bGrounded = TTRUE;
		}
	}
	else
	{
		if ( m_eActionState != STATE_JUMPING && m_pKinematics->GetVerticalSpeed() < 12.0f )
			m_pKinematics->SetVerticalSpeed( 12.0f );

		if ( fFallDistance > 0.0f && m_pKinematics->GetAirTime() > 0.5f )
		{
			// Falling
			m_bGrounded = TFALSE;

			if ( m_eActionState != STATE_JUMPING )
				SetState( STATE_FALLING );

			TVector4 vecPosition = a_rcPosition;

			if ( vecPosition.y > 100.0f )
			{
				// Fell out of the world
				if ( ++s_iNumFallsOutOfWorld < 1 )
				{
					vecPosition.y = -100.0f;
					SetPosition( vecPosition );
					SetState( STATE_WALKING );
				}
				else if ( s_iNumFallsOutOfWorld < 2 )
				{
					TVector4 vecLastGround = s_vecLastGroundPosition;
					vecLastGround.y        = -150.0f;
					SetPosition( vecLastGround );
					SetState( STATE_WALKING );
				}
				else if ( s_iNumFallsOutOfWorld < 3 )
				{
					PlaceAtNamedGoal( TPS8( barnyard_centre ), TFALSE, TTRUE );
					SetState( STATE_WALKING );
				}
				else
				{
					TVector4 vecRandom;
					vecRandom.x = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -1000.0f, 1000.0f );
					vecRandom.y = -150.0f;
					vecRandom.z = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -1000.0f, 1000.0f );
					vecRandom.w = 1.0f;
					SetPosition( vecRandom );
					SetState( STATE_WALKING );
				}
			}

			return;
		}

		m_bGrounded = TTRUE;
	}

	if ( m_eJumpState >= JUMPSTATE_APEX )
	{
		// Landed
		AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();
		pAnimatableObject->KillAllAnimations();

		Movement eMovement = GetMovementType();
		TBOOL    bPlayed;

		if ( eMovement == MOVEMENT_RUN || eMovement == MOVEMENT_SPRINT )
			bPlayed = pAnimatableObject->PlayAnimation( TPS8( jump_run_land ) );
		else
			bPlayed = pAnimatableObject->PlayAnimation( TPS8( jump_stat_land ) );

		if ( bPlayed )
		{
			GetKinematics3D( m_pKinematics )->SetTilt( 0.0f, 0.0f, TFALSE );
			m_eJumpState = JUMPSTATE_END;
		}
	}
}

// $Barnyard: FUNCTION 00569bf0
void APlayerSteer::TryStepUp( TFLOAT a_fDeltaTime, TBOOL a_bSlopeSlowdown )
{
	TFLOAT   fSpeed        = m_pKinematics->GetDesiredSpeed();
	TVector4 vecFacing     = *m_pKinematics->GetDesiredFacing();
	TVector4 vecPosition   = *m_pKinematics->GetPosition();
	TFLOAT   fGravityScale = 3.0f;

	if ( fSpeed > 0.0f && m_eActionState == STATE_WALKING )
	{
		// Raise the sphere above the step
		vecPosition.y = vecPosition.y - ( GetRadius() + 0.5f );

		TFLOAT   fRadius = TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject()->GetObjectRadius();
		TVector4 vecPushedOut;

		if ( !ACollisionManager::PushOutOfTerrain( vecPosition, fRadius, vecPushedOut, m_iIgnoreGroups ) )
		{
			TVector4 vecMovement;
			vecMovement.Multiply( vecFacing, fSpeed * a_fDeltaTime );

			TVector4 vecTarget( vecPosition.x + vecMovement.x, vecPosition.y + vecMovement.y, vecPosition.z + vecMovement.z, vecPosition.w );
			TVector4 vecDirection( vecTarget.x - vecPosition.x, vecTarget.y - vecPosition.y, vecTarget.z - vecPosition.z, vecTarget.w );
			vecDirection.Normalise();

			TVector4 vecNewFacing = vecFacing;
			TFLOAT   fDistance    = 0.3f;
			TBOOL    bUnk         = TFALSE;
			TVector4 vecNewPosition;

			// NOTE: retail stores the hit object over fSpeed
			ACollisionObject* pHitObject = TNULL;

			if ( CollideAndSlide( vecPosition, vecDirection, fDistance, vecNewPosition, vecNewFacing, vecMovement, bUnk, a_bSlopeSlowdown, &pHitObject, a_fDeltaTime ) )
			{
				// Nothing above the step, lift the steer up
				fGravityScale = -1.0f;
				m_bSteppedUp  = TTRUE;
			}
		}
	}

	GetKinematics3D( m_pKinematics )->SetGravityScale( fGravityScale, a_fDeltaTime );
}

// $Barnyard: FUNCTION 00568b50
void APlayerSteer::PlayMilkDrip()
{
	AModelInstance* pModelInstance = TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetModelInstance();
	TINT            iNeckBone      = pModelInstance->GetBoneID( TPS8( c_neck ) );

	TMatrix44 matNeck;
	pModelInstance->GetBoneTransform( iNeckBone, matNeck );

	TVector4               vecPosition = matNeck.GetTranslation();
	AParticleEffectHandle* pEffect     = AParticleEffectManager::GetSingleton()->PlayEffect( m_iMilkDripEffect, vecPosition, TVector4::VEC_NEGY );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail doesn't check the handle, PlayEffect returns TNULL once every effect instance is in use
	if ( !pEffect )
		return;
#endif

	pEffect->AttachToBone( pModelInstance, iNeckBone, TTRUE, TTRUE );
}

// $Barnyard: FUNCTION 0056aa70
void APlayerSteer::OnAnimEvent( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	switch ( a_pEvent->eAction )
	{
		case AAnimationBreakpointUnitAction::ACTION_HD_THROW:
		{
			// Throws the carried object
			AAnimatedUnit* pPlayer   = TSTATICCAST( AAnimatedUnit, &GetParentUnit() );
			AUnit*         pAttached = pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

			if ( pAttached->GetSteer() )
			{
				TVector4                  vecVelocity = *GetDirection();
				AInventoryGroupComponent* pInventory  = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponentExact( &TGetClass( AInventoryGroupComponent ) ) );
				AObjectTraits*            pTraits     = GetCarriedObjectTraits();
				AUnit*                    pCarried    = pPlayer->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

				if ( pCarried )
				{
					// The object leaves with the velocity of the hand
					AModelInstanceRef pModelInstance = pPlayer->GetAnimatableObject()->GetModelInstance();

					TMatrix44 matHand;
					pModelInstance->GetBoneTransform( pModelInstance->GetBoneIndex( TPS8( cb_rlocator ) ), matHand );

					TFLOAT fElapsedTime = g_oSystemManager.GetScheduler()->GetTotalTime() - m_fThrowHandTime;
					vecVelocity         = matHand.GetTranslation() - m_vThrowHandPosition;
					vecVelocity         = vecVelocity / fElapsedTime;
				}

				if ( pTraits && pTraits->m_pWhenThrow )
				{
					vecVelocity.y += pTraits->m_pWhenThrow->m_fUpDirection;
					pInventory->DropItemOfType( pAttached->GetType()->GetName(), TPS8( Hand ) );

					pCarried->ChangeSteer( &TGetClass( AThrowItemSteer ) );

					TVector4 vecThrowVelocity;
					vecThrowVelocity.Multiply( vecVelocity, pTraits->m_pWhenThrow->m_fStrength );
					pCarried->GetSteer()->SetVelocity( vecThrowVelocity );
				}
				else
				{
					pInventory->DropItemOfType( pAttached->GetType()->GetName(), TPS8( Hand ) );

					pCarried->ChangeSteer( &TGetClass( AThrowItemSteer ) );

					TVector4 vecThrowVelocity;
					vecThrowVelocity.Multiply( vecVelocity, 20.0f );
					pAttached->GetSteer()->SetVelocity( vecThrowVelocity );
				}
			}
		}
		break;
		case AAnimationBreakpointUnitAction::ACTION_START:
			break;
		case AAnimationBreakpointUnitAction::ACTION_END:
			// NOTE: retail also gets the animatable object here without using it
			m_bActionEnded = TTRUE;
			m_pKinematics->SetMeasuredSpeed( 0.0f );
			break;
		case AAnimationBreakpointUnitAction::ACTION_START_CLIMB_GETOFF:
		{
			// Climbing back down from the top of the ladder
			AAnimatableObject*  pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();
			AToshiAnimationRef* pGetOffAnimRef    = AToshiAnimationInterface::GetAnimationRef( pAnimatableObject->GetToshiAnimInterface().GetAnimRefs(), TPS8( ladder_getoff_start ) );

			if ( pGetOffAnimRef && pAnimatableObject->GetToshiAnimInterface().IsAnimPlayingBackwards( pGetOffAnimRef ) )
			{
				pAnimatableObject->PlayAnimation( TPS8( ladder_climb ) );
				m_eClimbState = CLIMBSTATE_OFFLOAD_DOWN;
			}

			break;
		}
		case AAnimationBreakpointUnitAction::ACTION_END_CLIMB_GETOFF:
			m_eClimbState = CLIMBSTATE_OFFLOAD_DOWN;
			break;
	}
}

// $Barnyard: FUNCTION 005680f0
TBOOL APlayerSteer::OnSinkFadeOver( AFade* a_pFade, TBOOL* a_pData )
{
	if ( m_pSinkFade )
	{
		// Fade back in from black
		AFade::Color oFadeTo( 0, 0, 0, 0 );
		AFade::Color oFadeFrom( 255, 0, 0, 0 );
		AFadeManager::GetSingleton()->StartFade( oFadeFrom, oFadeTo, 1.0f );

		m_pSinkFade = TNULL;
	}

	m_oSinkFadeListener.Disconnect();
	return TTRUE;
}

// $Barnyard: FUNCTION 0056c740
TBOOL __stdcall APlayerSteer::LadderAnimEndCallback( APlayerSteer* a_pSteer, ARenderer* a_pRenderer, TBOOL** a_ppData )
{
	return a_pSteer->UpdateClimbing( g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() );
}

// $Barnyard: FUNCTION 00568b30
TBOOL __stdcall APlayerSteer::SinkFadeOverCallback( APlayerSteer* a_pSteer, AFade* a_pFade, TBOOL* a_pData )
{
	return a_pSteer->OnSinkFadeOver( a_pFade, a_pData );
}

// $Barnyard: FUNCTION 0056b610
void __stdcall APlayerSteer::AnimEventCallback( APlayerSteer* a_pSteer, AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	TRef<AUnitProxy> pUnitProxy = a_pEvent->pUnitProxy;
	AUnit*           pUnit      = pUnitProxy ? pUnitProxy->GetUnit() : TNULL;

	// Only handle the events of the steer's own unit
	if ( &a_pSteer->GetParentUnit() == pUnit )
		a_pSteer->OnAnimEvent( a_pEvent );
}
