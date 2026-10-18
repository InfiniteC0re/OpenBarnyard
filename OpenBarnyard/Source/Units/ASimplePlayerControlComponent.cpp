#include "pch.h"
#include "ASimplePlayerControlComponent.h"
#include "AUnitPlayer.h"
#include "Projectiles/AProjectileLauncher.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "Player/APlayerProfileManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Render/ADebugOverlay.h"
#include "Physics/ACollisionManager.h"
#include "Physics/APhysics.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 00592230 / CreateObjectInPlace 00592290
// NOTE: GetClass 00592180, CreateObject 00592230, CreateObjectInPlace 00592290
TDEFINE_CLASS( ASimplePlayerControlComponent );

// $Barnyard: FUNCTION 00592190
ASimplePlayerControlComponent::ASimplePlayerControlComponent()
    : m_vMovement( TVector2::VEC_ZERO )
    , m_fIdleTime( 0.0f )
    , m_bSprinting( TFALSE )
    , m_bLauncherInput( TTRUE )
    , m_bUnk32( TTRUE )
    , m_bSecondaryLauncherInput( TTRUE )
    , m_bCanSprint( TTRUE )
    , m_bCanJump( TTRUE )
    , m_bMovementEnabled( TTRUE )
    , m_bCameraInput( TTRUE )
    , m_bMoving( TFALSE )
{
}

// $Barnyard: FUNCTION 005922d0
// $Barnyard: FUNCTION 005921e0
ASimplePlayerControlComponent::~ASimplePlayerControlComponent()
{
}

// $Barnyard: FUNCTION 00592a00
void ASimplePlayerControlComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	AUnit* pOwner = GetOwner();

	if ( !pOwner->GetSteer()->GetClass()->IsA( &TGetClass( APlayerSteer ) ) )
		pOwner->ChangeSteer( &TGetClass( APlayerSteer ) );

	ADEBUGLINE( LINE_INPUT, ADebugOverlay::INFO_COLOUR, "Stick Input (%f , %f)", m_vMovement.x, m_vMovement.y );

	if ( m_bMovementEnabled )
		UpdateMovement( a_fDeltaTime, 0.0f );

	m_fIdleTime += a_fDeltaTime;
}

// $Barnyard: FUNCTION 00592a60
TBOOL ASimplePlayerControlComponent::ProcessCommand( AInputCommand a_eCommand, const TInputInterface::InputEvent* a_pEvent )
{
	// NOTE: the owner is assumed to be an AUnitPlayer
	AUnitPlayer* pOwner = TSTATICCAST( AUnitPlayer, GetOwner() );

	if ( !pOwner )
		return TFALSE;

	AProjectileLauncher* pActiveLauncher = pOwner->GetActiveLauncher();

	if ( pActiveLauncher && pActiveLauncher->ProcessInput( a_eCommand, a_pEvent, m_bLauncherInput, TFALSE ) )
		return TTRUE;

	ACameraHelper* pCameraHelper = ACameraManager::GetSingleton()->GetCurrentCameraHelper();

	// NOTE: ACameraHelper's +0x18 slot (ProcessInputCommand) still takes void* arguments
	if ( pCameraHelper && m_bCameraInput && pCameraHelper->ProcessInputCommand( TREINTERPRETCAST( void*, TUINTPTR( a_eCommand ) ), const_cast<TInputInterface::InputEvent*>( a_pEvent ) ) )
		return TTRUE;

	TFLOAT fIdleTime = m_fIdleTime;
	m_fIdleTime      = 0.0f;

	if ( !GetOwner()->GetSteer()->CanStartAction() )
	{
		m_bSprinting = TFALSE;
		return TFALSE;
	}

	TBOOL bResult = TTRUE;

	// NOTE: for the digital moves retail calls 006c3cf0, the event's StartRepeat with zero times
	switch ( a_eCommand )
	{
		case AInputCommand_Move:
		{
			TFLOAT fX = a_pEvent->GetMagnitudeFloat( 0 );
			TFLOAT fY = a_pEvent->GetMagnitudeFloat( 1 );

			m_vMovement.x += fX;
			m_vMovement.y -= fY;

			// A released stick isn't input
			if ( TMath::Abs( fX ) < 0.05f && TMath::Abs( fY ) < 0.05f )
				m_fIdleTime = fIdleTime;

			return bResult;
		}
		case AInputCommand_Left:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vMovement.x -= 1.0f;
			return bResult;
		case AInputCommand_Right:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vMovement.x += 1.0f;
			return bResult;
		case AInputCommand_Forward:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vMovement.y += 1.0f;
			return bResult;
		case AInputCommand_Backward:
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vMovement.y -= 1.0f;
			return bResult;
		case AInputCommand_SuperSpeed:
			// Sprint
			bResult = TFALSE;
			a_pEvent->StartRepeat( 0.0f, 0.0f );

			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				m_bSprinting = TFALSE;
				return TTRUE;
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				m_bSprinting = m_bCanSprint;
				return TTRUE;
			}

			return bResult;
		case AInputCommand_Jump:
			// Jump
			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				ASteer* pSteer = pOwner->GetSteer();

				if ( m_bMovementEnabled && m_bCanJump && pSteer && !IsJumpBlocked() )
				{
					TSTATICCAST( APlayerSteer, pSteer )->SetState( APlayerSteer::STATE_JUMPING );
					return bResult;
				}
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, pOwner->GetSteer() );

				if ( m_bMovementEnabled && m_bCanJump && pSteer->GetState() == APlayerSteer::STATE_JUMPING )
				{
					pSteer->EndState( APlayerSteer::STATE_JUMPING );
					return bResult;
				}
			}

			return bResult;
		case AInputCommand_Switch:
			bResult = TFALSE;

			if ( !pActiveLauncher )
				return bResult;

			if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				if ( m_bSecondaryLauncherInput )
				{
					pOwner->SelectLauncher( TTRUE );
					return TTRUE;
				}
			}
			else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				if ( pActiveLauncher == pOwner->GetSecondaryLauncher() )
				{
					pOwner->SelectLauncher( TFALSE );
					return TTRUE;
				}
			}

			return bResult;
		default:
			return TFALSE;
	}
}

// $Barnyard: FUNCTION 005922f0
void ASimplePlayerControlComponent::OnDisable()
{
	m_vMovement = TVector2::VEC_ZERO;

	// NOTE: the owner is assumed to be an AUnitPlayer
	AUnitPlayer* pOwner = TSTATICCAST( AUnitPlayer, GetOwner() );
	pOwner->GetSteer()->SetSpeed( 0.0f );

	m_fIdleTime  = 0.0f;
	m_bSprinting = TFALSE;

	if ( pOwner->GetActiveLauncher() && pOwner->GetActiveLauncher() == pOwner->GetPrimaryLauncher() )
		pOwner->SelectLauncher( TFALSE );
}

// $Barnyard: FUNCTION 005921f0
void ASimplePlayerControlComponent::OnEnable()
{
	m_fIdleTime = 0.0f;
}

// NOTE: 00592720 isn't defined as a function in Ghidra
// $Barnyard: FUNCTION 00592720
void ASimplePlayerControlComponent::GetCameraForward( TVector4& a_rOutForward )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	pCamera->GetFlatDirection( a_rOutForward );
}

// $Barnyard: FUNCTION 00592200
void ASimplePlayerControlComponent::SetCanSprint( TBOOL a_bCanSprint )
{
	m_bCanSprint = a_bCanSprint;
	m_bSprinting = a_bCanSprint && m_bSprinting;
}

// $Barnyard: FUNCTION 00592360
void ASimplePlayerControlComponent::SetMovementEnabled( TBOOL a_bEnabled )
{
	m_bMovementEnabled = a_bEnabled;

	if ( !a_bEnabled )
		TSTATICCAST( ACommonSteer, GetOwner()->GetSteer() )->SetDesiredVelocity( TVector4::VEC_ZERO );
}

// $Barnyard: FUNCTION 00592390
void ASimplePlayerControlComponent::UpdateMovement( TFLOAT a_fDeltaTime, TFLOAT a_fStopSpeed )
{
	AUnit* pOwner = GetOwner();

	TVector4 vForward;
	GetCameraForward( vForward );

	TVector4 vRight = vForward;
	vRight.AsVector3().RotateY( TMath::HALF_PI );

	ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, pOwner->GetSteer() );
	TFLOAT        fMagSq = m_vMovement.x * m_vMovement.x + m_vMovement.y * m_vMovement.y;

	if ( fMagSq >= 0.05f )
	{
		TFLOAT fMagnitude = TMath::Sqrt( fMagSq );
		m_bMoving         = TTRUE;

		if ( fMagnitude > 1.0f )
		{
			TFLOAT fInvMagnitude = 1.0f / fMagnitude;
			m_vMovement.x *= fInvMagnitude;
			m_vMovement.y *= fInvMagnitude;
			fMagnitude = 1.0f;
		}

		if ( fMagnitude < 0.8f )
		{
			// Turns on the spot first if the input points away from the current direction
			TVector4 vDirection;
			pSteer->CopyDirection( vDirection );

			TVector4 vForwardPart;
			vForwardPart.Multiply( vForward, m_vMovement.y );

			TVector4 vRightPart;
			vRightPart.Multiply( vRight, m_vMovement.x );

			TVector4 vDesiredDirection = vRightPart + vForwardPart;
			vDesiredDirection.Normalise();

			if ( TVector4::DotProduct3( vDirection, vDesiredDirection ) < 0.97f && pSteer->StopAndFace( vDesiredDirection ) )
			{
				m_vMovement = TVector2::VEC_ZERO;
				return;
			}
		}

		TVector4 vRightPart;
		vRightPart.Multiply( vRight, m_vMovement.x );

		TVector4 vForwardPart;
		vForwardPart.Multiply( vForward, m_vMovement.y );

		TVector4 vVelocity = vForwardPart + vRightPart;

		const ASteerTraits* pTraits = pOwner->GetSteerTraits();
		TFLOAT              fSpeed;
		TFLOAT              fLeanFactor;

		if ( m_bSprinting )
		{
			fSpeed      = pTraits->m_fLogicalSprintSpeed;
			fLeanFactor = 12.0f;
		}
		else
		{
			fLeanFactor = 8.0f;

			if ( fMagnitude >= 0.9f )
				fSpeed = pTraits->m_fLogicalRunSpeed;
			else if ( fMagnitude * pTraits->m_fLogicalRunSpeed * 0.4f >= pTraits->m_fLogicalWalkSpeed )
				fSpeed = pTraits->m_fLogicalRunSpeed * 0.4f;
			else
				fSpeed = pTraits->m_fLogicalWalkSpeed / fMagnitude;
		}

		vVelocity.Multiply( fSpeed );

		TFLOAT fLean = TMath::Abs( m_vMovement.y ) * m_vMovement.x;
		TMath::Clip( fLean, -0.64f, 0.64f );
		fLean = fLean * fLeanFactor * 0.02727077f;

		pSteer->SetTilt( 0.0f, fLean, 0.0f );

		pSteer->SetDesiredVelocity( vVelocity );

		if ( m_bSprinting && pSteer->GetSpeed() > 0.01f )
		{
			APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

			if ( pProfile )
			{
				pProfile->AddStamina( -( a_fDeltaTime * pProfile->GetStaminaWasteSpeed() ) );

				if ( pProfile->GetStamina() <= 0.0f )
					m_bSprinting = TFALSE;
			}
		}
	}
	else if ( m_bMoving )
	{
		TVector4 vDirection;
		pSteer->CopyDirection( vDirection );
		pSteer->SetDesiredVelocity( vDirection );
		pSteer->SetDesiredSpeed( a_fStopSpeed );
		pSteer->SetTilt( 0.0f, 0.0f, 0.0f );

		m_bMoving = TFALSE;
	}

	m_vMovement = TVector2::VEC_ZERO;
}

// $Barnyard: FUNCTION 00592740
TBOOL ASimplePlayerControlComponent::IsJumpBlocked()
{
	AUnit* pOwner = GetOwner();

	if ( !pOwner )
		return TFALSE;

	ASteer* pSteer = pOwner->GetSteer();

	if ( !pSteer )
		return TFALSE;

	// Probe below the player
	TFLOAT fHeight         = pSteer->GetRadius() + 0.2f;
	TFLOAT fStepHeight     = 0.0f;
	TFLOAT fDistance       = fHeight + 0.2f;
	TFLOAT fRayDistance    = fDistance;
	TFLOAT fObjectDistance = fDistance;

	TVector4 vecStart = *pSteer->GetPosition();
	vecStart.y        = vecStart.y - fHeight;

	// Standing on flat ground?
	ACollisionModelRayHitList oHitList;

	if ( ATerrainInterface::GetSingleton()->CollideRay( vecStart, APhysics::g_vecGravityDirection, fRayDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) && oHitList.iNumHits > 0 )
	{
		TVector4 vecGroundNormal = oHitList.aHits[ 0 ].GetNormal();

		if ( TVector4::DotProduct3( TVector4::VEC_NEGY, vecGroundNormal ) > 0.5f )
			return TFALSE;
	}

	// NOTE: fRayDistance is reused as the penetration output
	TVector4 vecNormal;
	TBOOL    bTerrainHit = ATerrainInterface::GetSingleton()->CollideSweptSphere( vecStart, pSteer->GetRadius(), APhysics::g_vecGravityDirection, fDistance, vecNormal, 0.01f, fStepHeight, fRayDistance, TCollisionCommon::TOSHICGROUPFLAG_NONE );

	TVector4          vecObjectNormal;
	ACollisionObject* pHitObject = ACollisionManager::GetSingleton()->CollideSweptSphereObjects( vecStart, APhysics::g_vecGravityDirection, pSteer->GetRadius(), fObjectDistance, vecObjectNormal, ACollisionObject::TYPE_IGNORED, 0.0f, TNULL, 0 );

	if ( pHitObject && fObjectDistance < fDistance && ( pHitObject->GetType() & ACollisionObject::TYPE_GATE ) )
	{
		vecNormal = vecObjectNormal;
	}
	else if ( !bTerrainHit || pHitObject )
	{
		return TFALSE;
	}

	// Standing on a steep slope, can the player slide down it?
	if ( TVector4::DotProduct3( TVector4::VEC_NEGY, vecNormal ) < 0.5f )
	{
		TVector4 vecSlide( vecNormal.x, 0.0f, vecNormal.z, vecNormal.w );
		vecSlide.Normalise();

		TVector4 vecOffset;
		vecOffset.Multiply( APhysics::g_vecGravityDirection, fDistance );
		vecStart.AsVector3().Add( vecOffset.AsVector3() );

		fObjectDistance = pSteer->GetRadius();

		if ( !ATerrainInterface::GetSingleton()->CollideSweptSphere( vecStart, pSteer->GetRadius(), vecSlide, fObjectDistance, vecNormal, 0.01f, fStepHeight, fRayDistance, TCollisionCommon::TOSHICGROUPFLAG_NONE ) )
			return TTRUE;
	}

	return TFALSE;
}
