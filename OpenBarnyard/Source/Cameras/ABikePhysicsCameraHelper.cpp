#include "pch.h"
#include "ABikePhysicsCameraHelper.h"
#include "Physics/ACollisionObject.h"
#include "ACameraManager.h"
#include "AOptions.h"
#include "Input/AInputMap.h"
#include "GameInterface/AGameStateController.h"
#include "Render/ARenderer.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/Steering/ARacingCarSteer.h"
#include "Physics/APhysicsBike.h"
#include "Physics/APhysicsCar.h"
#include "Physics/AVehicleManager.h"
#include "MiniGame/ABikeRaceMicroGame.h"

#include <Math/TQuaternion.h>
#include <Input/TInputDeviceMouse.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ABikePhysicsCameraHelper );

TPSTRING8_DECLARE( body_loc );

// 0x00773768
static constexpr TVector4 DEFAULT_LOOKAT_OFFSET = TVector4( 0.0f, -1.2f, 0.0f, 1.0f );

// 0x00773798
static constexpr TVector4 DEFAULT_PIVOT_OFFSET = TVector4( 0.0f, -1.2f, 0.0f, 1.0f );

// 0x00773788
static constexpr TVector4 CARCHASE_LOOKAT_OFFSET = TVector4( 0.0f, -1.0f, 1.0f, 1.0f );

// 0x007737b8
static constexpr TVector4 CARCHASE_PIVOT_OFFSET = TVector4( 0.0f, -2.0f, 0.0f, 1.0f );

// 0x007737c8
static constexpr TVector4 DEFAULT_CAMERA_OFFSET = TVector4( 0.0f, -2.0f, -4.0f, 1.0f );

// 0x007737a8, the pivot moves towards it with the speed of the bike (MODE_DEFAULT)
static constexpr TVector4 FAST_PIVOT_OFFSET = TVector4( 0.0f, -1.4f, -1.0f, 1.0f );

static constexpr TFLOAT YAW_SPRING_STIFFNESS      = 20.0f; // 0x007737d8
static constexpr TFLOAT HEIGHT_SPRING_STIFFNESS   = 10.0f; // 0x007737dc
static constexpr TFLOAT DISTANCE_SPRING_STIFFNESS = 10.0f; // 0x007737e0
static constexpr TFLOAT CARCHASE_LOOKAT_SCALE     = 4.0f;  // 0x007737e4

// The springs are updated at a fixed rate
static constexpr TFLOAT SPRING_STEP = 0.033333335f;

#ifdef BARNYARD_LATEST
// The latest version updates the springs at 60 Hz
static constexpr TFLOAT SPRING_STEP_LATEST = 0.0166667f;

// Number of spring steps to do for the accumulated time (removed from it)
static TINT ConsumeSpringSteps( TFLOAT& a_rfTimeAccumulator )
{
	TINT iNumSteps = TMath::FloorToInt( a_rfTimeAccumulator / SPRING_STEP_LATEST );
	a_rfTimeAccumulator -= SPRING_STEP_LATEST * TFLOAT( iNumSteps );
	return iNumSteps;
}
#endif

// 0x0078237c, speed of the rotation by the input in MODE_DEFAULT
static TFLOAT s_fRotateSpeed = TMath::TWO_PI * 0.2f;

// Flags of the camera collision checks
static constexpr TINT CAMERA_COLLISION_FLAGS = ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_BUSH | ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_FOLIAGE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_UNIT;
#ifdef BARNYARD_LATEST
static constexpr TINT CAMERA_COLLISION_FLAGS_LATEST = ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_FOLIAGE | ACollisionObject::TYPE_UNIT; // the latest version (Wii Style camera)
#endif
static constexpr TINT CARCHASE_CAMERA_COLLISION_FLAGS = ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_BUSH | ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_FOLIAGE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_UNIT;

// NOTE: these classes aren't reconstructed yet, TClass::Find gives TNULL until they are
static TBOOL IsInstanceOf( const TObject* a_pObject, const TClass* a_pClass )
{
	return a_pClass && a_pObject->IsA( a_pClass );
}

static const TClass* GetBikeRaceMicroGameClass()
{
	return &TGetClass( ABikeRaceMicroGame );
}


// 0x007832f4
static const TClass* GetCarChaseMiniGameStateClass()
{
	static const TClass* s_pClass = TClass::Find( "ACarChaseMiniGameState" );
	return s_pClass;
}

// NOTE: defined in AGTAStyleCameraHelper.cpp (006186a0)
TBOOL SweepSphere( const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength, TFLOAT* a_pOutFraction, TVector4* a_pOutNormal, TINT a_iFlags, TUINT a_uiCollisionMask );

// Rotates a_rcVector by a_rcQuaternion (w of the result is 1)
// NOTE: a copy of 006c9740 (static in AModel.cpp)
static void RotateVector( TVector4& a_rOutVector, const TQuaternion& a_rcQuaternion, const TVector4& a_rcVector )
{
	TQuaternion quatConjugate( -a_rcQuaternion.x, -a_rcQuaternion.y, -a_rcQuaternion.z, a_rcQuaternion.w );
	TQuaternion quatVector( a_rcVector.x, a_rcVector.y, a_rcVector.z, 0.0f );

	TQuaternion quatResult = a_rcQuaternion;
	quatResult *= quatVector;
	quatResult *= quatConjugate;

	a_rOutVector.Set( quatResult.x, quatResult.y, quatResult.z, 1.0f );
}

// Builds the rotation of the euler angles (x, y, z): Z * Y * X
// $Barnyard: FUNCTION 0045da20
static TQuaternion& QuaternionFromEuler( TQuaternion& a_rOutQuat, const TVector3& a_rcEuler )
{
	TQuaternion quatX;
	quatX.SetRotation( TVector4::VEC_POSX.AsVector3(), a_rcEuler.x );

	TQuaternion quatY;
	quatY.SetRotation( TVector4::VEC_POSY.AsVector3(), a_rcEuler.y );

	TQuaternion quatZ;
	quatZ.SetRotation( TVector4::VEC_POSZ.AsVector3(), a_rcEuler.z );

	TQuaternion quatZY = quatZ;
	quatZY *= quatY;

	a_rOutQuat = quatZY;
	a_rOutQuat *= quatX;

	return a_rOutQuat;
}

// Integrates a vector spring: a_rPosition is pulled towards a_rcTarget
// $Barnyard: FUNCTION 0045da90
static void UpdateVectorSpring( const TVector4& a_rcTarget, TVector4& a_rVelocity, TVector4& a_rPosition, TFLOAT a_fDeltaTime, TFLOAT a_fStiffness, TFLOAT a_fDamping )
{
	TVector4 vToTarget;
	vToTarget.Substract( a_rcTarget, a_rPosition );

	TVector4 vScaled;
	vScaled.Multiply( vToTarget, a_fDeltaTime );

	TVector4 vAcceleration;
	vAcceleration.Multiply( vScaled, a_fStiffness );

	a_rVelocity.x = vAcceleration.x + a_rVelocity.x;
	a_rVelocity.y = vAcceleration.y + a_rVelocity.y;
	a_rVelocity.z = vAcceleration.z + a_rVelocity.z;
	a_rVelocity.Multiply( 1.0f - a_fDeltaTime * a_fDamping );

	TVector4 vMove;
	vMove.Multiply( a_rVelocity, a_fDeltaTime );

	a_rPosition.x = vMove.x + a_rPosition.x;
	a_rPosition.y = vMove.y + a_rPosition.y;
	a_rPosition.z = vMove.z + a_rPosition.z;
}

// Copies a_rcMatrix keeping its forward axis on the XZ plane (no pitch and roll)
// $Barnyard: FUNCTION 0045dcc0
static void FlattenMatrix( const TMatrix44& a_rcMatrix, TMatrix44& a_rOutMatrix )
{
	a_rOutMatrix = TMatrix44::IDENTITY;

	TVector4& rRight   = a_rOutMatrix.AsBasisVector4( BASISVECTOR_RIGHT );
	TVector4& rUp      = a_rOutMatrix.AsBasisVector4( BASISVECTOR_UP );
	TVector4& rForward = a_rOutMatrix.AsBasisVector4( BASISVECTOR_FORWARD );

	rForward   = a_rcMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	rForward.y = 0.0f;
	rForward.Normalise();

	rRight.CrossProduct( TVector4::VEC_POSY, rForward );
	rUp.CrossProduct( rForward, rRight );

	rRight.w   = 0.0f;
	rUp.w      = 0.0f;
	rForward.w = 0.0f;

	a_rOutMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $Barnyard: FUNCTION 0065e690
void ABikePhysicsCameraHelper::Spring::Update( TFLOAT a_fDeltaTime )
{
	TFLOAT fVelocityChange = ( -( ( fValue - fTarget ) * fStiffness ) - fDamping * fVelocity ) * fInvMass * a_fDeltaTime;

	fValue    = a_fDeltaTime * fVelocityChange * 0.5f + a_fDeltaTime * fVelocity + fValue;
	fVelocity = fVelocityChange + fVelocity;
}

// $Barnyard: FUNCTION 0065e740
void ABikePhysicsCameraHelper::Spring::UpdateAngle( TFLOAT a_fDeltaTime )
{
	// Wrap the difference to -PI..PI
	TFLOAT fDiff = ( fValue - fTarget ) * 0.15915494f + 100.5f;
	fDiff        = ( fDiff - TFLOAT( TINT( fDiff ) ) - 0.5f ) * 6.2831855f;

	TFLOAT fVelocityChange = ( -( fDiff * fStiffness ) - fDamping * fVelocity ) * fInvMass * a_fDeltaTime;

	fValue    = a_fDeltaTime * fVelocity + fVelocityChange * a_fDeltaTime * 0.5f + fValue;
	fVelocity = fVelocityChange + fVelocity;

	// Wrap the angle to -PI..PI
	TFLOAT fWrapped = 0.15915494f * fValue + 100.5f;
	fValue          = ( fWrapped - TFLOAT( TINT( fWrapped ) ) - 0.5f ) * 6.2831855f;
}

// $Barnyard: FUNCTION 0065e6d0
void ABikePhysicsCameraHelper::Spring::SetMass( TFLOAT a_fMass )
{
	fMass    = a_fMass;
	fInvMass = 1.0f / a_fMass;
}

// $Barnyard: FUNCTION 0065e6f0
void ABikePhysicsCameraHelper::Spring::SetDampingRatio( TFLOAT a_fRatio )
{
	fDamping = sqrtf( fMass * fStiffness * 4.0f ) * a_fRatio;
}

// $Barnyard: FUNCTION 0065e7e0
void ABikePhysicsCameraHelper::Spring::SetCriticalDamping()
{
	fDamping = sqrtf( fMass * fStiffness * 4.0f );
}

// $Barnyard: FUNCTION 0045e950
// $BarnyardWiiJP: FUNCTION 80177894
ABikePhysicsCameraHelper::ABikePhysicsCameraHelper()
    : m_fFOV( 1.3962634f )
    , m_vSpringPosition( TVector4::VEC_ZERO )
    , m_vSpringVelocity( TVector4::VEC_ZERO )
    , m_fTimeAccumulator( 0.0f )
    , m_eMode( MODE_DEFAULT )
    , m_iCarChase( 0 )
    , m_oYawSpring( 0.0f, 20.0f, 9.0f )
    , m_oDistanceSpring( 0.0f, 20.0f, 9.0f )
    , m_oHeightSpring( 0.0f, 20.0f, 9.0f )
    , m_fRotateInput( 0.0f )
    , m_vLookAtOffset( CARCHASE_LOOKAT_OFFSET )
    , m_vCameraOffset( DEFAULT_CAMERA_OFFSET )
    , m_vPivotOffset( CARCHASE_PIVOT_OFFSET )
    , m_uiCollisionMask( 0x90000 )
    , m_bBlendingFOV( TFALSE )
    , m_fFOVBlendTime( 0.0f )
    , m_fFOVBlendDuration( 1.0f )
    , m_fFOVBlendStart( 1.3962634f )
    , m_fFOVBlendTarget( 1.3962634f )
#ifdef BARNYARD_LATEST
    , m_fCollisionTime( 10.0f )
    , m_bCollided( TFALSE )
#endif
{
}

// $Barnyard: FUNCTION 0045e220
// $Barnyard: FUNCTION 0045eae0
ABikePhysicsCameraHelper::~ABikePhysicsCameraHelper()
{
}

// $Barnyard: FUNCTION 0045f080
// $BarnyardWiiJP: FUNCTION 80178a40
TBOOL ABikePhysicsCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_pTarget || !m_pTarget->GetUnit() )
		return TFALSE;

	m_fTimeAccumulator += a_fDeltaTime;

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	TMatrix44 matVehicle;
	GetVehicleTransform( matVehicle );

	TMatrix44 matFlat = TMatrix44::IDENTITY;
	FlattenMatrix( matVehicle, matFlat );

	const TVector4& rcVehiclePos     = matVehicle.AsBasisVector4( BASISVECTOR_TRANSLATION );
	const TVector4& rcVehicleForward = matVehicle.AsBasisVector4( BASISVECTOR_FORWARD );

	TVector4 vCameraPos;
	TVector4 vLookAt;

#ifdef BARNYARD_LATEST
	// Wii Style camera option: the bike camera of the latest version (60 Hz springs, smoothed collision push)
	const TBOOL bWiiCamera = AOptions::IsWiiCameraEnabled();

	// Render interpolation of the fixed-step springs, added to the camera that is drawn only. It's our addition,
	// not a Wii behaviour, and is always on in BARNYARD_LATEST builds. It only applies to the 60 Hz springs of the
	// Wii Style camera: without it m_oLastSpringStep stays zero and nothing is interpolated.
	TVector4 vRenderCorrection = TVector4::VEC_ZERO;
	TFLOAT   fInterpolation    = 0.0f;

	if ( !bWiiCamera )
		ResetSpringInterpolation();
#endif

	if ( m_eMode == MODE_FOLLOW )
	{
		TFLOAT fDamping = TFLOAT( sqrt( 40.0 ) );

		TMatrix44::TransformVector( vLookAt, matFlat, m_vLookAtOffset );

		TVector4 vTargetOffset;
		TMatrix44::RotateVector( vTargetOffset, matFlat, m_vCameraOffset );

#ifdef BARNYARD_LATEST
		if ( bWiiCamera )
		{
			TINT iNumSteps = ConsumeSpringSteps( m_fTimeAccumulator );

			for ( TINT i = 0; i < iNumSteps; i++ )
			{
				BeforeSpringStep();
				UpdateVectorSpring( vTargetOffset, m_vSpringVelocity, m_vSpringPosition, SPRING_STEP_LATEST, 10.0f, fDamping );
				m_oDistanceSpring.Update( SPRING_STEP_LATEST );
				m_oHeightSpring.Update( SPRING_STEP_LATEST );
			}

			// The camera that is drawn is moved back by the part of the last step that hasn't been reached yet
			fInterpolation = EndSpringSteps( iNumSteps );
			vRenderCorrection.Multiply( m_oLastSpringStep.vPosition, fInterpolation );
		}
		else
#endif
		{
			TINT iNumSteps = TMath::FloorToInt( m_fTimeAccumulator * 29.999998f );
			m_fTimeAccumulator -= iNumSteps * SPRING_STEP;

			for ( TINT i = iNumSteps; i > 0; i-- )
			{
				UpdateVectorSpring( vTargetOffset, m_vSpringVelocity, m_vSpringPosition, SPRING_STEP, 10.0f, fDamping );
				m_oDistanceSpring.Update( SPRING_STEP );
				m_oHeightSpring.Update( SPRING_STEP );
			}
		}

		vCameraPos.x = rcVehiclePos.x + m_vSpringPosition.x;
		vCameraPos.y = rcVehiclePos.y + m_vSpringPosition.y;
		vCameraPos.z = rcVehiclePos.z + m_vSpringPosition.z;
		vCameraPos.w = m_vSpringPosition.w;

#ifdef BARNYARD_LATEST
		if ( !bWiiCamera )
#endif
		{
			// Keep the distance and the height of the springs
			TVector4 vDirection;
			vDirection.x = vCameraPos.x - vLookAt.x;
			vDirection.y = 0.0f;
			vDirection.z = vCameraPos.z - vLookAt.z;
			vDirection.w = vCameraPos.w;
			vDirection.Normalise();

			TVector4 vOffset;
			vOffset.Multiply( vDirection, m_oDistanceSpring.fValue );

			vCameraPos.x = vOffset.x + vLookAt.x;
			vCameraPos.y = vOffset.y + vLookAt.y;
			vCameraPos.z = vOffset.z + vLookAt.z;
			vCameraPos.w = vLookAt.w;
			vCameraPos.y = vCameraPos.y - m_oHeightSpring.fValue;
		}

		TVector4 vPivot;
		TMatrix44::RotateVector( vPivot, matFlat, m_vPivotOffset );
		vPivot.x = rcVehiclePos.x + vPivot.x;
		vPivot.y = rcVehiclePos.y + vPivot.y;
		vPivot.z = rcVehiclePos.z + vPivot.z;

#ifdef BARNYARD_LATEST
		if ( bWiiCamera )
		{
			// The latest version moves the camera to the position the collisions push it to smoothly, and doesn't
			// reset the velocities of the springs
			static TFLOAT s_fFollowPushSpeed = 2.5f;

			TVector4 vUnpushedPos = vCameraPos;

			if ( CheckCollision( vCameraPos, vPivot ) )
				SmoothCollisionPush( vCameraPos, vUnpushedPos, vLookAt, s_fFollowPushSpeed, a_fDeltaTime );
			else if ( m_bCollided )
				m_bCollided = TFALSE;
			else
				m_fCollisionTime = 0.0f;
		}
		else
#endif
		    if ( CheckCollision( vCameraPos, vPivot ) )
		{
			TVector4 vToLookAt;
			vToLookAt.x   = vLookAt.x - vCameraPos.x;
			vToLookAt.y   = 0.0f;
			vToLookAt.z   = vLookAt.z - vCameraPos.z;
			vToLookAt.w   = vLookAt.w;
			TFLOAT fDiffY = vLookAt.y - vCameraPos.y;

			m_oDistanceSpring.fValue    = vToLookAt.Magnitude();
			m_oDistanceSpring.fVelocity = 0.0f;
			m_oHeightSpring.fValue      = TMath::Abs( fDiffY );
			m_oHeightSpring.fVelocity   = 0.0f;
		}
	}
	else if ( m_eMode == MODE_CAR_CHASE )
	{
		TVector4 vVehiclePos     = rcVehiclePos;
		TVector4 vVehicleForward = rcVehicleForward;

		m_oYawSpring.fTarget = TFLOAT( atan2( vVehicleForward.x, vVehicleForward.z ) );
		m_oYawSpring.UpdateAngle( a_fDeltaTime );

		TVector3 vEuler( 0.0f, m_oYawSpring.fValue, 0.0f );

		TQuaternion quatYaw;
		QuaternionFromEuler( quatYaw, vEuler );

		RotateVector( vCameraPos, quatYaw, m_vCameraOffset );
		vCameraPos.x = vVehiclePos.x + vCameraPos.x;
		vCameraPos.y = vVehiclePos.y + vCameraPos.y;
		vCameraPos.z = vVehiclePos.z + vCameraPos.z;

		TMatrix44::TransformVector( vLookAt, matFlat, m_vLookAtOffset );

		TVector4 vToLookAt;
		vToLookAt.Substract( vLookAt, vCameraPos );
		vToLookAt.Multiply( CARCHASE_LOOKAT_SCALE );
		vLookAt.Add( vCameraPos, vToLookAt );
	}
	else if ( m_eMode == MODE_LOOK_BACK )
	{
		// Turn around
		TVector4 vAxis;
		vAxis.Multiply( matFlat.AsBasisVector4( BASISVECTOR_RIGHT ), -1.0f );
		matFlat.SetBasisVector4( BASISVECTOR_RIGHT, vAxis );
		vAxis.Multiply( matFlat.AsBasisVector4( BASISVECTOR_FORWARD ), -1.0f );
		matFlat.SetBasisVector4( BASISVECTOR_FORWARD, vAxis );

		TMatrix44::TransformVector( vLookAt, matFlat, m_vLookAtOffset );

#ifdef BARNYARD_LATEST
		if ( bWiiCamera )
		{
			TINT iNumSteps = ConsumeSpringSteps( m_fTimeAccumulator );

			for ( TINT i = 0; i < iNumSteps; i++ )
			{
				BeforeSpringStep();
				m_oDistanceSpring.Update( SPRING_STEP_LATEST );
				m_oHeightSpring.Update( SPRING_STEP_LATEST );
			}

			fInterpolation = EndSpringSteps( iNumSteps );
		}
		else
#endif
		{
			TINT iNumSteps = TMath::FloorToInt( m_fTimeAccumulator * 29.999998f );
			m_fTimeAccumulator -= iNumSteps * SPRING_STEP;

			for ( TINT i = iNumSteps; i > 0; i-- )
			{
				m_oDistanceSpring.Update( SPRING_STEP );
				m_oHeightSpring.Update( SPRING_STEP );
			}
		}

		TVector4 vOffset;
		vOffset.Multiply( matFlat.AsBasisVector4( BASISVECTOR_FORWARD ), m_oDistanceSpring.fValue );
		vCameraPos.Substract( vLookAt, vOffset );

		vOffset.Multiply( matFlat.AsBasisVector4( BASISVECTOR_UP ), m_oHeightSpring.fValue );
		vCameraPos.x = vCameraPos.x - vOffset.x;
		vCameraPos.y = vCameraPos.y - vOffset.y;
		vCameraPos.z = vCameraPos.z - vOffset.z;

#ifdef BARNYARD_LATEST
		// fInterpolation and m_oLastSpringStep are zero without the Wii Style camera
		vRenderCorrection.Multiply( matFlat.AsBasisVector4( BASISVECTOR_FORWARD ), -m_oLastSpringStep.fDistance * fInterpolation );
		vOffset.Multiply( matFlat.AsBasisVector4( BASISVECTOR_UP ), -m_oLastSpringStep.fHeight * fInterpolation );
		vRenderCorrection.Add( vOffset );
#endif

		TVector4 vPivot;
		TMatrix44::TransformVector( vPivot, matFlat, m_vPivotOffset );

		if ( CheckCollision( vCameraPos, vPivot ) )
		{
			TVector4 vToLookAt;
			vToLookAt.Substract( vLookAt, vCameraPos );
			TFLOAT fDiffY = vToLookAt.y;
			vToLookAt.y   = 0.0f;

			m_oDistanceSpring.fValue = vToLookAt.Magnitude();
			m_oHeightSpring.fValue   = TMath::Abs( fDiffY );
		}
	}
	else if ( m_eMode == MODE_DEFAULT )
	{
		TMatrix44::TransformVector( vLookAt, matFlat, m_vLookAtOffset );

#ifdef BARNYARD_LATEST
		if ( bWiiCamera )
		{
			TINT iNumSteps = ConsumeSpringSteps( m_fTimeAccumulator );

			for ( TINT i = 0; i < iNumSteps; i++ )
			{
				BeforeSpringStep();
				m_oDistanceSpring.Update( SPRING_STEP_LATEST );
				m_oHeightSpring.Update( SPRING_STEP_LATEST );
			}

			fInterpolation = EndSpringSteps( iNumSteps );
		}
		else
#endif
		{
			TINT iNumSteps = TMath::FloorToInt( m_fTimeAccumulator * 29.999998f );
			m_fTimeAccumulator -= iNumSteps * SPRING_STEP;

			for ( TINT i = iNumSteps; i > 0; i-- )
			{
				m_oDistanceSpring.Update( SPRING_STEP );
				m_oHeightSpring.Update( SPRING_STEP );
			}
		}

		// Keep the direction from the look at point to the camera, rotated by the input
		vCameraPos = pCamera->GetMatrix().GetTranslation();

		TVector4 vDirection;
		vDirection.Substract( vCameraPos, vLookAt );
		vDirection.y = 0.0f;
		vDirection.Normalise();
		vDirection.AsVector3().RotateY( s_fRotateSpeed * m_fRotateInput * a_fDeltaTime );

		TVector4 vOffset;
		vOffset.Multiply( vDirection, m_oDistanceSpring.fValue );
		vCameraPos.Add( vLookAt, vOffset );
		vCameraPos.y = vCameraPos.y - m_oHeightSpring.fValue;

#ifdef BARNYARD_LATEST
		vRenderCorrection.Multiply( vDirection, m_oLastSpringStep.fDistance * fInterpolation );
		vRenderCorrection.y -= m_oLastSpringStep.fHeight * fInterpolation;
#endif

		TVector4 vPivotOffset = m_vPivotOffset;

		if ( m_iCarChase == 0 )
		{
			TFLOAT fSpeed = m_pTarget->GetUnit()->GetSteer()->GetSpeed();

			if ( fSpeed > 0.0f )
			{
				TFLOAT fRatio = fSpeed * 0.35999998f;
				vPivotOffset.Lerp4( FAST_PIVOT_OFFSET, TMath::Min( fRatio, 1.0f ) );
			}
		}

		TVector4 vPivot;
		TMatrix44::TransformVector( vPivot, matFlat, vPivotOffset );

#ifdef BARNYARD_LATEST
		if ( bWiiCamera )
		{
			static TFLOAT s_fDefaultPushSpeed = 2.5f;

			TVector4 vUnpushedPos = vCameraPos;

			if ( CheckCollision( vCameraPos, vPivot ) )
				SmoothCollisionPush( vCameraPos, vUnpushedPos, vLookAt, s_fDefaultPushSpeed, a_fDeltaTime );
			else if ( m_bCollided )
				m_bCollided = TFALSE;
			else
				m_fCollisionTime = 0.0f;
		}
		else
#endif
		    if ( CheckCollision( vCameraPos, vPivot ) )
		{
			TVector4 vToLookAt;
			vToLookAt.Substract( vLookAt, vCameraPos );
			TFLOAT fDiffY = vToLookAt.y;
			vToLookAt.y   = 0.0f;

			m_oDistanceSpring.fValue = vToLookAt.Magnitude();
			m_oHeightSpring.fValue   = TMath::Abs( fDiffY );
		}
	}
	else
	{
		vCameraPos = TVector4::VEC_NEGZ;
		vLookAt    = TVector4::VEC_POSZ;
	}

	FadeCloseRaceBikes( vCameraPos, vLookAt );

#ifdef BARNYARD_LATEST
	// NOTE: the fading and the near clip keep using the stepped position (what the original sees). The camera
	// that is drawn is interpolated. The correction is along the distance/height of the springs (MODE_DEFAULT
	// reads the direction back from the camera, which it doesn't change)
	TVector4 vRenderPos;
	vRenderPos.Add( vCameraPos, vRenderCorrection );
	vRenderPos.w                          = vCameraPos.w;
	pCamera->GetMatrix().GetTranslation() = vRenderPos;
#else
	pCamera->GetMatrix().GetTranslation() = vCameraPos;
#endif
	LookAtPoint( vLookAt );
	UpdateFOV( a_fDeltaTime );
	UpdateNearClip( rcVehiclePos, vCameraPos );

	m_fRotateInput = 0.0f;
	return TTRUE;
}

// $Barnyard: FUNCTION 0045ecb0
// $BarnyardWiiJP: FUNCTION 80177b6c
TBOOL ABikePhysicsCameraHelper::ProcessInputCommand( void* a_pCommand, void* a_pEvent )
{
	AInputCommand                      eCommand = AInputCommand( TUINTPTR( a_pCommand ) );
	const TInputInterface::InputEvent* pEvent   = TSTATICCAST( TInputInterface::InputEvent, a_pEvent );

	switch ( eCommand )
	{
		case AInputCommand_ZoomOut:
			PlaceCamera();
			return TTRUE;
		case AInputCommand_Trick1:
			// Look back while it's held in the car chase
			if ( m_iCarChase != 1 )
				return TFALSE;

			if ( pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
#ifdef BARNYARD_LATEST
				if ( AOptions::IsWiiCameraEnabled() )
				{
					SetMode( MODE_LOOK_BACK );
				}
				else
#endif
				{
					m_eMode = MODE_LOOK_BACK;
					PlaceCamera();
				}
				pEvent->StartRepeat( 0.0f, 0.0f );
				return TTRUE;
			}

			if ( pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
#ifdef BARNYARD_LATEST
				if ( AOptions::IsWiiCameraEnabled() )
				{
					// Goes back to MODE_FOLLOW in the car chase
					SetMode( MODE_DEFAULT );
				}
				else
#endif
				{
					m_eMode = MODE_DEFAULT;
					PlaceCamera();
				}
			}

			return TTRUE;
		case AInputCommand_RotateCameraCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_fRotateInput = m_fRotateInput + 1.0f;
			return TTRUE;
		case AInputCommand_RotateCamera:
#ifdef TOSHI_SKU_WII
			// The Wii steers with the stick of the Nunchuk, it doesn't rotate the camera
			pEvent->GetMagnitudeFloat( 0 );
			m_fRotateInput = 0.0f;
			return TTRUE;
#endif
			if ( pEvent->GetSource()->IsA( &TGetClass( TInputDeviceMouse ) ) )
			{
				if ( pEvent->GetEventType() != TInputInterface::EVENT_TYPE_MOVED )
					return TFALSE;

				m_fRotateInput = pEvent->GetMagnitudeFloat( 0 ) * AOptions::GetSingleton()->GetSensitivity() * 0.029999999f + m_fRotateInput;
				return TTRUE;
			}

			m_fRotateInput = pEvent->GetMagnitudeFloat( 0 ) + m_fRotateInput;
			return TTRUE;
		case AInputCommand_RotateCameraCCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_fRotateInput = m_fRotateInput - 1.0f;
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0045dc00
// $BarnyardWiiJP: FUNCTION 80177cb8
void ABikePhysicsCameraHelper::OnCreate()
{
	m_bUnk         = TFALSE;
	m_fRotateInput = 0.0f;
	m_bBlendingFOV = TFALSE;
	m_eMode        = MODE_DEFAULT;

#ifdef BARNYARD_LATEST
	m_fCollisionTime = 10.0f;
	m_bCollided      = TFALSE;
#endif
}

// $Barnyard: FUNCTION 0045ee30
void ABikePhysicsCameraHelper::OnDestroy()
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	// Show the bikes of the race again
	if ( IsInstanceOf( pState, GetBikeRaceMicroGameClass() ) )
	{
		ABikeRaceMicroGame* pRace      = TSTATICCAST( ABikeRaceMicroGame, pState );
		TINT                iNumRivals = pRace->GetNumRivals();

		for ( TINT i = 0; i < iNumRivals; i++ )
		{
			AUnit* pUnit = pRace->GetRivalUnit( i );

			if ( pUnit && pUnit->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
				SetBikeOpacity( pUnit, 1.0f );
		}
	}
}

// $Barnyard: FUNCTION 0045e350
// $BarnyardWiiJP: FUNCTION 80178168
void ABikePhysicsCameraHelper::OnAttach()
{
#ifdef BARNYARD_LATEST
	// NOTE: not in the original, see ResetSpringInterpolation
	ResetSpringInterpolation();

	const TBOOL bWiiCamera = AOptions::IsWiiCameraEnabled();
#endif

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( !IsInstanceOf( pState, GetCarChaseMiniGameStateClass() ) )
	{
		m_iCarChase     = 0;
		m_vLookAtOffset = DEFAULT_LOOKAT_OFFSET;
		m_vCameraOffset = DEFAULT_CAMERA_OFFSET;
		m_vPivotOffset  = DEFAULT_PIVOT_OFFSET;

#ifdef BARNYARD_LATEST
		if ( bWiiCamera )
			SetMode( MODE_DEFAULT );
#endif
	}
	else
	{
		m_iCarChase     = 1;
		m_vLookAtOffset = CARCHASE_LOOKAT_OFFSET;
		m_vCameraOffset = DEFAULT_CAMERA_OFFSET;
		m_vPivotOffset  = CARCHASE_PIVOT_OFFSET;

#ifdef BARNYARD_LATEST
		// The car chase follows the bike (MODE_FOLLOW)
		if ( bWiiCamera )
			SetMode( MODE_FOLLOW );
#endif
	}

	m_fRotateInput = 0.0f;

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( m_pTarget && m_pTarget->GetUnit() )
	{
		TMatrix44 matVehicle;
		GetVehicleTransform( matVehicle );

		const TVector4& rcCameraPos  = pCamera->GetMatrix().GetTranslation();
		const TVector4& rcVehiclePos = matVehicle.AsBasisVector4( BASISVECTOR_TRANSLATION );

		if ( m_bUnk )
			m_vSpringPosition.Substract( rcCameraPos, rcVehiclePos );
		else
			TMatrix44::RotateVector( m_vSpringPosition, matVehicle, m_vCameraOffset );

		m_vSpringVelocity = TVector4::VEC_ZERO;

		TVector4 vToVehicle;
		vToVehicle.Substract( rcVehiclePos, rcCameraPos );

		TFLOAT fYaw          = TFLOAT( atan2( vToVehicle.z, vToVehicle.x ) );
		m_oYawSpring.fValue  = fYaw;
		m_oYawSpring.fTarget = fYaw;
	}
	else
	{
		m_vSpringPosition = TVector4::VEC_ZERO;
		m_vSpringVelocity = TVector4::VEC_ZERO;
	}

	TFLOAT fHeight = TMath::Abs( m_vLookAtOffset.y - m_vCameraOffset.y );

	TVector4 vToLookAt;
	vToLookAt.x = m_vLookAtOffset.x - m_vCameraOffset.x;
	vToLookAt.y = 0.0f;
	vToLookAt.z = m_vLookAtOffset.z - m_vCameraOffset.z;
	vToLookAt.w = m_vLookAtOffset.w;

	TFLOAT fDistance = vToLookAt.Magnitude();

	m_oDistanceSpring.SetMass( 1.0f );
	m_oDistanceSpring.fStiffness = DISTANCE_SPRING_STIFFNESS;
	m_oDistanceSpring.SetDampingRatio( 1.02f );
	m_oDistanceSpring.fValue    = fDistance;
	m_oDistanceSpring.fTarget   = fDistance;
	m_oDistanceSpring.fVelocity = 0.0f;

	m_oHeightSpring.SetMass( 1.0f );
	m_oHeightSpring.fStiffness = HEIGHT_SPRING_STIFFNESS;
	m_oHeightSpring.SetDampingRatio( 1.02f );
	m_oHeightSpring.fValue    = fHeight;
	m_oHeightSpring.fTarget   = fHeight;
	m_oHeightSpring.fVelocity = 0.0f;

	m_oYawSpring.SetMass( 1.0f );
	m_oYawSpring.fStiffness = YAW_SPRING_STIFFNESS;
	m_oYawSpring.SetCriticalDamping();
	m_oYawSpring.fVelocity = 0.0f;

	m_bBlendingFOV = TFALSE;

#ifdef BARNYARD_LATEST
	if ( bWiiCamera )
	{
		m_fCollisionTime = 10.0f;
		m_bCollided      = TFALSE;
	}
	else
#endif
	{
		m_eMode = MODE_DEFAULT;
	}
}

// $Barnyard: FUNCTION 0045eb70
void ABikePhysicsCameraHelper::ResetPosition( TBOOL a_bSnapCamera )
{
	PlaceCamera();
}

// $Barnyard: FUNCTION 0045eb00
// $BarnyardWiiJP: FUNCTION 80177e28
void ABikePhysicsCameraHelper::SetTarget( TRef<AUnitProxy> a_pTarget )
{
	if ( m_pTarget.Get() != a_pTarget.Get() )
	{
		ACameraHelper::SetTarget( a_pTarget );

#ifdef BARNYARD_LATEST
		m_bCollided      = TFALSE;
		m_fCollisionTime = 10.0f;
#endif
	}
}

// $Barnyard: FUNCTION 0045e290
void ABikePhysicsCameraHelper::SetTargetUnit( AUnit* a_pUnit )
{
	if ( !m_pTarget || !m_pTarget->GetUnit() || m_pTarget->GetUnit() != a_pUnit )
	{
		ACameraHelper::SetTargetUnit( a_pUnit );
		OnAttach();
	}
}

// $Barnyard: FUNCTION 0045eeb0
void ABikePhysicsCameraHelper::OnDetach()
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	// Show the bikes of the race again
	if ( IsInstanceOf( pState, GetBikeRaceMicroGameClass() ) )
	{
		ABikeRaceMicroGame* pRace      = TSTATICCAST( ABikeRaceMicroGame, pState );
		TINT                iNumRivals = pRace->GetNumRivals();

		for ( TINT i = 0; i < iNumRivals; i++ )
		{
			AUnit* pUnit = pRace->GetRivalUnit( i );

			if ( pUnit && pUnit->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
				SetBikeOpacity( pUnit, 1.0f );
		}
	}

	ACameraHelper::OnDetach();
}

// NOTE: shares 0058ac00 with another class
TFLOAT ABikePhysicsCameraHelper::GetFOV() const
{
	return m_fFOV;
}

// $Barnyard: FUNCTION 0045dbc0
void ABikePhysicsCameraHelper::BlendFOV( TFLOAT a_fTargetFOV, TFLOAT a_fDuration )
{
	m_bBlendingFOV      = TTRUE;
	m_fFOVBlendTime     = 0.0f;
	m_fFOVBlendDuration = a_fDuration;
	m_fFOVBlendStart    = GetFOV();
	m_fFOVBlendTarget   = a_fTargetFOV;
}

// $Barnyard: FUNCTION 0045e6a0
// $BarnyardWiiJP: FUNCTION 80178574
void ABikePhysicsCameraHelper::PlaceCamera()
{
#ifdef BARNYARD_LATEST
	// NOTE: not in the original, see ResetSpringInterpolation
	ResetSpringInterpolation();
#endif

	m_oDistanceSpring.fValue    = m_oDistanceSpring.fTarget;
	m_oDistanceSpring.fVelocity = 0.0f;
	m_oHeightSpring.fValue      = m_oHeightSpring.fTarget;
	m_oHeightSpring.fVelocity   = 0.0f;
	m_fRotateInput              = 0.0f;

	if ( m_pTarget && m_pTarget->GetUnit() )
	{
		TMatrix44 matVehicle;
		GetVehicleTransform( matVehicle );

		TMatrix44 matFlat;
		FlattenMatrix( matVehicle, matFlat );

		ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

		TVector4 vCameraPos;
		TMatrix44::TransformVector( vCameraPos, matFlat, m_vCameraOffset );
		pCamera->GetMatrix().GetTranslation() = vCameraPos;

		TVector4 vLookAt;
		TMatrix44::TransformVector( vLookAt, matFlat, m_vLookAtOffset );
		LookAtPoint( vLookAt );

		pCamera->SetFOV( GetFOV() );
	}

#ifdef BARNYARD_LATEST
	// Also resets the spring of MODE_FOLLOW (like OnAttach does)
	if ( !AOptions::IsWiiCameraEnabled() )
	{
		// PC retail doesn't reset it
	}
	else if ( m_pTarget && m_pTarget->GetUnit() )
	{
		TMatrix44 matVehicle;
		GetVehicleTransform( matVehicle );

		if ( m_bUnk )
			m_vSpringPosition.Substract( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), matVehicle.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
		else
			TMatrix44::RotateVector( m_vSpringPosition, matVehicle, m_vCameraOffset );

		m_vSpringVelocity = TVector4::VEC_ZERO;
	}
	else
	{
		m_vSpringPosition = TVector4::VEC_ZERO;
		m_vSpringVelocity = TVector4::VEC_ZERO;
	}

	m_bCollided      = TFALSE;
	m_fCollisionTime = 10.0f;
#endif
}

#ifdef BARNYARD_LATEST

// $BarnyardWiiJP: FUNCTION 80177c84
void ABikePhysicsCameraHelper::SetMode( MODE a_eMode )
{
	// NOTE: not in the original, see ResetSpringInterpolation
	ResetSpringInterpolation();

	if ( m_iCarChase == 1 )
	{
		if ( a_eMode == MODE_CAR_CHASE || a_eMode == MODE_DEFAULT )
			a_eMode = MODE_FOLLOW;
	}
	else if ( a_eMode == MODE_FOLLOW || a_eMode == MODE_CAR_CHASE )
	{
		a_eMode = MODE_DEFAULT;
	}

	m_eMode = a_eMode;
	PlaceCamera();
}

void ABikePhysicsCameraHelper::SmoothCollisionPush( TVector4& a_rCameraPos, const TVector4& a_rcUnpushedPos, const TVector4& a_rcLookAt, TFLOAT& a_rfPushSpeed, TFLOAT a_fDeltaTime )
{
	// NOTE: inlined twice in OnUpdate (80178a40), with a static speed for each mode
	m_bCollided = TTRUE;
	m_fCollisionTime += a_fDeltaTime;

	TVector4 vPush;
	vPush.x = a_rCameraPos.x - a_rcUnpushedPos.x;
	vPush.y = a_rCameraPos.y - a_rcUnpushedPos.y;
	vPush.z = a_rCameraPos.z - a_rcUnpushedPos.z;
	vPush.w = a_rCameraPos.w;

	TFLOAT fPushDistance = vPush.NormaliseMag();

	if ( m_iCarChase == 1 )
		a_rfPushSpeed = 5.0f;

	// NOTE: the original also computes the length of the spring targets here and doesn't use it

	// Move from the unpushed position towards the pushed one
	TFLOAT fDistance = fPushDistance;
	MoveTowards( fDistance, fPushDistance, 0.0f, 2.0f * a_rfPushSpeed, 2.0f * a_rfPushSpeed, m_fCollisionTime );

	TVector4 vOffset;
	vOffset.Multiply( vPush, fDistance );

	a_rCameraPos.x = a_rcUnpushedPos.x + vOffset.x;
	a_rCameraPos.y = a_rcUnpushedPos.y + vOffset.y;
	a_rCameraPos.z = a_rcUnpushedPos.z + vOffset.z;
	a_rCameraPos.w = a_rcUnpushedPos.w;

	TVector4 vToLookAt;
	vToLookAt.x = a_rcLookAt.x - a_rCameraPos.x;
	vToLookAt.y = 0.0f;
	vToLookAt.z = a_rcLookAt.z - a_rCameraPos.z;
	vToLookAt.w = a_rcLookAt.w;

	m_oDistanceSpring.fValue = vToLookAt.Magnitude();
	m_oHeightSpring.fValue   = TMath::Abs( a_rcLookAt.y - a_rCameraPos.y );
}

void ABikePhysicsCameraHelper::GetSpringState( SpringState& a_rOutState ) const
{
	a_rOutState.vPosition = m_vSpringPosition;
	a_rOutState.fDistance = m_oDistanceSpring.fValue;
	a_rOutState.fHeight   = m_oHeightSpring.fValue;
}

TFLOAT ABikePhysicsCameraHelper::EndSpringSteps( TINT a_iNumSteps )
{
	if ( a_iNumSteps > 0 )
	{
		SpringState oState;
		GetSpringState( oState );

		m_oLastSpringStep.vPosition.Substract( oState.vPosition, m_oPrevSpringState.vPosition );
		m_oLastSpringStep.vPosition.w = 0.0f;
		m_oLastSpringStep.fDistance   = oState.fDistance - m_oPrevSpringState.fDistance;
		m_oLastSpringStep.fHeight     = oState.fHeight - m_oPrevSpringState.fHeight;
	}

	TFLOAT fAlpha = m_fTimeAccumulator / SPRING_STEP_LATEST;
	TMath::Clip( fAlpha, 0.0f, 1.0f );

	return fAlpha - 1.0f;
}

void ABikePhysicsCameraHelper::ResetSpringInterpolation()
{
	m_oLastSpringStep.vPosition = TVector4::VEC_ZERO;
	m_oLastSpringStep.fDistance = 0.0f;
	m_oLastSpringStep.fHeight   = 0.0f;
}

#endif // BARNYARD_LATEST

// $Barnyard: FUNCTION 0045e2c0
void ABikePhysicsCameraHelper::GetVehicleTransform( TMatrix44& a_rOutTransform )
{
	a_rOutTransform = TMatrix44::IDENTITY;

	ASteer* pSteer = m_pTarget->GetUnit()->GetSteer();

	if ( pSteer->IsA( &TGetClass( ABikeSteer ) ) )
	{
		a_rOutTransform = TSTATICCAST( ABikeSteer, pSteer )->GetPhysics()->GetBody().GetTransform();
		return;
	}

	if ( pSteer->IsA( &TGetClass( ARacingCarSteer ) ) )
		a_rOutTransform = TSTATICCAST( ARacingCarSteer, pSteer )->GetCar()->GetBody().GetTransform();
}

// $Barnyard: FUNCTION 0045df20
TBOOL ABikePhysicsCameraHelper::CheckCollision( TVector4& a_rCameraPos, const TVector4& a_rcPivot )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

#ifdef BARNYARD_LATEST
	const TINT iCameraFlags = AOptions::IsWiiCameraEnabled() ? CAMERA_COLLISION_FLAGS_LATEST : CAMERA_COLLISION_FLAGS;
	TINT       iFlags       = ( m_iCarChase != 1 ) ? iCameraFlags : CARCHASE_CAMERA_COLLISION_FLAGS;
#else
	TINT iFlags                           = ( m_iCarChase != 1 ) ? CAMERA_COLLISION_FLAGS : CARCHASE_CAMERA_COLLISION_FLAGS;
#endif
	TFLOAT fRadius = 0.25f / TFLOAT( cos( pCamera->GetFOV() * 0.5f ) );

	// Cast a ray 2 units down from the camera and push it up out of the ground
	ARigidBodyCollision oGroundHit;

	TVector4 vDown;
	vDown.Multiply( TVector4::VEC_POSY, 2.0f );

	TVector4 vRayEnd;
	vRayEnd.Add( a_rCameraPos, vDown );

	TBOOL bOnGround = AVehicleManager::CollideRay( a_rCameraPos, vRayEnd, &oGroundHit, iFlags, TCollisionCommon::TOSHICGROUPFLAG( m_uiCollisionMask ) );

	if ( bOnGround == TTRUE )
	{
		TVector4 vToCamera;
		vToCamera.Substract( a_rCameraPos, oGroundHit.m_vPosition );

		TFLOAT fHeight = vToCamera.Magnitude();

		if ( fHeight >= fRadius || -TFLOAT( cos( 1.047197543084621 ) ) >= TVector4::DotProduct3( TVector4::VEC_POSY, oGroundHit.m_vNormal ) )
		{
			bOnGround = TFALSE;
		}
		else
		{
			TVector4 vPush;
			vPush.Multiply( TVector4::VEC_POSY, fRadius - fHeight );

			a_rCameraPos.x = a_rCameraPos.x - vPush.x;
			a_rCameraPos.y = a_rCameraPos.y - vPush.y;
			a_rCameraPos.z = a_rCameraPos.z - vPush.z;
		}
	}

	// Sweep a sphere from the pivot to the camera
	TVector4 vDirection;
	vDirection.Substract( a_rCameraPos, a_rcPivot );

	TFLOAT fDistance = vDirection.NormaliseMag();

	TVector4 vHitNormal;
	TBOOL    bHit = SweepSphere( a_rcPivot, vDirection, fRadius, fDistance, &fDistance, &vHitNormal, iFlags, m_uiCollisionMask );

	if ( bHit )
	{
		TFLOAT fMinDistance = 0.6f;
		fDistance           = ( fDistance >= 0.6f ) ? fDistance : fMinDistance;

		TVector4 vOffset;
		vOffset.Multiply( vDirection, fDistance );

		a_rCameraPos.x = vOffset.x + a_rcPivot.x;
		a_rCameraPos.y = vOffset.y + a_rcPivot.y;
		a_rCameraPos.z = vOffset.z + a_rcPivot.z;
		a_rCameraPos.w = a_rcPivot.w;
	}

	return bOnGround | bHit;
}

// $Barnyard: FUNCTION 0045e770
void ABikePhysicsCameraHelper::UpdateFOV( TFLOAT a_fDeltaTime )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( m_bBlendingFOV )
	{
		m_fFOVBlendTime += a_fDeltaTime;

		if ( m_fFOVBlendTime > m_fFOVBlendDuration )
			m_fFOVBlendTime = m_fFOVBlendDuration;

		TFLOAT fProgress = m_fFOVBlendTime / m_fFOVBlendDuration;
		TMath::Clip( fProgress, 0.0f, 1.0f );

		pCamera->SetFOV( TMath::LERP( m_fFOVBlendStart, m_fFOVBlendTarget, fProgress ) );
		return;
	}

	ASteer* pSteer = m_pTarget->GetUnit()->GetSteer();
	TFLOAT  fFOV;
	TFLOAT  fProgress;

	if ( pSteer->IsA( &TGetClass( ABikeSteer ) ) )
	{
		// Widen the FOV while boosting
		APhysicsBike* pPhysics  = TSTATICCAST( ABikeSteer, pSteer )->GetPhysics();
		TFLOAT        fSpeed    = pPhysics->GetBody().GetVelocity().Magnitude();
		TFLOAT        fMinSpeed = pPhysics->m_oParams.fMaxSpeed;
		TFLOAT        fMaxSpeed = pPhysics->m_oParams.fMaxBoostSpeed;

		fFOV      = GetFOV();
		fProgress = ( fSpeed - fMinSpeed ) / ( fMaxSpeed - fMinSpeed );
		TMath::Clip( fProgress, 0.0f, 1.0f );
	}
	else
	{
		if ( !pSteer->IsA( &TGetClass( ARacingCarSteer ) ) )
			return;

		// Widen the FOV above a fifth of the max speed
		TFLOAT fSpeed    = pSteer->GetSpeed();
		TFLOAT fMaxSpeed = TSTATICCAST( ARacingCarSteer, pSteer )->GetMaxSpeed();

		// NOTE: retail keeps the min speed of the first car in a static (0x00782380)
		static TFLOAT s_fMinSpeed = fMaxSpeed * 0.2f;

		fFOV      = GetFOV();
		fProgress = ( fSpeed - s_fMinSpeed ) / ( fMaxSpeed - s_fMinSpeed );
		TMath::Clip( fProgress, 0.0f, 1.0f );
	}

	pCamera->SetFOV( TMath::LERP( fFOV, 1.5707964f, fProgress ) );
}

// $Barnyard: FUNCTION 0045de40
void ABikePhysicsCameraHelper::UpdateNearClip( const TVector4& a_rcVehiclePos, const TVector4& a_rcCameraPos )
{
	if ( m_iCarChase == 1 )
		return;

	TVector4 vToVehicle;
	vToVehicle.x = a_rcVehiclePos.x - a_rcCameraPos.x;
	vToVehicle.y = 0.0f;
	vToVehicle.z = a_rcVehiclePos.z - a_rcCameraPos.z;
	vToVehicle.w = a_rcVehiclePos.w;

	TFLOAT fDistance = vToVehicle.Magnitude();
	TFLOAT fNear;

	if ( fDistance < 0.5f )
		fNear = 0.01f;
	else if ( fDistance >= 1.5f )
		fNear = 0.25f;
	else
	{
		fNear = ( fDistance - 0.5f ) * 0.25f;

		if ( fNear < 0.01f )
			fNear = 0.01f;
	}

	ARenderer::GetSingleton()->SetNear( fNear );
}

// $Barnyard: FUNCTION 0045ef40
void ABikePhysicsCameraHelper::FadeCloseRaceBikes( const TVector4& a_rcCameraPos, const TVector4& a_rcLookAt )
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( !IsInstanceOf( pState, GetBikeRaceMicroGameClass() ) )
		return;

	ABikeRaceMicroGame* pRace      = TSTATICCAST( ABikeRaceMicroGame, pState );
	TINT                iNumRivals = pRace->GetNumRivals();

	for ( TINT i = 0; i < iNumRivals; i++ )
	{
		AUnit* pUnit = pRace->GetRivalUnit( i );

		if ( pUnit && pUnit->GetSteer()->IsA( &TGetClass( ABikeSteer ) ) )
		{
			// Fade the bikes out when the camera is in them
			const TVector4& rcPosition = *pUnit->GetSteer()->GetPosition();
			TVector4        vecToCamera( a_rcCameraPos.x - rcPosition.x, 0.0f, a_rcCameraPos.z - rcPosition.z, a_rcCameraPos.w );
			TFLOAT          fDistance = vecToCamera.Magnitude();
			TFLOAT          fOpacity  = 1.0f;

			if ( fDistance < 1.25f )
			{
				fOpacity = fDistance - 0.75f + fDistance - 0.75f;
				TMath::Clip( fOpacity, 0.0f, 1.0f );
			}

			SetBikeOpacity( pUnit, fOpacity );
		}
	}
}

// $Barnyard: FUNCTION 0045eb80
void ABikePhysicsCameraHelper::SetBikeOpacity( AUnit* a_pUnit, TFLOAT a_fOpacity )
{
	AAnimatedUnit*     pUnit = TSTATICCAST( AAnimatedUnit, a_pUnit );
	AAnimatableObject* pBike = AAnimatableObjectManager::GetSingleton()->GetAttachedObjectByBoneName( pUnit->GetAnimatableObject(), TPS8( body_loc ), 0 );

	TMath::Clip( a_fOpacity, 0.0f, 1.0f );

	// NOTE: the original passes a_fOpacity < 1.0f to 00610780, which ignores it
	pUnit->GetModelInstance()->DisableDrawWithDefaultMaterials();
	pUnit->GetModelInstance()->SetOpacity( a_fOpacity );

	if ( pBike )
	{
		pBike->GetModelInstance()->DisableDrawWithDefaultMaterials();
		pBike->GetModelInstance()->SetOpacity( a_fOpacity );
	}
}

// $Barnyard: FUNCTION 0045dc20
void ABikePhysicsCameraHelper::UpdateSpringTargets()
{
	TVector4 vDiff(
	    m_vLookAtOffset.x - m_vCameraOffset.x,
	    0.0f,
	    m_vLookAtOffset.z - m_vCameraOffset.z,
	    m_vLookAtOffset.w
	);

	m_oDistanceSpring.fTarget = vDiff.Magnitude();
	m_oHeightSpring.fTarget   = TMath::Abs( m_vLookAtOffset.y - m_vCameraOffset.y );
}

// $BarnyardWiiJP: FUNCTION 80178a20
void ABikePhysicsCameraHelper::SnapSprings()
{
#ifdef BARNYARD_LATEST
	// NOTE: not in the original, see ResetSpringInterpolation
	ResetSpringInterpolation();
#endif

	m_oDistanceSpring.fValue    = m_oDistanceSpring.fTarget;
	m_oDistanceSpring.fVelocity = 0.0f;
	m_oHeightSpring.fValue      = m_oHeightSpring.fTarget;
	m_oHeightSpring.fVelocity   = 0.0f;
}

// $Barnyard: FUNCTION 0045dd50
void ABikePhysicsCameraHelper::SetLookAtOffset( const TVector4& a_rcOffset )
{
	m_vLookAtOffset = a_rcOffset;
	UpdateSpringTargets();
	SnapSprings();
}

// $Barnyard: FUNCTION 0045dda0
void ABikePhysicsCameraHelper::SetCameraOffset( const TVector4& a_rcOffset )
{
	m_vCameraOffset = a_rcOffset;
	UpdateSpringTargets();
	SnapSprings();
}

// $Barnyard: FUNCTION 0045ddf0
void ABikePhysicsCameraHelper::SetPivotOffset( const TVector4& a_rcOffset )
{
	m_vPivotOffset = a_rcOffset;
	UpdateSpringTargets();
	SnapSprings();
}
