#include "pch.h"
#include "ACollisionObjectSimple.h"
#include "ACollisionObjectMesh.h"
#include "ACollisionModelInstance.h"

#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0061bd60, CreateObject 0061c7b0, CreateObjectInPlace 0061c840
TDEFINE_CLASS( ACollisionObjectSimple );

// $Barnyard: FUNCTION 0061c5b0
ACollisionObjectSimple::ACollisionObjectSimple()
    : m_fHeightScale( 0.0f )
    , m_fSphereRaise( 0.0f )
{
	m_eType = TYPE_SIMPLE;
}

// $Barnyard: FUNCTION 0061c5f0
// $Barnyard: FUNCTION 0061c5e0
// $Barnyard: FUNCTION 005edcd0
ACollisionObjectSimple::~ACollisionObjectSimple()
{
	if ( m_pModelInstance )
	{
		TFree( m_pModelInstance );
		m_pModelInstance = TNULL;
	}

	if ( IsCollisionObjectLinked() )
	{
		RemoveHash();
		Node::Remove();
	}
}

// $Barnyard: FUNCTION 0061c6e0
TBOOL ACollisionObjectSimple::Create( const TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fHeightScale, TFLOAT a_fSphereRaise, TUINT a_eTypeFlags, ACollisionModelInstance* a_pModelInstance )
{
	m_pModelInstance = a_pModelInstance;
	m_fRadius        = a_fRadius;
	m_fHeightScale   = a_fHeightScale;
	m_fSphereRaise   = a_fSphereRaise;
	m_vPosition      = a_rcPosition.AsVector3();

	if ( a_eTypeFlags )
		m_eType = TYPE( m_eType | a_eTypeFlags );

	if ( !IsCollisionObjectLinked() )
	{
		RemoveHash();
	}
	else
	{
		RemoveHash();
		Node::Remove();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0061cc70
TBOOL ACollisionObjectSimple::CreateActive( const TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fHeightScale, TFLOAT a_fSphereRaise, TUINT a_eTypeFlags, ACollisionModelInstance* a_pModelInstance )
{
	m_pModelInstance = a_pModelInstance;
	m_fSphereRaise   = a_fSphereRaise;
	m_fRadius        = a_fRadius;
	m_fHeightScale   = a_fHeightScale;
	m_vPosition      = a_rcPosition.AsVector3();

	if ( a_eTypeFlags )
		m_eType = TYPE( m_eType | a_eTypeFlags );

	Activate();
	return TTRUE;
}

// $Barnyard: FUNCTION 0061ce80
TBOOL ACollisionObjectSimple::CollideRay( const ARayData& a_rcRay, ACollisionHit& a_rHit )
{
	return CollideRayImpl( a_rcRay, a_rHit );
}

// $Barnyard: FUNCTION 0061c6a0
TBOOL ACollisionObjectSimple::CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	return CollideSweptSphereImpl( a_pObstacle, a_rfDistance, a_rcDirection, a_rcCenter, a_rNormal, a_fMaxStepHeight, a_rfStepHeight );
}

// $Barnyard: FUNCTION 0061cd00
TBOOL ACollisionObjectSimple::CollideSweptSphereNoNormal( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	// NOTE: the normal is written to a temporary vector initialised with the position
	TVector4 vecNormal( m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.0f );

	return CollideSweptSphereImpl( a_pObstacle, a_rfDistance, a_rcDirection, a_rcCenter, vecNormal, a_fMaxStepHeight, a_rfStepHeight );
}

// $Barnyard: FUNCTION 0061bf90
TBOOL ACollisionObjectSimple::CollideSphere( ACollisionObject* a_pObstacle, const TVector4& a_rcCenter, TFLOAT& a_rfDistance, TVector4& a_rNormal )
{
	return CollideSphereImpl( a_pObstacle, a_rcCenter, a_rfDistance, a_rNormal );
}

// $Barnyard: FUNCTION 0061c9d0
TBOOL ACollisionObjectSimple::CollideRayImpl( const ARayData& a_rcRay, ACollisionHit& a_rHit )
{
	TVector4 vecUp       = TVector4::VEC_NEGY;
	TFLOAT   fDistance   = a_rcRay.fLength;
	a_rHit.fDistance     = a_rcRay.fLength;
	a_rHit.pObject       = TNULL;
	TVector4 vecPosition = TVector4( m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.0f );

	if ( !TMath::RayVsCapsule( a_rcRay.vOrigin, a_rcRay.vDirection, vecPosition, m_fRadius, vecUp, m_fHeightScale, fDistance, a_rHit.vNormal, 0.001f ) )
		return TFALSE;

	a_rHit.pObject = this;

	if ( !m_pModelInstance )
	{
		a_rHit.fDistance = fDistance;
		return TTRUE;
	}

	// Test the collision model of the object
	ACollisionModelRayHitList oHitList;

	if ( m_pModelInstance->CollideRay( a_rcRay.vOrigin, a_rcRay.vDirection, a_rHit.fDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) != TTRUE )
		return TFALSE;

	// Average the normals of the triangles facing the ray
	TVector4 vecNormal = TVector4::VEC_ZERO;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		TVector4 vecHitNormal = oHitList.aHits[ i ].GetNormal();

		if ( TVector4::DotProduct3( vecHitNormal, a_rcRay.vDirection ) <= 0.0f )
		{
			vecNormal.x = vecHitNormal.x + vecNormal.x;
			vecNormal.y = vecHitNormal.y + vecNormal.y;
			vecNormal.z = vecHitNormal.z + vecNormal.z;
		}
	}

	// Tilt normals that point straight up a bit
	if ( TVector4::DotProduct3( vecNormal, TVector4::VEC_NEGY ) >= 0.9f )
		vecNormal.AsVector3().RotateX( 0.017453292f );

	vecNormal.Normalise();
	a_rHit.vNormal = vecNormal;

	return TTRUE;
}

// $Barnyard: FUNCTION 0061c3b0
// $BarnyardWiiJP: FUNCTION 803c44e0
TBOOL ACollisionObjectSimple::CollideSweptSphereImpl( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	if ( a_pObstacle->GetClass() == &TGetClass( ACollisionObjectMesh ) )
	{
		ACollisionObjectMesh* pMesh = TSTATICCAST( ACollisionObjectMesh, a_pObstacle );

		if ( !pMesh->GetSceneObject()->IsSkeletonUpdateEnabled() )
			return TFALSE;

		TFLOAT fPenetration = 0.0f;

#ifdef BARNYARD_LATEST
		// NOTE: the latest version passes the index of TOSHICGROUP_CAMERA (19) as the ignored group flags
		// during the camera checks, so it ignores the SUPPORT, PLAYER and GRASS groups instead
		if ( ACollisionObjectMesh::IsCameraCollision() )
			return pMesh->GetModelInstance()->CollideSweptSphere( a_rcCenter, m_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG( TCollisionCommon::TOSHICGROUP_CAMERA ) );
#endif

		return pMesh->GetModelInstance()->CollideSweptSphere( a_rcCenter, m_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );
	}

	const TVector4& rcObstaclePosition = a_pObstacle->GetObjectPosition();

	TVector4 vecObstaclePosition( rcObstaclePosition.x, rcObstaclePosition.y, rcObstaclePosition.z, 1.0f );
	TVector4 vecUp             = TVector4::VEC_NEGY;
	TFLOAT   fDistance         = a_rfDistance;
	TVector4 vecOriginalNormal = a_rNormal;

	TFLOAT fObstacleHeight = a_pObstacle->GetHeightScale();
	TFLOAT fObstacleRadius = a_pObstacle->GetObjectRadius();

	if ( TMath::SweptSphereVsCapsule( a_rcCenter, m_fRadius, a_rcDirection, vecObstaclePosition, fObstacleRadius, vecUp, fObstacleHeight, fDistance, a_rNormal, 0.001f ) != TTRUE )
		return TFALSE;

	if ( a_pObstacle->GetModelInstance() )
	{
		// NOTE: retail passes its unused stack arguments as the step height and penetration outputs
		TFLOAT fStepHeight;
		TFLOAT fPenetration;
		return a_pObstacle->GetModelInstance()->CollideSweptSphere( a_rcCenter, m_fRadius, a_rcDirection, a_rfDistance, a_rNormal, 0.0f, fStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );
	}

	if ( fDistance == a_rfDistance )
	{
		a_rNormal = vecOriginalNormal;
		return TFALSE;
	}

	// Don't push down
	if ( a_rNormal.y > 0.0f )
	{
		a_rNormal.y = 0.0f;
		a_rNormal.Normalise();
	}

	a_rfDistance = fDistance;
	return TTRUE;
}

// $Barnyard: FUNCTION 0061bd90
// $BarnyardWiiJP: FUNCTION 803c479c
TBOOL ACollisionObjectSimple::CollideSphereImpl( ACollisionObject* a_pObstacle, const TVector4& a_rcCenter, TFLOAT& a_rfDistance, TVector4& a_rNormal )
{
	const TVector4& rcObstaclePosition = a_pObstacle->GetObjectPosition();

	TVector4 vecObstaclePosition( rcObstaclePosition.x, rcObstaclePosition.y, rcObstaclePosition.z, 1.0f );
	TVector4 vecUp = TVector4::VEC_NEGY;

	// Raise the sphere
	TVector4 vecRaise;
	vecRaise.Multiply( vecUp, m_fSphereRaise );

	TVector4 vecCenter( vecRaise.x + m_vPosition.x, vecRaise.y + m_vPosition.y, vecRaise.z + m_vPosition.z, 1.0f );
	TFLOAT   fDistance = a_rfDistance;
	TBOOL    bResult;

	if ( a_pObstacle->GetHeightScale() <= 0.0f )
	{
		TVector4 vecObstacleSphere( vecObstaclePosition.x, vecObstaclePosition.y, vecObstaclePosition.z, a_pObstacle->GetObjectRadius() );
		TVector4 vecSphere( vecCenter.x, vecCenter.y, vecCenter.z, m_fRadius );

		bResult = TMath::SphereVsSphere( vecSphere, vecObstacleSphere );
	}
	else
	{
		TFLOAT fObstacleHeight = a_pObstacle->GetHeightScale();
		TFLOAT fObstacleRadius = a_pObstacle->GetObjectRadius();

		bResult = TMath::SphereVsCapsule( vecCenter, m_fRadius, vecObstaclePosition, fObstacleRadius, vecUp, fObstacleHeight, fDistance, a_rNormal, 0.001f );
	}

	if ( bResult != TTRUE )
		return TFALSE;

	if ( a_pObstacle->GetModelInstance() )
	{
		TVector4 vecSphere( a_rcCenter.x, a_rcCenter.y, a_rcCenter.z, m_fRadius );

#ifdef BARNYARD_LATEST
		// During the camera checks only the TOSHICGROUP_CAMERA triangles of the objects are tested
		if ( ACollisionObjectMesh::IsCameraCollision() )
			return a_pObstacle->GetModelInstance()->CollideSphere( vecSphere, a_rfDistance, a_rNormal, TCollisionCommon::TOSHICGROUP_CAMERA, TCollisionCommon::TOSHICGROUPFLAG_NONE );
#endif

		return a_pObstacle->GetModelInstance()->CollideSphere( vecSphere, a_rfDistance, a_rNormal, -1, TCollisionCommon::TOSHICGROUPFLAG_NONE );
	}

	a_rfDistance = fDistance;
	return bResult;
}

// $Barnyard: FUNCTION 0061c770
const TSphere& ACollisionObjectSimple::GetObjectSphere()
{
	return *TREINTERPRETCAST( const TSphere*, &m_vPosition );
}

const TVector4& ACollisionObjectSimple::GetObjectPosition()
{
	// NOTE: retail uses GetObjectSphere (0061c770) for this slot too, w is the radius
	return *TREINTERPRETCAST( const TVector4*, &m_vPosition );
}

// $Barnyard: FUNCTION 0061c780
TFLOAT ACollisionObjectSimple::GetObjectRadius()
{
	return m_fRadius;
}

void ACollisionObjectSimple::GetVelocity( TVector4& a_rOutVector )
{
	// 0061d680, shared
	a_rOutVector = TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 0061bfc0
TFLOAT ACollisionObjectSimple::GetHeightScale()
{
	return m_fHeightScale;
}

// $Barnyard: FUNCTION 0061e7e0
TBOOL ACollisionObjectSimple::UpdateHash( TFLOAT a_fRadius )
{
	if ( a_fRadius != m_fRadius )
	{
		m_fRadius = a_fRadius;
		ACollisionObject::CreateHash();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0061c080
void ACollisionObjectSimple::TranslateBy( const TVector4& a_rDirection, TFLOAT a_fLength )
{
	TVector4 vecTranslation;
	vecTranslation.Multiply( a_rDirection, a_fLength );

	m_vPosition.x += vecTranslation.x;
	m_vPosition.y += vecTranslation.y;
	m_vPosition.z += vecTranslation.z;
}

// $Barnyard: FUNCTION 0052de80
TBOOL ACollisionObjectSimple::SetSphereRaise( TFLOAT a_fSphereRaise )
{
	m_fSphereRaise = a_fSphereRaise;
	return TFALSE;
}

// $Barnyard: FUNCTION 0061c010
TBOOL ACollisionObjectSimple::UpdateHash( TFLOAT a_fRadius, const Toshi::TVector3& a_rcPosition )
{
	TBOOL bRadiusChanged = a_fRadius != m_fRadius;

	if ( bRadiusChanged )
		m_fRadius = a_fRadius;

	if ( !m_vPosition.Equals( a_rcPosition ) )
	{
		m_vPosition = a_rcPosition;
		ACollisionObject::CreateHash();
	}
	else if ( bRadiusChanged )
	{
		ACollisionObject::CreateHash();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0061bfd0
void ACollisionObjectSimple::UpdateHash( const Toshi::TVector3& a_rcPosition )
{
	if ( !m_vPosition.Equals( a_rcPosition ) )
	{
		m_vPosition = a_rcPosition;
		ACollisionObject::CreateHash();
	}
}
