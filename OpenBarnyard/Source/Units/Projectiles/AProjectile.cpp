#include "pch.h"
#include "AProjectile.h"
#include "ASimpleProjectileManager.h"
#include "AProjectileLauncher.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionObject.h"
#include "Physics/AObjectHashMain.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0066a0c0, CreateObject 0066a6b0, CreateObjectInPlace 0066a710
TDEFINE_CLASS( AProjectile );

// $Barnyard: FUNCTION 0066a550
AProjectile::AProjectile()
    : m_pLauncher( TNULL )
    , m_fLifetime( 5.0f )
    , m_fAge( 0.0f )
    , m_vVelocity( TVector4::VEC_ZERO )
    , m_vPrevPosition( TVector4::VEC_ZERO )
    , m_vPosition( TVector4::VEC_ZERO )
    , m_bUnk40( TFALSE )
    , m_uiCollisionMask( 0x80000 )
    , m_iPoolIndex( -1 )
{
	AProjectile::Reset();
}

// $Barnyard: FUNCTION 0066a750
// $Barnyard: FUNCTION 0066a630
AProjectile::~AProjectile()
{
	AProjectile::Reset();
}

// $Barnyard: FUNCTION 0066a1f0
void AProjectile::Restart()
{
	m_fAge          = 0.0f;
	m_vPrevPosition = TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 0066a0d0
TBOOL AProjectile::Update( TFLOAT a_fDeltaTime )
{
	m_fAge += a_fDeltaTime;
	m_vPrevPosition = m_vPosition;

	TVector4 vOffset;
	vOffset.Multiply( m_vVelocity, a_fDeltaTime );
	m_vPosition.x += vOffset.x;
	m_vPosition.y += vOffset.y;
	m_vPosition.z += vOffset.z;

	// Gravity (Y points down)
	m_vPosition.y += a_fDeltaTime * a_fDeltaTime * 4.9f;
	m_vVelocity.y += a_fDeltaTime * 9.8f;

	if ( m_fLifetime > 0.0f && m_fAge > m_fLifetime )
		Kill();

	return TFALSE;
}

// $Barnyard: FUNCTION 0066a290
void AProjectile::Kill()
{
	ASimpleProjectileManager::GetSingleton()->ReleaseProjectile( this );
	m_bUnk40 = TFALSE;
}

// $Barnyard: FUNCTION 0066a1a0
void AProjectile::SetVelocity( const Toshi::TVector4& a_rcVelocity )
{
	m_vVelocity = a_rcVelocity;
}

// $Barnyard: FUNCTION 0066a4b0
void AProjectile::Reset()
{
	m_vPrevPosition = TVector4::VEC_ZERO;
	m_vPosition     = TVector4::VEC_ZERO;
	m_vVelocity     = TVector4::VEC_ZERO;
	m_oHitUnit      = TNULL;
	m_fAge          = 0.0f;
}

// $Barnyard: FUNCTION 0066a2b0
TBOOL AProjectile::CheckWorldCollision()
{
	if ( m_vPrevPosition.MagnitudeSq() <= 0.1f )
		return TFALSE;

	if ( m_vPrevPosition.IsEqual( m_vPosition ) )
		return TFALSE;

	TVector4 vMovement;
	vMovement.Substract( m_vPosition, m_vPrevPosition );

	TFLOAT fDistance    = vMovement.NormaliseMag();
	TFLOAT fHitDistance = fDistance;

	// Line test against the collision world
	if ( ATerrainInterface::GetSingleton()->CollideRay( m_vPrevPosition, vMovement, fHitDistance, TCollisionCommon::TOSHICGROUPFLAG( m_uiCollisionMask ), TTRUE ) )
	{
		// Moves the projectile to the hit point
		vMovement.Multiply( fHitDistance );
		m_vPosition.x = vMovement.x + m_vPrevPosition.x;
		m_vPosition.y = vMovement.y + m_vPrevPosition.y;
		m_vPosition.z = vMovement.z + m_vPrevPosition.z;
		m_vPosition.w = m_vPrevPosition.w;
		return TTRUE;
	}

	// Line test against the collision objects around the segment
	fDistance *= 0.5f;

	TVector4 vHalfMovement;
	vHalfMovement.Multiply( vMovement, fDistance );

	TVector4 vCentre;
	vCentre.Add( m_vPrevPosition, vHalfMovement );

	TINT                      iNumObjects;
	AObjectHash::QueryResult* pObjects = AObjectHashMain::GetSingleton()->FindObjects( vCentre, fDistance, &iNumObjects, TFALSE, 20 );
	ARayData                  oRay( m_vPrevPosition, m_vPosition );

	for ( TINT i = 0; i < iNumObjects; i++ )
	{
		ACollisionObject* pObject = TSTATICCAST( ACollisionObject, pObjects[ i ].pObject );

		if ( pObject->GetOwnerUnit() == TNULL )
		{
			ACollisionHit oHit;

			if ( pObject->CollideRay( oRay, oHit ) )
			{
				vMovement.Multiply( oHit.fDistance );
				m_vPosition.x = vMovement.x + m_vPrevPosition.x;
				m_vPosition.y = vMovement.y + m_vPrevPosition.y;
				m_vPosition.z = vMovement.z + m_vPrevPosition.z;
				m_vPosition.w = m_vPrevPosition.w;
				return TTRUE;
			}
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0066a770
TBOOL AProjectile::CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit )
{
	TBOOL bHit = TFALSE;

	// Along the path of the last update
	if ( m_vPrevPosition.MagnitudeSq() > 0.1f && !m_vPrevPosition.IsEqual( m_vPosition ) )
	{
		ARayData oRay( m_vPrevPosition, m_vPosition );

		if ( ACollisionManager::CollideRayUnits( m_pLauncher->GetOwner()->GetUnit(), oRay, 10.0f, a_rHitUnit ) )
			bHit = TTRUE;
		else
			a_rHitUnit = TNULL;

		if ( bHit )
			return bHit;
	}

	// Around the projectile
	AUnit* pOwner = m_pLauncher->GetOwner()->GetUnit();

	if ( ACollisionManager::CollideSphereUnits( m_vPosition, GetRadius(), a_rHitUnit, 1, &pOwner ) )
		return TTRUE;

	return bHit;
}

// $Barnyard: FUNCTION 0066a220
Toshi::TVector4 AProjectile::GetDirection() const
{
	TVector4 vDirection = m_vVelocity;
	vDirection.Normalise();

	return vDirection;
}

// $Barnyard: FUNCTION 0066a920
void AProjectile::CalculateThrowVelocity( const TVector4& a_rcTarget, const TVector4& a_rcOrigin, TFLOAT a_fSpeed, TVector4& a_rOutVelocity )
{
	TVector4 vDirection;
	vDirection.x = a_rcTarget.x - a_rcOrigin.x;
	vDirection.y = 0.0f;
	vDirection.z = a_rcTarget.z - a_rcOrigin.z;
	vDirection.w = a_rcTarget.w;

	TFLOAT fDistance = vDirection.NormaliseMag();
	TFLOAT fSpeed    = TMath::Sqrt( fDistance ) * a_fSpeed * 0.33333331f;

	a_rOutVelocity.x = vDirection.x * fSpeed;
	a_rOutVelocity.z = vDirection.z * fSpeed;

	if ( fSpeed <= 0.001f )
	{
		a_rOutVelocity.w = 1.0f;
		a_rOutVelocity.y = 0.0f;
	}
	else
	{
		TFLOAT fTime     = fDistance / fSpeed;
		a_rOutVelocity.w = 1.0f;
		a_rOutVelocity.y = ( ( a_rcTarget.y - a_rcOrigin.y ) - ms_fTrajectoryGravity * fTime * fTime * 0.5f ) / fTime;
	}
}
