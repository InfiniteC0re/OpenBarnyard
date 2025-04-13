#include "pch.h"
#include "ACollisionInspector.h"

#include <AHooks.h>
#include <HookHelpers.h>

#include <BYardSDK/ASysShader.h>
#include <BYardSDK/ASysMesh.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

#include <Toshi/T2Map.h>
#include <ToshiTools/T2CommandLine.h>
#include <Platform/DX8/TModel_DX8.h>

TOSHI_NAMESPACE_USING

extern TBOOL g_bIsExperimentalMode;
TMemory::MemBlock* g_pCollMemBlock;
T2Map<TModelLOD*, void*> g_mapModelLODToOldMeshes;

MEMBER_HOOK( 0x006d9ed0, TModelHAL, TModelHAL_Unload, void )
{
	CallOriginal();

	for ( TINT i = 0; i < m_iLODCount; i++ )
	{
		if ( m_LODs[ i ].ppMeshes )
		{
			auto it = g_mapModelLODToOldMeshes.Find( &m_LODs[ i ] );

			if ( it != g_mapModelLODToOldMeshes.End() )
			{
				delete[] it->GetSecond();
				g_mapModelLODToOldMeshes.Remove( it );
			}
		}
	}
}

ACollisionInspector::ACollisionInspector()
{
	if ( g_bIsExperimentalMode )
	{
		// NOTE: it creates too many holes in memory so the game can even run out of memory and crash
		AHooks::AddHook( Hook_AModelLoader_LoadTRBCallback, HookType_Before, AModelLoader_LoadTRBCallbackBefore );
		AHooks::AddHook( Hook_AModelLoader_LoadTRBCallback, HookType_After, AModelLoader_LoadTRBCallbackAfter );
		AHooks::AddHook( Hook_ATerrain_Render, HookType_After, ATerrain_Render );

		InstallHook<TModelHAL_Unload>();

		void* pMemory   = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, 32 * 1024 * 1024 );
		g_pCollMemBlock = g_pMemory->CreateMemBlockInPlace( pMemory, 32 * 1024 * 1024, "Models" );
	}
}

void ACollisionInspector::SetCollisionVisible( TBOOL a_bVisible )
{
	m_bCollisionVisible = a_bVisible;
}

class ASysCollisionMesh : public ASysMesh
{
public:
	virtual TBOOL Render() override
	{
		if ( ACollisionInspector::GetSingleton()->IsCollisionVisible() )
		{
			return CALL_THIS( 0x005f1430, ASysCollisionMesh*, TBOOL, this );
		}

		return TTRUE;
	}

	virtual void SetZBias( TINT a_iZBias ) override
	{
		m_iZBias = a_iZBias;
	}

private:
	TINT m_iZBias = 0;
};

TMemory::MemBlock* g_pMemBlock     = TNULL;
TMemory::MemBlock* g_pGameMemBlock = TNULL;

TBOOL ACollisionInspector::AModelLoader_LoadTRBCallbackBefore( Toshi::TModel* a_pModel )
{
	TMemory* pGameMemory = *TREINTERPRETCAST( TMemory**, 0x007ce1d4 );

	g_pMemBlock     = g_pMemory->GetGlobalBlock();
	g_pGameMemBlock = pGameMemory->GetGlobalBlock();

	g_pMemory->SetGlobalBlock( g_pCollMemBlock );
	pGameMemory->SetGlobalBlock( g_pCollMemBlock );

	return TFALSE;
}

TBOOL ACollisionInspector::AModelLoader_LoadTRBCallbackAfter( TModel* a_pModel )
{
	TMemory* pGameMemory = *TREINTERPRETCAST( TMemory**, 0x007ce1d4 );

	if ( a_pModel->GetNumLODs() >= 1 )
	{
		struct CollisionMeshDef_t
		{
			TUINT32          Unk1;
			TVector3* pVertices;
			TUINT32          NumVertices;
			TUINT16*         pIndices;
			TUINT32          NumIndices;
			TUINT32          uiNumCollTypes;
			const char**     pszCollName;
			TUINT32          Unk3;
		};

		struct ASysMeshVertex_t
		{
			TVector3 Position;
			TUINT32         Diffuse;
			TVector2 UV;
		};

		struct Collision_t
		{
			TUINT               uiNumMeshes;
			CollisionMeshDef_t* pMeshDef;
		};

		auto pCollision = TSTATICCAST( Collision_t, a_pModel->GetSymbol( "Collision" ) );

		if ( pCollision )
		{
			if ( pCollision->uiNumMeshes > 0 )
			{
				if ( !ms_pCollisionMaterial )
				{
					auto pRender          = THookedRenderD3DInterface::GetSingleton();
					ms_pCollisionMaterial = ASysShader::GetSingleton()->CreateMaterial( "collision" );
					ms_pCollisionMaterial->SetBlendMode( ASysMaterial::BLENDMODE_1 );
					ms_pCollisionMaterial->SetTexture( pRender->GetInvalidTexture() );
					ms_pCollisionMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
				}

				for ( TINT j = 0; j < a_pModel->GetNumLODs(); j++ )
				{
					auto& rLOD = a_pModel->GetLOD( j );

					auto ppOldMeshes        = rLOD.ppMeshes;
					auto iOriginalNumMeshes = rLOD.iNumMeshes;

					TUINT uiNumValidCollMeshes = 0;
					for ( TUINT i = 0; i < pCollision->uiNumMeshes; i++ )
					{
						auto pMeshDef = &pCollision->pMeshDef[ i ];

						if ( pMeshDef->NumIndices <= TMath::TUINT16_MAX )
						{
							uiNumValidCollMeshes++;
						}
					}

					rLOD.ppMeshes = (TMesh**)TMalloc( sizeof( TMesh* ) * ( rLOD.iNumMeshes + uiNumValidCollMeshes ), g_pCollMemBlock );

					if ( rLOD.iNumMeshes > 0 )
					{
						TUtil::MemCopy( rLOD.ppMeshes, ppOldMeshes, rLOD.iNumMeshes * sizeof( TMesh* ) );
						g_mapModelLODToOldMeshes.Insert( &rLOD, ppOldMeshes );
					}

					for ( TUINT i = 0; i < pCollision->uiNumMeshes; i++ )
					{
						auto pMeshDef = &pCollision->pMeshDef[ i ];

						if ( pMeshDef->NumIndices <= TMath::TUINT16_MAX )
						{
							auto pMesh = new ( g_pCollMemBlock ) ASysCollisionMesh;
							pMesh->SetOwnerShader( ASysShader::GetSingleton() );
							pMesh->SetMaterial( ms_pCollisionMaterial );
							pMesh->CreatePools( 9, pMeshDef->NumVertices, pMeshDef->NumIndices );
							pMesh->SetZBias( 32 );

							ASysMesh::LockBuffer lockBuffer;
							pMesh->Lock( lockBuffer );

							ASysMeshVertex_t* pVertex = (ASysMeshVertex_t*)lockBuffer.VertexLock.apStreams[ 0 ];

							for ( TUINT k = 0; k < pMeshDef->NumVertices; k++ )
							{
								pVertex[ k ].Position = pMeshDef->pVertices[ k ];
								pVertex[ k ].Diffuse  = 0x50800000;
								pVertex[ k ].UV       = { 1.0f, 1.0f };
							}

							for ( TUINT k = 0; k < pMeshDef->NumIndices; k++ )
							{
								lockBuffer.IndexLock.pBuffer[ k ] = pMeshDef->pIndices[ k ];
							}

							pMesh->Unlock( pMeshDef->NumVertices, pMeshDef->NumIndices );
							rLOD.ppMeshes[ rLOD.iNumMeshes++ ] = pMesh;

							//TTRACE("  Done: %u vertices, %u indices\n", pMeshDef->NumVertices, pMeshDef->NumIndices);
						}
						else
						{
							TERROR( "  Unable to create collision mesh because is has too many indices!\n" );
						}
					}
				}

				g_pMemory->SetGlobalBlock( g_pMemBlock );
				pGameMemory->SetGlobalBlock( g_pGameMemBlock );

				return TTRUE;
			}
		}
	}

	g_pMemory->SetGlobalBlock( g_pMemBlock );
	pGameMemory->SetGlobalBlock( g_pGameMemBlock );

	return TFALSE;
}

void ACollisionInspector::ATerrain_Render( ATerrainInterface* a_pTerrain )
{
	if ( ACollisionInspector::GetSingleton()->m_bCollisionVisible )
	{
		auto iCurrGroup = a_pTerrain->m_iCurrentGroup;

		if ( iCurrGroup != -1 )
		{
			auto pCurrentGroup  = &a_pTerrain->m_pTerrainVIS->m_pSections[ iCurrGroup ];
			auto pCollModelData = pCurrentGroup->m_pCollisionModelData;

			for ( TINT i = 0; i < a_pTerrain->m_pTerrainVIS->m_iNumSections; i++ )
			{
				if ( pCurrentGroup->m_pOtherGroupsLODs[ i ] != ATerrainLODType_None )
				{
					auto pGroup     = &a_pTerrain->m_pTerrainVIS->m_pSections[ i ];
					auto pModelData = pGroup->m_pCollisionModelData;

					if ( pModelData )
					{
						auto pModel = pModelData->m_ModelRef.GetModel();

						if ( pModel && pModel->GetNumLODs() > 0 && pModel->GetLOD( 0 ).iNumMeshes > 0 )
						{
							CALL_THIS( 0x005e9380, ATerrainSection::ModelNode*, void, pModelData );
						}
					}
				}
			}
		}
	}
}
