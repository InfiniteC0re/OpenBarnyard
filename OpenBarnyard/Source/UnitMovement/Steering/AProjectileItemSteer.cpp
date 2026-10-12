#include "pch.h"
#include "AProjectileItemSteer.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0056de90, CreateObject 0056e1f0, CreateObjectInPlace 0056e250
TDEFINE_CLASS( AProjectileItemSteer, 3, 0 );

// $Barnyard: FUNCTION 0056dea0
AProjectileItemSteer::AProjectileItemSteer()
    : m_eState( STATE_IDLE )
    , m_vecPosition( TVector4::VEC_ZERO )
    , m_vecVelocity( TVector4::VEC_ZERO )
    , m_vecDirection( TVector4::VEC_POSZ )
    , m_vecUp( TVector4::VEC_NEGY )
    , m_vecRight( TVector4::VEC_POSX )
    , m_vecFallVelocity( TVector4::VEC_ZERO )
    , m_vecHitNormal( TVector4::VEC_NEGY )
    , m_fGravity( 9.8f )
    , m_fUnk90( 1.0f )
    , m_fLifeTime( 5.0f )
    , m_eIgnoreGroups( 0 )
{
	// NOTE: m_fTime isn't initialised (SetVelocity does it)
}

// $Barnyard: FUNCTION 0056e290
// $Barnyard: FUNCTION 0056dff0
AProjectileItemSteer::~AProjectileItemSteer()
{
}

// $Barnyard: FUNCTION 0056e040
TBOOL AProjectileItemSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );

	a_pOther->CopyPosition( m_vecPosition );
	a_pOther->CopyDirection( m_vecDirection );
	a_pOther->CopyVelocity( m_vecVelocity );

	return TTRUE;
}

// $Barnyard: FUNCTION 0056e000
TBOOL AProjectileItemSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );
	m_vecRight.CrossProduct( a_rcDirection, m_vecUp );

	return TTRUE;
}

// $Barnyard: FUNCTION 0056e480
TBOOL AProjectileItemSteer::Update( TFLOAT a_fDeltaTime )
{
	if ( m_eState != STATE_FLYING )
		return TTRUE;

	// Apply gravity
	TVector4 vecGravity;
	vecGravity.Multiply( TVector4::VEC_POSY, a_fDeltaTime );
	vecGravity.Multiply( m_fGravity );
	m_vecFallVelocity.x += vecGravity.x;
	m_vecFallVelocity.y += vecGravity.y;
	m_vecFallVelocity.z += vecGravity.z;

	TVector4 vecMoveDir( m_vecVelocity.x + m_vecFallVelocity.x, m_vecVelocity.y + m_vecFallVelocity.y, m_vecVelocity.z + m_vecFallVelocity.z, m_vecVelocity.w );
	TFLOAT   fDistance = vecMoveDir.NormaliseMag() * a_fDeltaTime;

	m_fTime += a_fDeltaTime;
	if ( m_fTime > m_fLifeTime )
		m_eState = STATE_TIMEDOUT;

	TBOOL bHit = ATerrainInterface::GetSingleton()->CollideRay( m_vecPosition, vecMoveDir, fDistance, TNULL, m_eIgnoreGroups, TFALSE );

	TVector4 vecMove;
	vecMove.Multiply( vecMoveDir, fDistance );
	m_vecPosition.x += vecMove.x;
	m_vecPosition.y += vecMove.y;
	m_vecPosition.z += vecMove.z;

	if ( bHit )
	{
		m_vecHitNormal = GetLandingNormal( m_vecPosition, vecMoveDir );
		m_eState       = STATE_LANDED;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0056e2b0
TVector4 AProjectileItemSteer::GetLandingNormal( TVector4 a_vecPosition, TVector4 a_vecDirection )
{
	TVector4 vecSphere( a_vecPosition.x, a_vecPosition.y, a_vecPosition.z, 0.5f );

	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideSphere( vecSphere, &oHitList, -1, 0 );

	TVector4 vecNormal = TVector4::VEC_ZERO;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		TVector4 vecHitNormal = oHitList.aHits[ i ].GetNormal();

		if ( TVector4::DotProduct3( vecHitNormal, a_vecDirection ) <= 0.0f )
		{
			vecNormal.x += vecHitNormal.x;
			vecNormal.y += vecHitNormal.y;
			vecNormal.z += vecHitNormal.z;
		}
	}

	if ( vecNormal.IsEqual( TVector4::VEC_ZERO ) )
		return TVector4::VEC_NEGY;

	vecNormal.Normalise();
	return vecNormal;
}

// $Barnyard: FUNCTION 0056e0e0
TBOOL AProjectileItemSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vecPosition     = a_rcPosition;
	m_vecFallVelocity = TVector4::VEC_ZERO;

	return TTRUE;
}

// $Barnyard: FUNCTION 0056e090
TBOOL AProjectileItemSteer::SetPosition3( const TVector3& a_rcPosition )
{
	m_vecPosition.Set( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	m_vecFallVelocity = TVector4::VEC_ZERO;

	return TTRUE;
}

// $Barnyard: FUNCTION 0056e130
void AProjectileItemSteer::SetDirection( const TVector4& a_rcDirection )
{
	m_vecDirection = a_rcDirection;
}

// $Barnyard: FUNCTION 0056e170
void AProjectileItemSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vecPosition;
}

// $Barnyard: FUNCTION 0056e150
void AProjectileItemSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vecPosition.AsVector3();
}

// NOTE: folded getter of a TVector4 at 0x1c (006cd0c0)
const TVector4* AProjectileItemSteer::GetPosition()
{
	return &m_vecPosition;
}

// NOTE: folded with ACarHeliSteer::CopyPosition (the position of a car steer is at 0x3c)
// $Barnyard: FUNCTION 0056e1a0
void AProjectileItemSteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = m_vecDirection;
}

// NOTE: folded with ACarHeliSteer::CopyPosition3
// $Barnyard: FUNCTION 0055c980
void AProjectileItemSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_vecDirection.AsVector3();
}

// NOTE: folded with ACarHeliSteer::GetPosition
// $Barnyard: FUNCTION 0056e190
const TVector4* AProjectileItemSteer::GetDirection()
{
	return &m_vecDirection;
}

// $Barnyard: FUNCTION 0056e1c0
void AProjectileItemSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vecVelocity = a_rcVelocity;
	m_eState      = STATE_FLYING;
	m_fTime       = 0.0f;
}
