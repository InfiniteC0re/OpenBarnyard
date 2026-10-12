#include "pch.h"
#include "ABarnStairCamera.h"
#include "AGTAStyleCameraHelper.h"
#include "ACameraManager.h"
#include "Animation/AModel.h"
#include "Units/AUnitPlayer.h"
#include "Units/APlayerControlComponent.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Physics/ACollisionObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Defined in AGTAStyleCameraHelper.cpp
TBOOL SweepSphere( const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength, TFLOAT* a_pOutFraction, TVector4* a_pOutNormal, TINT a_iFlags, TUINT a_uiCollisionMask );

// Set on the collision object of the target while the camera checks for collisions
static constexpr TINT COLLISIONOBJECT_FLAG_CAMERA_IGNORE = ACollisionObject::TYPE_IGNORED;

// Flags passed to the camera collision checks
static constexpr TINT CAMERA_COLLISION_FLAGS = ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_UNIT;

static constexpr TINT PATH_NUM_POINTS = 14;

// Points of the path (0x0071c9e8)
static const TVector4 s_aPathPoints[ PATH_NUM_POINTS ] = {
	TVector4( 26.0f, -8.3f, 30.0f, 1.0f ),
	TVector4( 36.5f, -8.3f, 35.0f, 1.0f ),
	TVector4( 36.5f, -8.3f, 38.0f, 1.0f ),
	TVector4( 36.5f, -6.5f, 41.5f, 1.0f ),
	TVector4( 35.0f, -6.0f, 43.0f, 1.0f ),
	TVector4( 33.5f, -5.8f, 41.5f, 1.0f ),
	TVector4( 33.5f, -4.5f, 38.5f, 1.0f ),
	TVector4( 33.5f, -3.02f, 35.03f, 1.0f ),
	TVector4( 33.5f, -1.5f, 29.0f, 1.0f ),
	TVector4( 35.0f, -1.5f, 28.1f, 1.0f ),
	TVector4( 36.5f, -1.5f, 29.0f, 1.0f ),
	TVector4( 36.5f, 1.2f, 32.0f, 1.0f ),
	TVector4( 36.0f, 2.25f, 34.5f, 1.0f ),
	TVector4( 26.0f, 2.35f, 35.5f, 1.0f ),
};

// Tangents of the path (0x0071cac8)
static const TVector4 s_aPathTangents[ PATH_NUM_POINTS ] = {
	TVector4( 1.0f, 0.0f, 0.0f, 1.0f ),
	TVector4( 0.8f, 0.0f, 2.0f, 1.0f ),
	TVector4( 0.0f, 0.0f, 1.0f, 1.0f ),
	TVector4( 0.7071f, 0.2f, 2.3071f, 1.0f ),
	TVector4( -2.0f, 0.0f, 0.0f, 1.0f ),
	TVector4( 0.7071f, 0.2f, -2.3071f, 1.0f ),
	TVector4( 0.0f, 0.3f, -1.7f, 1.0f ),
	TVector4( 0.0f, 0.0f, -1.0f, 1.0f ),
	TVector4( 0.7071f, 0.2f, -2.3071f, 1.0f ),
	TVector4( 2.0f, 0.0f, 0.0f, 1.0f ),
	TVector4( 0.7071f, 0.2f, 2.3071f, 1.0f ),
	TVector4( 0.0f, 0.3f, 1.7f, 1.0f ),
	TVector4( -1.7071f, 0.0f, 1.7071f, 1.0f ),
	TVector4( -1.0f, 0.0f, 0.0f, 1.0f ),
};

static AGTAStyleCameraHelper* GetGTAStyleCameraHelper()
{
	return TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
}

static void SetPlayerCanSprint( AUnit* a_pUnit, TBOOL a_bCanSprint )
{
	if ( a_pUnit->GetClass()->IsA( &TGetClass( AUnitPlayer ) ) )
	{
		APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, a_pUnit->FindComponent( &TGetClass( APlayerControlComponent ) ) );
		pPlayerControl->SetCanSprint( a_bCanSprint );
	}
}

// $Barnyard: FUNCTION 0045d8e0
ABarnStairCamera::ABarnStairCamera()
    : m_oCurve1( 13 )
    , m_oCurve2( 13 )
    , m_pCurrentCurve( TNULL )
    , m_bOnPath( TFALSE )
{
	// NOTE: retail reads m_bActive uninitialised here (inlined SetActive( TFALSE ))
	m_bActive = TFALSE;
	SetActive( TFALSE );
}

// $Barnyard: FUNCTION 0045cdb0
// $Barnyard: FUNCTION 0045ce10
ABarnStairCamera::~ABarnStairCamera()
{
}

// $Barnyard: FUNCTION 0045ce30
void ABarnStairCamera::Reset()
{
	TVector4 aPoints[ PATH_NUM_POINTS ];
	TVector4 aTangents[ PATH_NUM_POINTS ];

	for ( TINT i = 0; i < PATH_NUM_POINTS; i++ )
	{
		aPoints[ i ]   = s_aPathPoints[ i ];
		aTangents[ i ] = s_aPathTangents[ i ];
	}

	m_oCurve1.Create( PATH_NUM_POINTS - 1, aPoints, aTangents );

	// Mirror the path around x = 15
	for ( TINT i = 0; i < PATH_NUM_POINTS; i++ )
	{
		aPoints[ i ]   = s_aPathPoints[ i ];
		aPoints[ i ].x = ( aPoints[ i ].x - 30.0f ) * -1.0f;

		aTangents[ i ]   = s_aPathTangents[ i ];
		aTangents[ i ].x = aTangents[ i ].x * -1.0f;
	}

	m_oCurve2.Create( PATH_NUM_POINTS - 1, aPoints, aTangents );
}

// $Barnyard: FUNCTION 0045cf00
void ABarnStairCamera::UpdateReturn( TFLOAT a_fDeltaTime, AUnit* a_pUnit )
{
	if ( !m_bReturning )
		return;

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	TVector4 vCameraPos = pCamera->GetMatrix().GetTranslation();
	TVector4 vCameraDir( pCamera->GetMatrix().AsBasisVector3( BASISVECTOR_FORWARD ) );
	TFLOAT   fSpeed = 1.0f;

	TVector4 vStart     = *a_pUnit->GetSteer()->GetPosition();
	TVector4 vDirection = m_vPosition;

	AGTAStyleCameraHelper* pHelper = GetGTAStyleCameraHelper();
	vStart.y -= pHelper->GetHeightOffset();

	vDirection.x -= vStart.x;
	vDirection.y -= vStart.y;
	vDirection.z -= vStart.z;

	// Return faster if something is between the target and the camera
	TFLOAT fLength = vDirection.NormaliseMag();

	if ( fLength > 0.0001f )
	{
		ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, a_pUnit )->GetCollisionObject();
		TVector4          vHitNormal;

		pCollisionObject->SetType( pCollisionObject->GetType() | COLLISIONOBJECT_FLAG_CAMERA_IGNORE );
		TBOOL bHit = SweepSphere( vStart, vDirection, 0.4f, fLength, &fLength, &vHitNormal, CAMERA_COLLISION_FLAGS, pHelper->GetCollisionMask() );
		pCollisionObject->SetType( pCollisionObject->GetType() & ~COLLISIONOBJECT_FLAG_CAMERA_IGNORE );

		if ( bHit )
			fSpeed = 3.0f;
	}

	m_fBlendTime += fSpeed * a_fDeltaTime;

	if ( m_fBlendTime > 1.0f )
	{
		m_bReturning = TFALSE;
		m_fBlendTime = 1.0f;

		SetPlayerCanSprint( a_pUnit, TTRUE );
	}

	vCameraPos.Lerp4( m_vBlendPosition, vCameraPos, m_fBlendTime );

	TQuaternion quatFrom;
	quatFrom.SetVectorDirection( m_vBlendDirection.AsVector3(), TVector3::VEC_POSZ );

	TQuaternion quatTo;
	quatTo.SetVectorDirection( vCameraDir.AsVector3(), TVector3::VEC_POSZ );

	TQuaternion quatRotation;
	TQuaternion::Slerp( quatRotation, quatFrom, quatTo, m_fBlendTime );
	RotateVector( vCameraDir, quatRotation, TVector4::VEC_POSZ );

	pCamera->GetMatrix().GetTranslation() = vCameraPos;
	pCamera->LookAtDirection( vCameraDir );

	m_vPosition  = vCameraPos;
	m_vDirection = vCameraDir;
}

// $Barnyard: FUNCTION 0045d220
TBOOL ABarnStairCamera::Update( TFLOAT a_fDeltaTime, AUnit* a_pUnit )
{
	if ( !m_bActive )
		return TTRUE;

	TVector4 vTargetPos = *a_pUnit->GetSteer()->GetPosition();
	m_pCurrentCurve     = ( vTargetPos.x >= 15.0f ) ? &m_oCurve1 : &m_oCurve2;

	TFLOAT fProgress = m_pCurrentCurve->FindProgress( vTargetPos, 81.0f, 0.0f );

	if ( fProgress < 0.0f ||
	     ( vTargetPos.y > 1.0f && vTargetPos.x >= -1.0f && vTargetPos.x <= 30.0f ) ||
	     ( vTargetPos.y <= 1.0f && vTargetPos.x >= -1.0f && vTargetPos.x <= 31.0f ) )
	{
		// The target isn't in the region of the path
		if ( m_bOnPath )
		{
			// Give the control back to the GTA style camera
			TVector4 vLookAt( m_vPosition.x + m_vDirection.x, m_vPosition.y + m_vDirection.y, m_vPosition.z + m_vDirection.z, m_vPosition.w );
			GetGTAStyleCameraHelper()->SetPosition( m_vPosition, vLookAt, TFALSE );

			m_vBlendPosition  = m_vPosition;
			m_vBlendDirection = m_vDirection;
			m_bOnPath         = TFALSE;
			m_bBlending       = TFALSE;
			m_fProgressOffset = 0.0f;
			m_bReturning      = TTRUE;
			m_fBlendTime      = 0.0f;

			ACamera* pCamera                      = ACameraManager::GetSingleton()->GetCurrentCamera();
			pCamera->GetMatrix().GetTranslation() = m_vPosition;
			pCamera->LookAtDirection( m_vDirection );
			return TTRUE;
		}

		if ( m_bReturning )
			UpdateReturn( a_fDeltaTime, a_pUnit );

		return TTRUE;
	}

	TFLOAT fTargetProgress = fProgress + m_fProgressOffset;

	if ( !m_bOnPath )
	{
		m_fProgress       = fTargetProgress;
		m_fProgressOffset = 0.0f;

		if ( m_bReturning )
			UpdateReturn( a_fDeltaTime, a_pUnit );

		if ( !m_bBlending &&
		     ( ( vTargetPos.y <= 1.0f && vTargetPos.y >= -6.5f && ( vTargetPos.z < 32.0f || vTargetPos.z > 37.5f ) ) ||
		       ( vTargetPos.y > 1.0f && ( vTargetPos.x < 0.0f || vTargetPos.x > 31.0f ) ) ||
		       ( vTargetPos.y < -6.5f && ( vTargetPos.x < -1.0f || vTargetPos.x > 31.0f ) ) ) )
		{
			// Start moving the camera along the path
			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

			TVector4 vCameraPos;
			vCameraPos.Set( pCamera->GetMatrix().GetTranslation() );

			if ( m_bReturning )
				vCameraPos = m_vPosition;

			vCameraPos.y += 2.0f;
			TFLOAT fCameraProgress = m_pCurrentCurve->FindProgress( vCameraPos, 64.0f, 1.0f );

			if ( fCameraProgress < 0.0f )
				return TTRUE;

			m_fProgressOffset = ( fCameraProgress <= fTargetProgress ) ? -0.08f : 0.08f;
			m_bOnPath         = TTRUE;
			m_bBlending       = TTRUE;
			m_fProgress       = m_fProgressOffset + m_fProgress;
			m_fBlendTime      = 0.0f;

			if ( m_bReturning )
			{
				m_vBlendPosition  = m_vPosition;
				m_bReturning      = TFALSE;
				m_vBlendDirection = m_vDirection;
			}
			else
			{
				m_vBlendPosition  = pCamera->GetMatrix().GetTranslation();
				m_vBlendDirection = pCamera->GetOrientation();
				m_vPosition       = m_vBlendPosition;
				m_vDirection      = m_vBlendDirection;
			}

			SetPlayerCanSprint( a_pUnit, TFALSE );
		}

		return TTRUE;
	}

	// Move the progress towards the target's progress
	TFLOAT fStep = TMath::Abs( fTargetProgress - m_fProgress ) * 9.0f;

	if ( fStep > 0.13f )
		fStep = 0.13f;

	fStep *= a_fDeltaTime;

	if ( m_fProgress < fTargetProgress )
	{
		m_fProgress += fStep;

		if ( m_fProgress > fTargetProgress )
			m_fProgress = fTargetProgress;
	}
	else if ( m_fProgress > fTargetProgress )
	{
		m_fProgress -= fStep;

		if ( m_fProgress < fTargetProgress )
			m_fProgress = fTargetProgress;
	}

	TVector4 vPosition;
	m_pCurrentCurve->GetPoint( vPosition, m_fProgress );

	TVector4 vDirection( vTargetPos.x - vPosition.x, ( vTargetPos.y - 1.5f ) - vPosition.y, vTargetPos.z - vPosition.z, vTargetPos.w );
	vDirection.Normalise();

	if ( m_bBlending )
	{
		m_fBlendTime += a_fDeltaTime;

		if ( m_fBlendTime > 1.0f )
		{
			m_bBlending  = TFALSE;
			m_fBlendTime = 1.0f;
		}

		vPosition.Lerp4( m_vBlendPosition, vPosition, m_fBlendTime );

		TQuaternion quatFrom;
		quatFrom.SetVectorDirection( m_vBlendDirection.AsVector3(), TVector3::VEC_POSZ );

		TQuaternion quatTo;
		quatTo.SetVectorDirection( vDirection.AsVector3(), TVector3::VEC_POSZ );

		TQuaternion quatRotation;
		TQuaternion::Slerp( quatRotation, quatFrom, quatTo, m_fBlendTime );
		RotateVector( vDirection, quatRotation, TVector4::VEC_POSZ );
	}

	ACamera* pCamera                      = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->GetMatrix().GetTranslation() = vPosition;
	pCamera->LookAtDirection( vDirection );

	m_vPosition  = vPosition;
	m_vDirection = vDirection;

	return TTRUE;
}
