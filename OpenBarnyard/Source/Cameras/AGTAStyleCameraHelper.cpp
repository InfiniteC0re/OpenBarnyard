#include "pch.h"
#include "AGTAStyleCameraHelper.h"
#include "ABarnStairCamera.h"
#include "ACameraManager.h"
#include "AOptions.h"
#include "Input/AInputMap.h"
#include "Helpers/AGameLoader.h"
#include "Render/ARenderer.h"
#include "Units/ACharacterUnit.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitType.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Physics/ACollisionObject.h"
#include "Physics/ACollisionObjectSimpleDummy.h"
#include "Physics/ACollisionObjectMesh.h"
#include "GUI/AGUI2.h"
#include "Physics/ACollisionManager.h"
#include "Terrain/ATerrainInterface.h"

#include <Input/TInputDeviceMouse.h>
#include <Toshi/TSceneObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( generic_chick );

TDEFINE_CLASS_NORUNTIME( AGTAStyleCameraHelper );

static constexpr TFLOAT DEG2RAD = 0.017453292f;

// Set on the collision object of the target while the camera checks for collisions
static constexpr TINT COLLISIONOBJECT_FLAG_CAMERA_IGNORE = ACollisionObject::TYPE_IGNORED;

// Flags passed to the camera collision checks
static constexpr TINT CAMERA_COLLISION_FLAGS = ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_UNIT;

#ifdef BARNYARD_LATEST
// Flags passed to the camera collision checks once the camera stayed out of the objects for a while (fewer types ignored)
static constexpr TINT CAMERA_COLLISION_FLAGS_CLEAR = ACollisionObject::TYPE_IGNORED | ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_BUSH | ACollisionObject::TYPE_PLAYER | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_FOLIAGE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE;
#endif

// Makes the camera look down when it hits a wall
static TBOOL s_bPitchUpOnWalls = TTRUE; // 0x0077380c

// NOTE: the original compares with 006c92f0 that also compares w (TVector4::IsEqual doesn't)
static TBOOL IsEqual4( const TVector4& a_rcVec1, const TVector4& a_rcVec2 )
{
	return a_rcVec1.x == a_rcVec2.x && a_rcVec1.y == a_rcVec2.y && a_rcVec1.z == a_rcVec2.z && a_rcVec1.w == a_rcVec2.w;
}

// NOTE: the original computes lengths with fsqrt, which (unlike TMath::Sqrt) accepts zero
static TFLOAT Length( const TVector4& a_rcVector )
{
	return sqrtf( a_rcVector.MagnitudeSq() );
}

// Sweeps a sphere of a_fRadius from a_rcStart along a_rcDirection against the collision objects and the terrain.
// a_pOutFraction is the distance to sweep and receives the free part of the sweep, a_pOutNormal the normal of the hit.
// a_fLength is used as the height of the swept sphere (a capsule if not zero).
// NOTE: also used by ABikePhysicsCameraHelper
// $Barnyard: FUNCTION 006186a0
TBOOL SweepSphere( const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength, TFLOAT* a_pOutFraction, TVector4* a_pOutNormal, TINT a_iFlags, TUINT a_uiCollisionMask )
{
	ACollisionObjectSimpleDummy oSphere;
	oSphere.Create( a_rcStart, a_fRadius, a_fLength, 0.0f, 0, TNULL );

	TFLOAT fStepHeight = 0.0f;
	TUINT  uiHitType   = 0;

	ACollisionObject* pHitObject = ACollisionManager::GetSingleton()->CollideSweptSphereObjects( &oSphere, *a_pOutNormal, *a_pOutFraction, a_rcDirection, a_rcStart, a_iFlags, 0.0f, &fStepHeight, 0 );

	if ( pHitObject )
		uiHitType = pHitObject->GetType();

	TFLOAT fPenetration;
	TBOOL  bEdgeHit;

	// NOTE: retail passes a byte of its stack as the unused argument of the terrain test
	if ( ( a_iFlags & ACollisionObject::TYPE_TERRAIN ) == 0 && *a_pOutFraction > 0.0f &&
	     ATerrainInterface::GetSingleton()->CollideSweptSphere( a_rcStart, a_fRadius, a_rcDirection, *a_pOutFraction, *a_pOutNormal, 0.0f, fStepHeight, fPenetration, TINT( TUINTPTR( &bEdgeHit ) ), a_uiCollisionMask ) )
	{
		uiHitType |= 1;
	}

	return uiHitType != 0;
}

// $Barnyard: FUNCTION 00464490
AGTAStyleCameraHelper::AGTAStyleCameraHelper()
    : m_uiCollisionMask( 0x90000 )
    , m_vRotateInput( TVector2::VEC_ZERO )
    , m_fYawSpeed( 0.0f )
    , m_bRotating( TFALSE )
    , m_bPitchInput( TFALSE )
    , m_vLastTargetPos( TVector4::VEC_ZERO )
    , m_vPosition( TVector4::VEC_ZERO )
    , m_vPivot( TVector4::VEC_POSZ )
    , m_vLookAt( TVector4::VEC_POSZ )
    , m_vTargetPosOverride( TVector4::VEC_ZERO )
    , m_vTargetDirOverride( TVector4::VEC_ZERO )
    , m_fDistance( 5.0f )
    , m_fTargetDistance( 5.0f )
    , m_fDefaultDistance( 5.0f )
    , m_fDistanceSpeed( 3.0f )
    , m_fHeightOffset( 1.7f )
    , m_fPitch( 11.0f )
    , m_fYaw( 0.0f )
    , m_fTargetPitch( 11.0f )
    , m_fDefaultPitch( 11.0f )
    , m_fDefaultMaxPitch( 22.0f )
    , m_fMaxPitch( 22.0f )
    , m_fMinPitch( -8.0f )
    , m_fTargetOpacity( 1.0f )
#ifdef BARNYARD_LATEST
    , m_fCameraClearTime( 0.0f )
#endif
    , m_bUsePath( TFALSE )
    , m_bPathPaused( TFALSE )
    , m_pPath( TNULL )
    , m_fCollisionRadius( 0.43f )
    , m_fSmallCollisionRadius( 0.2f )
    , m_bForceUpdate( TFALSE )
    , m_fIdleTime( 100.0f )
#ifdef BARNYARD_LATEST
    , m_fHoldHeightTime( 0.0f ) // NOTE: not initialised by the original ctor, OnCreate sets it
#endif
    , m_bYawLimits( TFALSE )
    , m_fYawMin( 0.0f )
    , m_fYawMax( 0.0f )
    , m_bCharacterInTheWay( TFALSE )
    , m_bMouseInput( TTRUE )
{
	m_pPath = new ABarnStairCamera();
}

// $Barnyard: FUNCTION 00462fc0
// $Barnyard: FUNCTION 004646d0
AGTAStyleCameraHelper::~AGTAStyleCameraHelper()
{
	if ( m_pPath )
		delete m_pPath;

	m_pPath = TNULL;
}

// $Barnyard: FUNCTION 00463350
// $BarnyardWiiJP: FUNCTION 8017ec60
TBOOL AGTAStyleCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_pTarget && m_pTarget->GetUnit() )
	{
		m_pCamera->SetFOV( GetFOV() );

		const TVector4 vOldCameraPos = m_pCamera->GetMatrix().GetTranslation();

		AVisibleUnit* pUnit  = TSTATICCAST( AVisibleUnit, m_pTarget->GetUnit() );
		ASteer*       pSteer = pUnit->GetSteer();

		TVector4 vTargetPos;

		if ( IsEqual4( m_vTargetPosOverride, TVector4::VEC_ZERO ) )
			vTargetPos = *pSteer->GetPosition();
		else
			vTargetPos = m_vTargetPosOverride;

		// NOTE: the direction isn't used
		TVector4 vTargetDir;

		if ( IsEqual4( m_vTargetDirOverride, TVector4::VEC_ZERO ) )
			pSteer->CopyDirection( vTargetDir );
		else
			vTargetDir = m_vTargetDirOverride;

		// Update the yaw speed
		TBOOL bYawStopping = TFALSE;

		if ( TMath::Abs( m_vRotateInput.x ) <= 0.0001f )
		{
			if ( !MoveTowards( m_fYawSpeed, 0.0f, m_fYawSpeed, 8.0f, 8.0f, a_fDeltaTime ) )
			{
				m_bRotating  = TTRUE;
				bYawStopping = TTRUE;
			}
		}
		else
		{
			// Stop at once when the direction changes
			if ( ( m_fYawSpeed > 0.0f && m_vRotateInput.x < -0.01f ) || ( m_fYawSpeed < 0.0f && m_vRotateInput.x > 0.01f ) )
				m_fYawSpeed = 0.0f;

			MoveTowards( m_fYawSpeed, m_vRotateInput.x, m_fYawSpeed, 10.0f, 10.0f, a_fDeltaTime );
		}

		// Did the target move since the last update?
		TVector4 vTargetMove;
		vTargetMove.x = vTargetPos.x - m_vLastTargetPos.x;
		vTargetMove.y = vTargetPos.y - m_vLastTargetPos.y;
		vTargetMove.z = vTargetPos.z - m_vLastTargetPos.z;
		vTargetMove.w = vTargetPos.w;

		TFLOAT fTargetMoved = Length( vTargetPos - m_vLastTargetPos );
		TBOOL  bTargetMoved = TFALSE;

		if ( fTargetMoved > 0.001f )
		{
			vTargetMove.Divide( fTargetMoved );
			bTargetMoved = TTRUE;
		}

#ifdef BARNYARD_LATEST
		// Wii Style camera option: the latest version checks more collision types once the camera stayed out of
		// everything for 1.5 seconds, and the object collision models ignore the camera groups during the camera checks
		const TBOOL bWiiCamera       = AOptions::IsWiiCameraEnabled();
		TBOOL       bCameraWasInside = TTRUE;
		ACamera*    pCurrentCamera   = bWiiCamera ? ACameraManager::GetSingleton()->GetCurrentCamera() : TNULL;
		ACollisionObjectMesh::SetCameraCollision( bWiiCamera );

		if ( !bWiiCamera )
			m_fCameraClearTime = 0.0f;

		if ( pCurrentCamera )
		{
			TVector4 vCurrentCameraPos = pCurrentCamera->GetMatrix().GetTranslation();

			// Where the distance update below will move the camera this frame
			TFLOAT fPredictSpeedDown = 11.0f;

			if ( TMath::Abs( m_fTargetDistance - m_fDefaultDistance ) > 0.001f )
			{
				TFLOAT fRatio = ( m_fDefaultDistance - m_fDistance ) / m_fDefaultDistance;
				TMath::Clip( fRatio, 0.0f, 1.0f );

				fPredictSpeedDown += fRatio * fRatio * 14.0f;
			}

			TFLOAT fPredictedDistance = m_fDistance;
			MoveTowards( fPredictedDistance, m_fTargetDistance, m_fDistance, fPredictSpeedDown, m_fDistanceSpeed, a_fDeltaTime );

			TVector4 vPredicted;
			vPredicted.x = m_vPosition.x - m_vPivot.x;
			vPredicted.y = m_vPosition.y - m_vPivot.y;
			vPredicted.z = m_vPosition.z - m_vPivot.z;
			vPredicted.w = m_vPosition.w;
			vPredicted.Normalise();
			vPredicted.Multiply( fPredictedDistance );
			vPredicted.x += m_vPivot.x;
			vPredicted.y += m_vPivot.y;
			vPredicted.z += m_vPivot.z;

			TFLOAT fSphereRadius = 0.5f;

			ACollisionObjectSimple oSphere;
			oSphere.UpdateHash( fSphereRadius, vCurrentCameraPos.AsVector3() );

			TVector4 vHitNormal;

			if ( ACollisionManager::CollideSphereWithWorld( vPredicted, &oSphere, fSphereRadius, vHitNormal, ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_TERRAIN, m_uiCollisionMask ) )
			{
				m_fCameraClearTime = 0.0f;
			}
			else
			{
				m_fCameraClearTime += a_fDeltaTime;

				if ( m_fCameraClearTime > 1.5f )
					bCameraWasInside = TFALSE;
			}
		}

		const TINT iCollisionFlags = bCameraWasInside ? CAMERA_COLLISION_FLAGS : CAMERA_COLLISION_FLAGS_CLEAR;
#else
		const TINT iCollisionFlags = CAMERA_COLLISION_FLAGS;
#endif

		TBOOL  bReturnPitch     = TTRUE;
		TBOOL  bPitchAdjusted   = TFALSE;
		TFLOAT fPitchSpeedScale = 1.0f;

		ACollisionObject* pCollisionObject = pUnit->GetCollisionObject();

		if ( m_bForceUpdate || ( ( bTargetMoved || m_bRotating ) && m_fMaxPitch > m_fDefaultMaxPitch ) )
		{
			// Check if the camera can go back to the default pitch
			TVector4 vStart = vTargetPos;
			vStart.y -= m_fHeightOffset;

			TVector4 vDirection;
			vDirection.x = m_vPosition.x - vStart.x;
			vDirection.y = m_vPosition.y - vStart.y;
			vDirection.z = m_vPosition.z - vStart.z;
			vDirection.w = m_vPosition.w;

			vDirection.NormaliseXZ();
			vDirection.AsVector3().RotateY( m_fYaw * DEG2RAD );
			vDirection.y = -TMath::ATan( m_fDefaultPitch * DEG2RAD );
			vDirection.Normalise();

			TFLOAT   fFraction = 1.0f;
			TVector4 vHitNormal;

			pCollisionObject->SetType( pCollisionObject->GetType() | COLLISIONOBJECT_FLAG_CAMERA_IGNORE );
			TBOOL bHit = SweepSphere( vStart, vDirection, m_fCollisionRadius, fFraction, &fFraction, &vHitNormal, iCollisionFlags, m_uiCollisionMask );
			pCollisionObject->SetType( pCollisionObject->GetType() & ~COLLISIONOBJECT_FLAG_CAMERA_IGNORE );

			if ( !bHit )
			{
				m_fTargetPitch = m_fDefaultPitch;
				m_fMaxPitch -= a_fDeltaTime * 40.0f;

				if ( m_fMaxPitch < m_fDefaultMaxPitch )
					m_fMaxPitch = m_fDefaultMaxPitch;

				bPitchAdjusted   = TTRUE;
				fPitchSpeedScale = 2.0f;
			}
			else
			{
				bReturnPitch = TFALSE;
			}
		}

		// Check if any character stands between the camera and the target
		m_bCharacterInTheWay = TFALSE;

		if ( AGameLoader::IsMainGameLoaded() || AGameLoader::IsBarnExploreMiniGame() )
		{
			TFLOAT fQueryRadius = m_fTargetDistance * 0.7f;
			TFLOAT fLength      = m_fTargetDistance + 1.0f;

			TVector4 vDirection;
			vDirection.x = m_vPosition.x - m_vPivot.x;
			vDirection.y = 0.0f;
			vDirection.z = m_vPosition.z - m_vPivot.z;
			vDirection.w = m_vPosition.w;
			vDirection.Normalise();

			TVector4 vMiddle = vDirection;
			vMiddle.Multiply( m_fTargetDistance * 0.5f );
			vMiddle.x += m_vPivot.x;
			vMiddle.y += m_vPivot.y;
			vMiddle.z += m_vPivot.z;

			TVector4 vFrom = m_vPivot;

			TVector4 vTo = vDirection;
			vTo.Multiply( fLength );
			vTo.x += m_vPivot.x;
			vTo.y += m_vPivot.y;
			vTo.z += m_vPivot.z;

			TINT                    iNumUnits;
			AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( vMiddle, fQueryRadius, &iNumUnits, TFALSE, 420, TFALSE, TNULL );

			if ( IsCharacterInTheWay( pResults, iNumUnits, vFrom, vTo, vOldCameraPos, pUnit, 1.5f ) && m_fMaxPitch <= m_fDefaultMaxPitch )
			{
				bPitchAdjusted       = TTRUE;
				m_vRotateInput.y     = 0.0f;
				bReturnPitch         = TTRUE;
				fPitchSpeedScale     = 2.0f;
				m_bCharacterInTheWay = TTRUE;
			}
		}

		if ( bTargetMoved || m_bRotating || bPitchAdjusted )
		{
			// Update the pitch
			if ( bTargetMoved || m_bPitchInput || bPitchAdjusted )
			{
				m_fPitch += a_fDeltaTime * m_vRotateInput.y * 90.0f;

				if ( bReturnPitch && ( !m_bPitchInput || bPitchAdjusted ) )
					MoveTowards( m_fPitch, m_fTargetPitch, m_fPitch, fPitchSpeedScale + fPitchSpeedScale, 2.0f, a_fDeltaTime );
			}

			TMath::Clip( m_fPitch, m_fMinPitch, m_fMaxPitch );

			// Update the yaw
			m_fYaw += a_fDeltaTime * m_fYawSpeed * 120.0f;

			m_vPivot = vTargetPos;
			m_vPivot.y -= m_fHeightOffset;

			TVector4 vDirection;
			vDirection.x = m_vPosition.x - m_vPivot.x;
			vDirection.y = m_vPosition.y - m_vPivot.y;
			vDirection.z = m_vPosition.z - m_vPivot.z;
			vDirection.w = m_vPosition.w;

			vDirection.NormaliseXZ();
			vDirection.AsVector3().RotateY( m_fYaw * DEG2RAD );

			if ( m_bYawLimits )
			{
				TFLOAT fHalfRange = ( m_fYawMax - m_fYawMin ) * 0.5f;
				TFLOAT fCentre    = fHalfRange + m_fYawMin;

				TVector4 vBackward = vDirection;
				vBackward.Multiply( -1.0f );

				TVector4 vCentre = TVector4::VEC_POSX;
				vCentre.AsVector3().RotateY( -fCentre );

				TFLOAT fDot = TVector4::DotProduct3( vBackward, vCentre );

				if ( TMath::Cos( fHalfRange ) > fDot )
				{
					// Out of the limits, snap to the closest one
					TFLOAT fLimit = ( vBackward.z * vCentre.x - vCentre.z * vBackward.x >= 0.0f ) ? m_fYawMax : m_fYawMin;

					vDirection = TVector4::VEC_POSX;
					vDirection.AsVector3().RotateY( -fLimit );
					vDirection.Multiply( -1.0f );
				}
			}

			TVector4 vYawDirection = vDirection;

			vDirection.y = -TMath::ATan( m_fPitch * DEG2RAD );
			vDirection.Normalise();

			m_vPosition = vDirection;
			m_vPosition.Multiply( m_fDefaultDistance );
			m_vPosition.x += m_vPivot.x;
			m_vPosition.y += m_vPivot.y;
			m_vPosition.z += m_vPivot.z;

			// Don't let the camera go through walls
			TFLOAT   fFraction = m_fDefaultDistance;
			TVector4 vHitNormal;

			pCollisionObject->SetType( pCollisionObject->GetType() | COLLISIONOBJECT_FLAG_CAMERA_IGNORE );

			if ( SweepSphere( m_vPivot, vDirection, m_fCollisionRadius, fFraction, &fFraction, &vHitNormal, iCollisionFlags, m_uiCollisionMask ) )
			{
				if ( fFraction < 1.0f )
				{
					fFraction = m_fDefaultDistance;
					SweepSphere( m_vPivot, vDirection, m_fSmallCollisionRadius, fFraction, &fFraction, &vHitNormal, iCollisionFlags, m_uiCollisionMask );

					if ( fFraction > 1.0f )
						fFraction = 1.0f;

					if ( s_bPitchUpOnWalls && TMath::Abs( vHitNormal.y ) < 0.45f )
					{
						m_fTargetPitch = 80.0f;
						MoveTowards( m_fPitch, 80.0f, m_fPitch, 2.0f, 2.0f, a_fDeltaTime );
						m_fMaxPitch = 80.0f;
					}
				}

				if ( fFraction < 0.01f )
					fFraction = 0.01f;

				m_fTargetDistance = fFraction;
			}
			else if ( m_fMaxPitch > m_fDefaultMaxPitch )
			{
				if ( !bYawStopping || bTargetMoved )
				{
					// Go back to the default pitch if nothing is in the way
					TVector4 vDefaultDirection = vYawDirection;
					vDefaultDirection.y        = -TMath::ATan( m_fDefaultPitch * DEG2RAD );
					vDefaultDirection.Normalise();

					fFraction = m_fDefaultDistance;

					// NOTE: retail writes the hit normal over vYawDirection
					if ( !SweepSphere( m_vPivot, vDefaultDirection, m_fCollisionRadius, fFraction, &fFraction, &vYawDirection, iCollisionFlags, m_uiCollisionMask ) )
					{
						m_fTargetPitch = m_fDefaultPitch;
						m_fMaxPitch -= a_fDeltaTime * 40.0f;

						if ( m_fMaxPitch < m_fDefaultMaxPitch )
							m_fMaxPitch = m_fDefaultMaxPitch;

						m_fTargetDistance = m_fDefaultDistance;
					}
				}
			}
			else
			{
				m_fTargetDistance = m_fDefaultDistance;
			}

			pCollisionObject->SetType( pCollisionObject->GetType() & ~COLLISIONOBJECT_FLAG_CAMERA_IGNORE );
		}

		// Update the distance
		if ( m_fTargetPitch > m_fDefaultPitch )
		{
			m_fTargetDistance = ( 1.0f < m_fTargetDistance ) ? 1.0f : m_fTargetDistance;
			m_fDistanceSpeed  = 0.5f;
		}
		else
		{
			m_fDistanceSpeed += a_fDeltaTime + a_fDeltaTime;

			if ( m_fDistanceSpeed > 3.0f )
				m_fDistanceSpeed = 3.0f;
		}

		TFLOAT fDistanceSpeedDown = 11.0f;

		if ( TMath::Abs( m_fTargetDistance - m_fDefaultDistance ) > 0.001f )
		{
			TFLOAT fRatio = ( m_fDefaultDistance - m_fDistance ) / m_fDefaultDistance;
			TMath::Clip( fRatio, 0.0f, 1.0f );

			fDistanceSpeedDown = fRatio * fRatio * 14.0f + 11.0f;
		}

		MoveTowards( m_fDistance, m_fTargetDistance, m_fDistance, fDistanceSpeedDown, m_fDistanceSpeed, a_fDeltaTime );

		// Place the camera
		TVector4 vDirection;
		vDirection.x = m_vPosition.x - m_vPivot.x;
		vDirection.y = m_vPosition.y - m_vPivot.y;
		vDirection.z = m_vPosition.z - m_vPivot.z;
		vDirection.w = m_vPosition.w;
		vDirection.Normalise();
		vDirection.Multiply( m_fDistance );

		TVector4 vCameraPos( vDirection.x + m_vPivot.x, 0.0f, vDirection.z + m_vPivot.z, 1.0f );
		TVector4 vLookAt( m_vPivot.x, 0.0f, m_vPivot.z, 1.0f );

#ifdef BARNYARD_LATEST
		// Follow Jumps camera option: when it's on (PC retail), the height isn't held while the player jumps
		if ( m_fHoldHeightTime > 0.0f && !AOptions::IsJumpHeightHoldEnabled() )
			m_fHoldHeightTime = -1.0f;

		if ( m_fHoldHeightTime > 0.0f )
		{
			// Keep the height while a point above the target stays on screen
			TVector4 vAboveTarget( vTargetPos.x + 0.0f, vTargetPos.y + -2.0f, vTargetPos.z + 0.0f, vTargetPos.w );

			TVector2 vScreenPos;
			ARenderer::GetSingleton()->ProjectToScreen( GetCamera(), vAboveTarget, vScreenPos );

			TFLOAT fScreenWidth, fScreenHeight;
			AGUI2::GetSingleton()->GetDimensions( fScreenWidth, fScreenHeight );

			// NOTE: the latest version compares with 99.75 screen heights, so only the top limit matters
			if ( vScreenPos.y < fScreenHeight * 0.1f || vScreenPos.y > fScreenHeight * ( 100.0f - 0.25f ) )
			{
				m_fHoldHeightTime = -1.0f;
			}
			else
			{
				vCameraPos.y = vOldCameraPos.y;
				vLookAt.y    = m_vLookAt.y;
				m_fHoldHeightTime -= a_fDeltaTime;
			}
		}

		if ( m_fHoldHeightTime <= 0.0f )
		{
			// Wii Style camera option: the latest version moves the height slower
			const TFLOAT fHeightSpeed = bWiiCamera ? 5.5f : 10.0f;

			MoveTowards( vCameraPos.y, vDirection.y + m_vPivot.y, vOldCameraPos.y, fHeightSpeed, fHeightSpeed, a_fDeltaTime );
			MoveTowards( vLookAt.y, m_vPivot.y, m_vLookAt.y, fHeightSpeed, fHeightSpeed, a_fDeltaTime );
		}
#else
		MoveTowards( vCameraPos.y, vDirection.y + m_vPivot.y, vOldCameraPos.y, 10.0f, 10.0f, a_fDeltaTime );
		MoveTowards( vLookAt.y, m_vPivot.y, m_vLookAt.y, 10.0f, 10.0f, a_fDeltaTime );
#endif

		m_pCamera->LookAt( vCameraPos, vLookAt );

		if ( m_bUsePath && !m_bPathPaused )
			m_pPath->Update( a_fDeltaTime, pUnit );

		if ( m_bUsePath && !m_bPathPaused && m_pPath->IsOnPath() )
		{
			m_fTargetOpacity += a_fDeltaTime * 5.0f;

			if ( m_fTargetOpacity > 1.0f )
				m_fTargetOpacity = 1.0f;
		}
		else
		{
			// Fade the target out when the camera is too close to it
			TVector4 vToTarget;
			vToTarget.x = vTargetPos.x - vCameraPos.x;
			vToTarget.y = ( vTargetPos.y - m_fHeightOffset ) - vCameraPos.y;
			vToTarget.z = vTargetPos.z - vCameraPos.z;
			vToTarget.w = vTargetPos.w;

			TFLOAT fDistanceSq = vToTarget.MagnitudeSq();

			if ( fDistanceSq < 0.09f )
			{
				m_fTargetOpacity -= a_fDeltaTime * 5.0f;

				if ( m_fTargetOpacity < 0.0f )
					m_fTargetOpacity = 0.0f;
			}
			else
			{
				m_fTargetOpacity += a_fDeltaTime * 5.0f;

				if ( m_fTargetOpacity > 1.0f )
					m_fTargetOpacity = 1.0f;
			}

			// Move the near plane closer when the camera is close to the target
			TFLOAT fNear = sqrtf( fDistanceSq ) * 1.1111112f;

			if ( fNear >= 1.0f )
			{
				ARenderer::GetSingleton()->SetNear( 0.5f );
			}
			else
			{
				fNear *= 0.1f;

				if ( fNear < 0.01f )
					fNear = 0.01f;

				ARenderer::GetSingleton()->SetNear( fNear );
			}
		}

		if ( m_fTargetOpacity >= 1.0f )
		{
			pUnit->GetModelInstance()->DisableDrawWithDefaultMaterials(); // NOTE: retail passes an unused argument (TFALSE)
			pUnit->SetOpacity( 1.0f );

			// NOTE: inlined AModelInstance method 00610700
			pUnit->GetModelInstance()->ResetOpacity();
		}
		else
		{
			pUnit->GetModelInstance()->DisableDrawWithDefaultMaterials(); // NOTE: retail passes an unused argument (TTRUE)
			pUnit->SetOpacity( m_fTargetOpacity );
		}

		m_vLookAt        = vLookAt;
		m_fYaw           = 0.0f;
		m_bRotating      = TFALSE;
		m_bPitchInput    = TFALSE;
		m_vRotateInput   = TVector2::VEC_ZERO;
		m_vLastTargetPos = vTargetPos;
		m_bForceUpdate   = TFALSE;
	}

#ifdef BARNYARD_LATEST
	ACollisionObjectMesh::SetCameraCollision( TFALSE );
#endif

	m_fIdleTime += a_fDeltaTime;
	return TTRUE;
}

// $Barnyard: FUNCTION 004630e0
// $BarnyardWiiJP: FUNCTION 8017e930
TBOOL AGTAStyleCameraHelper::ProcessInputCommand( void* a_pCommand, void* a_pEvent )
{
	AInputCommand                      eCommand = AInputCommand( TUINTPTR( a_pCommand ) );
	const TInputInterface::InputEvent* pEvent   = TSTATICCAST( TInputInterface::InputEvent, a_pEvent );

	switch ( eCommand )
	{
		case AInputCommand_ElevateCameraCCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_fIdleTime = 0.0f;
			m_vRotateInput.y -= 1.0f;
			return TTRUE;
		case AInputCommand_ElevateCameraCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_fIdleTime = 0.0f;
			m_vRotateInput.y += 1.0f;
			return TTRUE;
		case AInputCommand_RotateCameraCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_fIdleTime = 0.0f;
			m_vRotateInput.x += 1.0f;
			return TTRUE;
		case AInputCommand_RotateCamera:
		{
			if ( m_bMouseInput != TTRUE )
				return TFALSE;

#ifdef TOSHI_SKU_WII
			{
				// The Wii only rotates with the stick of the Nunchuk, and not while the player aims with the pointer
				AUnit*                   pUnit    = m_pTarget ? m_pTarget->GetUnit() : TNULL;
				APlayerControlComponent* pControl = pUnit ? TSTATICCAST( APlayerControlComponent, pUnit->FindComponentExact( &TGetClass( APlayerControlComponent ) ) ) : TNULL;

				if ( !pEvent->GetSource()->IsA( &TGetClass( TInputWPADDeviceController ) ) || ( pControl && pControl->IsPointerAiming() ) )
					return TFALSE;

				if ( TSTATICCAST( TInputWPADDeviceController, pEvent->GetSource() )->GetNunchukStatus() <= 0 )
				{
					m_bPitchInput  = TFALSE;
					m_vRotateInput = TVector2::VEC_ZERO;
					m_bRotating    = TFALSE;
					return TTRUE;
				}

				// Ignore the inner half of the range and rescale the rest to 0-1
				auto fnDeadZone = []( TFLOAT a_fValue ) {
					if ( a_fValue > 0.5f ) return ( a_fValue - 0.5f ) / 0.5f;
					if ( a_fValue < -0.5f ) return ( a_fValue + 0.5f ) / 0.5f;
					return 0.0f;
				};

				m_vRotateInput.x = fnDeadZone( pEvent->GetMagnitudeFloat( 0 ) );
				m_vRotateInput.y = fnDeadZone( AOptions::GetSingleton()->IsMouseInverted() ? pEvent->GetMagnitudeFloat( 1 ) : -pEvent->GetMagnitudeFloat( 1 ) );
				m_vRotateInput.x *= ( AOptions::GetSingleton()->GetCameraSpeed() + 1 ) / 6.0f;

				m_bPitchInput = m_bPitchInput || m_vRotateInput.y != 0.0f;
				m_bRotating   = m_bRotating || m_bPitchInput || m_vRotateInput.x != 0.0f;

				if ( m_bRotating )
					m_fIdleTime = 0.0f;

				return TTRUE;
			}
#endif

			AOptions* pOptions = AOptions::GetSingleton();
			TBOOL     bRotating;

			if ( pEvent->GetSource()->IsA( &TGetClass( TInputDeviceMouse ) ) )
			{
				if ( pEvent->GetEventType() != TInputInterface::EVENT_TYPE_MOVED )
					return TFALSE;

				TFLOAT fX = pEvent->GetMagnitudeFloat( 0 );
				TFLOAT fY = pEvent->GetMagnitudeFloat( 1 );

				m_fYaw = fX * pOptions->GetSensitivity() * 0.15f + m_fYaw;

				if ( !m_bCharacterInTheWay )
				{
					TFLOAT fPitch = fY * pOptions->GetSensitivity() * 0.2f;
					m_fPitch      = pOptions->IsMouseInverted() ? m_fPitch - fPitch : fPitch + m_fPitch;
				}

				m_bPitchInput = m_bPitchInput || fY != 0.0f;
				bRotating     = m_bRotating || m_bPitchInput || fX != 0.0f;
			}
			else
			{
				m_vRotateInput.x = pEvent->GetMagnitudeFloat( 0 );
				m_vRotateInput.y = pOptions->IsMouseInverted() ? -pEvent->GetMagnitudeFloat( 1 ) : pEvent->GetMagnitudeFloat( 1 );

#ifdef BARNYARD_LATEST
				// Wii Style camera option: the yaw is scaled by the camera speed option: ( speed + 1 ) / 6
				// NOTE: the Wii only reads the stick of the Nunchuk here, it also ignores the inner half of its range
				// (|x| <= 0.5 is 0, the rest is rescaled to 0-1) and skips it while the player aims with the pointer
				if ( pOptions->IsWiiStyleCamera() )
					m_vRotateInput.x *= ( pOptions->GetCameraSpeed() + 1 ) / 6.0f;
#endif

				m_bPitchInput = m_bPitchInput || m_vRotateInput.y != 0.0f;
				bRotating     = m_bRotating || m_bPitchInput || m_vRotateInput.x != 0.0f;
			}

			m_bRotating = bRotating;

			if ( bRotating )
				m_fIdleTime = 0.0f;

			return TTRUE;
		}
		case AInputCommand_RotateCameraCCW:
			pEvent->StartRepeat( 0.0f, 0.0f );
			m_fIdleTime = 0.0f;
			m_vRotateInput.x -= 1.0f;
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00462dd0
// $BarnyardWiiJP: FUNCTION 8017e6e0
void AGTAStyleCameraHelper::OnCreate()
{
	if ( !m_pCamera )
		m_pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	m_fTargetPitch       = m_fDefaultPitch;
	m_fPitch             = m_fDefaultPitch;
	m_fTargetOpacity     = 1.0f;
	m_fDistance          = m_fDefaultDistance;
	m_fTargetDistance    = m_fDefaultDistance;
	m_fMaxPitch          = m_fDefaultMaxPitch;
	m_fDistanceSpeed     = 3.0f;
	m_fYaw               = 0.0f;
	m_bYawLimits         = TFALSE;
	m_fIdleTime          = 100.0f;
	m_bCharacterInTheWay = TFALSE;

#ifdef BARNYARD_LATEST
	m_fHoldHeightTime = -1.0f;

	if ( AOptions::IsWiiCameraEnabled() )
		m_bMouseInput = TTRUE;
#endif
}

// $Barnyard: FUNCTION 00462e60
void AGTAStyleCameraHelper::OnDestroy()
{
	OnDetach();
	m_pTarget = TNULL;
}

// $Barnyard: FUNCTION 00463030
// $BarnyardWiiJP: FUNCTION 8017e828
void AGTAStyleCameraHelper::OnAttach()
{
	m_vRotateInput = TVector2::VEC_ZERO;
	m_fYawSpeed    = 0.0f;
	m_bRotating    = TFALSE;
	m_bPitchInput  = TFALSE;
	m_fDistance    = m_fTargetDistance;

#ifdef BARNYARD_LATEST
	m_fCameraClearTime = 0.0f;
#endif

	if ( m_pCamera )
		m_pCamera->SetFOV( GetFOV() );

	if ( m_pTarget )
	{
		// Snap to the target
		TBOOL bUsePath = m_bUsePath;
		m_bRotating    = TTRUE;
		m_bUsePath     = TFALSE;

		m_vLastTargetPos = *m_pTarget->GetUnit()->GetSteer()->GetPosition();
		OnUpdate( 1000.0f );

		m_bUsePath = bUsePath;
	}

#ifdef BARNYARD_LATEST
	if ( AOptions::IsWiiCameraEnabled() )
		m_bMouseInput = TTRUE;
#endif

	m_fIdleTime = 100.0f;
}

// $Barnyard: FUNCTION 00464360
void AGTAStyleCameraHelper::ResetPosition( TBOOL a_bSnapCamera )
{
	TVector4 vTarget   = TVector4::VEC_ZERO;
	TVector4 vPosition = TVector4::VEC_POSZ;

	if ( m_pTarget && m_pTarget->GetUnit() )
	{
		ASteer* pSteer = m_pTarget->GetUnit()->GetSteer();

		TVector4 vSteerPos;
		TVector4 vSteerDir;
		pSteer->CopyPosition( vSteerPos );
		pSteer->CopyDirection( vSteerDir );

		// Behind the target
		vTarget.x = vSteerPos.x;
		vTarget.y = vSteerPos.y - m_fHeightOffset;
		vTarget.z = vSteerPos.z;
		vTarget.w = 1.0f;

		TVector4 vOffset;
		vOffset.Multiply( vSteerDir, -10.0f );

		vPosition = vTarget + vOffset;
	}

	SetPosition( vPosition, vTarget, a_bSnapCamera );
}

// $Barnyard: FUNCTION 004642e0
void AGTAStyleCameraHelper::SetTarget( TRef<AUnitProxy> a_pTarget )
{
	ResetTarget();
	ACameraHelper::SetTarget( a_pTarget );
}

// $Barnyard: FUNCTION 00464260
void AGTAStyleCameraHelper::ResetTarget()
{
	if ( m_pTarget && m_pTarget->GetUnit() )
	{
		// Make the old target fully visible again
		AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, m_pTarget->GetUnit() );

		pUnit->GetModelInstance()->DisableDrawWithDefaultMaterials(); // NOTE: retail passes an unused argument (TFALSE)
		pUnit->SetOpacity( 1.0f );

		// NOTE: inlined AModelInstance method 00610700
		pUnit->GetModelInstance()->ResetOpacity();
	}

	ACameraHelper::ResetTarget();
}

// $Barnyard: FUNCTION 00462a10
// $BarnyardWiiJP: FUNCTION 8017e77c
void AGTAStyleCameraHelper::OnDetach()
{
	m_fYawSpeed          = 0.0f;
	m_bRotating          = TFALSE;
	m_bPitchInput        = TFALSE;
	m_vRotateInput       = TVector2::VEC_ZERO;
	m_bCharacterInTheWay = TFALSE;

#ifdef BARNYARD_LATEST
	m_fCameraClearTime = 0.0f;
#endif
}

// $Barnyard: FUNCTION 00462ac0
void AGTAStyleCameraHelper::SetPosition( const TVector4& a_rcPosition, const TVector4& a_rcTarget, TBOOL a_bSnapCamera )
{
	// Remember the state of the camera to restore it
	TVector4 vOldCameraPos;
	TVector4 vOldCameraLookAt;
	TFLOAT   fOldFOV = TMath::PI / 3.0f;

	if ( !a_bSnapCamera && m_pCamera )
	{
		const TMatrix44& rcMatrix = m_pCamera->GetMatrix();

		const TVector4& rcForward = rcMatrix.AsBasisVector4( BASISVECTOR_FORWARD );

		vOldCameraPos      = rcMatrix.GetTranslation();
		vOldCameraLookAt.x = vOldCameraPos.x + rcForward.x;
		vOldCameraLookAt.y = vOldCameraPos.y + rcForward.y;
		vOldCameraLookAt.z = vOldCameraPos.z + rcForward.z;
		vOldCameraLookAt.w = 1.0f;
		fOldFOV            = m_pCamera->GetFOV();
	}

	m_fDistance       = m_fDefaultDistance;
	m_fTargetPitch    = m_fDefaultPitch;
	m_fTargetOpacity  = 1.0f;
	m_fTargetDistance = m_fDefaultDistance;

	// Get the pitch from the direction
	TVector4 vDirection;
	vDirection.x = a_rcPosition.x - a_rcTarget.x;
	vDirection.y = a_rcPosition.y - a_rcTarget.y;
	vDirection.z = a_rcPosition.z - a_rcTarget.z;
	vDirection.w = a_rcPosition.w;

	TFLOAT fY    = -vDirection.y;
	vDirection.y = 0.0f;

	m_fPitch = TMath::ATan( fY / Length( vDirection ) ) * 57.295776f;

	// NOTE: retail clips with the limits swapped (TMath::Clip with min = m_fMaxPitch and max = m_fMinPitch)
	if ( m_fPitch > m_fMinPitch )
		m_fPitch = m_fMinPitch;
	else if ( m_fPitch < m_fMaxPitch )
		m_fPitch = m_fMaxPitch;

	m_fMaxPitch      = m_fDefaultMaxPitch;
	m_fDistanceSpeed = 3.0f;
	m_fYaw           = 0.0f;
	m_vRotateInput   = TVector2::VEC_ZERO;
	m_bPitchInput    = TFALSE;

	m_vLastTargetPos = a_rcTarget;
	m_vLastTargetPos.y -= m_fHeightOffset;
	m_vPivot    = a_rcTarget;
	m_vLookAt   = a_rcTarget;
	m_vPosition = a_rcPosition;

	LookAt( m_vPosition, m_vPivot );

	// Snap to the new position
	TBOOL bUsePath = m_bUsePath;
	m_bRotating    = TTRUE;
	m_bUsePath     = TFALSE;
	OnUpdate( 1000.0f );
	m_bUsePath = bUsePath;

	if ( !a_bSnapCamera && m_pCamera )
	{
		m_pCamera->LookAt( vOldCameraPos, vOldCameraLookAt );
		m_pCamera->SetFOV( fOldFOV );
	}
}

// $Barnyard: FUNCTION 00462a90
void AGTAStyleCameraHelper::SetUsePath( TBOOL a_bUsePath )
{
	if ( m_bUsePath != a_bUsePath )
	{
		m_bUsePath = a_bUsePath;
		m_pPath->SetActive( a_bUsePath );
	}
}

// $Barnyard: FUNCTION 00462d20
void AGTAStyleCameraHelper::SetForceUpdate( TBOOL a_bForce )
{
	m_bRotating    = TTRUE;
	m_bForceUpdate = a_bForce;
}

// $Barnyard: FUNCTION 00462d40
void AGTAStyleCameraHelper::SetPathPaused( TBOOL a_bPaused )
{
	m_bPathPaused = a_bPaused;
}

// $Barnyard: FUNCTION 00462d50
void AGTAStyleCameraHelper::SetYawLimits( TBOOL a_bEnabled, TFLOAT a_fMin, TFLOAT a_fMax )
{
	m_bYawLimits = a_bEnabled;
	m_fYawMin    = a_fMin;
	m_fYawMax    = a_fMax;
}

// $Barnyard: FUNCTION 00462a40
TBOOL AGTAStyleCameraHelper::AddCollisionFlag( TINT a_iBit )
{
	m_uiCollisionMask |= 1 << a_iBit;
	return TTRUE;
}

// $Barnyard: FUNCTION 00462a60
TBOOL AGTAStyleCameraHelper::RemoveCollisionFlag( TINT a_iBit )
{
	if ( !( m_uiCollisionMask & ( 1 << a_iBit ) ) )
		return TFALSE;

	m_uiCollisionMask &= ~( 1 << a_iBit );
	return TTRUE;
}

// $Barnyard: FUNCTION 00462e90
TBOOL AGTAStyleCameraHelper::IsCharacterInTheWay( AUnitHash::QueryResult* a_pResults, TUINT a_uiNumResults, const TVector4& a_rcFrom, const TVector4& a_rcTo, const TVector4& a_rcCameraPos, AUnit* a_pIgnoreUnit, TFLOAT a_fRadius )
{
	TFLOAT fRadiusSq = a_fRadius * a_fRadius;

	TVector4 vFrom = a_rcFrom;
	vFrom.y        = 0.0f;

	TVector4 vTo = a_rcTo;
	vTo.y        = 0.0f;

	for ( TUINT i = 0; i < a_uiNumResults; i++ )
	{
		AUnit* pUnit = a_pResults[ i ].pUnit;

		if ( !pUnit || pUnit == a_pIgnoreUnit )
			continue;

		if ( !pUnit->IsA( &TGetClass( ACharacterUnit ) ) || pUnit->GetType()->GetName() == TPS8( generic_chick ) )
			continue;

		ASteer* pSteer = pUnit->GetSteer();

		if ( !pSteer )
			continue;

		TVector4 vPosition;
		pSteer->CopyPosition( vPosition );

		if ( IsPositionInFadeRange( a_rcCameraPos, vPosition ) )
			TSTATICCAST( ACharacterUnit, pUnit )->SetFadeOut( TTRUE );

		vPosition.y = 0.0f;

		TFLOAT fT;

		if ( TMath::DistancePointToSegmentSq( vPosition, vFrom, vTo, fT ) < fRadiusSq )
			return TTRUE;
	}

	return TFALSE;
}
