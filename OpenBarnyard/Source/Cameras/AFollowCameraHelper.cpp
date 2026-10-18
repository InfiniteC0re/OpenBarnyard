#include "pch.h"
#include "AFollowCameraHelper.h"
#include "ACameraManager.h"
#include "Input/AInputMap.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Render/ADebugOverlay.h"

#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00461950, CreateObject 004626a0, CreateObjectInPlace 00462700
TDEFINE_CLASS( AFollowCameraHelper );

// $Barnyard: FUNCTION 00462380
AFollowCameraHelper::AFollowCameraHelper()
    : m_vDirection( TVector4::VEC_ZERO )
    , m_fYawSpeed( 2.0f )
    , m_fPitchSpeed( 1.0f )
    , m_pFollowCamera( TNULL )
    , m_bKeepRotatingOnRelease( TTRUE )
    , m_bYawReleased( TFALSE )
    , m_bPitchReleased( TFALSE )
    , m_iYaw( 0 )
    , m_iPitch( 0 )
    , m_iZoom( 0 )
    , m_iUnk48( 2 )
    , m_bFixedPosition( TFALSE )
{
}

// $Barnyard: FUNCTION 00462250
// $Barnyard: FUNCTION 00462420
AFollowCameraHelper::~AFollowCameraHelper()
{
}

// $Barnyard: FUNCTION 004622b0
TBOOL AFollowCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_iYawInput )
	{
		m_iYaw         = m_iYawInput;
		m_bYawReleased = TFALSE;
	}

	if ( m_iPitchInput )
	{
		m_iPitch         = m_iPitchInput;
		m_bPitchReleased = TFALSE;
	}

	if ( m_iZoomInput )
		m_iZoom = m_iZoomInput;

	UpdateCamera( a_fDeltaTime );

	ADEBUGLINE( LINE_FOLLOW_CAMERA, ADebugOverlay::INFO_COLOUR, "Follow Vector(%f,%f,%f)", m_vDirection.x, m_vDirection.y, m_vDirection.z );

	// Remember the released input
	if ( !m_iYawInput && m_iYaw )
	{
		m_iLastYaw = m_iYaw;
		m_iYaw     = 0;

		if ( m_bKeepRotatingOnRelease )
			m_bYawReleased = TTRUE;
	}

	if ( !m_iPitchInput && m_iPitch )
	{
		m_iLastPitch = m_iPitch;
		m_iPitch     = 0;

		if ( m_bKeepRotatingOnRelease )
			m_bPitchReleased = TTRUE;
	}

	if ( !m_iZoomInput )
		m_iZoom = 0;

	GetCameraRotation( m_vRotation );

	m_vUnk60      = TVector2::VEC_ZERO;
	m_vStickInput = TVector2::VEC_ZERO;
	m_iYawInput   = 0;
	m_iPitchInput = 0;
	m_iZoomInput  = 0;

	return TTRUE;
}

// $Barnyard: FUNCTION 00461c10
TBOOL AFollowCameraHelper::ProcessInputCommand( void* a_pCommand, void* a_pEvent )
{
	AInputCommand                      eCommand = AInputCommand( TUINTPTR( a_pCommand ) );
	const TInputInterface::InputEvent* pEvent   = TSTATICCAST( TInputInterface::InputEvent, a_pEvent );

	switch ( eCommand )
	{
		case AInputCommand_ZoomIn:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_iZoomInput = -1;
			return TTRUE;
		case AInputCommand_ZoomOut:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_iZoomInput = 1;
			return TTRUE;
		case AInputCommand_ElevateCameraCCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_iPitchInput = -1;
			return TTRUE;
		case AInputCommand_ElevateCameraCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_iPitchInput = 1;
			return TTRUE;
		case AInputCommand_RotateCameraCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_iYawInput = 1;
			return TTRUE;
		case AInputCommand_RotateCamera:
		{
			m_vStickInput.x = pEvent->GetMagnitudeFloat( 0 ) + m_vStickInput.x;

			TFLOAT fY       = pEvent->GetMagnitudeFloat( 1 ) + m_vStickInput.y;
			m_vStickInput.y = fY;

			if ( m_vStickInput.x > 0.3f )
				m_iYawInput = -1;
			else if ( m_vStickInput.x < -0.3f )
				m_iYawInput = 1;

			if ( fY > 0.3f )
				m_iPitchInput = 1;
			else if ( fY < -0.3f )
				m_iPitchInput = -1;

			return TTRUE;
		}
		case AInputCommand_RotateCameraCCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_iYawInput = -1;
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00462570
void AFollowCameraHelper::OnCreate()
{
	m_pFollowCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	m_pTarget       = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSelfProxy();

	Setup( m_pTarget, m_pFollowCamera, TMath::HALF_PI * 0.3f, 0.0f, 10.0f, 1.0f, 2.0f, 1.0f );

	m_iYawInput   = 0;
	m_iPitchInput = 0;
	m_vStickInput = TVector2::VEC_ZERO;
	m_vUnk60      = TVector2::VEC_ZERO;
}

// $Barnyard: FUNCTION 00462440
void AFollowCameraHelper::Setup( TRef<AUnitProxy> a_pTarget, ACamera* a_pCamera, TFLOAT a_fPitch, TFLOAT a_fYaw, TFLOAT a_fDistance, TFLOAT a_fZoomSpeed, TFLOAT a_fYawSpeed, TFLOAT a_fPitchSpeed )
{
	m_pTarget = a_pTarget;

	const TVector4* pTargetDirection = m_pTarget->GetUnit()->GetSteer()->GetDirection();
	m_vDirection.x                   = pTargetDirection->x;
	m_vDirection.z                   = pTargetDirection->z;
	m_vDirection.y                   = 0.0f;
	m_vDirection.w                   = 1.0f;

	m_pFollowCamera = a_pCamera;
	SetYaw( a_fYaw );
	SetPitch( a_fPitch );
	m_vDirection.Multiply( a_fDistance / m_vDirection.Magnitude() );

	m_fYawSpeed      = a_fYawSpeed;
	m_fPitchSpeed    = a_fPitchSpeed;
	m_fZoomSpeed     = a_fZoomSpeed;
	m_bYawReleased   = TFALSE;
	m_bPitchReleased = TFALSE;
	m_iPitch         = 0;
	m_iYaw           = 0;
	m_iZoom          = 0;
}

// NOTE: shares 0045b680 with ACameraHelper::ResetTarget
void AFollowCameraHelper::OnDestroy()
{
	m_pTarget = TNULL;
}

// $Barnyard: FUNCTION 00461960
void AFollowCameraHelper::GetCameraRotation( TVector4& a_rOutRotation ) const
{
	a_rOutRotation.x = m_vDirection.x;
	a_rOutRotation.y = 0.0f;
	a_rOutRotation.z = m_vDirection.z;
	a_rOutRotation.w = 1.0f;

	// NOTE: the original computes the length with fsqrt, which (unlike TMath::Sqrt) accepts zero
	if ( sqrtf( a_rOutRotation.MagnitudeSq() ) > 0.1f )
		a_rOutRotation.Normalise();
}

// $Barnyard: FUNCTION 00461f60
void AFollowCameraHelper::UpdateCamera( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fYawReleaseTime;   // 0x0078244c
	static TFLOAT s_fPitchReleaseTime; // 0x00782448

	if ( m_iUnk48 == 2 )
	{
		if ( m_bKeepRotatingOnRelease )
		{
			if ( m_bYawReleased )
			{
				s_fYawReleaseTime += a_fDeltaTime;
				TFLOAT fSpeed = GetReleaseSpeed( m_fYawSpeed, &s_fYawReleaseTime );

				if ( fSpeed < 0.1f )
				{
					m_bPitchReleased = TFALSE;
					m_bYawReleased   = TFALSE;
				}

				m_vDirection.AsVector3().RotateY( m_iLastYaw * fSpeed * a_fDeltaTime );
			}

			if ( m_bPitchReleased )
			{
				s_fPitchReleaseTime += a_fDeltaTime;

				// NOTE: the original uses the yaw speed here as well
				TFLOAT fSpeed = GetReleaseSpeed( m_fYawSpeed, &s_fPitchReleaseTime );

				if ( fSpeed < 0.1f )
				{
					m_bYawReleased   = TFALSE;
					m_bPitchReleased = TFALSE;
				}

				Elevate( m_iLastPitch * fSpeed * a_fDeltaTime );
			}
		}

		if ( m_iYaw != 0 )
			m_vDirection.AsVector3().RotateY( m_iYaw * m_fYawSpeed * a_fDeltaTime );

		if ( m_iPitch != 0 )
			Elevate( m_iPitch * m_fPitchSpeed * a_fDeltaTime );

		if ( m_iZoom != 0 )
			Zoom( m_iZoom * m_fZoomSpeed * a_fDeltaTime );
	}

	TVector4 vTargetPos = *m_pTarget->GetUnit()->GetSteer()->GetPosition();
	TVector4 vLookDirection;

	if ( !m_bFixedPosition )
	{
		TVector4& rCameraPos = m_pFollowCamera->GetMatrix().GetTranslation();
		rCameraPos.x         = vTargetPos.x - m_vDirection.x;
		rCameraPos.y         = vTargetPos.y - m_vDirection.y - 1.5f;
		rCameraPos.z         = vTargetPos.z - m_vDirection.z;
		rCameraPos.w         = vTargetPos.w;

		vLookDirection = m_vDirection;

		if ( vLookDirection.Magnitude() > 0.1f )
			vLookDirection.Normalise();

		m_pFollowCamera->LookAtDirection( vLookDirection );
	}
	else
	{
		m_pFollowCamera->GetMatrix().GetTranslation() = m_vFixedPosition;

		vLookDirection.x = vTargetPos.x - m_vFixedPosition.x;
		vLookDirection.y = vTargetPos.y - m_vFixedPosition.y;
		vLookDirection.z = vTargetPos.z - m_vFixedPosition.z;
		vLookDirection.w = 1.0f;
		m_vDirection     = vLookDirection;

		if ( vLookDirection.Magnitude() > 0.1f )
			vLookDirection.Normalise();

		m_pFollowCamera->LookAtDirection( vLookDirection );

		m_oSpline.SetDestination( m_vFixedPosition, vLookDirection, vLookDirection, 0.5f );
		m_oSpline.Update( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 00461a30
void AFollowCameraHelper::Elevate( TFLOAT a_fAngle )
{
	TVector4 vAxis( m_vDirection.x, 0.0f, m_vDirection.z, 1.0f );
	vAxis.AsVector3().RotateY( TMath::HALF_PI );
	vAxis.Normalise();

	TQuaternion quatRotation;
	quatRotation.SetRotation( vAxis.AsVector3(), a_fAngle );
	TQuaternion::RotateVector( m_vDirection.AsVector3(), quatRotation, m_vDirection.AsVector3() );

	TVector4 vDirection = m_vDirection;
	vDirection.Normalise();

	TFLOAT fDot = TVector4::DotProduct3( vDirection, TVector4::VEC_NEGY );

	if ( fDot < -0.8f )
	{
		quatRotation.SetRotation( vAxis.AsVector3(), -TMath::ACos( -0.8f ) );

		TVector4 vDown;
		vDown.Multiply( TVector4::VEC_NEGY, m_vDirection.Magnitude() );
		TQuaternion::RotateVector( m_vDirection.AsVector3(), quatRotation, vDown.AsVector3() );
	}
	else if ( fDot > -0.2f )
	{
		quatRotation.SetRotation( vAxis.AsVector3(), -TMath::ACos( -0.2f ) );

		TVector4 vDown;
		vDown.Multiply( TVector4::VEC_NEGY, m_vDirection.Magnitude() );
		TQuaternion::RotateVector( m_vDirection.AsVector3(), quatRotation, vDown.AsVector3() );
	}
}

// $Barnyard: FUNCTION 004619a0
void AFollowCameraHelper::Zoom( TFLOAT a_fAmount )
{
	TFLOAT fDistance = m_vDirection.Magnitude();
	TFLOAT fScale    = fDistance * a_fAmount * 0.1f + a_fAmount / fDistance + 1.0f;
	TMath::Clip( fScale, 0.7f, 1.3f );

	TFLOAT fNewDistance = fScale * fDistance;

	if ( fNewDistance > 3.0f && fNewDistance < 100.0f )
		m_vDirection.Multiply( fScale );
}

// $Barnyard: FUNCTION 00461e30
void AFollowCameraHelper::SetPitch( TFLOAT a_fPitch )
{
	TVector4 vFlatDirection( m_vDirection.x, 0.0f, m_vDirection.z, 1.0f );
	TFLOAT   fDelta = a_fPitch - GetAngleAroundY( vFlatDirection, m_vDirection );

	TVector4 vAxis = m_vDirection;
	vAxis.AsVector3().RotateY( TMath::HALF_PI );
	vAxis.Normalise();

	TQuaternion quatRotation;
	quatRotation.SetRotation( vAxis.AsVector3(), -fDelta );
	TQuaternion::RotateVector( m_vDirection.AsVector3(), quatRotation, m_vDirection.AsVector3() );
}

// $Barnyard: FUNCTION 00461ed0
void AFollowCameraHelper::SetYaw( TFLOAT a_fYaw )
{
	TVector4 vTargetDirection = *m_pTarget->GetUnit()->GetSteer()->GetDirection();
	TVector4 vFlatDirection( m_vDirection.x, 0.0f, m_vDirection.z, 1.0f );

	m_vDirection.AsVector3().RotateY( -( a_fYaw - GetAngleAroundY( vTargetDirection, vFlatDirection ) ) );
}

// $Barnyard: FUNCTION 00461bd0
TFLOAT AFollowCameraHelper::GetReleaseSpeed( TFLOAT a_fSpeed, TFLOAT* a_pTime )
{
	TFLOAT fSpeed = TMath::LERP( a_fSpeed, 0.0f, *a_pTime * 3.0f );

	if ( fSpeed < 0.1f )
		*a_pTime = 0.0f;

	return fSpeed;
}

// $Barnyard: FUNCTION 00461db0
TFLOAT AFollowCameraHelper::GetAngleAroundY( const TVector4& a_rcVec1, const TVector4& a_rcVec2 )
{
	TFLOAT fAngle = a_rcVec1.AsVector3().GetAngleBetweenVector( a_rcVec2.AsVector3() );

	TVector4 vPerpendicular( a_rcVec1.x, a_rcVec1.y, a_rcVec1.z, 1.0f );
	vPerpendicular.AsVector3().RotateY( TMath::HALF_PI );

	if ( vPerpendicular.AsVector3().GetAngleBetweenVector( a_rcVec2.AsVector3() ) >= TMath::HALF_PI )
		return fAngle;

	return TMath::PI + TMath::PI - fAngle;
}
