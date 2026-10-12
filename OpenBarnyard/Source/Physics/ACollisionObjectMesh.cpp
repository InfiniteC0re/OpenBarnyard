#include "pch.h"
#include "ACollisionObjectMesh.h"
#include "ACollisionModelSet.h"
#include "ACollisionModelInstance.h"

#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0061da40, CreateObject 0061ded0, CreateObjectInPlace 0061df30
TDEFINE_CLASS( ACollisionObjectMesh );

// $Barnyard: FUNCTION 0061da50
ACollisionObjectMesh::ACollisionObjectMesh()
    : m_pModelSet( TNULL )
    , m_pSceneObject( TNULL )
    , m_oModelSphere( 0.0f, 0.0f, 0.0f, 1.0f )
    , m_vecWorldPosition( TVector4::VEC_ZERO )
    , m_fWorldRadius( 1.0f )
    , m_fMeshHeightScale( 0.0f )
{
	m_eType = TYPE_MESH;
}

// $Barnyard: FUNCTION 0061df90
// $Barnyard: FUNCTION 0061e020
// $Barnyard: FUNCTION 0061dec0
// 005dd560: unwind thunk destroying AGateManager::m_oTomatoCollision (add ecx, 0xc; jmp 0061df90)
// $Barnyard: FUNCTION 005dd560
ACollisionObjectMesh::~ACollisionObjectMesh()
{
	// NOTE: 0061dec0 is the this-adjusting thunk of the TObject vtable
	if ( m_pModelInstance )
	{
		TFree( m_pModelInstance );
		m_pModelInstance = TNULL;
	}

	if ( m_pModelSet )
	{
		delete m_pModelSet;
		m_pModelSet = TNULL;
	}
}

// $Barnyard: FUNCTION 0061db30
TBOOL ACollisionObjectMesh::Create( TSceneObject* a_pSceneObject )
{
	if ( m_pSceneObject == TNULL )
	{
		m_pSceneObject = a_pSceneObject;

		TModel*        pModel   = a_pSceneObject->GetInstance()->GetModel();
		const TSphere& rcSphere = pModel->GetLOD( 0 ).BoundingSphere;

		m_vecWorldPosition = TVector4( rcSphere.GetOrigin().x, rcSphere.GetOrigin().y, rcSphere.GetOrigin().z, 1.0f );
		m_fWorldRadius     = rcSphere.GetRadius();
		m_oModelSphere     = TSphere( m_vecWorldPosition.AsVector3(), m_fWorldRadius );

		// NOTE: retail reads the transform object's matrix directly (AVisibleUnit::Create switches it to Mode::Matrix)
		const TMatrix44& rcTransform = *TREINTERPRETCAST( const TMatrix44*, &a_pSceneObject->GetTransform() );
		TVector4         vecOrigin   = m_vecWorldPosition;
		TMatrix44::TransformVector( m_vecWorldPosition, rcTransform, vecOrigin );

		for ( TINT i = 0; i < pModel->GetNumCollisionMeshes(); i++ )
		{
			// NOTE: retail creates a model set and a model instance for every collision mesh of the model,
			// all but the last ones leak (ACollisionModelSet::Create handles every mesh anyway)
			m_pModelSet = new ACollisionModelSet();
			m_pModelSet->Create( pModel );

			m_pModelInstance = new ACollisionModelInstance();
			m_pModelInstance->Create( m_pModelSet, m_pSceneObject );
		}

		Activate();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0061dac0
void ACollisionObjectMesh::UpdateWorldSphere()
{
	if ( !m_pSceneObject )
		return;

	const TSphere& rcSphere = m_pSceneObject->GetInstance()->GetModel()->GetLOD( 0 ).BoundingSphere;

	m_vecWorldPosition = TVector4( rcSphere.GetOrigin().x, rcSphere.GetOrigin().y, rcSphere.GetOrigin().z, 1.0f );
	m_fWorldRadius     = rcSphere.GetRadius();
	m_oModelSphere     = TSphere( m_vecWorldPosition.AsVector3(), m_fWorldRadius );

	// NOTE: retail reads the transform object's matrix directly (see Create)
	const TMatrix44& rcTransform = *TREINTERPRETCAST( const TMatrix44*, &m_pSceneObject->GetTransform() );
	TVector4         vecOrigin   = m_vecWorldPosition;
	TMatrix44::TransformVector( m_vecWorldPosition, rcTransform, vecOrigin );

	CreateHash();
}

// $Barnyard: FUNCTION 0061dc70
TBOOL ACollisionObjectMesh::CollideRay( const ARayData& a_rcRay, ACollisionHit& a_rHit )
{
	if ( !m_pSceneObject->IsSkeletonUpdateEnabled() )
		return TFALSE;

	return CollideRay( a_rcRay.vOrigin, a_rcRay.vDirection, a_rcRay.fLength, a_rHit );
}

// $Barnyard: FUNCTION 0061de30
TBOOL ACollisionObjectMesh::CollideSphere( ACollisionObject* a_pObstacle, const TVector4& a_rcCenter, TFLOAT& a_rfDistance, TVector4& a_rNormal )
{
	if ( !m_pSceneObject->IsSkeletonUpdateEnabled() )
		return TFALSE;

	TVector4 vecSphere( a_rcCenter.x, a_rcCenter.y, a_rcCenter.z, m_fWorldRadius );
	return a_pObstacle->GetModelInstance()->CollideSphere( vecSphere, a_rfDistance, a_rNormal, -1, TCollisionCommon::TOSHICGROUPFLAG_NONE );
}

// $Barnyard: FUNCTION 005748d0
const TSphere& ACollisionObjectMesh::GetObjectSphere()
{
	return m_oModelSphere;
}

// $Barnyard: FUNCTION 0061d660
const TVector4& ACollisionObjectMesh::GetObjectPosition()
{
	return m_vecWorldPosition;
}

// $Barnyard: FUNCTION 0061dea0
TFLOAT ACollisionObjectMesh::GetObjectRadius()
{
	return m_fWorldRadius;
}

// $Barnyard: FUNCTION 0061deb0
TFLOAT ACollisionObjectMesh::GetHeightScale()
{
	return m_fMeshHeightScale;
}

// $Barnyard: FUNCTION 0061e040
TBOOL ACollisionObjectMesh::CollideRay( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, TFLOAT a_fMaxDistance, ACollisionHit& a_rHit )
{
	if ( !m_pSceneObject->IsSkeletonUpdateEnabled() )
		return TFALSE;

	ACollisionModelRayHitList oHitList;
	TBOOL                     bResult = m_pModelInstance->CollideRay( a_rcRayOrigin, a_rcRayDir, a_fMaxDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_NONE, TTRUE );

	// Find the closest hit
	TINT iClosestHit = -1;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		if ( oHitList.aHits[ i ].fDistance <= a_fMaxDistance )
		{
			a_fMaxDistance = oHitList.aHits[ i ].fDistance;
			iClosestHit    = i;
		}
	}

	if ( iClosestHit == -1 )
		return TFALSE;

	ACollisionModelRayHitList::Hit& rHit = oHitList.aHits[ iClosestHit ];
	a_rHit.fDistance                     = rHit.fDistance;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail passes the index of the hit as the index of the collision model
	TModelCollisionData* pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );
#else
	TModelCollisionData* pCollData = rHit.pInstance->GetCollisionData( iClosestHit );
#endif

	a_rHit.iCollGroup = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).eCollGroup;
	return bResult;
}

// $Barnyard: FUNCTION 0061dd20
// $BarnyardWiiJP: FUNCTION 803c6cf8
TBOOL ACollisionObjectMesh::SlideSphereAlongModel( TFLOAT& a_rfDistance, const TVector4& a_rcDirection, void* a_pUnused, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT a_fSpeedScale, TVector4& a_rOutPosition )
{
	if ( !m_pSceneObject->IsSkeletonUpdateEnabled() )
		return TFALSE;

	TVector4 vecPosition = m_vecWorldPosition;

#ifdef BARNYARD_LATEST
	// NOTE: the latest version passes the camera ignore groups as a float in place of the minimum slope
	// (the other functions take the groups at this position). Kept as is, nothing slides during the camera checks.
	if ( IsCameraCollision() )
		return m_pModelInstance->SlideSweptSphere( vecPosition, m_fWorldRadius, a_rcDirection, a_rfDistance, a_fMaxStepHeight, a_fSpeedScale, a_rfStepHeight, a_rOutPosition, TFLOAT( CAMERA_IGNORE_GROUPS ), 0.4f );
#endif

	return m_pModelInstance->SlideSweptSphere( vecPosition, m_fWorldRadius, a_rcDirection, a_rfDistance, a_fMaxStepHeight, a_fSpeedScale, a_rfStepHeight, a_rOutPosition, -0.13f, 0.4f );
}

// $Barnyard: FUNCTION 0061dcb0
// $BarnyardWiiJP: FUNCTION 803c6c2c
TBOOL ACollisionObjectMesh::SweepSphereAgainstModel( TFLOAT& a_rfDistance, const TVector4& a_rcDirection, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration )
{
	if ( !m_pSceneObject->IsSkeletonUpdateEnabled() )
		return TFALSE;

	TVector4 vecPosition = m_vecWorldPosition;

#ifdef BARNYARD_LATEST
	if ( IsCameraCollision() )
		return m_pModelInstance->CollideSweptSphere( vecPosition, m_fWorldRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, a_rfPenetration, CAMERA_IGNORE_GROUPS );
#endif

	return m_pModelInstance->CollideSweptSphere( vecPosition, m_fWorldRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, a_rfPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );
}

// $Barnyard: FUNCTION 0061ddb0
// $BarnyardWiiJP: FUNCTION 803c6dbc
TBOOL ACollisionObjectMesh::SweepObjectAgainstModel( ACollisionObject* a_pObject, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal )
{
	if ( !m_pSceneObject->IsSkeletonUpdateEnabled() )
		return TFALSE;

	// NOTE: the result is unused
	a_pObject->GetObjectPosition();

	TVector4 vecPosition = a_rcCenter;

	// NOTE: retail passes a local and the stack slot of a_pObject as the step height and penetration outputs
	TFLOAT fStepHeight;
	TFLOAT fPenetration;

#ifdef BARNYARD_LATEST
	if ( IsCameraCollision() )
		return a_pObject->GetModelInstance()->CollideSweptSphere( vecPosition, m_fWorldRadius, a_rcDirection, a_rfDistance, a_rNormal, 0.0f, fStepHeight, fPenetration, CAMERA_IGNORE_GROUPS );
#endif

	return a_pObject->GetModelInstance()->CollideSweptSphere( vecPosition, m_fWorldRadius, a_rcDirection, a_rfDistance, a_rNormal, 0.0f, fStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );
}
