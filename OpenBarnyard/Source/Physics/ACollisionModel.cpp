#include "pch.h"
#include "ACollisionModel.h"

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
		m_vecTreeCenter  = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		m_fAverageExtent = TMath::MAXFLOAT;

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

	TFLOAT fScale = TMath::Min( 0.001f, a_pModelInstance->GetScale() );

	// Perform fast ray collision detection
	if ( !CollideRayFast( a_rcRayOrigin, a_rcRayDir, a_rfMaxDistance, matBoneTransform, fScale ) )
		return TFALSE;

	// Construct ray object
	IceMaths::Ray ray;
	if ( fScale == 1.0f )
	{
		// No need to do any math here
		ray.mOrig.x       = a_rcRayOrigin.x;
		ray.mOrig.y       = a_rcRayOrigin.y;
		ray.mOrig.z       = a_rcRayOrigin.z;
		ray.mDir.x        = a_rcRayDir.x;
		ray.mDir.y        = a_rcRayDir.y;
		ray.mDir.z        = a_rcRayDir.z;
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
	rayCollider.SetPrimitiveTests( TFALSE );
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
			const Opcode::CollisionFace* pFace = &s_pCache->m_StabbedFaces.GetFaces()[i];
			
			// Skip far placed faces 
			if ( pFace->mDistance >= a_rfMaxDistance )
				continue;

			TINT iGroup = pCollData->GetGroupForTriangleImp( pFace->mFaceID );

			if ( iGroup != m_aIgnoreGroups[ 1 ] &&
				 iGroup != m_aIgnoreGroups[ 0 ] &&
			     ( a_eCGroupsMask & pCollData->GetCollGroup( i ).GetGroupFlag() ) == 0 )
			{
				// This triangle is not ignored and matches the collision group
				uiFaceIndex = uiFaceIndex;
				fDistance   = pFace->mDistance;
			}
		}

		if ( uiFaceIndex != -1 )
		{
			// TODO: SET DISTANCE
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

// $Barnyard: FUNCTION 00619140
void ACollisionModel::DestroyStaticCache()
{
	TASSERT( s_pCache != TNULL );

	if ( s_pCache )
		delete s_pCache;
	
	s_pCache = TNULL;
}
