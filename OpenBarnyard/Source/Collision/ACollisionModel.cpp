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

	TModelCollisionMesh* pCollMesh = &m_pModel->GetCollisionMeshes()[ a_iMeshIndex ];

	if ( pCollMesh->GetNumVertices() > 0 && pCollMesh->GetNumIndices() > 0 )
	{
		field7_0x48            = -1;
		field8_0x4c            = -1;
		m_vecTreeCenter        = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		m_fAverageExtent       = TMath::MAXFLOAT;

		// We are always having AABBNoLeafTree as the tree structure
		Opcode::AABBTRBNoLeafTree*   pCollTree = a_pModel->CastSymbol<Opcode::AABBTRBNoLeafTree>( "CollisionTree" );
		const Opcode::CollisionAABB* pAABB     = TNULL;

		TBOOL bSuccess;

		if ( pCollTree )
		{
			Opcode::OPCODECREATE oCreate;
			oCreate.mIMesh = &m_MeshInterface;
			oCreate.mIMesh->SetNbTriangles( pCollMesh->GetNumIndices() / 3 );
			oCreate.mIMesh->SetNbVertices( pCollMesh->GetNumVertices() );
			oCreate.mIMesh->SetPointers(
			    TREINTERPRETCAST( IceMaths::IndexedTriangle*, pCollMesh->GetIndices() ),
			    TREINTERPRETCAST( IceMaths::Point*, pCollMesh->GetVertices() )
			);
			oCreate.mNoLeaf       = TTRUE;
			oCreate.mQuantized    = TFALSE;
			oCreate.mKeepOriginal = TFALSE;
			oCreate.mCanRemap     = TFALSE;
			bSuccess              = m_OpcodeModel.BuildGivenTree( oCreate, pCollTree );
			pAABB                 = &pCollTree->GetNodes()->mAABB;
		}
		else
		{
			Opcode::OPCODECREATE oCreate;
			oCreate.mIMesh = &m_MeshInterface;
			oCreate.mIMesh->SetNbTriangles( pCollMesh->GetNumIndices() / 3 );
			oCreate.mIMesh->SetNbVertices( pCollMesh->GetNumVertices() );
			oCreate.mIMesh->SetPointers(
			    TREINTERPRETCAST( IceMaths::IndexedTriangle*, pCollMesh->GetIndices() ),
			    TREINTERPRETCAST( IceMaths::Point*, pCollMesh->GetVertices() )
			);
			oCreate.mNoLeaf       = TTRUE;
			oCreate.mQuantized    = TFALSE;
			oCreate.mKeepOriginal = TFALSE;
			oCreate.mCanRemap     = TFALSE;
			bSuccess              = m_OpcodeModel.Build( oCreate );
			pAABB                 = &TREINTERPRETCAST( Opcode::AABBNoLeafTree*, m_OpcodeModel.GetTree() )->GetNodes()->mAABB;
		}

		// Calculate bounding
		m_vecTreeCenter.x = pAABB->mCenter.x;
		m_vecTreeCenter.y = pAABB->mCenter.y;
		m_vecTreeCenter.z = pAABB->mCenter.z;
		m_vecTreeCenter.w = 1.0f;
		m_fAverageExtent  = pAABB->mExtents.Magnitude();

		return bSuccess;
	}

	return TTRUE;
}
