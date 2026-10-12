#include "pch.h"
#include "ABallSteer2.h"
#include "ASteerTraits.h"
#include "Units/AVisibleUnit.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/barnyard.h"

#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionObject.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00557770, CreateObject 005579b0, CreateObjectInPlace 00557a10 (the constructor is inlined in both)
TDEFINE_CLASS( ABallSteer2, 1, 0 );

// $Barnyard: FUNCTION 00557780
ABallSteer2::ABallSteer2()
    : m_vecPosition( TVector4::VEC_ZERO )
    , m_vecMoveDirection( TVector4::VEC_POSX )
    , m_vecDirection( TVector4::VEC_POSX )
    , m_vecOffset( TVector4::VEC_ZERO )
    , m_fGravity( 9.8f )
    , m_uiCollisionFlags( ACollisionObject::TYPE_IGNORED )
    , m_fSpeed( 0.0f )
    , m_iNumStillUpdates( NUM_STILL_UPDATES_TO_STOP )
    , m_fDamping( 2.2f )
    , m_fBounceDistanceFactor( 0.01f )
    , m_fLandSoundTime( 0.0f )
    , m_bAlignToGround( TFALSE )
    , m_bUnk75( TTRUE )
    , m_bIgnoreHitsWhenLeaving( TFALSE )
    , m_pNextSteerClass( TNULL )
{
}

// $Barnyard: FUNCTION 00557a50
// $Barnyard: FUNCTION 00557960
ABallSteer2::~ABallSteer2()
{
}

// $Barnyard: FUNCTION 00557910
TBOOL ABallSteer2::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );

	a_pOther->CopyPosition( m_vecPosition );
	m_vecMoveDirection = m_vecDirection;
	m_iNumStillUpdates = 3;

	return TTRUE;
}

// $Barnyard: FUNCTION 005578a0
TBOOL ABallSteer2::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );

	m_vecPosition      = a_rcPosition;
	m_vecMoveDirection = a_rcDirection;
	m_iNumStillUpdates = 3;

	return TTRUE;
}

// $Barnyard: FUNCTION 00557ec0
TBOOL ABallSteer2::Update( TFLOAT a_fDeltaTime )
{
	if ( m_iNumStillUpdates >= NUM_STILL_UPDATES_TO_STOP )
	{
		// Lies still, hand the unit over to the next steer
		if ( m_pNextSteerClass )
			GetParentUnit().ChangeSteer( m_pNextSteerClass );

		return TFALSE;
	}

	// Go from the position of the unit to the position of the ball
	TVector4 vecUnitPosition = *GetPosition();
	SetPosition( TVector4( vecUnitPosition.x - m_vecOffset.x, vecUnitPosition.y - m_vecOffset.y, vecUnitPosition.z - m_vecOffset.z, vecUnitPosition.w ) );

	TVector4 vecOldPosition = *GetPosition();

	// Damping and gravity
	TVector4 vecVelocity;
	vecVelocity.Multiply( m_vecMoveDirection, m_fSpeed );
	vecVelocity.Multiply( 1.0f - a_fDeltaTime * m_fDamping );

	TFLOAT   fGravity = a_fDeltaTime * m_fGravity;
	TVector4 vecGravity;
	vecGravity.Multiply( TVector4::VEC_POSY, fGravity );
	vecVelocity.Add( vecGravity );

	TFLOAT fSpeed      = vecVelocity.NormaliseMag();
	m_fSpeed           = fSpeed;
	m_vecMoveDirection = vecVelocity;

	TBOOL bResult = Move( fSpeed * a_fDeltaTime, fGravity, TTRUE );

	TVector4 vecNewPosition = *GetPosition();

	m_fLandSoundTime -= a_fDeltaTime;

	if ( m_fLandSoundTime <= 0.0f )
		m_fLandSoundTime = 0.0f;

	if ( !vecOldPosition.IsEqual( vecNewPosition ) || fSpeed == 0.0f )
	{
		if ( m_fSpeed != 0.0f )
			m_iNumStillUpdates = 0;
	}
	else
	{
		// It can't move
		TVector4 vecGround = vecNewPosition;
		ATerrainInterface::GetSingleton()->SetToGroundHeightTop( vecGround, TCollisionCommon::TOSHICGROUPFLAG_NONE );

		if ( GetRadius() * 1.5f + vecNewPosition.y > vecGround.y )
		{
			// Close to the ground, stop rolling
			m_iNumStillUpdates++;
			m_fSpeed             = 0.0f;
			m_vecMoveDirection.y = 0.0f;
			m_vecMoveDirection.Normalise();
		}

		if ( m_bAlignToGround )
		{
			TVector4 vecShadowPosition;
			TINT     iAlpha;
			TVector3 vecRotation;
			TVector4 vecNormal;

			ATerrainInterface::GetSingleton()->GetGroundShadowInfo( m_vecPosition, 0.0f, vecShadowPosition, iAlpha, vecRotation, vecNormal, GetRadius(), GetRadius() * 2, TCollisionCommon::TOSHICGROUPFLAG_NONE );
			vecNormal.Normalise();
			SetDirection( vecNormal );
		}
	}

	// Go back to the position of the unit
	SetPosition( TVector4( vecNewPosition.x + m_vecOffset.x, vecNewPosition.y + m_vecOffset.y, vecNewPosition.z + m_vecOffset.z, vecNewPosition.w ) );

	return bResult;
}

// $Barnyard: FUNCTION 00557a70
TBOOL ABallSteer2::Move( TFLOAT a_fDistance, TFLOAT a_fGravity, TBOOL a_bCheckStop )
{
	if ( a_fDistance <= 0.01f )
		return TTRUE;

	AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, m_oOwner->GetUnit() );

	TFLOAT            fDistance = a_fDistance;
	TBOOL             bEdgeHit  = TFALSE;
	TVector4          vecNormal;
	TFLOAT            fStepHeight;
	TFLOAT            fPenetration;
	ACollisionObject* pHitObject = TNULL;

	TUINT uiHit = ACollisionManager::GetSingleton()->CollideSweptSphere( m_vecPosition, m_pTraits->m_fRadius, pUnit->GetCollisionObject(), m_vecMoveDirection, fDistance, vecNormal, 0.0f, fStepHeight, fPenetration, TINT( TUINTPTR( &bEdgeHit ) ), m_uiCollisionFlags, &pHitObject, TCollisionCommon::TOSHICGROUPFLAG_NONE );

	TBOOL bLeaving = TFALSE;

	if ( m_bIgnoreHitsWhenLeaving && TVector4::DotProduct3( m_vecMoveDirection, vecNormal ) > 0.0f )
		bLeaving = TTRUE;

	if ( uiHit == 0 || bLeaving )
	{
		// Nothing in the way
		TVector4 vecMove;
		vecMove.Multiply( m_vecMoveDirection, fDistance );
		m_vecPosition.Add( vecMove );

		return TTRUE;
	}

	if ( uiHit & ~ACollisionObject::TYPE_TERRAIN )
	{
		// Hit a collision object
		if ( ( uiHit & ACollisionObject::TYPE_PLAYER ) && ( pHitObject->GetType() & ACollisionObject::TYPE_PLAYER ) )
		{
			// Pushed away from the object
			const TVector4& rcObjectPosition = pHitObject->GetObjectPosition();
			vecNormal.Set( m_vecPosition.x - rcObjectPosition.x, m_vecPosition.y - rcObjectPosition.y, m_vecPosition.z - rcObjectPosition.z, 1.0f );
			vecNormal.Normalise();

			// Right above or below it, use the facing of its unit
			if ( TMath::Abs( vecNormal.x ) < 0.1f && TMath::Abs( vecNormal.z ) < 0.1f )
				vecNormal = *TSTATICCAST( AUnit, pHitObject->GetOwnerUnit() )->GetSteer()->GetDirection();

			vecNormal.y = 0.01f;
			vecNormal.Normalise();

			m_vecMoveDirection = vecNormal;
			fDistance          = a_fDistance;
			m_iNumStillUpdates = 0;

			ACollisionManager::GetSingleton()->CollideSweptSphere( m_vecPosition, m_pTraits->m_fRadius, pUnit->GetCollisionObject(), m_vecMoveDirection, fDistance, vecNormal, 0.0f, fStepHeight, fPenetration, TINT( TUINTPTR( &bEdgeHit ) ), ACollisionObject::TYPE_PLAYER, &pHitObject, TCollisionCommon::TOSHICGROUPFLAG_NONE );

			TVector4 vecMove;
			vecMove.Multiply( m_vecMoveDirection, fDistance );
			m_vecPosition.AsVector3().Add( vecMove.AsVector3() );

			return TTRUE;
		}

		if ( vecNormal.y == 1.0f )
		{
			m_fSpeed = 0.0f;
			return TTRUE;
		}

		if ( vecNormal.y > 0.0f )
		{
			vecNormal.y = 0.0f;
			vecNormal.Normalise();
		}
	}

	// Move up to the hit
	TVector4 vecMove;
	vecMove.Multiply( m_vecMoveDirection, fDistance );
	m_vecPosition.x += vecMove.x;
	m_vecPosition.y += vecMove.y;
	m_vecPosition.z += vecMove.z;

	if ( a_bCheckStop )
	{
		if ( m_vecMoveDirection.y > 0.0f && m_fSpeed < 0.3f )
		{
			// Too slow to bounce
			m_iNumStillUpdates++;
			m_fSpeed             = 0.0f;
			m_vecMoveDirection.y = 0.0f;
			m_vecMoveDirection.Normalise();

			return TTRUE;
		}

		if ( m_fSpeed > 1.0f && m_fLandSoundTime <= 0.0f )
		{
			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::BARNYARD_LAND, *GetPosition() );
			m_fLandSoundTime = 1.0f;
		}
	}

	// Bounce off the normal
	TVector4 vecProjected = m_vecMoveDirection;
	vecProjected.ProjectOnto( vecNormal );

	TFLOAT   fBounce = m_pTraits->m_fBounce;
	TVector4 vecReflect;
	vecReflect.Multiply( vecProjected, 2.0f );
	vecReflect.Multiply( fBounce );

	m_vecMoveDirection.x -= vecReflect.x;
	m_vecMoveDirection.y -= vecReflect.y;
	m_vecMoveDirection.z -= vecReflect.z;

	// Move the rest of the distance in the new direction
	TFLOAT fDistanceLeft = ( a_fDistance - fDistance ) * m_fBounceDistanceFactor;
	TFLOAT fGravity      = fDistanceLeft / a_fDistance * a_fGravity;

	TVector4 vecGravity;
	vecGravity.Multiply( TVector4::VEC_POSY, fGravity );
	m_vecMoveDirection.x += vecGravity.x;
	m_vecMoveDirection.y += vecGravity.y;
	m_vecMoveDirection.z += vecGravity.z;
	m_vecMoveDirection.Normalise();

	return Move( fDistanceLeft, fGravity, TFALSE );
}

// NOTE: same code as ASteerKinematics3D::SetPosition (005773f0), folded in retail
TBOOL ABallSteer2::SetPosition( const TVector4& a_rcPosition )
{
	m_vecPosition = a_rcPosition;
	return TTRUE;
}

// $Barnyard: FUNCTION 00560a80
TBOOL ABallSteer2::SetPosition3( const TVector3& a_rcPosition )
{
	return SetPosition( TVector4( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f ) );
}

// NOTE: same code as ABallSteer::SetDirection (00556b40), folded in retail
void ABallSteer2::SetDirection( const TVector4& a_rcDirection )
{
	m_vecDirection = a_rcDirection;
}

// NOTE: same code as AThrowItemSteer::CopyPosition (0055fd50), folded in retail
void ABallSteer2::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vecPosition;
}

// NOTE: same code as AThrowItemSteer::CopyPosition3 (00556a20), folded in retail
void ABallSteer2::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vecPosition.AsVector3();
}

// NOTE: same code as ASteerKinematics3D::GetPosition (005772b0), folded in retail
const TVector4* ABallSteer2::GetPosition()
{
	return &m_vecPosition;
}

// NOTE: same code as AThrowItemSteer::CopyDirection (00556a60), folded in retail
void ABallSteer2::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = m_vecDirection;
}

// NOTE: same code as AThrowItemSteer::GetDirection (00572640), folded in retail
const TVector4* ABallSteer2::GetDirection()
{
	return &m_vecDirection;
}

// NOTE: same code as ACollisionObjectMesh::GetObjectRadius (0061dea0), folded in retail
TFLOAT ABallSteer2::GetSpeed()
{
	return m_fSpeed;
}

// $Barnyard: FUNCTION 00557970
void ABallSteer2::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vecMoveDirection = a_rcVelocity;
	m_fSpeed           = m_vecMoveDirection.NormaliseMag();
	m_iNumStillUpdates = 0;
}

// NOTE: shared setter of +0x68 (0063e550), folded in retail
void ABallSteer2::SetDamping( TFLOAT a_fDamping )
{
	m_fDamping = a_fDamping;
}

// NOTE: shared setter of +0x6c (0063e560), folded in retail
void ABallSteer2::SetBounceDistanceFactor( TFLOAT a_fFactor )
{
	m_fBounceDistanceFactor = a_fFactor;
}

// $Barnyard: FUNCTION 00557870
void ABallSteer2::SetAlignToGround( TBOOL a_bAlign )
{
	m_bAlignToGround = a_bAlign;
}

// $Barnyard: FUNCTION 00557880
TBOOL ABallSteer2::IsStopped()
{
	return m_iNumStillUpdates >= NUM_STILL_UPDATES_TO_STOP;
}

// $Barnyard: FUNCTION 00557890
void ABallSteer2::SetUnk75( TBOOL a_bValue )
{
	m_bUnk75 = a_bValue;
}
