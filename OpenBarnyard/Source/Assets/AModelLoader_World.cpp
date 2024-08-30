#include "pch.h"
#include "AModelLoader.h"
#include "World/AWorldVIS.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/AWorldShader/AWorldMesh_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#endif // TOSHI_SKU_WINDOWS

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TUINT   s_iMeshIndex          = 0;
static TClass* s_pWorldMaterialClass = TNULL;

static void LoadTreeIntersect( CellSphereTreeBranchNode* a_pRenderGroup, Cell*& a_ppModel, TModelLOD* a_pModelLOD )
{
	TVALIDPTR( a_pRenderGroup );

	// Load sub groups
	auto pShader      = TDYNAMICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );
	auto pRenderGroup = a_pRenderGroup;
	while ( !pRenderGroup->IsLeaf() )
	{
		LoadTreeIntersect( pRenderGroup->GetSubNode(), a_ppModel, a_pModelLOD );
		pRenderGroup = pRenderGroup->m_pRight;
	}

	TASSERT( pRenderGroup->IsLeaf() );
	auto pRenderData = pRenderGroup->GetLeafNode();

	for ( TUINT i = 0; i < pRenderData->m_uiNumMeshes; i++ )
	{
		auto pTerrainMesh = a_ppModel->ppCellMeshSpheres[ pRenderData->GetMeshIndex( i ) ]->m_pCellMesh;
		auto pMesh        = pShader->CreateMesh( TNULL );

		pMesh->Create( 0, pTerrainMesh->uiNumVertices1 );
		pTerrainMesh->pMesh = pMesh;

		if ( TNULL == s_pWorldMaterialClass )
		{
			s_pWorldMaterialClass = TClass::Find( "AWorldMaterial" );
		}

		TVALIDPTR( s_pWorldMaterialClass );

		pMesh->SetMaterial( AModelLoader::CreateMaterial( pShader, pTerrainMesh->szMaterialName ) );
		a_pModelLOD->ppMeshes[ s_iMeshIndex++ ] = pMesh;

		auto pSubMesh             = pMesh->GetSubMesh( 0 );
		pSubMesh->pAssociatedMesh = pMesh;

		// Create index pool
		if ( pSubMesh->pIndexPool == TNULL )
		{
			auto pIndexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TIndexFactoryResourceInterface>( SYSRESOURCE_IFSYS );
			TVALIDPTR( pIndexFactory );

			pSubMesh->pIndexPool = pIndexFactory->CreatePoolResource( pTerrainMesh->uiNumIndices, 9 );
		}

		pSubMesh->uiNumVertices = pTerrainMesh->uiNumVertices2;

		// Write vertices
		if ( pTerrainMesh->uiNumVertices1 != 0 )
		{
			AWorldMesh::LockBuffer lockBuffer;
			TBOOL                  bLocked = pMesh->Lock( lockBuffer );

			if ( bLocked )
			{
				TUtil::MemCopy( lockBuffer.apStreams[ 0 ], pTerrainMesh->pVertices, sizeof( WorldVertex ) * pTerrainMesh->uiNumVertices1 );
				pMesh->Unlock( pTerrainMesh->uiNumVertices1 );
			}
			else
			{
				TASSERT( !"Couldn't lock buffer" );
			}
		}

		// Write indices
		if ( pTerrainMesh->uiNumIndices != 0 )
		{
			TVALIDPTR( pSubMesh->pIndexPool );
			TIndexPoolResourceInterface::LockBuffer lockBuffer;

			pSubMesh->pIndexPool->Lock( &lockBuffer );
			TUtil::MemCopy( lockBuffer.pBuffer, pTerrainMesh->pIndices, pTerrainMesh->uiNumIndices * sizeof( TUINT16 ) );
			pSubMesh->pIndexPool->Unlock( pTerrainMesh->uiNumIndices );
		}
	}
}

void AModelLoader::LoadWorldMeshTRB( Toshi::TModel* a_pModel, TINT a_iLODIndex, Toshi::TModelLOD* a_pLOD )
{
	TPROFILER_SCOPE();

	TVALIDPTR( a_pModel );
	TVALIDPTR( a_pLOD );

	s_iMeshIndex   = 0;
	auto pDatabase = a_pModel->CastSymbol<WorldDatabase>( "Database" );

	for ( TUINT i = 0; i < pDatabase->m_uiNumWorlds; i++ )
	{
		auto pWorld = pDatabase->m_ppWorlds[ i ];

		for ( TINT k = 0; k < pWorld->m_iNumCells; k++ )
		{
			auto pModel = pWorld->m_ppCells[ k ];
			LoadTreeIntersect( pModel->pTreeBranchNodes, pModel, a_pLOD );
		}
	}
}
