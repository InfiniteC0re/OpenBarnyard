#include "pch.h"
#include "AModelLoader.h"

#ifdef TOSHI_SKU_WINDOWS
#include "Platform/DX8/AWorldShader/AWorldMesh_DX8.h"
#include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#endif // TOSHI_SKU_WINDOWS

#include <Render/TRenderInterface.h>
#include <Render/TTerrainMDL.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TUINT s_iMeshIndex = 0;
static TClass* s_pWorldMaterialClass = TNULL;

static void LoadFromRenderGroups(TTerrainMDL::RenderGroup* a_pRenderGroup, TTerrainMDL::Model*& a_ppModel, TModelLOD* a_pModelLOD)
{
	TVALIDPTR(a_pRenderGroup);

	// Load sub groups
	auto pShader = AWorldShaderHAL::Upcast(AWorldShader::GetSingleton());
	auto pRenderGroup = a_pRenderGroup;
	while (!pRenderGroup->IsDataNext())
	{
		LoadFromRenderGroups(pRenderGroup->GetSubGroup(), a_ppModel, a_pModelLOD);
		pRenderGroup = pRenderGroup->m_pNextGroup;
	}

	TASSERT(pRenderGroup->IsDataNext());
	auto pRenderData = pRenderGroup->GetData();

	for (TUINT i = 0; i < pRenderData->m_uiNumMeshes; i++)
	{
		auto pTerrainMesh = a_ppModel->m_ppMeshBoundings[pRenderData->GetMeshIndex(i)]->m_pMesh;
		auto pMesh = pShader->CreateMesh(TNULL);
		
		pMesh->Create(0, pTerrainMesh->m_uiNumVertices1);
		pTerrainMesh->m_pRealMesh = pMesh;

		if (TNULL == s_pWorldMaterialClass)
		{
			s_pWorldMaterialClass = TClass::Find("AWorldMaterial");
		}

		TVALIDPTR(s_pWorldMaterialClass);

		pMesh->SetMaterial(AModelLoader::CreateMaterial(pShader, pTerrainMesh->m_szMaterialName));
		a_pModelLOD->ppMeshes[s_iMeshIndex++] = pMesh;

		auto pSubMesh = pMesh->GetSubMesh(0);
		pSubMesh->pAssociatedMesh = pMesh;

		// Create index pool
		if (pSubMesh->pIndexPool == TNULL)
		{
			auto pIndexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TIndexFactoryResourceInterface>(SYSRESOURCE_IFSYS);
			TVALIDPTR(pIndexFactory);

			pSubMesh->pIndexPool = pIndexFactory->CreatePoolResource(pTerrainMesh->m_uiNumIndices, 9);
		}

		pSubMesh->uiNumVertices = pTerrainMesh->m_uiNumVertices2;

		// Write vertices
		if (pTerrainMesh->m_uiNumVertices1 != 0)
		{
			AWorldMesh::LockBuffer lockBuffer;
			TBOOL bLocked = pMesh->Lock(lockBuffer);

			if (bLocked)
			{
				TUtil::MemCopy(lockBuffer.apStreams[0], pTerrainMesh->m_pVertices, sizeof(TTerrainMDL::WorldVertex) * pTerrainMesh->m_uiNumVertices1);
				pMesh->Unlock(pTerrainMesh->m_uiNumVertices1);
			}
			else
			{
				TASSERT(!"Couldn't lock buffer");
			}
		}

		// Write indices
		if (pTerrainMesh->m_uiNumIndices != 0)
		{
			TVALIDPTR(pSubMesh->pIndexPool);
			TIndexPoolResourceInterface::LockBuffer lockBuffer;

			pSubMesh->pIndexPool->Lock(&lockBuffer);
			TUtil::MemCopy(lockBuffer.pBuffer, pTerrainMesh->m_pIndices, pTerrainMesh->m_uiNumIndices * sizeof(TUINT16));
			pSubMesh->pIndexPool->Unlock(pTerrainMesh->m_uiNumIndices);
		}
	}
}

void AModelLoader::LoadWorldLOD(Toshi::TModel* a_pModel, TINT a_iLODIndex, Toshi::TModelLOD* a_pLOD)
{
	TVALIDPTR(a_pModel);
	TVALIDPTR(a_pLOD);

	s_iMeshIndex = 0;
	auto pDatabase = a_pModel->CastSymbol<TTerrainMDL::TRBHeader>("Database");

	for (TUINT i = 0; i < pDatabase->m_uiNumEntries; i++)
	{
		auto pEntry = pDatabase->m_ppEntries[i];
		
		for (TUINT k = 0; k < pEntry->m_uiNumModels; k++)
		{
			auto pModel = pEntry->m_ppModels[k];
			LoadFromRenderGroups(pModel->m_pRenderGroups, pModel, a_pLOD);
		}
	}
}