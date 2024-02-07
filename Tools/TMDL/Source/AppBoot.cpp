#include "pch.h"
#include "AArgumentParser.h"
#include "NvTriStrip/NvTriStrip.h"

#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>

#include <Core/TMemoryInitialiser.h>

#include <Render/TTMDBase.h>
#include <Render/TTMDWin.h>
#include <Render/TTerrainMDL.h>

#include <Plugins/PTRB.h>

#include <assimp/cexport.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

static inline void SetFaceAndAdvance3(aiFace*& face, unsigned int a, unsigned int b, unsigned int c)
{
	face->mNumIndices = 3;
	face->mIndices = new unsigned int[3];
	face->mIndices[0] = a;
	face->mIndices[1] = b;
	face->mIndices[2] = c;
	++face;
}

static void LogRenderGroup(TTerrainMDL::RenderGroup* a_pRenderGroup)
{
	TOSHI_INFO("      New render group:");
	TOSHI_INFO(
		"        Bounding: {} {} {} {}",
		a_pRenderGroup->m_BoundingSphere.GetOrigin().x,
		a_pRenderGroup->m_BoundingSphere.GetOrigin().y,
		a_pRenderGroup->m_BoundingSphere.GetOrigin().z,
		a_pRenderGroup->m_BoundingSphere.GetRadius()
	);

	TBOOL bHasNext = a_pRenderGroup->m_pNextGroup;

	while (a_pRenderGroup->m_pNextGroup)
	{
		LogRenderGroup(a_pRenderGroup->GetSubGroup());
		a_pRenderGroup = a_pRenderGroup->m_pNextGroup;
	}

	if (bHasNext)
	{
		TOSHI_INFO(
			"        Final bounding: {} {} {} {}",
			a_pRenderGroup->m_BoundingSphere.GetOrigin().x,
			a_pRenderGroup->m_BoundingSphere.GetOrigin().y,
			a_pRenderGroup->m_BoundingSphere.GetOrigin().z,
			a_pRenderGroup->m_BoundingSphere.GetRadius()
		);
	}
}

int main(int argc, char** argv)
{
	Toshi::TUtil::ToshiCreate(GetCommandLineA(), 0, 0);
	TUtil::SetTPStringPool(new TPString8Pool(1024, 0, &T2Allocator::s_GlobalAllocator, TNULL));

	AArgumentParser args(argv, argc);
	if (args.GetMode() == AArgumentParser::Mode::Info)
	{
		PTRB::TRBF inTRB(args.GetInPath());

		auto pInSECT = inTRB.GetSECT();
		auto pInSYMB = inTRB.GetSYMB();

		auto pInFileHeader = pInSYMB->Find<TTMDBase::FileHeader>(pInSECT, "FileHeader");
		auto pInDatabase = pInSYMB->Find<TTerrainMDL::TRBHeader>(pInSECT, "Database");
		auto pInSkeletonHeader = pInSYMB->Find<TTMDBase::SkeletonHeader>(pInSECT, "SkeletonHeader");
		auto pInSkeleton = pInSYMB->Find<TSkeleton>(pInSECT, "Skeleton");
		auto pInMaterials = pInSYMB->Find<TTMDBase::Materials>(pInSECT, "Materials");
		auto pInCollision = pInSYMB->Find<TTMDBase::Collision>(pInSECT, "Collision");
		auto pInHeader = pInSYMB->Find<TTMDWin::TRBWinHeader>(pInSECT, "Header");

		const TBOOL bIsSkin = pInFileHeader;
		const TBOOL bIsTerrain = pInDatabase;
		const TBOOL bHasSkeleton = pInSkeletonHeader;

		if (!bIsSkin && !bIsTerrain)
		{
			TOSHI_ERROR("Invalid input file!");
			return 1;
		}

		if (bIsSkin)
		{
			if (pInFileHeader->m_uiMagic != TMAKEFOUR("TMDL"))
			{
				TOSHI_ERROR("Invalid input file format!");
				return 1;
			}

			TOSHI_INFO("Successfully opened TMD file!");
			TOSHI_INFO("  Version: {}.{}", pInFileHeader->m_uiVersionMajor, pInFileHeader->m_uiVersionMinor);
		}
		else if (bIsTerrain)
		{
			TOSHI_INFO("Successfully opened Terrain Model file!");
			TOSHI_INFO("  Num Entries: {}", pInDatabase->m_uiNumEntries);

			for (TUINT i = 0; i < pInDatabase->m_uiNumEntries; i++)
			{
				auto pEntry = pInDatabase->m_ppEntries[i];
				TOSHI_INFO("  Entry {}:", i);
				TOSHI_INFO("    Num Models: {}", pEntry->m_uiNumModels);

				for (TUINT k = 0; k < pEntry->m_uiNumModels; k++)
				{
					auto pModel = pEntry->m_ppModels[k];
					TOSHI_INFO("    Model {}:", k);
					TOSHI_INFO("      Num Meshes: {}", pModel->m_uiNumMeshes);

					for (TUINT j = 0; j < pModel->m_uiNumMeshes; j++)
					{
						TOSHI_INFO("      Mesh {}:", j);
						auto pMeshBounding = pModel->m_ppMeshBoundings[j];

						TOSHI_INFO(
							"        Bounding Sphere: {} {} {} {}",
							pMeshBounding->m_BoundingSphere.GetOrigin().x,
							pMeshBounding->m_BoundingSphere.GetOrigin().y,
							pMeshBounding->m_BoundingSphere.GetOrigin().z,
							pMeshBounding->m_BoundingSphere.GetRadius()
						);

						auto pMesh = pMeshBounding->m_pMesh;
						TOSHI_INFO("        Unk: {}", pMesh->m_Unk);
						TOSHI_INFO("        Num Indices: {}", pMesh->m_uiNumIndices);
						TOSHI_INFO("        Num Vertices 1: {}", pMesh->m_uiNumVertices1);
						TOSHI_INFO("        Num Vertices 2: {}", pMesh->m_uiNumVertices2);
						TOSHI_INFO("        Material Name: {}", pMesh->m_szMaterialName);
					}

					LogRenderGroup(pModel->m_pRenderGroups);
				}
			}
		}
		
		if (bHasSkeleton)
		{
			TOSHI_INFO("  Keyframe Library: {}", pInSkeletonHeader->m_szTKLName);
			TOSHI_INFO("  Translation Base Index: {}", pInSkeletonHeader->m_iTBaseIndex);
			TOSHI_INFO("  Quaternion Base Index: {}", pInSkeletonHeader->m_iQBaseIndex);
			TOSHI_INFO("  Scale Base Index: {}", pInSkeletonHeader->m_iSBaseIndex);
			TOSHI_INFO("  Translation Key Count: {}", pInSkeletonHeader->m_iTKeyCount);
			TOSHI_INFO("  Quaternion Key Count: {}", pInSkeletonHeader->m_iQKeyCount);
			TOSHI_INFO("  Scale Key Count: {}", pInSkeletonHeader->m_iSKeyCount);
			TOSHI_INFO("  Num Bones: {}", pInSkeleton->GetBoneCount());

			for (TINT i = 0; i < pInSkeleton->GetBoneCount(); i++)
			{
				auto pBone = pInSkeleton->GetBone(i);

				TOSHI_INFO("    {}:", pBone->GetName());
				TOSHI_INFO("      Position: ({0}; {1}; {2})", pBone->GetPosition().x, pBone->GetPosition().y, pBone->GetPosition().z);
			}
		}

		TOSHI_INFO("  Materials Size: {}", pInMaterials->m_uiSectionSize);
		TOSHI_INFO("  Material Count: {}", pInMaterials->m_uiNumMaterials);
		TOSHI_INFO("  Collision Model Count: {}", pInCollision->m_iNumModels);
		TOSHI_INFO("  LOD Count: {}", pInHeader->m_iNumLODs);
		TOSHI_INFO("  LOD Distance: {}", pInHeader->m_fLODDistance);

		for (TINT i = 0; i < pInHeader->m_iNumLODs; i++)
		{
			auto pLOD = pInHeader->GetLOD(i);
			TINT iMeshCount = pLOD->m_iMeshCount1 + pLOD->m_iMeshCount2;

			TOSHI_INFO("  Information about LOD{}:", i);
			TOSHI_INFO("    Mesh Count: {}", iMeshCount);
			TOSHI_INFO("    Shader Type: {}", pLOD->m_iShader);

			if (bIsSkin)
			{
				for (TINT k = 0; k < iMeshCount; k++)
				{
					char szSymbolName[128];
					TStringManager::String8Format(szSymbolName, sizeof(szSymbolName), "LOD%d_Mesh_%d", i, k);

					auto pInLODMesh = pInSYMB->Find<TTMDWin::TRBLODMesh>(pInSECT, szSymbolName);

					TOSHI_INFO("    {}:", szSymbolName);
					TOSHI_INFO("      Sub Mesh Count: {}", pInLODMesh->m_uiNumSubMeshes);
					TOSHI_INFO("      Index Count: {}", pInLODMesh->m_uiNumIndices);
					TOSHI_INFO("      Vertex Count: {}", pInLODMesh->m_uiNumVertices);
					TOSHI_INFO("      Material Name: {}", pInLODMesh->m_szMaterialName);

					for (TUINT j = 0; j < pInLODMesh->m_uiNumSubMeshes; j++)
					{
						auto pSubMesh = &pInLODMesh->m_pSubMeshes[j];

						TOSHI_INFO("      Sub Mesh {}:", j);
						TOSHI_INFO("        Unk1: {}", pSubMesh->m_Unk1);
						TOSHI_INFO("        Vertex Count 1: {}", pSubMesh->m_uiNumVertices1);
						TOSHI_INFO("        Vertex Count 2: {}", pSubMesh->m_uiNumVertices2);
						TOSHI_INFO("        Index Count: {}", pSubMesh->m_uiNumIndices);
						TOSHI_INFO("        Bone Count: {}", pSubMesh->m_uiNumBones);
						TOSHI_INFO("        Zero: {}", pSubMesh->m_Zero);
						TOSHI_INFO("        Unk2: {}", pSubMesh->m_Unk2);
						TOSHI_INFO("        Unk3: {}", pSubMesh->m_Unk3);
						TOSHI_INFO("        Unk4: {}", pSubMesh->m_Unk4);
						TOSHI_INFO("        Unk5: {}", pSubMesh->m_Unk5);
						TOSHI_INFO("        Unk6: {}", pSubMesh->m_Unk6);
					}
				}
			}
		}
	}
	else if (args.GetMode() == AArgumentParser::Mode::Compile)
	{
		TString8 inFilepath = args.GetInPath();
		TString8 texturesPath = args.GetTexturesPath();
		TString8 outFilepath;

		for (TUINT i = 0; i < inFilepath.Length(); i++)
		{
			if (inFilepath[i] == '/')
			{
				inFilepath[i] = '\\';
			}
		}

		TString8 inputFileDirName;
		inputFileDirName.Copy(inFilepath, inFilepath.FindReverse('\\', -1) + 1);

		TString8 inputFileName = inFilepath.GetString(inFilepath.FindReverse('\\', -1) + 1);
		inputFileName.Truncate(inputFileName.FindReverse('.', -1));

		if (!args.GetOutPath())
		{
			outFilepath = TString8::Format("%s\\%s.trb", inputFileDirName.GetString(), inputFileName.GetString());
		}
		else
		{
			outFilepath = args.GetOutPath();
		}

		const aiScene* pImportScene = aiImportFile(inFilepath, aiProcessPreset_TargetRealtime_MaxQuality);
		
		PTRB::TRBF outTRB;
		auto pOutSECT = outTRB.GetSECT();
		auto pOutSYMB = outTRB.GetSYMB();

		auto pStack = pOutSECT->CreateStack();

		if (args.IsTerrain())
		{
			auto pOutDatabase = pStack->Alloc<TTerrainMDL::TRBHeader>();
			pOutDatabase->m_uiNumEntries = 1;

			pStack->Alloc<TTerrainMDL::Entry*>(&pOutDatabase->m_ppEntries, pOutDatabase->m_uiNumEntries);

			for (TUINT i = 0; i < pOutDatabase->m_uiNumEntries; i++)
			{
				auto pEntry = pStack->Alloc<TTerrainMDL::Entry>(&pOutDatabase->m_ppEntries[0]);
				pEntry->m_uiNumModels = 1;

				pStack->Alloc<TTerrainMDL::Model*>(&pEntry->m_ppModels, pEntry->m_uiNumModels);

				for (TUINT k = 0; k < pEntry->m_uiNumModels; k++)
				{
					TASSERT(k < 1, "Importing few models into a single file is not implemeted yet!");

					auto pModel = pStack->Alloc<TTerrainMDL::Model>(&pEntry->m_ppModels[0]);
					
					TUtil::MemClear(pModel->m_MetaData, sizeof(pModel->m_MetaData));
					pModel->m_uiNumMeshes = pImportScene->mNumMeshes;

					auto ppMeshBoundings = pStack->Alloc<TTerrainMDL::MeshBounding*>(&pModel->m_ppMeshBoundings, pModel->m_uiNumMeshes);

					TFLOAT fGlobalBoundingMinX = TMath::MAXFLOAT;
					TFLOAT fGlobalBoundingMinY = TMath::MAXFLOAT;
					TFLOAT fGlobalBoundingMinZ = TMath::MAXFLOAT;
					TFLOAT fGlobalBoundingMaxX = TMath::MINFLOAT;
					TFLOAT fGlobalBoundingMaxY = TMath::MINFLOAT;
					TFLOAT fGlobalBoundingMaxZ = TMath::MINFLOAT;

					for (TUINT j = 0; j < pModel->m_uiNumMeshes; j++)
					{
						auto pMeshBounding = pStack->Alloc<TTerrainMDL::MeshBounding>(&pModel->m_ppMeshBoundings[j]);

						auto pImportMesh = pImportScene->mMeshes[j];
						
						std::vector<TUINT16> indices;
						indices.reserve(pImportMesh->mNumFaces * 3);

						for (TUINT i = 0; i < pImportMesh->mNumFaces; i++)
						{
							indices.push_back(pImportMesh->mFaces[i].mIndices[0]);
							indices.push_back(pImportMesh->mFaces[i].mIndices[1]);
							indices.push_back(pImportMesh->mFaces[i].mIndices[2]);
						}

						TUINT16 uiNumPrimGroups;
						PrimitiveGroup* apPrimitiveGroups;

						TBOOL bGenerated = GenerateStrips(indices.data(), indices.size(), &apPrimitiveGroups, &uiNumPrimGroups);
						TASSERT(bGenerated, "Couldn't generate triangle strips!");
						TASSERT(uiNumPrimGroups == 1);

						auto pExportMesh = pStack->Alloc<TTerrainMDL::Mesh>(&pMeshBounding->m_pMesh);
						pExportMesh->m_Unk = 0;
						pExportMesh->m_uiNumIndices = apPrimitiveGroups[0].numIndices;
						pExportMesh->m_uiNumVertices1 = pImportMesh->mNumVertices;
						pExportMesh->m_uiNumVertices2 = pImportMesh->mNumVertices;

						auto pOutIndices = pStack->Alloc<TUINT16>(&pExportMesh->m_pIndices, pExportMesh->m_uiNumIndices);
						TUtil::MemCopy(pOutIndices.get(), apPrimitiveGroups[0].indices, sizeof(TUINT16) * pExportMesh->m_uiNumIndices);

						auto pOutVertices = pStack->Alloc<TTerrainMDL::WorldVertex>(&pExportMesh->m_pVertices, pExportMesh->m_uiNumVertices1);

						TFLOAT fBoundingMinX = TMath::MAXFLOAT;
						TFLOAT fBoundingMinY = TMath::MAXFLOAT;
						TFLOAT fBoundingMinZ = TMath::MAXFLOAT;
						TFLOAT fBoundingMaxX = TMath::MINFLOAT;
						TFLOAT fBoundingMaxY = TMath::MINFLOAT;
						TFLOAT fBoundingMaxZ = TMath::MINFLOAT;

						for (TUINT h = 0; h < pImportMesh->mNumVertices; h++)
						{
							auto pExportVertex = pOutVertices + h;
							auto pImportPos = &pImportMesh->mVertices[h];
							auto pImportNormal = &pImportMesh->mNormals[h];
							auto pImportColor = &pImportMesh->mColors[0][h];
							auto pImportUV = &pImportMesh->mTextureCoords[0][h];

							pExportVertex->Position = { pImportPos->x, pImportPos->y, pImportPos->z };
							pExportVertex->Normal = { pImportNormal->x, pImportNormal->y, pImportNormal->z };

							if (pImportMesh->HasVertexColors(0))
							{
								pExportVertex->Color = { pImportColor->r, pImportColor->g, pImportColor->b };
							}
							else
							{
								pExportVertex->Color = { 1.0f, 1.0f, 1.0f };
							}

							pExportVertex->UV = { pImportUV->x, -pImportUV->y };

							fBoundingMinX = TMath::Min(fBoundingMinX, pImportPos->x);
							fBoundingMinY = TMath::Min(fBoundingMinY, pImportPos->y);
							fBoundingMinZ = TMath::Min(fBoundingMinZ, pImportPos->z);
							fBoundingMaxX = TMath::Max(fBoundingMaxX, pImportPos->x);
							fBoundingMaxY = TMath::Max(fBoundingMaxY, pImportPos->y);
							fBoundingMaxZ = TMath::Max(fBoundingMaxZ, pImportPos->z);
						}

						fGlobalBoundingMinX = TMath::Min(fGlobalBoundingMinX, fBoundingMinX);
						fGlobalBoundingMinY = TMath::Min(fGlobalBoundingMinY, fBoundingMinY);
						fGlobalBoundingMinZ = TMath::Min(fGlobalBoundingMinZ, fBoundingMinZ);
						fGlobalBoundingMaxX = TMath::Max(fGlobalBoundingMaxX, fBoundingMaxX);
						fGlobalBoundingMaxY = TMath::Max(fGlobalBoundingMaxY, fBoundingMaxY);
						fGlobalBoundingMaxZ = TMath::Max(fGlobalBoundingMaxZ, fBoundingMaxZ);

						TFLOAT fBoundingCenterX = (fBoundingMaxX + fBoundingMinX) / 2.0f;
						TFLOAT fBoundingCenterY = (fBoundingMaxY + fBoundingMinY) / 2.0f;
						TFLOAT fBoundingCenterZ = (fBoundingMaxZ + fBoundingMinZ) / 2.0f;

						TFLOAT fBoundingRadius = 0.0f;
						fBoundingRadius = TMath::Max(fBoundingMaxX - fBoundingMinX, fBoundingRadius);
						fBoundingRadius = TMath::Max(fBoundingMaxY - fBoundingMinY, fBoundingRadius);
						fBoundingRadius = TMath::Max(fBoundingMaxZ - fBoundingMinZ, fBoundingRadius);

						pMeshBounding->m_BoundingSphere.Set(
							fBoundingCenterX,
							fBoundingCenterY,
							fBoundingCenterZ,
							fBoundingRadius
						);

						auto pImportMaterial = pImportScene->mMaterials[pImportMesh->mMaterialIndex];
						aiString importMaterialName = pImportMaterial->GetName();

						auto pszExportMaterial = pStack->Alloc<char>(&pExportMesh->m_szMaterialName, importMaterialName.length + 1);
						TStringManager::String8Copy(pszExportMaterial.get(), importMaterialName.C_Str());
					}

					TFLOAT fGlobalBoundingCenterX = (fGlobalBoundingMaxX + fGlobalBoundingMinX) / 2.0f;
					TFLOAT fGlobalBoundingCenterY = (fGlobalBoundingMaxY + fGlobalBoundingMinY) / 2.0f;
					TFLOAT fGlobalBoundingCenterZ = (fGlobalBoundingMaxZ + fGlobalBoundingMinZ) / 2.0f;

					TFLOAT fGlobalBoundingRadius = 0.0f;
					fGlobalBoundingRadius = TMath::Max(fGlobalBoundingMaxX - fGlobalBoundingMinX, fGlobalBoundingRadius);
					fGlobalBoundingRadius = TMath::Max(fGlobalBoundingMaxY - fGlobalBoundingMinY, fGlobalBoundingRadius);
					fGlobalBoundingRadius = TMath::Max(fGlobalBoundingMaxZ - fGlobalBoundingMinZ, fGlobalBoundingRadius);

					auto pGlobalRenderGroup = pStack->Alloc<TTerrainMDL::RenderGroup>(&pModel->m_pRenderGroups);
					
					pGlobalRenderGroup->m_BoundingSphere.Set(
						fGlobalBoundingCenterX,
						fGlobalBoundingCenterY,
						fGlobalBoundingCenterZ,
						fGlobalBoundingRadius
					);

					// TODO: Support sub render groups to minimalize the performance impact caused by big models
					pGlobalRenderGroup->m_pNextGroup = TNULL;

					auto pRenderGroupData = pStack->Alloc<TTerrainMDL::RenderGroupData>();
					pRenderGroupData->m_uiNumMeshes = pModel->m_uiNumMeshes;

					auto pRenderGroupDataMeshes = pStack->Alloc<TUINT16>(pRenderGroupData->m_uiNumMeshes);
					TASSERT(pRenderGroupData->m_uiNumMeshes < TMath::TUINT16_MAX);

					for (TUINT h = 0; h < pRenderGroupData->m_uiNumMeshes; h++)
					{
						*(pRenderGroupDataMeshes + h) = h;
					}
				}
			}

			pOutSYMB->Add(pStack, "Database", pOutDatabase.get());
		}
		else
		{
			// TTMDBase::FileHeader
			auto pOutFileHeader = pStack->Alloc<TTMDBase::FileHeader>();
			pOutFileHeader->m_uiMagic = TMAKEFOUR("TMDL");
			pOutFileHeader->m_uiZero1 = 0;
			pOutFileHeader->m_uiZero2 = 0;
			pOutFileHeader->m_uiVersionMajor = 2;
			pOutFileHeader->m_uiVersionMinor = 0;

			pOutSYMB->Add(pStack, "FileHeader", pOutFileHeader.get());
		}

		if (TFALSE == args.NoSkeleton())
		{
			// TTMDBase::SkeletonHeader
			sizeof(TTMDBase::FileHeader);
			auto pOutSkeletonHeader = pStack->Alloc<TTMDBase::SkeletonHeader>();
			TStringManager::String8Copy(pOutSkeletonHeader->m_szTKLName, args.GetKeyLibName());
			pOutSkeletonHeader->m_iTKeyCount = 0;
			pOutSkeletonHeader->m_iQKeyCount = 0;
			pOutSkeletonHeader->m_iSKeyCount = 0;
			pOutSkeletonHeader->m_iTBaseIndex = 0;
			pOutSkeletonHeader->m_iQBaseIndex = 0;
			pOutSkeletonHeader->m_iSBaseIndex = 0;

			pOutSYMB->Add(pStack, "SkeletonHeader", pOutSkeletonHeader.get());

			// TSkeleton
			auto pOutSkeleton = pStack->Alloc<TSkeleton>();

			TINT iNumBones = 0;
			for (TUINT i = 0; i < pImportScene->mNumMeshes; i++)
			{
				iNumBones += pImportScene->mMeshes[i]->mNumBones;
			}

			if (iNumBones == 0)
			{
				// Create root bone if it doesn't exist
				pOutSkeleton->m_iBoneCount = 1;
				auto pRootBone = pStack->Alloc(&pOutSkeleton->m_pBones);

				pRootBone->m_Rotation = TQuaternion::IDENTITY;
				pRootBone->m_Position = { 0, 5, 0 };
				pRootBone->m_Transform = TMatrix44::IDENTITY;
				pRootBone->m_Transform.SetTranslation(pRootBone->m_Position);
				pRootBone->m_TransformInv.Invert(pRootBone->m_Transform);
				pRootBone->m_iParentBone = -1;

				pRootBone->m_iNameLength = inputFileName.Length();
				TStringManager::String8Copy(pRootBone->m_szName, inputFileName);
			}
			else
			{
				TASSERT(!"Importing meshes with bones is not implemented yet!");
				/*pOutSkeleton->m_iBoneCount = iNumBones;
				auto pOutBones = pStack->Alloc(&pOutSkeleton->m_pBones, pOutSkeleton->m_iBoneCount);

				for (TINT i = 0; i < pOutSkeleton->m_iBoneCount; i++)
				{
					auto pBone = pOutBones + i;

					pBone->m_Rotation = TQuaternion::IDENTITY;
					pBone->m_Position = TVector3::VEC_ZERO;
					pBone->m_Transform = TMatrix44::IDENTITY;
					pBone->m_TransformInv.Invert(pBone->m_Transform);
					pBone->m_iParentBone = -1;

					pBone->m_iNameLength = pInSkeleton->m_pBones[i].m_iNameLength;
					TStringManager::String8Copy(pBone->m_szName, pInSkeleton->m_pBones[i].m_szName);
				}*/
			}

			pOutSYMB->Add(pStack, "Skeleton", pOutSkeleton.get());
		}

		// TTMDBase::Materials
		auto pOutMaterials = pStack->Alloc<TTMDBase::Materials>();
		pOutMaterials->m_uiNumMaterials = pImportScene->mNumMaterials;
		pOutMaterials->m_uiSectionSize = pOutMaterials->m_uiNumMaterials * sizeof(TTMDBase::Material);

		auto pMaterials = pStack->Alloc<TTMDBase::Material>(pOutMaterials->m_uiNumMaterials);

		for (TUINT i = 0; i < pImportScene->mNumMaterials; i++)
		{
			auto pMaterial = pMaterials + i;
			auto pImportMaterial = pImportScene->mMaterials[i];

			aiString aiTexPath;
			pImportMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath);

			char texAbsPathFull[MAX_PATH];

			TString8 texAbsPath = inputFileDirName;
			texAbsPath += aiTexPath.C_Str();

			GetFullPathName(texAbsPath, MAX_PATH, texAbsPathFull, nullptr);
			TString8 texPath = texAbsPathFull + TStringManager::String8Length(texturesPath);

			if (texPath.Find(".dds", texPath.Length() - 4))
			{
				texPath[texPath.Length() - 3] = 't';
				texPath[texPath.Length() - 2] = 'g';
				texPath[texPath.Length() - 1] = 'a';
			}

			TStringManager::String8Copy(pMaterial->m_szMatName, pImportMaterial->GetName().C_Str());
			TStringManager::String8Copy(pMaterial->m_szTextureFile, texPath);
		}

		pOutSYMB->Add(pStack, "Materials", pOutMaterials.get());

		// TTMDBase::Collsion
		auto pOutCollision = pStack->Alloc<TTMDBase::Collision>();
		pOutCollision->m_iNumModels = 0;
		pOutCollision->m_pModels = TNULL;

		pOutSYMB->Add(pStack, "Collision", pOutCollision.get());

		// TTMDWin::TRBWinHeader
		auto pOutHeader = pStack->Alloc<TTMDWin::TRBWinHeader>();
		pOutHeader->m_iNumLODs = 1;
		pOutHeader->m_fLODDistance = 0.5f;

		pOutSYMB->Add(pStack, "Header", pOutHeader.get());

		auto pOutLODs = pStack->Alloc<TTMDWin::TRBLODHeader>(pOutHeader->m_iNumLODs);

		for (TINT i = 0; i < pOutHeader->m_iNumLODs; i++)
		{
			auto pOutLOD = pOutLODs + i;

			pOutLOD->m_iMeshCount1 = pImportScene->mNumMeshes;
			pOutLOD->m_iMeshCount2 = 0;

			/*
			* MODELSHADER_SKIN = 0
			* MODELSHADER_WORLD = 2
			* MODELSHADER_GRASS = 4
			*/
			pOutLOD->m_iShader = args.IsTerrain() ? 2 : 0;

			TFIXME("Calculate render volume of LOD");
			pOutLOD->m_RenderVolume.Set(0.0f, 0.0f, 0.0f, 5000.0f);

			TINT iMeshCount = pOutLOD->m_iMeshCount1 + pOutLOD->m_iMeshCount2;

			if (!args.IsTerrain())
			{
				for (TINT k = 0; k < iMeshCount; k++)
				{
					char szSymbolName[128];
					TStringManager::String8Format(szSymbolName, sizeof(szSymbolName), "LOD%d_Mesh_%d", i, k);

					auto pOutMesh = pStack->Alloc<TTMDWin::TRBLODMesh>();
					auto pImportMesh = pImportScene->mMeshes[k];

					auto pImportMeshMaterial = pImportScene->mMaterials[pImportMesh->mMaterialIndex];
					pStack->Alloc<char>(&pOutMesh->m_szMaterialName, pImportMeshMaterial->GetName().length + 1);
					TStringManager::String8Copy(pOutMesh->m_szMaterialName, pImportMeshMaterial->GetName().C_Str());

					std::vector<TUINT16> indices;
					indices.reserve(pImportMesh->mNumFaces * 3);

					for (TUINT i = 0; i < pImportMesh->mNumFaces; i++)
					{
						indices.push_back(pImportMesh->mFaces[i].mIndices[0]);
						indices.push_back(pImportMesh->mFaces[i].mIndices[1]);
						indices.push_back(pImportMesh->mFaces[i].mIndices[2]);
					}

					TUINT16 uiNumPrimGroups;
					PrimitiveGroup* apPrimitiveGroups;

					TBOOL bGenerated = GenerateStrips(indices.data(), indices.size(), &apPrimitiveGroups, &uiNumPrimGroups);

					if (bGenerated)
					{
						pOutMesh->m_uiNumSubMeshes = uiNumPrimGroups;
						pOutMesh->m_uiNumVertices = pImportMesh->mNumVertices;
						pOutMesh->m_uiNumIndices = 0;

						auto pOutSubMeshes = pStack->Alloc<TTMDWin::SubMesh>(&pOutMesh->m_pSubMeshes, pOutMesh->m_uiNumSubMeshes);

						for (TUINT j = 0; j < pOutMesh->m_uiNumSubMeshes; j++)
						{
							auto pPG = &apPrimitiveGroups[j];
							auto pOutSubMesh = pOutSubMeshes + j;

							pOutSubMesh->m_uiNumBones = 1;
							pOutSubMesh->m_uiNumIndices = pPG->numIndices;
							pOutSubMesh->m_uiNumVertices1 = pOutMesh->m_uiNumVertices;
							pOutSubMesh->m_uiNumVertices2 = pOutMesh->m_uiNumVertices;
							pOutSubMesh->m_Unk2 = 1277680;
							pOutSubMesh->m_Unk3 = 2034559604;
							pOutSubMesh->m_Unk4 = 2034728964;
							pOutSubMesh->m_Unk5 = 2034474770;
							pOutSubMesh->m_Unk6 = 2034728980;

							auto pOutBones = pStack->Alloc<TUINT32>(&pOutSubMesh->m_pBones, pOutSubMesh->m_uiNumBones);

							for (TUINT h = 0; h < pOutSubMesh->m_uiNumBones; h++)
							{
								*(pOutBones + h) = 0;
							}

							auto pOutVertices = pStack->Alloc<TTMDWin::SkinVertex>(&pOutSubMesh->m_pVertices, pOutSubMesh->m_uiNumVertices1);

							for (TINT h = 0; h < pOutSubMesh->m_uiNumVertices1; h++)
							{
								auto pOutVertex = pOutVertices + h;

								pOutVertex->Position.x = pImportMesh->mVertices[h].x;
								pOutVertex->Position.y = pImportMesh->mVertices[h].y;
								pOutVertex->Position.z = pImportMesh->mVertices[h].z;
								pOutVertex->Normal.x = pImportMesh->mNormals[h].x;
								pOutVertex->Normal.y = pImportMesh->mNormals[h].y;
								pOutVertex->Normal.z = pImportMesh->mNormals[h].z;
								pOutVertex->Weights[0] = TUINT8(1.0f * 255);
								pOutVertex->Weights[1] = 0;
								pOutVertex->Weights[2] = 0;
								pOutVertex->Weights[4] = 0;
								pOutVertex->Bones[0] = 0;
								pOutVertex->Bones[1] = 0;
								pOutVertex->Bones[2] = 0;
								pOutVertex->Bones[4] = 0;
								pOutVertex->UV.x = pImportMesh->mTextureCoords[0][h].x;
								pOutVertex->UV.y = pImportMesh->mTextureCoords[0][h].y * -1.0f;
							}

							auto pOutIndices = pStack->Alloc<TUINT16>(&pOutSubMesh->m_pIndices, pOutSubMesh->m_uiNumIndices);

							for (TUINT h = 0; h < pOutSubMesh->m_uiNumIndices; h++)
							{
								*(pOutIndices + h) = pPG->indices[h];
							}

							pOutMesh->m_uiNumIndices += pOutSubMesh->m_uiNumIndices;
						}
					}
					else
					{
						TOSHI_ERROR("Unable to generate tri strips from mesh!");
					}

					pOutSYMB->Add(pStack, szSymbolName, pOutMesh.get());
				}
			}
		}

		outTRB.WriteToFile(outFilepath.GetString(), args.IsUsingBTEC());
	}
	else if (args.GetMode() == AArgumentParser::Mode::Decompile)
	{
		TString8 inFilepath = args.GetInPath();
		TString8 texturesPath = args.GetTexturesPath();

		for (TUINT i = 0; i < inFilepath.Length(); i++)
		{
			if (inFilepath[i] == '/')
			{
				inFilepath[i] = '\\';
			}
		}

		TString8 inputFileDirName;
		inputFileDirName.Copy(inFilepath, inFilepath.FindReverse('\\', -1) + 1);

		TString8 inputFileName = inFilepath.GetString(inFilepath.FindReverse('\\', -1) + 1);
		inputFileName.Truncate(inputFileName.FindReverse('.', -1));

		PTRB::TRBF inTRB(inFilepath.GetString());

		auto pInSECT = inTRB.GetSECT();
		auto pInSYMB = inTRB.GetSYMB();

		auto pInFileHeader = pInSYMB->Find<TTMDBase::FileHeader>(pInSECT, "FileHeader");
		auto pInDatabase = pInSYMB->Find<TTerrainMDL::TRBHeader>(pInSECT, "Database");
		auto pInSkeletonHeader = pInSYMB->Find<TTMDBase::SkeletonHeader>(pInSECT, "SkeletonHeader");
		auto pInSkeleton = pInSYMB->Find<TSkeleton>(pInSECT, "Skeleton");
		auto pInMaterials = pInSYMB->Find<TTMDBase::Materials>(pInSECT, "Materials");
		auto pInCollision = pInSYMB->Find<TTMDBase::Collision>(pInSECT, "Collision");
		auto pInHeader = pInSYMB->Find<TTMDWin::TRBWinHeader>(pInSECT, "Header");

		const TBOOL bIsSkin = pInFileHeader;
		const TBOOL bIsTerrain = pInDatabase;
		const TBOOL bHasSkeleton = pInSkeletonHeader;

		aiScene scene;
		scene.mRootNode = new aiNode();

		scene.mNumMaterials = pInMaterials->m_uiNumMaterials;
		scene.mMaterials = new aiMaterial*[scene.mNumMaterials];

		for (TUINT i = 0; i < scene.mNumMaterials; i++)
		{
			scene.mMaterials[i] = new aiMaterial();

			aiString matName(pInMaterials->GetMaterial(i)->m_szMatName);

			aiString texName(args.GetTexturesPath());
			texName.Append(pInMaterials->GetMaterial(i)->m_szTextureFile);
			texName.data[texName.length - 3] = 'd';
			texName.data[texName.length - 2] = 'd';
			texName.data[texName.length - 1] = 's';

			scene.mMaterials[i]->AddProperty(
				&matName,
				AI_MATKEY_NAME
			);

			scene.mMaterials[i]->AddProperty(
				&texName,
				AI_MATKEY_TEXTURE_DIFFUSE(0)
			);
		}

		if (bIsSkin)
		{
			auto pLOD = pInHeader->GetLOD(0);
			TINT iMeshCount = pLOD->m_iMeshCount1 + pLOD->m_iMeshCount2;

			TUINT uiTotalNumMeshes = 0;

			for (TINT i = 0; i < iMeshCount; i++)
			{
				char szSymbolName[128];
				TStringManager::String8Format(szSymbolName, sizeof(szSymbolName), "LOD0_Mesh_%d", i);

				auto pInLODMesh = pInSYMB->Find<TTMDWin::TRBLODMesh>(pInSECT, szSymbolName);
				uiTotalNumMeshes += pInLODMesh->m_uiNumSubMeshes;
			}

			scene.mNumMeshes = uiTotalNumMeshes;
			scene.mMeshes = new aiMesh * [scene.mNumMeshes];

			scene.mRootNode->mNumMeshes = scene.mNumMeshes;
			scene.mRootNode->mMeshes = new unsigned int[scene.mRootNode->mNumMeshes];

			for (TUINT i = 0; i < scene.mNumMeshes; i++)
			{
				scene.mRootNode->mMeshes[i] = i;
			}

			for (TINT i = 0; i < uiTotalNumMeshes;)
			{
				for (TINT k = 0; k < iMeshCount; k++)
				{
					char szSymbolName[128];
					TStringManager::String8Format(szSymbolName, sizeof(szSymbolName), "LOD0_Mesh_%d", k);

					auto pInLODMesh = pInSYMB->Find<TTMDWin::TRBLODMesh>(pInSECT, szSymbolName);

					for (TUINT j = 0; j < pInLODMesh->m_uiNumSubMeshes; j++)
					{
						auto pInSubMesh = &pInLODMesh->m_pSubMeshes[j];
						auto pMesh = new aiMesh();

						// TODO: support exporting bones
						pMesh->mNumBones = pInSubMesh->m_uiNumBones;
						pMesh->mBones = new aiBone * [pMesh->mNumBones];

						for (TUINT h = 0; h < pMesh->mNumBones; h++)
						{
							auto pImportBone = pInSkeleton->GetBone(pInSubMesh->m_pBones[h]);

							auto pExportBone = new aiBone();
							pExportBone->mName = pImportBone->GetName();

							pMesh->mBones[h] = pExportBone;
						}

						pMesh->mMaterialIndex = 0;

						for (TUINT h = 0; h < scene.mNumMaterials; h++)
						{
							if (scene.mMaterials[h]->GetName() == aiString(pInLODMesh->m_szMaterialName))
							{
								pMesh->mMaterialIndex = i;
								break;
							}
						}

						pMesh->mNumVertices = pInSubMesh->m_uiNumVertices2;
						pMesh->mVertices = new aiVector3D[pMesh->mNumVertices];
						pMesh->mNormals = new aiVector3D[pMesh->mNumVertices];
						pMesh->mTextureCoords[0] = new aiVector3D[pMesh->mNumVertices];
						pMesh->mNumUVComponents[0] = pMesh->mNumVertices;
						pMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
						pMesh->mNumFaces = pInSubMesh->m_uiNumIndices - 2;

						pMesh->mFaces = new aiFace[pMesh->mNumFaces];
						auto facePtr = pMesh->mFaces;

						for (TUINT h = 0; h < pMesh->mNumFaces; ++h)
						{
							//The ordering is to ensure that the triangles are all drawn with the same orientation

							if ((h + 1) % 2 == 0)
							{
								//For even n, vertices n + 1, n, and n + 2 define triangle n
								SetFaceAndAdvance3(facePtr, pInSubMesh->m_pIndices[h + 1], pInSubMesh->m_pIndices[h], pInSubMesh->m_pIndices[h + 2]);
							}
							else
							{
								//For odd n, vertices n, n+1, and n+2 define triangle n
								SetFaceAndAdvance3(facePtr, pInSubMesh->m_pIndices[h], pInSubMesh->m_pIndices[h + 1], pInSubMesh->m_pIndices[h + 2]);
							}
						}

						for (TUINT h = 0; h < pInSubMesh->m_uiNumVertices2; h++)
						{
							auto pVertex = &pInLODMesh->m_pSubMeshes[0].m_pVertices[h];

							pMesh->mVertices[h] = aiVector3D(pVertex->Position.x, pVertex->Position.y, pVertex->Position.z);
							pMesh->mNormals[h] = aiVector3D(pVertex->Normal.x, pVertex->Normal.y, pVertex->Normal.z);
							pMesh->mTextureCoords[0][h] = aiVector3D(pVertex->UV.x, -pVertex->UV.y, 0.0f);
						}

						scene.mMeshes[i++] = pMesh;
					}
				}
			}
		}
		else if (bIsTerrain)
		{
			TUINT uiNumMeshes = 0;

			for (TUINT i = 0; i < pInDatabase->m_uiNumEntries; i++)
			{
				auto pEntry = pInDatabase->m_ppEntries[i];

				for (TUINT k = 0; k < pEntry->m_uiNumModels; k++)
				{
					uiNumMeshes += pEntry->m_ppModels[k]->m_uiNumMeshes;
				}
			}

			scene.mNumMeshes = uiNumMeshes;
			scene.mMeshes = new aiMesh * [scene.mNumMeshes];

			scene.mRootNode->mNumMeshes = scene.mNumMeshes;
			scene.mRootNode->mMeshes = new unsigned int[scene.mRootNode->mNumMeshes];

			for (TUINT i = 0; i < scene.mNumMeshes; i++)
			{
				scene.mRootNode->mMeshes[i] = i;
			}

			for (TUINT i = 0; i < pInDatabase->m_uiNumEntries; i++)
			{
				auto pEntry = pInDatabase->m_ppEntries[i];

				for (TUINT k = 0; k < pEntry->m_uiNumModels; k++)
				{
					auto pModel = pEntry->m_ppModels[k];

					for (TUINT j = 0; j < pModel->m_uiNumMeshes; j++)
					{
						auto pMeshBounding = pModel->m_ppMeshBoundings[j];
						auto pImportMesh = pMeshBounding->m_pMesh;

						auto pMesh = new aiMesh();

						pMesh->mMaterialIndex = 0;

						for (TUINT h = 0; h < scene.mNumMaterials; h++)
						{
							if (scene.mMaterials[h]->GetName() == aiString(pImportMesh->m_szMaterialName))
							{
								pMesh->mMaterialIndex = h;
								break;
							}
						}

						pMesh->mNumVertices = pImportMesh->m_uiNumVertices1;
						pMesh->mVertices = new aiVector3D[pMesh->mNumVertices];
						pMesh->mNormals = new aiVector3D[pMesh->mNumVertices];
						pMesh->mColors[0] = new aiColor4D[pMesh->mNumVertices];
						pMesh->mTextureCoords[0] = new aiVector3D[pMesh->mNumVertices];
						pMesh->mNumUVComponents[0] = pMesh->mNumVertices;
						pMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
						pMesh->mNumFaces = pImportMesh->m_uiNumIndices - 2;

						pMesh->mFaces = new aiFace[pMesh->mNumFaces];
						auto facePtr = pMesh->mFaces;

						for (TUINT h = 0; h < pMesh->mNumFaces; ++h)
						{
							//The ordering is to ensure that the triangles are all drawn with the same orientation

							if ((h + 1) % 2 == 0)
							{
								//For even n, vertices n + 1, n, and n + 2 define triangle n
								SetFaceAndAdvance3(facePtr, pImportMesh->m_pIndices[h + 1], pImportMesh->m_pIndices[h], pImportMesh->m_pIndices[h + 2]);
							}
							else
							{
								//For odd n, vertices n, n+1, and n+2 define triangle n
								SetFaceAndAdvance3(facePtr, pImportMesh->m_pIndices[h], pImportMesh->m_pIndices[h + 1], pImportMesh->m_pIndices[h + 2]);
							}
						}

						for (TUINT h = 0; h < pImportMesh->m_uiNumVertices1; h++)
						{
							auto pVertex = &pImportMesh->m_pVertices[h];

							pMesh->mVertices[h] = aiVector3D(pVertex->Position.x, pVertex->Position.y, pVertex->Position.z);
							pMesh->mNormals[h] = aiVector3D(pVertex->Normal.x, pVertex->Normal.y, pVertex->Normal.z);
							pMesh->mColors[0][h] = aiColor4D(pVertex->Color.x, pVertex->Color.y, pVertex->Color.z, 1.0f);
							pMesh->mTextureCoords[0][h] = aiVector3D(pVertex->UV.x, -pVertex->UV.y, 0.0f);
						}

						scene.mMeshes[i++] = pMesh;
					}
				}
			}
		}

		const char* szFileFormat = "fbx";

		auto uiNumExportFormats = aiGetExportFormatCount();
		const aiExportFormatDesc* pExportFormat = TNULL;

		for (TUINT i = 0; i < uiNumExportFormats; i++)
		{
			pExportFormat = aiGetExportFormatDescription(i);

			if (TStringManager::String8Compare(pExportFormat->fileExtension, szFileFormat) == 0)
			{
				break;
			}
		}

		unsigned int processFlags =
			aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
			aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
			aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
			aiProcess_LimitBoneWeights | // limit bone weights to 4 per vertex
			aiProcess_PreTransformVertices | //-- fixes the transformation issue.
			aiProcess_SplitByBoneCount | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
			0;

		TString8 outFilepath;
		if (!args.GetOutPath())
		{
			outFilepath = TString8::Format("%s\\%s.%s", inputFileDirName.GetString(), inputFileName.GetString(), szFileFormat);
		}
		else
		{
			outFilepath = args.GetOutPath();
		}

		aiExportScene(&scene, pExportFormat->id, outFilepath, processFlags);
	}

	return 0;
}