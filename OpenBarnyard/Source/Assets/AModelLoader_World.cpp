#include "pch.h"
#include "AModelLoader.h"
#include "World/AWorldVIS.h"
#include "AAssetLoader.h"
#include "AMaterialLibraryManager.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/AWorldShader/AWorldMesh_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#  include "Platform/DX8/AGrassShader/AGrassMeshHAL_DX8.h"
#  include "Platform/DX8/AGrassShader/AGrassShaderHAL_DX8.h"
#  include "Platform/DX8/AGrassShader/AGrassMaterialHAL_DX8.h"
#endif // TOSHI_SKU_WINDOWS

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TUINT      s_iMeshIndex      = 0;
static TUINT      s_iGrassMeshIndex = 0;
AMaterialLibrary* g_pGrassLayersMatLib;
Toshi::T2Texture* g_aGrassLayers[ 7 ];
static TClass*    s_pWorldMaterialClass = TNULL;

// $Barnyard: FUNCTION 00613270
void AModelLoader::InitialiseGrassLayersTextures()
{
	g_pGrassLayersMatLib = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
	    "Data\\MiscTextures\\GrasslayertexWin.ttl",
	    AAssetLoader::GetAssetTRB( AAssetType_Startup )
	);

	TVALIDPTR( g_pGrassLayersMatLib );

	g_aGrassLayers[ 1 ] = g_pGrassLayersMatLib->GetTexture( g_pGrassLayersMatLib->FindTextureIndex( "treeb3.tga" ) )->pTexture;
	g_aGrassLayers[ 2 ] = g_aGrassLayers[ 1 ];
	g_aGrassLayers[ 3 ] = g_pGrassLayersMatLib->GetTexture( g_pGrassLayersMatLib->FindTextureIndex( "treeb2.tga" ) )->pTexture;
	g_aGrassLayers[ 4 ] = g_aGrassLayers[ 3 ];
	g_aGrassLayers[ 5 ] = g_pGrassLayersMatLib->GetTexture( g_pGrassLayersMatLib->FindTextureIndex( "treeb1.tga" ) )->pTexture;
	g_aGrassLayers[ 6 ] = g_aGrassLayers[ 5 ];
}

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

// $Barnyard: FUNCTION 00613a40
void AModelLoader::LoadWorldMeshTRB( Toshi::TModel* a_pModel, TINT a_iLODIndex, Toshi::TModelLOD* a_pLOD, Toshi::TTMDWin::TRBLODHeader* a_pLODHeader )
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

static void LoadGrassTreeIntersect( CellSphereTreeBranchNode* a_pRenderGroup, Cell*& a_ppModel, TModelLOD* a_pModelLOD )
{
	TVALIDPTR( a_pRenderGroup );

	// Load sub groups
	auto pShader      = TDYNAMICCAST( AGrassShaderHAL, AGrassShader::GetSingleton() );
	auto pRenderGroup = a_pRenderGroup;
	while ( !pRenderGroup->IsLeaf() )
	{
		LoadGrassTreeIntersect( pRenderGroup->GetSubNode(), a_ppModel, a_pModelLOD );
		pRenderGroup = pRenderGroup->m_pRight;
	}

	TASSERT( pRenderGroup->IsLeaf() );
	auto pRenderData = pRenderGroup->GetLeafNode();

	for ( TUINT i = 0; i < pRenderData->m_uiNumMeshes; i++ )
	{
		CellMeshSphere* pCellMeshSphere = a_ppModel->ppCellMeshSpheres[ pRenderData->GetMeshIndex( i ) ];
		CellMesh*       pTerrainMesh    = pCellMeshSphere->m_pCellMesh;
		AGrassMeshHAL*  pMesh           = TSTATICCAST( AGrassMeshHAL, pShader->CreateMesh( TNULL ) );

		pMesh->CreateHAL( 0, pTerrainMesh->uiNumVertices1, pCellMeshSphere );
		pTerrainMesh->pMesh = pMesh;

		AGrassMaterialHAL* pMaterial = TSTATICCAST( AGrassMaterialHAL, AModelLoader::CreateMaterial( pShader, pTerrainMesh->szMaterialName ) );

		if ( !pMaterial )
		{
			pMaterial = TSTATICCAST( AGrassMaterialHAL, pShader->CreateMaterial( TNULL ) );
			pMaterial->Create( 0 );
			pMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TFALSE );
			pMaterial->SetTexture( 0, TNULL );
			pMaterial->SetTextureNum( 1 );
		}

		pMesh->SetMaterial( pMaterial );
		a_pModelLOD->ppMeshes[ s_iGrassMeshIndex++ ] = pMesh;

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

void AModelLoader::LoadGrassMeshTRB( TModel* a_pModel, TINT a_iLODIndex, TModelLOD* a_pLOD, Toshi::TTMDWin::TRBLODHeader* a_pLODHeader )
{
	TPROFILER_SCOPE();

	TVALIDPTR( a_pModel );
	TVALIDPTR( a_pLOD );

	s_iGrassMeshIndex = 0;
	auto pDatabase    = a_pModel->CastSymbol<WorldDatabase>( "Database" );

	for ( TUINT i = 0; i < pDatabase->m_uiNumWorlds; i++ )
	{
		auto pWorld = pDatabase->m_ppWorlds[ i ];

		for ( TINT k = 0; k < pWorld->m_iNumCells; k++ )
		{
			auto pModel = pWorld->m_ppCells[ k ];
			LoadGrassTreeIntersect( pModel->pTreeBranchNodes, pModel, a_pLOD );
		}
	}
}
