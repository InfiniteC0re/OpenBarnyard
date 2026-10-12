#include "pch.h"
#include "ACollisionModel.h"
#include "ACollisionModelInstance.h"

#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00618b40
ACollisionModel::ACollisionModel()
    : m_pModel( TNULL )
{
}

// $Barnyard: FUNCTION 0061a7b0
// $Barnyard: FUNCTION 00618fe0
ACollisionModel::~ACollisionModel()
{
}

// $Barnyard: FUNCTION 00618cb0
TBOOL ACollisionModel::Create( TINT a_iMeshIndex, Toshi::TModel* a_pModel )
{
	m_iMeshIndex = a_iMeshIndex;
	m_pModel     = a_pModel;

	TModelCollisionData* pCollMesh = &m_pModel->GetCollisionMeshes()[ a_iMeshIndex ];

	if ( pCollMesh->GetNumVertices() > 0 && pCollMesh->GetNumIndices() > 0 )
	{
		m_aIgnoreGroups[ 0 ] = -1;
		m_aIgnoreGroups[ 1 ] = -1;
		m_vecTreeCenter      = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		m_fAverageExtent     = TMath::MAXFLOAT;

		// We are always having AABBNoLeafTree as the tree structure
		Opcode::AABBNoLeafTree*       pCollTree = a_pModel->CastSymbol<Opcode::AABBNoLeafTree>( "CollisionTree" );
		const Opcode::AABBNoLeafNode* pAABBNode = TNULL;

		TBOOL bSuccess;

		if ( pCollTree )
		{
			Opcode::OPCODECREATE oCreate;
			m_MeshInterface.SetNbTriangles( pCollMesh->GetNumIndices() / 3 );
			m_MeshInterface.SetNbVertices( pCollMesh->GetNumVertices() );
			m_MeshInterface.SetPointers(
			    TREINTERPRETCAST( IceMaths::IndexedTriangle*, pCollMesh->GetIndices() ),
			    TREINTERPRETCAST( IceMaths::Point*, pCollMesh->GetVertices() )
			);
			oCreate.mIMesh        = &m_MeshInterface;
			oCreate.mNoLeaf       = TTRUE;
			oCreate.mQuantized    = TFALSE;
			oCreate.mKeepOriginal = TFALSE;
			oCreate.mCanRemap     = TFALSE;
			bSuccess              = m_OpcodeModel.BuildGivenTree( oCreate, pCollTree );
			pAABBNode             = pCollTree->GetNodes();
		}
		else
		{
			Opcode::OPCODECREATE oCreate;
			m_MeshInterface.SetNbTriangles( pCollMesh->GetNumIndices() / 3 );
			m_MeshInterface.SetNbVertices( pCollMesh->GetNumVertices() );
			m_MeshInterface.SetPointers(
			    TREINTERPRETCAST( IceMaths::IndexedTriangle*, pCollMesh->GetIndices() ),
			    TREINTERPRETCAST( IceMaths::Point*, pCollMesh->GetVertices() )
			);
			oCreate.mIMesh        = &m_MeshInterface;
			oCreate.mNoLeaf       = TTRUE;
			oCreate.mQuantized    = TFALSE;
			oCreate.mKeepOriginal = TFALSE;
			oCreate.mCanRemap     = TFALSE;
			bSuccess              = m_OpcodeModel.Build( oCreate );
			pAABBNode             = m_OpcodeModel.GetTree()->GetNodes();
		}

		// Calculate bounding
		m_vecTreeCenter.x = pAABBNode->mAABB.mCenter.x;
		m_vecTreeCenter.y = pAABBNode->mAABB.mCenter.y;
		m_vecTreeCenter.z = pAABBNode->mAABB.mCenter.z;
		m_vecTreeCenter.w = 1.0f;
		m_fAverageExtent  = pAABBNode->mAABB.mExtents.Magnitude();

		return bSuccess;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00619170
TBOOL ACollisionModel::CollideRay( ACollisionModelInstance* a_pModelInstance, const Toshi::TVector4& a_rcRayOrigin, const Toshi::TVector4& a_rcRayDir, TFLOAT& a_rfMaxDistance, TCollisionCommon::TOSHICGROUPFLAG a_eCGroupsMask, TBOOL a_bCulling )
{
	// Get transform of the collision mesh attached to a bone
	TMatrix44 matBoneTransform;
	a_pModelInstance->GetTransformForBone( m_pModel->GetCollisionMeshes()[ m_iMeshIndex ].GetBoneID(), matBoneTransform );

	TFLOAT fScale = TMath::Max( 0.001f, a_pModelInstance->GetScale() );

	// Perform fast ray collision detection
	if ( !CollideRayFast( a_rcRayOrigin, a_rcRayDir, a_rfMaxDistance, matBoneTransform, fScale ) )
		return TFALSE;

	// Construct ray object
	IceMaths::Ray ray;
	if ( fScale == 1.0f )
	{
		// No need to do any math here
		ray.mOrig.x = a_rcRayOrigin.x;
		ray.mOrig.y = a_rcRayOrigin.y;
		ray.mOrig.z = a_rcRayOrigin.z;
		ray.mDir.x  = a_rcRayDir.x;
		ray.mDir.y  = a_rcRayDir.y;
		ray.mDir.z  = a_rcRayDir.z;
	}
	else
	{
		// OPCODE requires us to apply the scaling for it
		TMatrix44 matInverseTransform = matBoneTransform;
		matInverseTransform.Invert( matBoneTransform );

		TVector4 vecTransformedOrigin;
		TMatrix44::TransformVector( vecTransformedOrigin, matInverseTransform, a_rcRayOrigin );

		TVector4 vecTransformedDir;
		TMatrix44::RotateVector( vecTransformedDir, matInverseTransform, a_rcRayDir );
		vecTransformedDir.Normalise();

		ray.mOrig.x = vecTransformedOrigin.x;
		ray.mOrig.y = vecTransformedOrigin.y;
		ray.mOrig.z = vecTransformedOrigin.z;
		ray.mDir.x  = vecTransformedDir.x;
		ray.mDir.y  = vecTransformedDir.y;
		ray.mDir.z  = vecTransformedDir.z;
	}

	// Setup collider settings
	Opcode::RayCollider& rayCollider = s_pCache->m_RayCollider;
	rayCollider.SetClosestHit( TFALSE );
	rayCollider.SetFirstContact( TFALSE );
	rayCollider.SetTemporalCoherence( TFALSE );
	rayCollider.SetPrimitiveTests( TTRUE );
	rayCollider.SetCulling( a_bCulling );
	rayCollider.SetMaxDist( a_rfMaxDistance / fScale );
	rayCollider.SetDestination( &s_pCache->m_StabbedFaces );

	// Perform the collision check
	rayCollider.Collide(
	    ray,
	    m_OpcodeModel,
	    (IceMaths::Matrix4x4*)( ( fScale == 1.0f ) ? &matBoneTransform : &TMatrix44::IDENTITY ),
	    &s_pCache->m_uiRayColliderCache
	);

	if ( rayCollider.GetContactStatus() )
	{
		TModelCollisionData* pCollData = &m_pModel->GetCollisionMeshes()[ m_iMeshIndex ];

		TUINT  uiFaceIndex = -1;
		TFLOAT fDistance   = a_rfMaxDistance;
		udword nbFaces     = s_pCache->m_StabbedFaces.GetNbFaces();

		for ( udword i = 0; i < nbFaces; i++ )
		{
			const Opcode::CollisionFace* pFace = &s_pCache->m_StabbedFaces.GetFaces()[ i ];

			// Only faces closer than the closest one found so far
			if ( pFace->mDistance >= fDistance )
				continue;

			TINT iGroup = pCollData->GetGroupForTriangleImp( pFace->mFaceID );

			if ( iGroup != m_aIgnoreGroups[ 1 ] &&
			     iGroup != m_aIgnoreGroups[ 0 ] &&
			     ( a_eCGroupsMask & pCollData->GetCollGroup( iGroup ).GetGroupFlag() ) == 0 )
			{
				// This triangle is not ignored and matches the collision group
				uiFaceIndex = pFace->mFaceID;
				fDistance   = pFace->mDistance;
			}
		}

		if ( uiFaceIndex != -1 )
		{
			a_rfMaxDistance = fDistance * fScale;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00618ba0
TBOOL ACollisionModel::CollideRayFast( const Toshi::TVector4& a_rcRayOrigin, const Toshi::TVector4& a_rcRayDir, TFLOAT a_fMaxDistance, const Toshi::TMatrix44& a_rcModelTransform, TFLOAT a_fModelScale )
{
	TVector4 vecCenter = m_vecTreeCenter;
	TMatrix44::TransformVector( vecCenter, a_rcModelTransform, vecCenter );

	TFLOAT fNearIntersection, fFarIntersection;
	TINT   iNumIntersections = TMath::RayVsCircle3D( a_rcRayOrigin, a_rcRayDir, vecCenter, a_fModelScale * m_fAverageExtent, fNearIntersection, fFarIntersection );

	return iNumIntersections != 0 && fNearIntersection <= a_fMaxDistance;
}

// $Barnyard: FUNCTION 00619040
void ACollisionModel::CreateStaticCache()
{
	TASSERT( s_pCache == TNULL );

	s_pCache = new AOpcodeCache();

	constexpr TUINT MAX_PRIMITIVES = 256;

	s_pCache->m_AABBCache.TouchedPrimitives.SetSize( MAX_PRIMITIVES );
	s_pCache->m_AABBCache.TouchedPrimitives.SetGrowthFactor( 0.0f );
	s_pCache->m_SphereCache.TouchedPrimitives.SetSize( MAX_PRIMITIVES );
	s_pCache->m_SphereCache.TouchedPrimitives.SetGrowthFactor( 0.0f );
	s_pCache->m_LSSCache.TouchedPrimitives.SetSize( MAX_PRIMITIVES );
	s_pCache->m_LSSCache.TouchedPrimitives.SetGrowthFactor( 0.0f );
	s_pCache->m_OBBCache.TouchedPrimitives.SetSize( MAX_PRIMITIVES );
	s_pCache->m_OBBCache.TouchedPrimitives.SetGrowthFactor( 0.0f );

	Opcode::CollisionFace dummyFace;
	for ( TUINT i = 0; i < MAX_PRIMITIVES; i++ )
		s_pCache->m_StabbedFaces.AddFace( dummyFace );
}

// Returns the highest of three values
// $Barnyard: FUNCTION 00618e60
static const TFLOAT& Max3( const TFLOAT& a_rA, const TFLOAT& a_rB, const TFLOAT& a_rC )
{
	if ( a_rA <= a_rB )
		return ( a_rC <= a_rB ) ? a_rB : a_rC;
	else
		return ( a_rC <= a_rA ) ? a_rA : a_rC;
}

// $Barnyard: FUNCTION 00619140
void ACollisionModel::DestroyStaticCache()
{
	TASSERT( s_pCache != TNULL );

	if ( s_pCache )
		delete s_pCache;

	s_pCache = TNULL;
}

// $Barnyard: FUNCTION 00618e40
TModelCollisionData* ACollisionModel::GetCollisionData() const
{
	return &m_pModel->GetCollisionMeshes()[ m_iMeshIndex ];
}

// $Barnyard: FUNCTION 00618c30
TBOOL ACollisionModel::CollideSphereFast( const TVector4& a_rcSphere, const TMatrix44& a_rcModelTransform, TFLOAT a_fModelScale )
{
	TVector4 vecCenter = m_vecTreeCenter;
	TMatrix44::TransformVector( vecCenter, a_rcModelTransform, vecCenter );

	TFLOAT fRadius = a_fModelScale * m_fAverageExtent + a_rcSphere.w;
	return TVector4::DistanceSq( vecCenter, a_rcSphere ) <= fRadius * fRadius;
}

// $Barnyard: FUNCTION 0061a7d0
TBOOL ACollisionModel::CollideRay( ACollisionModelInstance* a_pModelInstance, const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, TFLOAT& a_rfMaxDistance, ACollisionModelRayHitList* a_pHitList, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bCulling )
{
	TMatrix44 matBoneTransform;
	a_pModelInstance->GetTransformForBone( GetCollisionData()->GetBoneID(), matBoneTransform );

	TFLOAT fScale = TMath::Max( a_pModelInstance->GetScale(), 0.001f );

	if ( !CollideRayFast( a_rcRayOrigin, a_rcRayDir, a_rfMaxDistance, matBoneTransform, fScale ) )
		return TFALSE;

	IceMaths::Ray ray;

	if ( fScale == 1.0f )
	{
		ray.mOrig.x = a_rcRayOrigin.x;
		ray.mOrig.y = a_rcRayOrigin.y;
		ray.mOrig.z = a_rcRayOrigin.z;
		ray.mDir.x  = a_rcRayDir.x;
		ray.mDir.y  = a_rcRayDir.y;
		ray.mDir.z  = a_rcRayDir.z;
	}
	else
	{
		// OPCODE requires us to apply the scaling for it
		TMatrix44 matInverseTransform = matBoneTransform;
		matInverseTransform.Invert( matInverseTransform );

		TVector4 vecTransformedOrigin;
		TMatrix44::TransformVector( vecTransformedOrigin, matInverseTransform, a_rcRayOrigin );

		TVector4 vecTransformedDir;
		TMatrix44::RotateVector( vecTransformedDir, matInverseTransform, a_rcRayDir );
		vecTransformedDir.Normalise();

		ray.mOrig.x = vecTransformedOrigin.x;
		ray.mOrig.y = vecTransformedOrigin.y;
		ray.mOrig.z = vecTransformedOrigin.z;
		ray.mDir.x  = vecTransformedDir.x;
		ray.mDir.y  = vecTransformedDir.y;
		ray.mDir.z  = vecTransformedDir.z;
	}

	TFLOAT fInvScale = 1.0f / fScale;

	// Setup collider settings
	Opcode::RayCollider& rayCollider = s_pCache->m_RayCollider;
	rayCollider.SetClosestHit( TFALSE );
	rayCollider.SetFirstContact( TFALSE );
	rayCollider.SetTemporalCoherence( TFALSE );
	rayCollider.SetPrimitiveTests( TTRUE );
	rayCollider.SetCulling( a_bCulling );
	rayCollider.SetMaxDist( fInvScale * a_rfMaxDistance );
	rayCollider.SetDestination( &s_pCache->m_StabbedFaces );

	rayCollider.Collide(
	    ray,
	    m_OpcodeModel,
	    (IceMaths::Matrix4x4*)( ( fScale == 1.0f ) ? &matBoneTransform : &TMatrix44::IDENTITY ),
	    &s_pCache->m_uiRayColliderCache
	);

	TBOOL bContact = rayCollider.GetContactStatus();

	if ( !bContact )
		return bContact;

	TModelCollisionData* pCollData = GetCollisionData();

	TFLOAT fClosestDistance = fInvScale * a_rfMaxDistance;
	TUINT  uiClosestFace    = TUINT( -1 );
	udword uiNumFaces       = s_pCache->m_StabbedFaces.GetNbFaces();

	for ( udword i = 0; i < uiNumFaces; i++ )
	{
		const Opcode::CollisionFace* pFace = &s_pCache->m_StabbedFaces.GetFaces()[ i ];

		if ( pFace->mDistance < fClosestDistance )
		{
			TINT iGroup = pCollData->GetGroupForTriangleImp( pFace->mFaceID );

			if ( iGroup != m_aIgnoreGroups[ 1 ] && iGroup != m_aIgnoreGroups[ 0 ] &&
			     ( pCollData->GetCollGroup( iGroup ).GetGroupFlag() & a_eIgnoreGroups ) == 0 )
			{
				fClosestDistance = pFace->mDistance;
				uiClosestFace    = pFace->mFaceID;
			}
		}
	}

	if ( uiClosestFace == TUINT( -1 ) )
		bContact = TFALSE;

	if ( a_pHitList )
	{
		if ( uiClosestFace != TUINT( -1 ) )
		{
			// Remove hits that are further than the closest one
			// NOTE: the index is incremented after removing an element, so the following hit isn't checked
			for ( TINT i = 0; i < a_pHitList->iNumHits; i++ )
			{
				if ( a_pHitList->aHits[ i ].fDistance > fClosestDistance )
					a_pHitList->Erase( i );
			}
		}

		// Add the faces that are closer than the closest valid one (faces of the ignored groups)
		for ( udword i = 0; i < uiNumFaces; i++ )
		{
			const Opcode::CollisionFace* pFace = &s_pCache->m_StabbedFaces.GetFaces()[ i ];

			if ( pFace->mDistance < fClosestDistance &&
			     ( pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( pFace->mFaceID ) ).GetGroupFlag() & a_eIgnoreGroups ) == 0 )
			{
				ACollisionModelRayHitList::Hit& rHit = a_pHitList->aHits[ a_pHitList->iNumHits++ ];
				rHit.pInstance                       = a_pModelInstance;
				rHit.iModel                          = m_iMeshIndex;
				rHit.uiFaceId                        = pFace->mFaceID;
				rHit.fDistance                       = pFace->mDistance;
				rHit.pTransform                      = a_pModelInstance->GetTransformObject();

				if ( a_pHitList->IsFull() )
					break;
			}
		}

		if ( !bContact )
			return bContact;

		if ( !a_pHitList->IsFull() )
		{
			ACollisionModelRayHitList::Hit& rHit = a_pHitList->aHits[ a_pHitList->iNumHits++ ];
			rHit.pInstance                       = a_pModelInstance;
			rHit.iModel                          = m_iMeshIndex;
			rHit.uiFaceId                        = uiClosestFace;
			rHit.fDistance                       = fClosestDistance;
			rHit.pTransform                      = a_pModelInstance->GetTransformObject();
		}
	}

	if ( bContact )
		a_rfMaxDistance = fClosestDistance * fScale;

	return bContact;
}

// $Barnyard: FUNCTION 00619c10
TBOOL ACollisionModel::CollideSphere( ACollisionModelInstance* a_pModelInstance, const TVector4& a_rcSphere, TFLOAT& a_rfDistance, TVector4& a_rNormal, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TMatrix44 matBoneTransform;
	a_pModelInstance->GetTransformForBone( GetCollisionData()->GetBoneID(), matBoneTransform );

	TFLOAT fScale = TMath::Max( a_pModelInstance->GetScale(), 0.001f );

	if ( !CollideSphereFast( a_rcSphere, matBoneTransform, fScale ) )
		return TFALSE;

	// Move the sphere to the model space
	TMatrix44 matInverseTransform = matBoneTransform;

	if ( fScale == 1.0f )
		matInverseTransform.InvertOrthogonal();
	else
		matInverseTransform.Invert( matInverseTransform );

	TVector4 vecCenter( a_rcSphere.x, a_rcSphere.y, a_rcSphere.z, 1.0f );
	TVector4 vecLocalCenter;
	TMatrix44::TransformVector( vecLocalCenter, matInverseTransform, vecCenter );

	TFLOAT fLocalRadius = a_rcSphere.w / fScale;

	IceMaths::Sphere sphere( IceMaths::Point( vecLocalCenter.x, vecLocalCenter.y, vecLocalCenter.z ), fLocalRadius );

	Opcode::SphereCollider& sphereCollider = s_pCache->m_SphereCollider;
	sphereCollider.SetFirstContact( TFALSE );
	sphereCollider.SetPrimitiveTests( TTRUE );
	sphereCollider.SetTemporalCoherence( TFALSE );
	sphereCollider.Collide( s_pCache->m_SphereCache, sphere, m_OpcodeModel, TNULL, TNULL );

	if ( sphereCollider.GetContactStatus() )
	{
		a_rfDistance = fLocalRadius;

		TModelCollisionData* pCollData = GetCollisionData();

		TBOOL         bHit               = TFALSE;
		udword        uiNumPrimitives    = sphereCollider.GetNbTouchedPrimitives();
		const udword* pTouchedPrimitives = sphereCollider.GetTouchedPrimitives();

		for ( udword i = 0; i < uiNumPrimitives; i++ )
		{
			udword uiFaceId   = pTouchedPrimitives[ i ];
			TINT   iCollGroup = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( uiFaceId ) ).eCollGroup;

			if ( ( a_iCollGroup == -1 || iCollGroup == a_iCollGroup ) && ( ( 1 << iCollGroup ) & a_eIgnoreGroups ) == 0 )
			{
				TVector3* apVertices[ 3 ];
				a_pModelInstance->GetCollisionData( m_iMeshIndex )->GetTriangleVertices( uiFaceId, apVertices );

				TPlane oPlane;
				TMath::TrianglePlane( oPlane, *apVertices[ 0 ], *apVertices[ 1 ], *apVertices[ 2 ] );

				if ( TMath::SphereVsPolygon( vecLocalCenter, fLocalRadius, a_rfDistance, a_rNormal, apVertices, oPlane, 3, 0.0001f ) )
					bHit = TTRUE;
			}
		}

		if ( bHit )
		{
			TMatrix44::RotateVector( a_rNormal, matBoneTransform, a_rNormal );

			if ( fScale != 1.0f )
			{
				a_rfDistance = fScale * a_rfDistance;
				a_rNormal.Normalise();
			}

			return TTRUE;
		}
	}

	a_rfDistance = fScale * a_rfDistance;
	return TFALSE;
}

// $Barnyard: FUNCTION 00619460
TBOOL ACollisionModel::CollideSphere( ACollisionModelInstance* a_pModelInstance, const TVector4& a_rcSphere, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TMatrix44 matBoneTransform;
	a_pModelInstance->GetTransformForBone( GetCollisionData()->GetBoneID(), matBoneTransform );

	TFLOAT fScale = TMath::Max( a_pModelInstance->GetScale(), 0.001f );

	if ( !CollideSphereFast( a_rcSphere, matBoneTransform, fScale ) )
		return TFALSE;

	IceMaths::Sphere sphere;

	if ( fScale == 1.0f )
	{
		// The model is transformed by the collider
		sphere.mCenter.Set( a_rcSphere.x, a_rcSphere.y, a_rcSphere.z );
		sphere.mRadius = a_rcSphere.w;
	}
	else
	{
		TMatrix44 matInverseTransform = matBoneTransform;
		matInverseTransform.Invert( matInverseTransform );

		TVector4 vecCenter( a_rcSphere.x, a_rcSphere.y, a_rcSphere.z, 1.0f );
		TVector4 vecLocalCenter;
		TMatrix44::TransformVector( vecLocalCenter, matInverseTransform, vecCenter );

		sphere.mCenter.Set( vecLocalCenter.x, vecLocalCenter.y, vecLocalCenter.z );
		sphere.mRadius = a_rcSphere.w / fScale;
	}

	Opcode::SphereCollider& sphereCollider = s_pCache->m_SphereCollider;
	sphereCollider.SetFirstContact( TFALSE );
	sphereCollider.SetPrimitiveTests( TTRUE );
	sphereCollider.SetTemporalCoherence( TFALSE );
	sphereCollider.Collide(
	    s_pCache->m_SphereCache,
	    sphere,
	    m_OpcodeModel,
	    TNULL,
	    (IceMaths::Matrix4x4*)( ( fScale == 1.0f ) ? &matBoneTransform : &TMatrix44::IDENTITY )
	);

	TBOOL bContact = sphereCollider.GetContactStatus();

	if ( bContact )
	{
		TModelCollisionData* pCollData          = GetCollisionData();
		udword               uiNumPrimitives    = sphereCollider.GetNbTouchedPrimitives();
		const udword*        pTouchedPrimitives = sphereCollider.GetTouchedPrimitives();

		if ( a_pHitList && !a_pHitList->IsFull() )
		{
			for ( udword i = 0; i < uiNumPrimitives; i++ )
			{
				udword uiFaceId   = pTouchedPrimitives[ i ];
				TINT   iCollGroup = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( uiFaceId ) ).eCollGroup;

				if ( ( a_iCollGroup == -1 || a_iCollGroup == iCollGroup ) && ( ( 1 << iCollGroup ) & a_eIgnoreGroups ) == 0 )
				{
					ACollisionModelRayHitList::Hit& rHit = a_pHitList->aHits[ a_pHitList->iNumHits++ ];
					rHit.pInstance                       = a_pModelInstance;
					rHit.iModel                          = m_iMeshIndex;
					rHit.uiFaceId                        = uiFaceId;
					rHit.fDistance                       = 0.0f;
					rHit.pTransform                      = a_pModelInstance->GetTransformObject();

					if ( a_pHitList->IsFull() )
						break;
				}
			}
		}
	}

	return bContact;
}

// $Barnyard: FUNCTION 00619720
TBOOL ACollisionModel::CollideBox( ACollisionModelInstance* a_pModelInstance, const TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TMatrix44 matBoneTransform;
	a_pModelInstance->GetTransformForBone( GetCollisionData()->GetBoneID(), matBoneTransform );

	TFLOAT fScale = TMath::Max( a_pModelInstance->GetScale(), 0.001f );

	// Bounding sphere of the box
	TVector4 vecSphere( a_rcBox.GetCentre().x, a_rcBox.GetCentre().y, a_rcBox.GetCentre().z, a_rcBox.m_fRadius );

	if ( !CollideSphereFast( vecSphere, matBoneTransform, fScale ) )
		return TFALSE;

	IceMaths::OBB box;

	if ( fScale == 1.0f )
	{
		// The model is transformed by the collider
		box.mCenter.Set( a_rcBox.GetCentre().x, a_rcBox.GetCentre().y, a_rcBox.GetCentre().z );
		box.mExtents.Set( a_rcBox.m_vExtents.x, a_rcBox.m_vExtents.y, a_rcBox.m_vExtents.z );

		for ( TINT i = 0; i < 3; i++ )
		{
			const TVector4& rcAxis = a_rcBox.GetAxis( i );
			box.mRot.m[ i ][ 0 ]   = rcAxis.x;
			box.mRot.m[ i ][ 1 ]   = rcAxis.y;
			box.mRot.m[ i ][ 2 ]   = rcAxis.z;
		}
	}
	else
	{
		TMatrix44 matInverseTransform = matBoneTransform;
		matInverseTransform.Invert( matInverseTransform );

		TVector4 vecLocalCenter;
		TMatrix44::TransformVector( vecLocalCenter, matInverseTransform, a_rcBox.GetCentre() );

		TVector4 avecLocalAxes[ 3 ];
		TMatrix44::RotateVector( avecLocalAxes[ 0 ], matInverseTransform, a_rcBox.GetAxis( 0 ) );
		TMatrix44::RotateVector( avecLocalAxes[ 1 ], matInverseTransform, a_rcBox.GetAxis( 1 ) );
		TMatrix44::RotateVector( avecLocalAxes[ 2 ], matInverseTransform, a_rcBox.GetAxis( 2 ) );

		box.mCenter.Set( vecLocalCenter.x, vecLocalCenter.y, vecLocalCenter.z );
		box.mExtents.Set( a_rcBox.m_vExtents.x, a_rcBox.m_vExtents.y, a_rcBox.m_vExtents.z );

		for ( TINT i = 0; i < 3; i++ )
		{
			box.mRot.m[ i ][ 0 ] = avecLocalAxes[ i ].x;
			box.mRot.m[ i ][ 1 ] = avecLocalAxes[ i ].y;
			box.mRot.m[ i ][ 2 ] = avecLocalAxes[ i ].z;
		}
	}

	Opcode::OBBCollider& obbCollider = s_pCache->m_OBBCollider;
	obbCollider.SetFirstContact( TFALSE );
	obbCollider.SetPrimitiveTests( TTRUE );
	obbCollider.SetTemporalCoherence( TFALSE );

	// NOTE: the OPCODE version of the original has no OBBCache argument
	obbCollider.Collide(
	    s_pCache->m_OBBCache,
	    box,
	    m_OpcodeModel,
	    TNULL,
	    (IceMaths::Matrix4x4*)( ( fScale != 1.0f ) ? &TMatrix44::IDENTITY : &matBoneTransform )
	);

	TBOOL bContact = obbCollider.GetContactStatus();

	if ( bContact )
	{
		TModelCollisionData* pCollData          = GetCollisionData();
		udword               uiNumPrimitives    = obbCollider.GetNbTouchedPrimitives();
		const udword*        pTouchedPrimitives = obbCollider.GetTouchedPrimitives();

		if ( a_pHitList && !a_pHitList->IsFull() )
		{
			for ( udword i = 0; i < uiNumPrimitives; i++ )
			{
				udword uiFaceId   = pTouchedPrimitives[ i ];
				TINT   iCollGroup = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( uiFaceId ) ).eCollGroup;

				if ( ( a_iCollGroup == -1 || a_iCollGroup == iCollGroup ) && ( ( 1 << iCollGroup ) & a_eIgnoreGroups ) == 0 )
				{
					ACollisionModelRayHitList::Hit& rHit = a_pHitList->aHits[ a_pHitList->iNumHits++ ];
					rHit.pInstance                       = a_pModelInstance;
					rHit.iModel                          = m_iMeshIndex;
					rHit.uiFaceId                        = uiFaceId;
					rHit.fDistance                       = 0.0f;

#ifdef BARNYARD_COMMUNITY_PATCH
					// NOTE: the original doesn't set the transform but the box queries of the physics use it (ACollisionModelRayHitList::Hit::GetTriangle)
					rHit.pTransform = a_pModelInstance->GetTransformObject();
#endif

					if ( a_pHitList->IsFull() )
						break;
				}
			}
		}
	}

	return bContact;
}

// $Barnyard: FUNCTION 00619ae0
TBOOL ACollisionModel::CollideSweptBox( ACollisionModelInstance* a_pModelInstance, const TOBBox& a_rcPrevBox, const TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	// Sphere around both positions of the box
	TVector4 vecMiddle(
	    a_rcBox.GetCentre().x + a_rcPrevBox.GetCentre().x,
	    a_rcBox.GetCentre().y + a_rcPrevBox.GetCentre().y,
	    a_rcBox.GetCentre().z + a_rcPrevBox.GetCentre().z,
	    a_rcPrevBox.GetCentre().w
	);
	vecMiddle.Multiply( vecMiddle, 0.5f );

	TVector4 vecMovement(
	    a_rcPrevBox.GetCentre().x - a_rcBox.GetCentre().x,
	    a_rcPrevBox.GetCentre().y - a_rcBox.GetCentre().y,
	    a_rcPrevBox.GetCentre().z - a_rcBox.GetCentre().z,
	    a_rcPrevBox.GetCentre().w
	);

	TVector4 vecSphere( vecMiddle.x, vecMiddle.y, vecMiddle.z, sqrtf( vecMovement.MagnitudeSq() ) + a_rcPrevBox.m_fRadius );

	return CollideSphere( a_pModelInstance, vecSphere, a_pHitList, a_iCollGroup, a_eIgnoreGroups );
}

// $Barnyard: FUNCTION 00619f30
TBOOL ACollisionModel::CollideSweptSphere( ACollisionModelInstance* a_pModelInstance, const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, TBOOL& a_rbEdgeHit, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TMatrix44 matBoneTransform;
	a_pModelInstance->GetTransformForBone( GetCollisionData()->GetBoneID(), matBoneTransform );

	TFLOAT fScale = TMath::Max( a_pModelInstance->GetScale(), 0.001f );

	// Bounding sphere of the whole movement
	TVector4 vecHalfMovement;
	vecHalfMovement.Multiply( a_rcDirection, a_rfDistance );
	vecHalfMovement.Multiply( 0.5f );

	TVector4 vecBounding;
	vecBounding.x = vecHalfMovement.x + a_rcCenter.x;
	vecBounding.y = vecHalfMovement.y + a_rcCenter.y;
	vecBounding.z = vecHalfMovement.z + a_rcCenter.z;
	vecBounding.w = a_rfDistance * 0.5f + a_fRadius;

	if ( !CollideSphereFast( vecBounding, matBoneTransform, fScale ) )
		return TFALSE;

	// Move everything to the model space
	TMatrix44 matInverseTransform = matBoneTransform;

	if ( fScale == 1.0f )
		matInverseTransform.InvertOrthogonal();
	else
		matInverseTransform.Invert( matInverseTransform );

	TVector4 vecLocalCenter;
	TMatrix44::TransformVector( vecLocalCenter, matInverseTransform, a_rcCenter );

	TVector4 vecLocalDirection;
	TMatrix44::RotateVector( vecLocalDirection, matInverseTransform, a_rcDirection );

	if ( fScale != 1.0f )
	{
		vecLocalDirection.Normalise();

		TFLOAT fInvScale = 1.0f / fScale;
		a_fRadius        = a_fRadius * fInvScale;
		a_rfDistance     = fInvScale * a_rfDistance;
		a_fMaxStepHeight = fInvScale * a_fMaxStepHeight;
	}

	TModelCollisionData* pCollData = GetCollisionData();

	Opcode::LSSCollider& lssCollider = s_pCache->m_LSSCollider;
	lssCollider.SetTemporalCoherence( TFALSE );
	lssCollider.SetFirstContact( TFALSE );

	TVector4 vecMovement;
	vecMovement.Multiply( vecLocalDirection, a_rfDistance );

	IceMaths::LSS lss(
	    IceMaths::Segment(
	        IceMaths::Point( vecLocalCenter.x, vecLocalCenter.y, vecLocalCenter.z ),
	        IceMaths::Point( vecLocalCenter.x + vecMovement.x, vecLocalCenter.y + vecMovement.y, vecLocalCenter.z + vecMovement.z )
	    ),
	    a_fRadius + 0.005f
	);

	TBOOL bHit = TFALSE;
	lssCollider.Collide( s_pCache->m_LSSCache, lss, m_OpcodeModel, TNULL, TNULL );

	if ( lssCollider.GetContactStatus() )
	{
		TVector4 vecClosestNormal( 0.0f, 0.0f, 0.0f, 1.0f );
		TFLOAT   fStepHeight = -1.0f;

		TINT          iNumPrimitives     = TINT( lssCollider.GetNbTouchedPrimitives() );
		const udword* pTouchedPrimitives = lssCollider.GetTouchedPrimitives();

		if ( iNumPrimitives > 200 )
			iNumPrimitives = 200;

		TFLOAT fClosestDistance = a_rfDistance;
		TFLOAT fMinPenetration  = TMath::MAXFLOAT;
		TFLOAT fMaxDistance     = a_rfDistance;

		for ( TINT i = 0; i < iNumPrimitives; i++ )
		{
			udword uiFaceId = pTouchedPrimitives[ i ];

			if ( ( ( 1 << pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( uiFaceId ) ).eCollGroup ) & a_eIgnoreGroups ) != 0 )
				continue;

			TVector3* apVertices[ 3 ];
			pCollData->GetTriangleVertices( uiFaceId, apVertices );

			TPlane oPlane;
			TMath::TrianglePlane( oPlane, *apVertices[ 0 ], *apVertices[ 1 ], *apVertices[ 2 ] );

			TVector4 vecNormal( oPlane.GetNormal().x, oPlane.GetNormal().y, oPlane.GetNormal().z, 1.0f );
			TINT     iResult = TMath::SweptSphereVsTriangle( vecLocalCenter, a_fRadius, vecLocalDirection, apVertices, fMaxDistance, oPlane, vecNormal );

			if ( iResult == 0 )
				continue;

			TFLOAT fHitDistance = fMaxDistance;

			// Allow a bit more distance for the next triangles
			fMaxDistance = fMaxDistance + 0.25f;
			if ( fMaxDistance > a_rfDistance )
				fMaxDistance = a_rfDistance;

			if ( fClosestDistance > fHitDistance )
			{
				TVector4 vecOffset;
				vecOffset.Multiply( vecLocalDirection, fHitDistance );

				TVector4 vecHitCenter = vecLocalCenter;
				vecHitCenter.Add( vecOffset );

				TFLOAT fPenetration = oPlane.GetNormal().z * vecHitCenter.z + oPlane.GetNormal().y * vecHitCenter.y + oPlane.GetNormal().x * vecHitCenter.x - oPlane.GetD();

				if ( fMinPenetration > fPenetration )
					fMinPenetration = fPenetration;

				fClosestDistance = fHitDistance;
				vecClosestNormal = vecNormal;
				a_rbEdgeHit      = iResult > 1;
				bHit             = TTRUE;

				// NOTE: retail uses the z coordinate here
				TFLOAT fStep = vecLocalCenter.z - a_fRadius - Max3( apVertices[ 0 ]->z, apVertices[ 1 ]->z, apVertices[ 2 ]->z );
				fStepHeight  = ( fStep > a_fMaxStepHeight ) ? -1.0f : fStep;
			}
		}

		if ( bHit )
		{
			a_rNormal.Set( vecClosestNormal.x, vecClosestNormal.y, vecClosestNormal.z, 1.0f );
			a_rNormal.Normalise();

			a_rfPenetration = fMinPenetration;
			a_rfDistance    = fClosestDistance;
			a_rfStepHeight  = fStepHeight;

			TMatrix44::RotateVector( a_rNormal, matBoneTransform, a_rNormal );

			if ( fScale != 1.0f )
			{
				a_rNormal.Normalise();
				a_rfPenetration = fScale * a_rfPenetration;
				a_rfStepHeight  = fScale * a_rfStepHeight;
			}
		}
	}

	a_rfDistance = fScale * a_rfDistance;
	return bHit;
}
