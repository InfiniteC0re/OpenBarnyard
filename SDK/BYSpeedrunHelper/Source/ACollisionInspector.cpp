#include "pch.h"
#include "ACollisionInspector.h"

#include <AHooks.h>

#include <BYardSDK/ASysShader.h>
#include <BYardSDK/ASysMesh.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

ACollisionInspector::ACollisionInspector()
{
	AHooks::AddHook(Hook_AModelLoader_LoadTRBCallback, HookType_After, AModelLoader_LoadTRBCallback);
	AHooks::AddHook(Hook_ATerrain_Render, HookType_After, ATerrain_Render);
}

void ACollisionInspector::SetCollisionVisible(TBOOL a_bVisible)
{
	m_bCollisionVisible = a_bVisible;
}

class ASysCollisionMesh : public ASysMesh
{
public:
	virtual TBOOL Render() override
	{
		if (ACollisionInspector::GetSingleton()->IsCollisionVisible())
		{
			return CALL_THIS(0x005f1430, ASysCollisionMesh*, TBOOL, this);
		}

		return TTRUE;
	}

	virtual void SetZBias(TINT a_iZBias) override
	{
		m_iZBias = a_iZBias;
	}

private:
	TINT m_iZBias = 0;
};

TBOOL ACollisionInspector::AModelLoader_LoadTRBCallback(Toshi::TModel* a_pModel)
{
	if (a_pModel->GetNumLODs() >= 1)
	{
		struct CollisionMeshDef_t
		{
			TUINT32 Unk1;
			Toshi::TVector3* pVertices;
			TUINT32 NumVertices;
			TUINT16* pIndices;
			TUINT32 NumIndices;
			TUINT32 uiNumCollTypes;
			const char** pszCollName;
			TUINT32 Unk3;
		};

		struct ASysMeshVertex_t
		{
			Toshi::TVector3 Position;
			TUINT32 Diffuse;
			Toshi::TVector2 UV;
		};

		struct Collision_t
		{
			TUINT uiNumMeshes;
			CollisionMeshDef_t* pMeshDef;
		};

		auto pCollision = TSTATICCAST(Collision_t*, a_pModel->GetSymbol("Collision"));

		if (pCollision)
		{
			if (pCollision->uiNumMeshes > 0)
			{
				TTRACE("Creating collision mesh through ASysShader...\n");

				if (!ms_pCollisionMaterial)
				{
					auto pRender = THookedRenderD3DInterface::GetSingleton();
					ms_pCollisionMaterial = ASysShader::GetSingleton()->CreateMaterial("collision");
					ms_pCollisionMaterial->SetBlendMode(ASysMaterial::BLENDMODE_1);
					ms_pCollisionMaterial->SetTexture(pRender->GetInvalidTexture());
					ms_pCollisionMaterial->SetFlag(Toshi::TMaterial::Flags_NO_CULL, TTRUE);
				}

				for (TINT j = 0; j < a_pModel->GetNumLODs(); j++)
				{
					auto& rLOD = a_pModel->GetLOD(j);

					auto ppOldMeshes = rLOD.ppMeshes;
					auto iOriginalNumMeshes = rLOD.iNumMeshes;

					rLOD.ppMeshes = new Toshi::TMesh*[rLOD.iNumMeshes + pCollision->uiNumMeshes];

					if (rLOD.iNumMeshes > 0)
					{
						Toshi::TUtil::MemCopy(rLOD.ppMeshes, ppOldMeshes, rLOD.iNumMeshes * sizeof(Toshi::TMesh*));
						delete[] ppOldMeshes;
						ppOldMeshes = TNULL;
					}

					for (TUINT i = 0; i < pCollision->uiNumMeshes; i++)
					{
						auto pMeshDef = &pCollision->pMeshDef[i];

						if (pMeshDef->NumIndices <= Toshi::TMath::TUINT16_MAX)
						{
							auto pMesh = new ASysCollisionMesh;
							pMesh->SetOwnerShader(ASysShader::GetSingleton());
							pMesh->SetMaterial(ms_pCollisionMaterial);
							pMesh->CreatePools(9, pMeshDef->NumVertices, pMeshDef->NumIndices);
							pMesh->SetZBias(32);

							ASysMesh::LockBuffer lockBuffer;
							pMesh->Lock(lockBuffer);

							ASysMeshVertex_t* pVertex = (ASysMeshVertex_t*)lockBuffer.VertexLock.apStreams[0];

							for (TUINT k = 0; k < pMeshDef->NumVertices; k++)
							{
								pVertex[k].Position = pMeshDef->pVertices[k];
								pVertex[k].Diffuse = 0x50800000;
								pVertex[k].UV = { 1.0f, 1.0f };
							}

							for (TUINT k = 0; k < pMeshDef->NumIndices; k++)
							{
								lockBuffer.IndexLock.pBuffer[k] = pMeshDef->pIndices[k];
							}

							pMesh->Unlock(pMeshDef->NumVertices, pMeshDef->NumIndices);
							rLOD.ppMeshes[iOriginalNumMeshes + i] = pMesh;
							rLOD.iNumMeshes += 1;

							TTRACE("  Done: %u vertices, %u indices\n", pMeshDef->NumVertices, pMeshDef->NumIndices);
						}
						else
						{
							TERROR("  Unable to create collision mesh because is has too many indices!\n");
						}
					}
				}

				return TTRUE;
			}
		}
	}

	return TFALSE;
}

void ACollisionInspector::ATerrain_Render(ATerrain* a_pTerrain)
{
	if (ACollisionInspector::GetSingleton()->m_bCollisionVisible)
	{
		auto iCurrGroup = a_pTerrain->m_iCurrentGroup;

		if (iCurrGroup != -1)
		{
			auto pCurrentGroup = &a_pTerrain->m_pTerrainVIS->m_pSections[iCurrGroup];
			auto pCollModelData = pCurrentGroup->m_pCollisionModelData;

			for (TINT i = 0; i < a_pTerrain->m_pTerrainVIS->m_iNumSections; i++)
			{
				if (pCurrentGroup->m_pOtherGroupsLODs[i] != ATerrainLODType_None)
				{
					auto pGroup = &a_pTerrain->m_pTerrainVIS->m_pSections[i];
					auto pModelData = pGroup->m_pCollisionModelData;

					if (pModelData)
					{
						auto pModel = pModelData->m_ModelRef.GetModel();

						if (pModel && pModel->GetNumLODs() > 0 && pModel->GetLOD(0).iNumMeshes > 0)
						{
							CALL_THIS(0x005e9380, ATerrainSection::ModelNode*, void, pModelData);
						}
					}
				}
			}
		}
	}
}
