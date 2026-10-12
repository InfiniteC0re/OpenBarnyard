#include "pch.h"
#include "ACollisionObjectModel.h"
#include "ACollisionModelInstance.h"

#include <Math/TMath.h>
#include <Toshi/TSceneObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0061e140, CreateObject 0061ea60, CreateObjectInPlace 0061ead0
TDEFINE_CLASS( ACollisionObjectModel );

// $Barnyard: FUNCTION 0061e810
ACollisionObjectModel::ACollisionObjectModel()
    : m_vDirection( TVector4::VEC_NEGY )
    , m_fLength( 0.0f )
{
	m_eType = TYPE_SIMPLE;
}

// $Barnyard: FUNCTION 0061e8a0
// $Barnyard: FUNCTION 0061edf0
// $Barnyard: FUNCTION 0061e8b0
ACollisionObjectModel::~ACollisionObjectModel()
{
	if ( IsCollisionObjectLinked() )
		UnlinkCollisionObject();
}

// $Barnyard: FUNCTION 0061e730
TBOOL ACollisionObjectModel::Create( const TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fLength, const TVector4& a_rcDirection, TUINT a_eTypeFlags, ACollisionModelInstance* a_pModelInstance, const TTransformObject* a_pTransform )
{
	m_pModelInstance = a_pModelInstance;

	if ( a_pTransform )
		m_oTransform = *a_pTransform;
	else
		m_oTransform.SetMatrix( TMatrix44::IDENTITY );

	m_fRadius    = a_fRadius;
	m_fLength    = a_fLength;
	m_vDirection = a_rcDirection;
	m_vPosition  = a_rcPosition.AsVector3();

	if ( a_eTypeFlags )
		m_eType = TYPE( m_eType | a_eTypeFlags );

	Activate();
	return TTRUE;
}

void ACollisionObjectModel::UseTransform()
{
	if ( m_pModelInstance )
		m_pModelInstance->SetTransformObject( &m_oTransform );
}

// $Barnyard: FUNCTION 0061ee10
TBOOL ACollisionObjectModel::CollideRay( const ARayData& a_rcRay, ACollisionHit& a_rHit )
{
	UseTransform();
	return CollideRayImpl( a_rcRay, a_rHit );
}

// $Barnyard: FUNCTION 0061e990
TBOOL ACollisionObjectModel::CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	UseTransform();
	return CollideSweptSphereImpl( a_pObstacle, a_rfDistance, a_rcDirection, a_rcCenter, a_rNormal, a_fMaxStepHeight, a_rfStepHeight );
}

// $Barnyard: FUNCTION 0061e920
TBOOL ACollisionObjectModel::CollideSweptSphereNoNormal( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	UseTransform();

	// NOTE: the discarded normal starts as the position
	TVector4 vecNormal( m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.0f );
	return CollideSweptSphereImpl( a_pObstacle, a_rfDistance, a_rcDirection, a_rcCenter, vecNormal, a_fMaxStepHeight, a_rfStepHeight );
}

// $Barnyard: FUNCTION 0061e9d0
TBOOL ACollisionObjectModel::CollideSphere( ACollisionObject* a_pObstacle, const TVector4& a_rcCenter, TFLOAT& a_rfDistance, TVector4& a_rNormal )
{
	UseTransform();
	return CollideSphereImpl( a_pObstacle, a_rcCenter, a_rfDistance, a_rNormal );
}

// $Barnyard: FUNCTION 0061e7b0
void ACollisionObjectModel::UpdateTransform()
{
	if ( m_pModelInstance )
	{
		// NOTE: retail copies the transform over the one of the scene object directly
		m_pModelInstance->GetSceneObject()->GetTransform() = m_oTransform;
		m_pModelInstance->SetTransformObject( &m_oTransform );
		m_pModelInstance->Update();
	}
}

// $Barnyard: FUNCTION 005dfaf0
TFLOAT ACollisionObjectModel::GetRadius()
{
	// NOTE: retail jumps to a method of the capsule (m_vPosition, m_fRadius, m_vDirection, m_fLength) at 0061e710
	return ( m_vDirection.MagnitudeXZ() * m_fLength * 0.5f + m_fRadius ) * 1.5f;
}

// $Barnyard: FUNCTION 005dfae0
// $Barnyard: FUNCTION 0061e650
const TVector4& ACollisionObjectModel::GetPosition()
{
	// NOTE: retail jumps to a method of the capsule (0061e650) that returns its centre
	static TVector4 s_vCentre;

	TVector4 vHalfDirection;
	vHalfDirection.Divide( m_vDirection, 2.0f );

	s_vCentre.x = vHalfDirection.x + m_vPosition.x;
	s_vCentre.y = vHalfDirection.y + m_vPosition.y;
	s_vCentre.z = vHalfDirection.z + m_vPosition.z;
	s_vCentre.w = 1.0f;

	return s_vCentre;
}

const TSphere& ACollisionObjectModel::GetObjectSphere()
{
	// NOTE: 0061c770, shared with ACollisionObjectSimple
	return *TREINTERPRETCAST( const TSphere*, &m_vPosition );
}

const TVector4& ACollisionObjectModel::GetObjectPosition()
{
	// NOTE: retail uses GetObjectSphere (0061c770) for this slot too, w is the radius
	return *TREINTERPRETCAST( const TVector4*, &m_vPosition );
}

TFLOAT ACollisionObjectModel::GetObjectRadius()
{
	// NOTE: 0061c780, shared with ACollisionObjectSimple
	return m_fRadius;
}

void ACollisionObjectModel::GetVelocity( TVector4& a_rOutVector )
{
	// NOTE: 0061d680, shared with ACollisionObjectSimple
	a_rOutVector = TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 005dfb00
TFLOAT ACollisionObjectModel::GetHeightScale()
{
	return m_fLength * m_vDirection.y;
}

TBOOL ACollisionObjectModel::UpdateHash( TFLOAT a_fRadius )
{
	// NOTE: 0061e7e0, shared with ACollisionObjectSimple
	if ( a_fRadius != m_fRadius )
	{
		m_fRadius = a_fRadius;
		ACollisionObject::CreateHash();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0061ea10
TBOOL ACollisionObjectModel::ReciprocateSweptSphere( ACollisionObject* a_pObject, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	UseTransform();
	return ReciprocateSweptSphereImpl( a_pObject->GetObjectRadius(), a_rfDistance, a_rcDirection, a_rcCenter, a_rNormal, a_fMaxStepHeight, a_rfStepHeight );
}

void ACollisionObjectModel::TranslateBy( const TVector4& a_rDirection, TFLOAT a_fLength )
{
	// NOTE: 0061c080, shared with ACollisionObjectSimple
	TVector4 vecTranslation;
	vecTranslation.Multiply( a_rDirection, a_fLength );

	m_vPosition.x += vecTranslation.x;
	m_vPosition.y += vecTranslation.y;
	m_vPosition.z += vecTranslation.z;
}

TBOOL ACollisionObjectModel::UpdateHash( TFLOAT a_fRadius, const TVector3& a_rcPosition )
{
	// NOTE: 0061c010, shared with ACollisionObjectSimple
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

// $Barnyard: FUNCTION 0061eb30
TBOOL ACollisionObjectModel::CollideRayImpl( const ARayData& a_rcRay, ACollisionHit& a_rHit )
{
	TFLOAT fDistance     = a_rcRay.fLength;
	a_rHit.fDistance     = a_rcRay.fLength;
	TVector4 vecPosition = TVector4( m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.0f );
	a_rHit.pObject       = TNULL;

	// Rays starting or ending inside of the capsule always hit the bounds of the model
	TBOOL bInside = TFALSE;

	if ( m_pModelInstance )
	{
		bInside = TMath::IsPointInCapsule( a_rcRay.vOrigin, vecPosition, m_vDirection, m_fRadius, m_fLength );

		if ( !bInside )
		{
			TVector4 vecOffset;
			vecOffset.Multiply( a_rcRay.vDirection, a_rcRay.fLength );

			TVector4 vecEnd;
			vecEnd.Add( a_rcRay.vOrigin, vecOffset );

			bInside = TMath::IsPointInCapsule( vecEnd, vecPosition, m_vDirection, m_fRadius, m_fLength );
		}
	}

	if ( bInside != TTRUE )
	{
		if ( !TMath::RayVsCapsule( a_rcRay.vOrigin, a_rcRay.vDirection, vecPosition, m_fRadius, m_vDirection, m_fLength, fDistance, a_rHit.vNormal, 0.001f ) )
			return TFALSE;
	}

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

// $Barnyard: FUNCTION 0061e150
TBOOL ACollisionObjectModel::CollideSweptSphereImpl( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	const TVector4& rcObstaclePosition = a_pObstacle->GetObjectPosition();

	TVector4 vecObstaclePosition( rcObstaclePosition.x, rcObstaclePosition.y, rcObstaclePosition.z, 1.0f );
	TFLOAT   fDistance = a_rfDistance;
	TVector4 vecUp     = TVector4::VEC_NEGY;

	TFLOAT fObstacleHeight = a_pObstacle->GetHeightScale();
	TFLOAT fObstacleRadius = a_pObstacle->GetObjectRadius();

	if ( TMath::SweptSphereVsCapsule( a_rcCenter, m_fRadius, a_rcDirection, vecObstaclePosition, fObstacleRadius, vecUp, fObstacleHeight, fDistance, a_rNormal, 0.001f ) != TTRUE )
		return TFALSE;

	if ( a_pObstacle->GetModelInstance() )
	{
		TVector4 vecCenter = a_rcCenter;

		// NOTE: retail passes the stack slot of a_rfDistance as the penetration output
		TFLOAT fPenetration;
		return a_pObstacle->GetModelInstance()->CollideSweptSphere( vecCenter, m_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );
	}

	if ( fDistance < 0.0f )
	{
		// Inside of the obstacle, push out on the XZ plane
		TVector4 vecOffset( a_rcCenter.x - vecObstaclePosition.x, 0.0f, a_rcCenter.z - vecObstaclePosition.z, a_rcCenter.w );
		TFLOAT   fMagSq = vecOffset.MagnitudeSq();

		if ( fMagSq > 0.0f )
		{
			TFLOAT fMag = TMath::Sqrt( fMagSq );
			vecOffset.Divide( fMag );
			a_rNormal = vecOffset;

			fDistance    = fMag - ( a_pObstacle->GetObjectRadius() + m_fRadius ) - 0.01f;
			a_rfDistance = fDistance;
			return TTRUE;
		}
	}

	a_rfDistance = fDistance;
	return TTRUE;
}

// $Barnyard: FUNCTION 0061e340
TBOOL ACollisionObjectModel::CollideSphereImpl( ACollisionObject* a_pObstacle, const TVector4& a_rcCenter, TFLOAT& a_rfDistance, TVector4& a_rNormal )
{
	const TVector4& rcObstaclePosition = a_pObstacle->GetObjectPosition();

	TVector4 vecObstaclePosition( rcObstaclePosition.x, rcObstaclePosition.y, rcObstaclePosition.z, 1.0f );
	TVector4 vecCenter = a_rcCenter;
	TFLOAT   fDistance = a_rfDistance;

	TFLOAT fObstacleHeight = a_pObstacle->GetHeightScale();
	TFLOAT fObstacleRadius = a_pObstacle->GetObjectRadius();

	TBOOL bResult = TMath::SphereVsCapsule( vecCenter, m_fRadius, vecObstaclePosition, fObstacleRadius, TVector4::VEC_NEGY, fObstacleHeight, fDistance, a_rNormal, 0.001f );

	if ( bResult != TTRUE )
		return TFALSE;

	if ( a_pObstacle->GetModelInstance() )
	{
		TVector4 vecSphere( a_rcCenter.x, a_rcCenter.y, a_rcCenter.z, m_fRadius );
		return a_pObstacle->GetModelInstance()->CollideSphere( vecSphere, a_rfDistance, a_rNormal, -1, TCollisionCommon::TOSHICGROUPFLAG_NONE );
	}

	a_rfDistance = fDistance;
	return bResult;
}

// $Barnyard: FUNCTION 0061e450
TBOOL ACollisionObjectModel::ReciprocateSweptSphereImpl( TFLOAT a_fRadius, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	TVector4 vecOriginalNormal = a_rNormal;
	TFLOAT   fDistance         = a_rfDistance;
	TVector4 vecPosition( m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.0f );

	// The bounds
	if ( TMath::SweptSphereVsCapsule( a_rcCenter, a_fRadius, a_rcDirection, vecPosition, m_fRadius, m_vDirection, m_fLength, fDistance, a_rNormal, 0.001f ) != TTRUE )
		return TFALSE;

	if ( !m_pModelInstance )
	{
		// The capsule is the collision
		if ( fDistance == a_rfDistance )
			return TFALSE;

		// Don't push down
		if ( a_rNormal.y > 0.0f )
		{
			a_rNormal.y = 0.0f;
			a_rNormal.Normalise();
		}

		a_rfDistance = fDistance;
		return TTRUE;
	}

	// The collision model
	a_rNormal          = vecOriginalNormal;
	TVector4 vecCenter = a_rcCenter;

	// NOTE: retail passes the stack slot of a_fRadius as the penetration output
	TFLOAT fPenetration;
	TBOOL  bResult;

	if ( a_rfDistance >= 0.0f )
	{
		if ( m_oTransform.GetScale().x == 1.0f )
			bResult = m_pModelInstance->CollideSweptSphere( vecCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );
		else
			bResult = m_pModelInstance->CollideSweptSphere( vecCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration );
	}
	else
	{
		a_rfDistance = -a_rfDistance;

		if ( m_oTransform.GetScale().x == 1.0f )
			bResult = m_pModelInstance->CollideSweptSphere( vecCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );
		else
			bResult = m_pModelInstance->CollideSweptSphere( vecCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration );

		a_rfDistance = -a_rfDistance;
	}

	return bResult;
}
