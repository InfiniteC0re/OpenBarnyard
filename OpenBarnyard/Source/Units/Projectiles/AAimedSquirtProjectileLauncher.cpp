#include "pch.h"
#include "AAimedSquirtProjectileLauncher.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AInteractionComponent.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ASteerKinematics.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "GUI/AGUI2.h"
#include "GUI/AMilkReticleManager.h"
#include "Render/ARenderer.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionObject.h"
#ifdef BARNYARD_LATEST
#  include "AOptions.h"
#  include "Terrain/ATerrainInterface.h"
#endif

#include <Math/TMatrix44.h>
#include <Input/TInputDeviceMouse.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( cb_udder );
TPSTRING8_DECLARE( c_pelvis );
TPSTRING8_DECLARE( c_pip_pelvis );

// NOTE: GetClass 00663400, CreateObject 00663f10, CreateObjectInPlace 00663f70
TDEFINE_CLASS( AAimedSquirtProjectileLauncher );

// NOTE: inlined TVector4::NormaliseXZ (006c7540), which flattens the vector onto the XZ plane and normalises it
static void FlattenAndNormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y         = 0.0f;
	TFLOAT fMagnitudeSq = a_rVector.z * a_rVector.z + a_rVector.x * a_rVector.x;

	if ( fMagnitudeSq != 0.0f )
	{
		TFLOAT fMultiplier = 1.0f / TMath::Sqrt( fMagnitudeSq );
		a_rVector.x        = fMultiplier * a_rVector.x;
		a_rVector.z        = fMultiplier * a_rVector.z;
		return;
	}

	a_rVector.x = 0.0f;
	a_rVector.z = 1.0f;
}

// NOTE: inlined TMath::MoveTowards (006b66b0), moves a_rfValue towards a_fTarget by a_fStep without going past it
static void MoveTowards( TFLOAT& a_rfValue, TFLOAT a_fTarget, TFLOAT a_fStep )
{
	if ( a_rfValue < a_fTarget )
	{
		a_rfValue += a_fStep;

		if ( a_rfValue > a_fTarget )
			a_rfValue = a_fTarget;
	}
	else
	{
		a_rfValue -= a_fStep;

		if ( a_rfValue < a_fTarget )
			a_rfValue = a_fTarget;
	}
}

// NOTE: inlined TVector4::GetAngleBetweenVector (006c7a70), the angle between two vectors in radians
static TFLOAT GetAngleBetween( const TVector4& a_rcVec1, const TVector4& a_rcVec2 )
{
	TFLOAT fCos = ( a_rcVec2.x * a_rcVec1.x + a_rcVec1.y * a_rcVec2.y + a_rcVec1.z * a_rcVec2.z ) /
	    ( TMath::Sqrt( a_rcVec1.x * a_rcVec1.x + a_rcVec1.z * a_rcVec1.z + a_rcVec1.y * a_rcVec1.y ) *
	      TMath::Sqrt( a_rcVec2.x * a_rcVec2.x + a_rcVec2.z * a_rcVec2.z + a_rcVec2.y * a_rcVec2.y ) );

	if ( fCos > 1.0f )
		return 0.0f;

	if ( fCos < -1.0f )
		return TMath::PI;

	return TMath::ACos( fCos );
}

// $Barnyard: FUNCTION 006638d0
AAimedSquirtProjectileLauncher::AAimedSquirtProjectileLauncher()
    : m_fTargetHalfHeight( 0.0f )
    , m_bCursorAtLeft( TFALSE )
    , m_bCursorAtRight( TFALSE )
    , m_bCursorAtTop( TFALSE )
    , m_bCursorAtBottom( TFALSE )
    , m_bNPCControlled( TFALSE )
    , m_bLockedOnTarget( TFALSE )
    , m_bAimCameraActive( TFALSE )
    , m_bCursorInput( TFALSE )
    , m_bMoveAimsCursor( TFALSE )
    , m_iTargetBone( -1 )
    , m_bDisableInteractions( TTRUE )
    , m_fCursorEdgeX( 0.0f )
    , m_fCursorEdgeY( 0.0f )
    , m_iUnk500( 0 )
    , m_pPrevCameraHelper( TNULL )
#ifdef BARNYARD_LATEST
    , m_vAimCameraTurn( Toshi::TVector2::VEC_ZERO )
    , m_vAimCameraOffset( Toshi::TVector4::VEC_ZERO )
    , m_vAimCameraSmoothOffset( Toshi::TVector4::VEC_ZERO )
    , m_bAimCameraLookInput( TFALSE )
    , m_bWiiAimCamera( TFALSE )
    , m_vAimCameraPCTurn( Toshi::TVector2::VEC_ZERO )
    , m_vAimCameraMouseTurn( Toshi::TVector2::VEC_ZERO )
    , m_bAimCameraMousePitch( TFALSE )
#endif
{
	m_oReticle.Create( TNULL );
}

// $Barnyard: FUNCTION 00663fb0
// $Barnyard: FUNCTION 006639b0
AAimedSquirtProjectileLauncher::~AAimedSquirtProjectileLauncher()
{
	m_oTarget          = TNULL;
	m_oUnitUnderCursor = TNULL;
}

// $Barnyard: FUNCTION 006659a0
void AAimedSquirtProjectileLauncher::Activate()
{
	m_oTarget         = TNULL;
	m_bLockedOnTarget = TFALSE;
	m_oReticle.SetActive( TFALSE, TFALSE, 0.0f );

	if ( !m_bNPCControlled )
		EnterAimCamera();

	GetOwnerUnit()->GetSteer()->CopyDirection( m_vAimDirection );
	m_vLaunchDirection = m_vAimDirection;

	AGUI2::GetRootElement()->GetDimensions( m_vAimCursor.x, m_vAimCursor.y );
	m_fScreenWidth  = m_vAimCursor.x;
	m_fScreenHeight = m_vAimCursor.y;
	m_vAimCursor.x  = m_fScreenWidth * 0.5f;
	m_vAimCursor.y  = m_fScreenHeight * 0.5f;

	m_oUnitUnderCursor = TNULL;

	if ( m_bDisableInteractions )
	{
		AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, GetOwnerUnit()->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pInteraction )
		{
			m_bInteractionsWereDisabled = pInteraction->IsInteractionsDisabled();
			pInteraction->SetInteractionsDisabled( TTRUE );
		}
	}

	AMilkSquirtProjectileLauncher::Activate();
	m_fLaunchSpeed = 20.0f;
}

// $Barnyard: FUNCTION 006640f0
void AAimedSquirtProjectileLauncher::Deactivate()
{
	m_oTarget          = TNULL;
	m_oUnitUnderCursor = TNULL;
	m_oReticle.SetActive( TFALSE, TFALSE, 0.0f );

	LeaveAimCamera();

	if ( !m_bNPCControlled )
		TSTATICCAST( AAnimatedUnit, GetOwnerUnit() )->GetAnimatableObject()->SetOpacity( 1.0f, TTRUE );

	if ( m_bDisableInteractions )
	{
		AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, GetOwnerUnit()->FindComponent( &TGetClass( AInteractionComponent ) ) );

		if ( pInteraction )
			pInteraction->SetInteractionsDisabled( m_bInteractionsWereDisabled );
	}

	// NOTE: retail calls 006686e0 which both AMilkSquirtProjectileLauncher's Activate and Deactivate slots point at
	AMilkSquirtProjectileLauncher::Activate();
}

// $Barnyard: FUNCTION 00663800
// $BarnyardWiiJP: FUNCTION 8043c5f4
TBOOL AAimedSquirtProjectileLauncher::ProcessInput( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent, TBOOL a_bUnk1, TBOOL a_bUnk2 )
{
	if ( m_bNPCControlled )
		return TFALSE;

#ifdef BARNYARD_LATEST
	// The latest version turns the aim camera with the look buttons (the D-pad), at twice the speed of the pointer.
	// NOTE: only the look buttons are recovered. The Wii aims with the pointer and has no aim cursor input; it keeps the
	// stick X of AInputCommand_Move (|x| > 0.1) for the Wii-only facing code (+0xd24), see UpdateAimCamera
	if ( m_bWiiAimCamera && a_eCommand >= AInputCommand_LookUpWithUp && a_eCommand <= AInputCommand_LookRightWithUp )
	{
		TBOOL   bYaw   = ( a_eCommand == AInputCommand_LookLeftWithUp || a_eCommand == AInputCommand_LookRightWithUp );
		TFLOAT  fSpeed = ( a_eCommand == AInputCommand_LookUpWithUp || a_eCommand == AInputCommand_LookLeftWithUp ) ? -2.0f : 2.0f;
		TFLOAT& rfTurn = bYaw ? m_vAimCameraTurn.x : m_vAimCameraTurn.y;

		if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		{
			a_pEvent->StartRepeat();
		}
		else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT )
		{
			m_bAimCameraLookInput = TTRUE;
			rfTurn                = fSpeed;
		}
		else
		{
			rfTurn = 0.0f;
		}

		return TTRUE;
	}

	// NOTE: not in the original. Without the aim cursor (the main game), the Wii turns the camera with the pointer near
	// the edges of the screen. The PC has no pointer there, so the mouse, the right stick and the camera keys turn the
	// camera. A big enough input breaks the target lock, like the aim cursor does (AddAimCursorInput)
	if ( m_bWiiAimCamera && m_bAimCameraActive && !m_bCursorInput )
	{
		if ( a_eCommand == AInputCommand_RotateCamera )
		{
			TFLOAT fX = a_pEvent->GetMagnitudeFloat( 0 );
			TFLOAT fY = a_pEvent->GetMagnitudeFloat( 1 );

			if ( a_pEvent->GetSource()->IsA( &TGetClass( TInputDeviceMouse ) ) )
			{
				if ( a_pEvent->GetEventType() != TInputInterface::EVENT_TYPE_MOVED )
					return TFALSE;

				// Same scale as the mouse yaw of AGTAStyleCameraHelper (degrees per count), on both axes. UpdateAimCamera
				// turns both angles by exactly this much, so the sensitivity is linear and equal both ways
				AOptions* pOptions = AOptions::GetSingleton();
				TFLOAT    fScale   = pOptions->GetSensitivity() * 0.15f * TMath::PI / 180.0f;

				m_vAimCameraMouseTurn.x += fX * fScale;
				m_vAimCameraMouseTurn.y += ( pOptions->IsMouseInverted() ? -fY : fY ) * fScale;

				if ( TMath::Abs( fX ) > 10.0f || TMath::Abs( fY ) > 10.0f )
					m_bLockedOnTarget = TFALSE;
			}
			else
			{
				m_vAimCameraPCTurn.x += fX;
				m_vAimCameraPCTurn.y += fY;

				if ( TMath::Abs( fX ) > 0.3f || TMath::Abs( fY ) > 0.3f )
					m_bLockedOnTarget = TFALSE;
			}

			return TTRUE;
		}

		if ( a_eCommand == AInputCommand_RotateCameraCW || a_eCommand == AInputCommand_RotateCameraCCW )
		{
			a_pEvent->StartRepeat( 0.0f, 0.0f );
			m_vAimCameraPCTurn.x += ( a_eCommand == AInputCommand_RotateCameraCW ) ? 1.0f : -1.0f;
			m_bLockedOnTarget = TFALSE;
			return TTRUE;
		}
	}
#endif

	if ( m_bCursorInput )
	{
		TBOOL bMovesCursor = ( a_eCommand == AInputCommand_Move ) ? m_bMoveAimsCursor : ( a_eCommand == AInputCommand_RotateCamera );

		if ( bMovesCursor )
		{
			TBOOL  bHandled = TFALSE;
			TBOOL  bMouse   = a_pEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) );
			TFLOAT fY       = a_pEvent->GetMagnitudeFloat( 1 );
			TFLOAT fX       = a_pEvent->GetMagnitudeFloat( 0 );

			AddAimCursorInput( bHandled, bMouse, fX, fY );

			if ( bHandled )
				return bHandled;
		}
	}

	return AProjectileLauncher::ProcessInput( a_eCommand, a_pEvent, a_bUnk1, a_bUnk2 );
}

// $Barnyard: FUNCTION 00665b70
TBOOL AAimedSquirtProjectileLauncher::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bNPCControlled )
	{
		// Without the cursor input, the aim cursor goes back to the centre of the screen
		if ( !m_bCursorInput && !m_bLockedOnTarget )
		{
			MoveTowards( m_vAimCursor.x, m_fScreenWidth * 0.5f, a_fDeltaTime * m_fScreenWidth );
			MoveTowards( m_vAimCursor.y, m_fScreenHeight * 0.5f, a_fDeltaTime * m_fScreenHeight );
		}

		if ( m_bLockedOnTarget )
		{
			// Unlock if the target is gone, hidden or isn't a milk target anymore
			if ( !m_oTarget )
			{
				m_bLockedOnTarget = TFALSE;
			}
			else if ( !m_oTarget->GetUnit() )
			{
				m_bLockedOnTarget = TFALSE;
				m_oTarget         = TNULL;
			}

			if ( m_bLockedOnTarget )
			{
				AVisibleUnit* pTarget = TSTATICCAST( AVisibleUnit, m_oTarget->GetUnit() );

				if ( !pTarget->IsVisible() || !( pTarget->GetFlags() & AMilkReticleManager::UNIT_FLAG_MILK_TARGET ) )
				{
					m_bLockedOnTarget = TFALSE;
					m_oTarget         = TNULL;
				}
			}
		}

		UpdateCursorEdges();
		UpdateAimCamera( a_fDeltaTime );

		if ( m_bLockedOnTarget )
		{
			UpdateLockedCursor();
		}
		else
		{
			// Find the point under the aim cursor
			TVector2 vCursor = m_vAimCursor;

#ifdef BARNYARD_COMMUNITY_PATCH
			// The aim cursor is in the AGUI2 space, GetScreenRay works with the pixels of the main viewport
			TRenderContext* pViewportContext = ARenderer::GetSingleton()->GetMainViewport()->GetRenderContext();
			vCursor.x                        = vCursor.x / m_fScreenWidth * pViewportContext->GetWidth();
			vCursor.y                        = vCursor.y / m_fScreenHeight * pViewportContext->GetHeight();
#endif

			TVector4 vOrigin;
			TVector4 vDirection;
			ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), vCursor, vOrigin, vDirection );

			TFLOAT   fDistance = 50.0f;
			ARayData oRay( vOrigin, vDirection, 50.0f );
			ACollisionManager::GetSingleton()->CollideRayWithTerrain( GetOwnerUnit(), oRay, fDistance, 0x80000 );
			oRay.fLength = fDistance;

			T2DynamicVector<ACollisionHit> vecHits( GetGlobalAllocator(), 64, 64 );
			ACollisionHit                  oHit;

			TVector4 vOffset;
			vOffset.Multiply( vDirection, fDistance );
			m_vAimPoint.Add( vOrigin, vOffset );

			m_oUnitUnderCursor = TNULL;

			TBOOL bHit = ACollisionManager::GetSingleton()->CollideRayObjects( GetOwnerUnit(), oRay, &vecHits, oHit, 0x80000, ACollisionObject::TYPE_IGNORED );

			if ( bHit && oHit.pObject )
			{
				vOffset.Multiply( vDirection, oHit.fDistance );
				m_vAimPoint.Add( vOrigin, vOffset );

				AUnit* pUnit = TSTATICCAST( AUnit, oHit.pObject->GetOwnerUnit() );

				if ( pUnit && pUnit->IsA( &TGetClass( AVisibleUnit ) ) && TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
					m_oUnitUnderCursor = pUnit->GetSelfProxy();
			}

			m_oReticle.SetPosition( m_vAimCursor );
		}

		if ( m_bLockedOnTarget )
		{
			// Lead the target
			TVector4 vVelocity = TVector4::VEC_ZERO;

			if ( a_fDeltaTime > 0.0f )
			{
				vVelocity.Set( m_vAimPoint.x - m_vPrevTargetPosition.x, m_vAimPoint.y - m_vPrevTargetPosition.y, m_vAimPoint.z - m_vPrevTargetPosition.z, m_vAimPoint.w );
				vVelocity.Divide( a_fDeltaTime );
				m_vPrevTargetPosition = m_vAimPoint;
			}

			AimAt( m_vAimPoint, vVelocity );
		}
		else
		{
			AimAt( m_vAimPoint );
		}

		FaceTarget( m_vAimPoint );

		// Follow the trajectory of the milk
		m_vTracePoint       = GetLaunchPosition();
		TFLOAT fMaxDistance = TVector4::Distance( m_vTracePoint, m_vAimPoint ) * 2.5f;

		if ( !m_bLockedOnTarget )
		{
			TraceTrajectory( GetOwnerUnit(), m_vTracePoint, m_vAimDirection, m_fLaunchSpeed, fMaxDistance, 2.0f, TFALSE, TNULL );
		}
		else
		{
			m_oUnitUnderCursor = TNULL;

			AUnit* pHitUnit = TNULL;
			TraceTrajectory( GetOwnerUnit(), m_vTracePoint, m_vAimDirection, m_fLaunchSpeed, fMaxDistance, 2.0f, TTRUE, &pHitUnit );

			if ( pHitUnit )
				m_oUnitUnderCursor = pHitUnit->GetSelfProxy();
		}

		m_oReticle.Update( a_fDeltaTime );
	}

	AMilkSquirtProjectileLauncher::Update( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 006643e0
Toshi::TVector4 AAimedSquirtProjectileLauncher::GetLaunchPosition()
{
	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, GetOwnerUnit() )->GetAnimatableObject();

	TINT iBone = pAnimatableObject->GetModelInstance()->GetBoneIndex( TPS8( cb_udder ) );

	TMatrix44 matBone;
	pAnimatableObject->GetModelInstance()->GetBoneTransform( iBone, matBone );

	return TVector4( matBone.m_f41, matBone.m_f42, matBone.m_f43, 1.0f );
}

// $Barnyard: FUNCTION 006646c0
Toshi::TVector4 AAimedSquirtProjectileLauncher::GetLaunchDirection()
{
	if ( m_bNPCControlled )
		return m_vAimDirection;

	TVector4 vSteerDirection = *GetOwnerUnit()->GetSteer()->GetDirection();
	TVector4 vAimDirection   = m_vAimDirection;

	FlattenAndNormaliseXZ( vAimDirection );
	FlattenAndNormaliseXZ( vSteerDirection );

	TFLOAT fAngle = GetAngleBetween( vSteerDirection, vAimDirection );

	if ( fAngle >= 1.0f )
	{
		TVector4 vCross;
		vCross.CrossProduct( vAimDirection, vSteerDirection );

		TVector4 vDirection = m_vAimDirection;
		vDirection.AsVector3().RotateY( ( vCross.y > 0.0f ) ? fAngle : -fAngle );

		return vDirection;
	}

	return m_vAimDirection;
}

// $Barnyard: FUNCTION 006635a0
TFLOAT AAimedSquirtProjectileLauncher::GetThrowSpeed( AUnit* a_pObject )
{
	return m_fLaunchSpeed;
}

// $Barnyard: FUNCTION 006645f0
Toshi::TVector4 AAimedSquirtProjectileLauncher::GetThrowVelocity( AUnit* a_pObject )
{
	TVector4 vVelocity;

	if ( !m_bNPCControlled )
	{
		TVector4 vSteerDirection = *GetOwnerUnit()->GetSteer()->GetDirection();

		if ( TVector4::DotProduct3( m_vLaunchDirection, vSteerDirection ) < 0.1f )
		{
			vVelocity.Multiply( vSteerDirection, m_fLaunchSpeed );
			return vVelocity;
		}
	}

	vVelocity.Multiply( m_vLaunchDirection, m_fLaunchSpeed );
	return vVelocity;
}

// $Barnyard: FUNCTION 006635b0
void AAimedSquirtProjectileLauncher::AddAimCursorInput( TBOOL& a_rHandled, TBOOL a_bMouse, TFLOAT a_fX, TFLOAT a_fY )
{
	if ( !m_bAimCameraActive )
	{
		if ( !m_bLockedOnTarget )
		{
			m_vAimCursor.x = m_fScreenWidth * 0.5f;
			m_vAimCursor.y = m_fScreenHeight * 0.5f;
		}

		return;
	}

	TFLOAT fUnlockThreshold;

	if ( !a_bMouse )
	{
		if ( !m_bLockedOnTarget )
		{
			m_vAimCursor.x = a_fX * 15.0f + m_vAimCursor.x;
			m_vAimCursor.y = a_fY * 15.0f + m_vAimCursor.y;
		}

		fUnlockThreshold = 0.3f;
	}
	else
	{
		if ( !m_bLockedOnTarget )
		{
			m_vAimCursor.x = a_fX * 0.5f + m_vAimCursor.x;
			m_vAimCursor.y = a_fY * 0.5f + m_vAimCursor.y;
		}

		fUnlockThreshold = 10.0f;
	}

	// Big enough input breaks the target lock
	if ( TMath::Abs( a_fX ) > fUnlockThreshold || TMath::Abs( a_fY ) > fUnlockThreshold )
		m_bLockedOnTarget = TFALSE;

	a_rHandled = TTRUE;
}

// $Barnyard: FUNCTION 00665800
// $BarnyardWiiJP: FUNCTION 80439b6c
void AAimedSquirtProjectileLauncher::EnterAimCamera()
{
	if ( m_pPrevCameraHelper )
		return;

	m_bAimCameraActive = TTRUE;

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->GetCurrentCamera();
	m_pPrevCameraHelper = pCameraManager->GetCurrentCameraHelper();
	pCameraManager->DetachCameraHelpers();

#ifdef BARNYARD_LATEST
	// Wii Style camera option, kept until the aim mode ends
	m_bWiiAimCamera = AOptions::IsWiiCameraEnabled();

	// The latest version starts orbiting from where the camera is, slightly above the owner's head
	if ( m_bWiiAimCamera )
	{
		m_bAimCameraLookInput  = TFALSE;
		m_bAimCameraMousePitch = TFALSE;

		TVector4 vLookAt;
		vLookAt.Add( *GetOwnerUnit()->GetSteer()->GetPosition(), TVector4( 0.0f, -1.7f, 0.0f, 1.0f ) );

		TVector4 vOffset;
		vOffset.Substract( pCameraManager->GetCurrentCamera()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), vLookAt );

		m_vAimCameraSmoothOffset = vOffset;
		m_vAimCameraOffset       = vOffset;
		m_vAimCameraOffset.y     = 0.0f;
		m_vAimCameraOffset.Normalise();
		m_vAimCameraOffset.y = -0.2f;
		m_vAimCameraOffset.Normalise();
		m_vAimCameraSmoothOffset.Normalise();
	}
#endif

	const TMatrix44& rcCameraMatrix = pCameraManager->GetCurrentCamera()->GetMatrix();
	m_vCameraDirection              = TVector4( rcCameraMatrix.m_f31, rcCameraMatrix.m_f32, rcCameraMatrix.m_f33, rcCameraMatrix.m_f34 );
	m_vCameraDirection.w            = 1.0f;

	TVector4 vFacing( m_vCameraDirection.x, 0.0f, m_vCameraDirection.z, m_vCameraDirection.w );
	vFacing.Normalise();

	m_vCameraDirection.x = -m_vCameraDirection.x;
	m_vCameraDirection.y = -m_vCameraDirection.y;
	m_vCameraDirection.z = -m_vCameraDirection.z;

	if ( GetOwnerUnit()->GetSteer()->IsA( &TGetClass( ASteer ) ) )
	{
		ASteer* pSteer = GetOwnerUnit()->GetSteer();
		pSteer->GetKinematics()->SetDesiredFacing( vFacing );
		pSteer->GetKinematics()->SetDesiredSpeed( 0.0f );
	}

#ifdef BARNYARD_LATEST
	// NOTE: the latest version leaves the camera where it is until the next update
	if ( !m_bWiiAimCamera )
#endif
		UpdateAimCamera( 0.0f );
}

// $Barnyard: FUNCTION 00663fd0
// $BarnyardWiiJP: FUNCTION 80439d34
void AAimedSquirtProjectileLauncher::LeaveAimCamera()
{
	m_bAimCameraActive = TFALSE;

	const TMatrix44& rcCameraMatrix = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
	TVector4         vCameraPosition( rcCameraMatrix.m_f41, rcCameraMatrix.m_f42, rcCameraMatrix.m_f43, rcCameraMatrix.m_f44 );
	TVector4         vCameraDirection( rcCameraMatrix.m_f31, rcCameraMatrix.m_f32, rcCameraMatrix.m_f33, 1.0f );

	if ( m_bNPCControlled )
		return;

	// NOTE: retail pushes an extra 0 to DisableDrawWithDefaultMaterials (00610780)
	TSTATICCAST( AVisibleUnit, GetOwnerUnit() )->GetModelInstance()->DisableDrawWithDefaultMaterials();

	if ( !m_pPrevCameraHelper )
		return;

#ifdef BARNYARD_LATEST
	if ( m_bWiiAimCamera )
	{
		// The latest version puts the follow camera where the aim camera is, looking the same way (PC retail passes the
		// direction as the point to look at), and blends back to the previous helper in 0.3 seconds
		if ( m_pPrevCameraHelper->IsA( &TGetClass( AGTAStyleCameraHelper ) ) )
		{
			TVector4 vTarget;
			vTarget.Add( vCameraPosition, vCameraDirection );
			vTarget.w = vCameraPosition.w;

			TSTATICCAST( AGTAStyleCameraHelper, m_pPrevCameraHelper )->SetPosition( vCameraPosition, vTarget, TFALSE );
		}

		ACameraManager::GetSingleton()->SetSmoothTransition( m_pPrevCameraHelper, 0.3f );
	}
	else
#endif
	    if ( m_pPrevCameraHelper->IsA( &TGetClass( AGTAStyleCameraHelper ) ) )
	{
		// Put the follow camera where the aim camera is
		TSTATICCAST( AGTAStyleCameraHelper, m_pPrevCameraHelper )->SetPosition( vCameraPosition, vCameraDirection, TFALSE );
		ACameraManager::GetSingleton()->SetSplitscreenCameraHelper( 1 );
	}
	else
	{
		ACameraManager::GetSingleton()->SetCameraHelper( m_pPrevCameraHelper );
	}

	m_pPrevCameraHelper = TNULL;
}

// $Barnyard: FUNCTION 00663e90
AUnit* AAimedSquirtProjectileLauncher::GetLockedTarget()
{
	if ( !m_bLockedOnTarget )
		return TNULL;

	if ( !m_oTarget )
	{
		m_bLockedOnTarget = TFALSE;
		return TNULL;
	}

	AUnit* pUnit = m_oTarget->GetUnit();

	if ( !pUnit )
	{
		m_bLockedOnTarget = TFALSE;
		m_oTarget         = TNULL;
		return TNULL;
	}

	return pUnit;
}

// $Barnyard: FUNCTION 00663ef0
AUnit* AAimedSquirtProjectileLauncher::GetUnitUnderCursor() const
{
	if ( !m_oUnitUnderCursor )
		return TNULL;

	return m_oUnitUnderCursor->GetUnit();
}

// Returns the index of the pelvis bone (c_pelvis or c_pip_pelvis) of a_pUnit, -1 if it has none
// NOTE: inlined in LockOn, CanHit and AMilkReticleManager::SetTarget
static TINT FindPelvisBone( AUnit* a_pUnit )
{
	AAnimatedUnit* pAnimatedUnit = TSTATICCAST( AAnimatedUnit, a_pUnit );
	TINT           iBone         = pAnimatedUnit->GetAnimatableObject()->GetModelInstance()->GetBoneID( TPS8( c_pelvis ) );

	if ( iBone < 0 )
		iBone = pAnimatedUnit->GetAnimatableObject()->GetModelInstance()->GetBoneID( TPS8( c_pip_pelvis ) );

	return iBone;
}

// $Barnyard: FUNCTION 006651e0
void AAimedSquirtProjectileLauncher::LockOn( AUnit* a_pUnit )
{
	m_iTargetBone       = -1;
	m_fTargetHalfHeight = 0.0f;

	if ( !a_pUnit )
	{
		m_bLockedOnTarget = TFALSE;
		m_oTarget         = TNULL;
	}
	else
	{
		m_oTarget         = a_pUnit->GetSelfProxy();
		m_bLockedOnTarget = TTRUE;

		AUnit* pTarget = m_oTarget->GetUnit();

		if ( pTarget->IsA( &TGetClass( AAnimatedUnit ) ) && TSTATICCAST( AAnimatedUnit, pTarget )->GetAnimatableObject() )
			m_iTargetBone = FindPelvisBone( pTarget );

		if ( m_iTargetBone >= 0 )
		{
			// Aim at the pelvis
			TMatrix44 matBone;
			TSTATICCAST( AAnimatedUnit, pTarget )->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( m_iTargetBone, matBone );

			m_vAimPoint = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
		}
		else
		{
			// Aim at the middle of the unit
			m_vAimPoint = *pTarget->GetSteer()->GetPosition();

			ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, pTarget )->GetCollisionObject();

			if ( pCollisionObject )
			{
				m_fTargetHalfHeight = pCollisionObject->GetHeightScale() * 0.5f;
				m_vAimPoint.y -= m_fTargetHalfHeight;
			}
		}

		m_vPrevTargetPosition = m_vAimPoint;
	}

	if ( !m_bNPCControlled )
		m_oReticle.SetActive( TTRUE, TTRUE, AGUI2::GetRootElement()->GetWidth() * 0.1f );

	Update( 0.0f );
}

// $Barnyard: FUNCTION 00664850
void AAimedSquirtProjectileLauncher::UpdateLockedCursor()
{
	if ( !m_bLockedOnTarget )
		return;

	AUnit* pTarget = m_oTarget->GetUnit();

	if ( m_iTargetBone < 0 )
	{
		m_vAimPoint = *pTarget->GetSteer()->GetPosition();
		m_vAimPoint.y -= m_fTargetHalfHeight;
	}
	else
	{
		TMatrix44 matBone;
		TSTATICCAST( AAnimatedUnit, pTarget )->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( m_iTargetBone, matBone );

		m_vAimPoint = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	}

	TVector4 vScreenPos = m_vAimPoint;
	ARenderer::GetSingleton()->WorldToGUI( vScreenPos );

	m_vAimCursor.x = vScreenPos.x;
	m_vAimCursor.y = vScreenPos.y;
	m_oReticle.SetPosition( m_vAimCursor );
}

// $Barnyard: FUNCTION 006654f0
TBOOL AAimedSquirtProjectileLauncher::CanHit( AUnit* a_pUnit )
{
	if ( !a_pUnit )
		return TFALSE;

	TVector4 vTarget;
	TINT     iBone = -1;

	if ( a_pUnit->IsA( &TGetClass( AAnimatedUnit ) ) && TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject() )
		iBone = FindPelvisBone( a_pUnit );

	if ( iBone >= 0 )
	{
		TMatrix44 matBone;
		TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( iBone, matBone );

		vTarget = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
	}
	else
	{
		vTarget = *a_pUnit->GetSteer()->GetPosition();

		ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, a_pUnit )->GetCollisionObject();

		if ( pCollisionObject )
			vTarget.y -= pCollisionObject->GetHeightScale() * 0.5f;
	}

	TVector4 vDirection;
	GetBallisticDirection( GetLaunchPosition(), vTarget, m_fLaunchSpeed, vDirection );

	TVector4 vPosition    = GetLaunchPosition();
	TFLOAT   fMaxDistance = TVector4::Distance( vPosition, vTarget ) * 1.5f;

	AUnit* pHitUnit = TNULL;
	TraceTrajectory( GetOwnerUnit(), vPosition, vDirection, m_fLaunchSpeed, fMaxDistance, 2.0f, TTRUE, &pHitUnit );

	return ( pHitUnit && pHitUnit == a_pUnit ) || TVector4::DistanceSq( vPosition, vTarget ) <= 1.0f;
}

// $Barnyard: FUNCTION 00664510
TBOOL AAimedSquirtProjectileLauncher::FireAt( const Toshi::TVector4& a_rcTarget, TBOOL a_bThrow )
{
	FaceTarget( a_rcTarget );
	AimAt( a_rcTarget );

	if ( a_bThrow )
		Throw();
	else
		AMilkSquirtProjectileLauncher::Fire();

	return TTRUE;
}

// $Barnyard: FUNCTION 00664550
TBOOL AAimedSquirtProjectileLauncher::FireAt( const Toshi::TVector4& a_rcTarget, TFLOAT a_fAngle, TBOOL a_bThrow )
{
	FaceTarget( a_rcTarget );

	// Launch with the speed that gets there at that angle
	TFLOAT fOldLaunchSpeed = m_fLaunchSpeed;

	if ( a_bThrow )
	{
		GetBallisticVelocity( a_fAngle, GetThrowPosition(), a_rcTarget, m_vLaunchDirection );
		m_fLaunchSpeed = m_vLaunchDirection.NormaliseMag();
		Throw();
	}
	else
	{
		GetBallisticVelocity( a_fAngle, GetThrowPosition(), a_rcTarget, m_vAimDirection );
		m_fLaunchSpeed = m_vAimDirection.NormaliseMag();
		AMilkSquirtProjectileLauncher::Fire();
	}

	m_fLaunchSpeed = fOldLaunchSpeed;
	return TTRUE;
}

// $Barnyard: FUNCTION 006634a0
void AAimedSquirtProjectileLauncher::GetBallisticDirection( const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo, TFLOAT a_fSpeed, Toshi::TVector4& a_rOutDirection )
{
	a_rOutDirection.Set( a_rcTo.x - a_rcFrom.x, a_rcTo.y - a_rcFrom.y, a_rcTo.z - a_rcFrom.z, a_rcTo.w );
	FlattenAndNormaliseXZ( a_rOutDirection );

	// The time it takes to get there horizontally and the vertical speed needed for that
	TFLOAT fInvSpeed = 1.0f / a_fSpeed;
	TFLOAT fTime     = TVector4::DistanceXZ( a_rcFrom, a_rcTo ) * fInvSpeed;

	a_rOutDirection.y = ( ( a_rcTo.y - a_rcFrom.y ) / fTime - fTime * 4.9f ) * fInvSpeed;
	a_rOutDirection.Normalise();
}

// $Barnyard: FUNCTION 00663520
void AAimedSquirtProjectileLauncher::GetBallisticVelocity( TFLOAT a_fAngle, const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo, Toshi::TVector4& a_rOutVelocity )
{
	a_rOutVelocity.Set( a_rcTo.x - a_rcFrom.x, 0.0f, a_rcTo.z - a_rcFrom.z, a_rcTo.w );

	TFLOAT fDistance = a_rOutVelocity.NormaliseMag();
	TFLOAT fSpeed    = TFLOAT( fDistance / sqrt( ( tan( a_fAngle ) * fDistance + a_rcTo.y - a_rcFrom.y ) * ( 1.0 / 4.9 ) ) );

	a_rOutVelocity.Multiply( fSpeed );
	a_rOutVelocity.y = TFLOAT( tan( -a_fAngle ) * fSpeed );
}

// $Barnyard: FUNCTION 00663ac0
TBOOL AAimedSquirtProjectileLauncher::TraceTrajectory( AUnit* a_pIgnoreUnit, Toshi::TVector4& a_rPosition, Toshi::TVector4 a_vDirection, TFLOAT a_fSpeed, TFLOAT a_fMaxDistance, TFLOAT a_fStep, TBOOL a_bFindUnit, AUnit** a_ppOutUnit )
{
	TBOOL bHit = TFALSE;

	TVector4 vVelocity;
	vVelocity.Multiply( a_vDirection, a_fSpeed );

	// The time it takes to move a step
	TFLOAT fTime = a_fStep / a_fSpeed;

	if ( a_bFindUnit )
		*a_ppOutUnit = TNULL;

	do
	{
		if ( a_fMaxDistance <= 0.0f )
			break;

		// The next point of the trajectory
		TVector4 vNext;
		vNext.x = vVelocity.x * fTime + a_rPosition.x;
		vNext.y = vVelocity.y * fTime + fTime * fTime * 4.9f + a_rPosition.y;
		vNext.z = vVelocity.z * fTime + a_rPosition.z;
		vVelocity.y += fTime * 9.8f;

		TVector4 vSegment( vNext.x - a_rPosition.x, vNext.y - a_rPosition.y, vNext.z - a_rPosition.z, a_rPosition.w );
		TFLOAT   fLength = vSegment.NormaliseMag();

		a_fMaxDistance -= a_fStep;

		ARayData oRay( a_rPosition, vSegment, a_fStep );
		bHit = ACollisionManager::GetSingleton()->CollideRay( a_pIgnoreUnit, oRay, fLength, 0x80000, ACollisionObject::TYPE_IGNORED );

		if ( a_bFindUnit && bHit )
		{
			T2DynamicVector<ACollisionHit> vecHits( GetGlobalAllocator(), 64, 64 );
			ACollisionHit                  oHit;

			if ( ACollisionManager::GetSingleton()->CollideRayObjects( a_pIgnoreUnit, oRay, &vecHits, oHit, 0x80000, ACollisionObject::TYPE_IGNORED ) && oHit.pObject )
			{
				AUnit* pUnit = TSTATICCAST( AUnit, oHit.pObject->GetOwnerUnit() );

				if ( pUnit && pUnit->IsA( &TGetClass( AVisibleUnit ) ) && TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
					*a_ppOutUnit = pUnit;
			}
		}

		// Move to the end of the step (or to the hit)
		TVector4 vOffset;
		vOffset.Multiply( vSegment, fLength );
		a_rPosition.x += vOffset.x;
		a_rPosition.y += vOffset.y;
		a_rPosition.z += vOffset.z;
	} while ( !bHit );

	return bHit;
}

// $Barnyard: FUNCTION 006636f0
void AAimedSquirtProjectileLauncher::AimAt( const Toshi::TVector4& a_rcTarget )
{
	GetBallisticDirection( GetLaunchPosition(), a_rcTarget, m_fLaunchSpeed, m_vAimDirection );
	GetBallisticDirection( GetThrowPosition(), a_rcTarget, m_fLaunchSpeed, m_vLaunchDirection );
}

// $Barnyard: FUNCTION 00663770
void AAimedSquirtProjectileLauncher::AimAt( Toshi::TVector4& a_rTarget, const Toshi::TVector4& a_rcVelocity )
{
	TFLOAT fTime = TVector4::DistanceXZ( GetLaunchPosition(), a_rTarget ) / m_fLaunchSpeed;

	TVector4 vOffset;
	vOffset.Multiply( a_rcVelocity, fTime );

	a_rTarget.x += vOffset.x;
	a_rTarget.y += vOffset.y;
	a_rTarget.z += vOffset.z;

	AimAt( a_rTarget );
}

// $Barnyard: FUNCTION 00664200
void AAimedSquirtProjectileLauncher::FaceTarget( const Toshi::TVector4& a_rcTarget )
{
	if ( !m_bCursorInput && !m_bLockedOnTarget )
		return;

	ASteer*  pSteer = GetOwnerUnit()->GetSteer();
	TVector4 vFacing( a_rcTarget.x - pSteer->GetPosition()->x, 0.0f, a_rcTarget.z - pSteer->GetPosition()->z, a_rcTarget.w );
	vFacing.Normalise();

	if ( pSteer->IsA( &TGetClass( APlayerSteer ) ) )
		TSTATICCAST( APlayerSteer, pSteer )->StopAndFace( vFacing );
	else
		pSteer->SetDesiredDirection( vFacing );
}

// $Barnyard: FUNCTION 006649b0
void AAimedSquirtProjectileLauncher::UpdateCursorEdges()
{
	static TFLOAT s_fEdgeSize = m_fScreenHeight * 0.16f; // 0x007cd0b0

	m_bCursorAtLeft   = TFALSE;
	m_bCursorAtRight  = TFALSE;
	m_bCursorAtTop    = TFALSE;
	m_bCursorAtBottom = TFALSE;

	if ( m_bLockedOnTarget )
	{
		// Turn the camera towards the target
		static TFLOAT s_fCentreX = m_fScreenWidth * 0.5f;  // 0x007cd0ac
		static TFLOAT s_fCentreY = m_fScreenHeight * 0.3f; // 0x007cd0a8

		m_fCursorEdgeX = TMath::Abs( s_fCentreX - m_vAimCursor.x ) / s_fCentreX;
		m_fCursorEdgeY = TMath::Abs( s_fCentreY - m_vAimCursor.y ) / s_fCentreY;

		if ( m_vAimCursor.x > s_fCentreX )
			m_bCursorAtRight = TTRUE;
		else if ( m_vAimCursor.x < s_fCentreX )
			m_bCursorAtLeft = TTRUE;

		if ( m_vAimCursor.y > s_fCentreY )
			m_bCursorAtBottom = TTRUE;
		else if ( m_vAimCursor.y < s_fCentreY )
			m_bCursorAtTop = TTRUE;

		return;
	}

	// NOTE: retail projects the owner on the screen here but doesn't use the result
	TVector2 vOwnerScreenPos;

	if ( !ARenderer::GetSingleton()->GetScreenPosition( *GetOwnerUnit()->GetSteer()->GetPosition(), vOwnerScreenPos ) )
	{
		vOwnerScreenPos.x = m_fScreenWidth * 0.5f;
		vOwnerScreenPos.y = m_fScreenHeight * 0.76f;
	}

	// Keep the cursor within the screen, it's pushed back from the edges
	TMath::Clip( m_vAimCursor.x, 1.0f, m_fScreenWidth - 1.0f );
	TMath::Clip( m_vAimCursor.y, 1.0f, m_fScreenHeight - 1.0f );

	TFLOAT fRightEdge = m_fScreenWidth - s_fEdgeSize;

	if ( fRightEdge <= m_vAimCursor.x )
	{
		m_bCursorAtRight = TTRUE;
		m_fCursorEdgeX   = ( m_vAimCursor.x - fRightEdge ) / s_fEdgeSize;
		m_vAimCursor.x   = m_vAimCursor.x - 1.0f - m_fCursorEdgeX * 6.0f;
	}
	else if ( m_vAimCursor.x <= s_fEdgeSize )
	{
		m_bCursorAtLeft = TTRUE;
		m_fCursorEdgeX  = ( s_fEdgeSize - m_vAimCursor.x ) / s_fEdgeSize;
		m_vAimCursor.x  = m_fCursorEdgeX * 6.0f + m_vAimCursor.x + 1.0f;
	}

	if ( m_vAimCursor.y > s_fEdgeSize )
	{
		TFLOAT fBottomEdge = m_fScreenHeight - s_fEdgeSize;

		if ( fBottomEdge <= m_vAimCursor.y )
		{
			m_bCursorAtBottom = TTRUE;
			m_fCursorEdgeY    = ( m_vAimCursor.y - fBottomEdge ) / s_fEdgeSize;
			m_fCursorEdgeX    = m_fCursorEdgeY;
			m_vAimCursor.y    = m_vAimCursor.y - 1.0f - m_fCursorEdgeY * 6.0f;

			if ( m_fScreenWidth * 0.5f <= m_vAimCursor.x )
				m_bCursorAtRight = TTRUE;
			else
				m_bCursorAtLeft = TTRUE;
		}
	}
	else
	{
		m_bCursorAtTop = TTRUE;
		m_fCursorEdgeY = ( s_fEdgeSize - m_vAimCursor.y ) / s_fEdgeSize;
		m_vAimCursor.y = m_fCursorEdgeY * 6.0f + m_vAimCursor.y + 1.0f;
	}
}

// $Barnyard: FUNCTION 00664d50
// $BarnyardWiiJP: FUNCTION 8043bd10
void AAimedSquirtProjectileLauncher::UpdateAimCamera( TFLOAT a_fDeltaTime )
{
	if ( m_bNPCControlled || !m_bAimCameraActive )
		return;

#ifdef BARNYARD_LATEST
	// Wii Style camera option (kept from EnterAimCamera)
	if ( m_bWiiAimCamera )
	{
		// The latest version orbits the camera around the owner's head at up to 7 units, turned by the pointer near the
		// edges of the screen or by the look buttons, at a speed set by the camera speed option. The camera collides
		// with the terrain and the objects, comes closer quickly and goes back slowly, and the pitch is limited.
		static TFLOAT s_fAimFOV = TMath::HALF_PI * 0.85f; // 0x806b5500 (Wii)

		// NOTE: the Wii takes the turn from the Wii Remote pointer: the pointer position relative to the centre of the
		// screen (-1 to 1) with the inner half ignored. The aim cursor of the PC has the same role, so it's used here
		if ( m_bAimCameraLookInput )
		{
			m_bAimCameraLookInput = TFALSE;
		}
		else if ( m_bCursorInput )
		{
			TFLOAT fX = m_vAimCursor.x / ( m_fScreenWidth * 0.5f ) - 1.0f;
			TFLOAT fY = m_vAimCursor.y / ( m_fScreenHeight * 0.5f ) - 1.0f;
			TMath::Clip( fX, -1.0f, 1.0f );
			TMath::Clip( fY, -1.0f, 1.0f );

			if ( fX < -0.5f )
				m_vAimCameraTurn.x = ( fX + 0.5f ) / ( 1.0f - 0.5f );
			else if ( fX > 0.5f )
				m_vAimCameraTurn.x = ( fX - 0.5f ) / ( 1.0f - 0.5f );

			if ( fY < -0.5f )
				m_vAimCameraTurn.y = ( fY + 0.5f ) / ( 1.0f - 0.5f );
			else if ( fY > 0.5f )
				m_vAimCameraTurn.y = ( fY - 0.5f ) / ( 1.0f - 0.5f );
		}
		else
		{
			// NOTE: not in the original, see ProcessInput. The PC input turns the camera; without it, the camera turns
			// towards the locked target (the aim cursor follows it) the way the pointer near the edges would
			m_vAimCameraTurn = m_vAimCameraPCTurn;
			TMath::Clip( m_vAimCameraTurn.x, -2.0f, 2.0f );
			TMath::Clip( m_vAimCameraTurn.y, -2.0f, 2.0f );

			if ( m_bLockedOnTarget && m_vAimCameraTurn.x == 0.0f && m_vAimCameraTurn.y == 0.0f && m_vAimCameraMouseTurn.x == 0.0f && m_vAimCameraMouseTurn.y == 0.0f )
			{
				TFLOAT fX = m_vAimCursor.x / ( m_fScreenWidth * 0.5f ) - 1.0f;
				TMath::Clip( fX, -1.0f, 1.0f );

				if ( fX < -0.25f )
					m_vAimCameraTurn.x = ( fX + 0.25f ) / ( 1.0f - 0.25f );
				else if ( fX > 0.25f )
					m_vAimCameraTurn.x = ( fX - 0.25f ) / ( 1.0f - 0.25f );
			}
		}

		m_vAimCameraPCTurn = TVector2::VEC_ZERO;

		TVector4 vLookAt;
		vLookAt.Add( *GetOwnerUnit()->GetSteer()->GetPosition(), TVector4( 0.0f, -1.7f, 0.0f, 1.0f ) );

		ACamera*   pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
		TMatrix44& rMatrix = pCamera->GetMatrix();

		pCamera->SetFOV( TMath::LERPClamped( pCamera->GetFOV(), s_fAimFOV, a_fDeltaTime * TMath::HALF_PI ) );

		TVector4 vCameraPosition = rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
		TFLOAT   fForwardY       = rMatrix.AsBasisVector4( BASISVECTOR_FORWARD ).y;

		// Turn speed (radians per second): 0.2 PI to 0.75 PI with the camera speed option (0-5)
		TFLOAT fTurnSpeed = TMath::LERP( 0.2f * TMath::PI, 0.75f * TMath::PI, AOptions::GetSingleton()->GetCameraSpeed() / 5.0f );

		m_vAimCameraOffset.AsVector3().RotateY( m_vAimCameraTurn.x * fTurnSpeed * a_fDeltaTime );

		// NOTE: not in the original, the mouse turns the camera directly (see ProcessInput). The smooth offset turns
		// with it, so the mouse yaw isn't delayed by the blend below (the blend still applies to the other input)
		if ( m_vAimCameraMouseTurn.x != 0.0f )
		{
			m_vAimCameraOffset.AsVector3().RotateY( m_vAimCameraMouseTurn.x );
			m_vAimCameraSmoothOffset.AsVector3().RotateY( m_vAimCameraMouseTurn.x );
		}

		// m_vAimCameraSmoothOffset follows the wanted direction (LerpClamped3)
		for ( TINT i = 0; i < 3; i++ )
		{
			TFLOAT& rfValue = m_vAimCameraSmoothOffset.AsArray()[ i ];
			TFLOAT  fEnd    = m_vAimCameraOffset.AsArray()[ i ];
			TFLOAT  fStart  = rfValue;
			rfValue         = ( fEnd - fStart ) * ( 5.0f * a_fDeltaTime ) + fStart;

			if ( ( fStart < fEnd && rfValue > fEnd ) || ( fStart > fEnd && rfValue < fEnd ) )
				rfValue = fEnd;
		}

		TVector4 vOffset;
		vOffset.Multiply( m_vAimCameraSmoothOffset, 7.0f );

		TVector4 vEnd;
		vEnd.Add( vLookAt, vOffset );

		ARayData oRay( vLookAt, vEnd );
		TFLOAT   fFreeDistance = oRay.fLength;

		TVector4 vNormal;
		TFLOAT   fStepHeight;
		TFLOAT   fPenetration;
		TBOOL    bTerrainHit = ATerrainInterface::GetSingleton()->CollideSweptSphere( oRay.vOrigin, 0.4f, oRay.vDirection, fFreeDistance, vNormal, 0.01f, fStepHeight, fPenetration, 0x80000 );

		if ( bTerrainHit )
			oRay.fLength = fFreeDistance;

		T2DynamicVector<ACollisionHit> vecHits( GetGlobalAllocator(), 64, 64 );
		ACollisionHit                  oHit;
		TBOOL                          bObjectHit = ACollisionManager::GetSingleton()->CollideRayObjects( GetOwnerUnit(), oRay, &vecHits, oHit, 0x80000, ACollisionObject::TYPE_IGNORED );

		if ( bObjectHit )
			fFreeDistance = oHit.fDistance;

		if ( bTerrainHit || bObjectHit )
		{
			fFreeDistance -= 0.046f;

			if ( fFreeDistance < 0.05f )
				fFreeDistance = 0.05f;
		}

		// Comes closer quickly, goes back slowly
		TFLOAT fDistance = TVector4::Distance( vCameraPosition, vLookAt );

		if ( fFreeDistance > fDistance )
			fDistance = TMath::LERPClamped( fDistance, fFreeDistance, a_fDeltaTime * 15.0f * 0.1f );
		else
			fDistance = TMath::LERPClamped( fDistance, fFreeDistance, a_fDeltaTime * 15.0f );

		TMath::Clip( fDistance, 0.01f, 7.0f );

		vOffset.Multiply( oRay.vDirection, fDistance );
		vCameraPosition.Add( vLookAt, vOffset );

		// Look the opposite way of the offset, pitched by the turn input or going back to 0.2
		TVector4 vLookDirection( -m_vAimCameraSmoothOffset.x, fForwardY, -m_vAimCameraSmoothOffset.z, m_vAimCameraSmoothOffset.w );

		if ( m_vAimCameraTurn.y != 0.0f )
		{
			vLookDirection.y       = fForwardY + a_fDeltaTime * ( 0.5f * fTurnSpeed * m_vAimCameraTurn.y );
			m_bAimCameraMousePitch = TFALSE;
		}
		else if ( m_vAimCameraMouseTurn.y != 0.0f || m_bAimCameraMousePitch )
		{
			// NOTE: not in the original, see ProcessInput. The Wii adds the turn to the sine of the pitch; the mouse
			// turns the pitch angle instead, so the pitch stays where the mouse left it. The limits are the same
			TFLOAT fHorizontal = TMath::Sqrt( vLookDirection.x * vLookDirection.x + vLookDirection.z * vLookDirection.z );
			TFLOAT fSinPitch   = fForwardY;
			TMath::Clip( fSinPitch, -1.0f, 1.0f );

			TFLOAT fPitch = TMath::ASin( fSinPitch ) + m_vAimCameraMouseTurn.y;
			TMath::Clip( fPitch, -TMath::HALF_PI * 0.9f, TMath::HALF_PI * 0.9f );

			vLookDirection.y       = TMath::Tan( fPitch ) * fHorizontal;
			m_bAimCameraMousePitch = TTRUE;
		}
		else if ( TMath::Abs( fDistance ) > 0.01f )
			vLookDirection.y = TMath::LERPClamped( fForwardY, 0.2f, a_fDeltaTime * 1.2f );

		TMath::Clip( vLookDirection.y, -0.08f, 0.4f );
		vLookDirection.Normalise();

		// Hide the owner when the camera is inside it
		AAnimatedUnit* pOwner = TSTATICCAST( AAnimatedUnit, GetOwnerUnit() );
		pOwner->GetAnimatableObject()->SetOpacity( ( TVector4::Distance( vCameraPosition, vLookAt ) >= 0.5f ) ? 1.0f : 0.0f, TTRUE );

		rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vCameraPosition;
		pCamera->LookAtDirection( vLookDirection );

		m_vAimCameraTurn      = TVector2::VEC_ZERO;
		m_vAimCameraMouseTurn = TVector2::VEC_ZERO;
		return;
	}
#endif

	TFLOAT fTurnSpeed = m_bCursorInput ? 2.0f : 10.0f;

	// NOTE: retail keeps a field (+0x8c) of the first splitscreen camera helper of group 1 in a static (0x007cd0b8) and never uses it
	[[maybe_unused]] static ACameraHelper* s_pSplitscreenHelper = ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 );

	// The camera looks at the owner's head
	TVector4 vLookAt;
	vLookAt.Add( *GetOwnerUnit()->GetSteer()->GetPosition(), TVector4( 0.0f, -1.7f, 0.0f, 1.0f ) );

	ACamera* pCamera         = ACameraManager::GetSingleton()->GetCurrentCamera();
	TVector4 vCameraPosition = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

	TVector4 vOffset;
	vOffset.Substract( vCameraPosition, vLookAt );
	TFLOAT fDistance = vOffset.NormaliseMag();

	// The aim cursor at the left or right edge turns the camera around the owner
	if ( m_bCursorAtLeft )
		vOffset.AsVector3().RotateY( -( m_fCursorEdgeX * m_fCursorEdgeX * fTurnSpeed * a_fDeltaTime ) );

	if ( m_bCursorAtRight )
		vOffset.AsVector3().RotateY( m_fCursorEdgeX * m_fCursorEdgeX * fTurnSpeed * a_fDeltaTime );

	TMath::Clip( fDistance, 4.0f, 5.0f );

	TVector4 vScaledOffset;
	vScaledOffset.Multiply( vOffset, fDistance );

	TVector4 vDesiredPosition;
	vDesiredPosition.Add( vLookAt, vScaledOffset );

	// The top or bottom edge moves it up or down
	if ( m_bCursorAtTop )
		vDesiredPosition.y = m_fCursorEdgeY * m_fCursorEdgeY * fTurnSpeed * a_fDeltaTime + vDesiredPosition.y;

	if ( m_bCursorAtBottom )
		vDesiredPosition.y = vDesiredPosition.y - m_fCursorEdgeY * m_fCursorEdgeY * fTurnSpeed * a_fDeltaTime;

	TMath::Clip( vDesiredPosition.y, vLookAt.y - 1.6f, vLookAt.y + 1.0f );

	// Don't go through the terrain
	ARayData oRay( vLookAt, vDesiredPosition );

	TVector4 vDirection;
	vDirection.Substract( vDesiredPosition, vLookAt );
	vDirection.Normalise();

	if ( fDistance > 0.0f )
	{
		if ( ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fDistance, 0x80000 ) )
			fDistance *= 0.96f;

		TMath::Clip( fDistance, 0.0f, 5.0f );
	}

	// Fade the owner out when the camera gets too close to it
	AVisibleUnit* pOwner = TSTATICCAST( AVisibleUnit, GetOwnerUnit() );
	TFLOAT        fOpacity;

	// NOTE: retail pushes TTRUE or TFALSE to DisableDrawWithDefaultMaterials (00610780), which ignores it
	pOwner->GetModelInstance()->DisableDrawWithDefaultMaterials();

	if ( fDistance <= 3.0f )
		fOpacity = ( fDistance >= 2.0f ) ? fDistance - 2.0f : 0.0f;
	else
		fOpacity = 1.0f;

	TSTATICCAST( AAnimatedUnit, pOwner )->GetAnimatableObject()->SetOpacity( fOpacity, TTRUE );

	vOffset.Multiply( vDirection, fDistance );
	vDesiredPosition.Add( vLookAt, vOffset );

	pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) = vDesiredPosition;
	pCamera->LookAtPoint( vLookAt );
}
