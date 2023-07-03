#include "pch.h"
#include "AModelLoader.h"
#include <Toshi2/T2Material.h>
#include <Toshi/Render/TAssetInit.h>
#include <Toshi/Render/TShader.h>

using namespace Toshi;

AModelLoader::AModelLoader()
{
	TModel::SetTRBLoadCallback(AModelLoaderLoadTRBCallback);

	for (size_t i = 0; i < MAX_MATERIAL_COUNT; i++)
	{
		ms_oFreeMaterials.PushBack(&m_oNodesAlloc[i]);
	}
}

void AModelLoader::GetMaterial(TShader* pShader, const char* pName, TMaterial*& pOutMaterial, TXSMaterial::TextureType eTextureType)
{
	TASSERT(TAssetInit::GetInitialisingTRB());

	pOutMaterial = TNULL;
	auto pShaderPrefix = pShader->GetShaderPrefix();
	auto iShaderPrefixLength = TStringManager::String8Length(pShaderPrefix);

	if (iShaderPrefixLength == 0)
		return;

	TString8 sFullMatName = pShaderPrefix;
	sFullMatName += pName;

	auto pCreatedMaterial = GetCreatedMaterial(sFullMatName, TAssetInit::GetInitialisingTRB());
	
	if (!pCreatedMaterial)
	{
		char matName[128];
		T2String8::CopySafe(matName, pName, sizeof(matName));
		T2String8::ToLowerCase(matName);

		TMaterialManager* pMatManager = TMaterialManager::GetSingletonWeak();
		auto metaMat = pMatManager->FindMaterial(matName);
		TASSERT(TNULL != metaMat);

		TTexture* pMatTexture = metaMat->GetTexture(0);
		TBOOL bValidFormat = pMatTexture->CheckFormat();
		
		TTODO("Create ASkinMaterial if the shader if ASkinShader");

		TASSERT(!ms_oFreeMaterials.IsEmpty());
		auto pMatNode = ms_oFreeMaterials.PopFront();

		pMatNode->m_pTRB = TAssetInit::GetInitialisingTRB();
		pMatNode->m_pMaterial = pCreatedMaterial;
		pMatNode->m_iReferenceCount = 1;
		T2String8::Copy(pMatNode->m_Name, sFullMatName, 64);

		if (sFullMatName.Length() >= 31)
			sFullMatName[31] = '\0';
		
		pCreatedMaterial->SetName(sFullMatName);
		ms_oMaterialList.PushFront(pMatNode);

		TTODO("Set material flags, save info about material passes");
	}
	else
	{
		pOutMaterial = pCreatedMaterial;
	}
}

TBOOL AModelLoader::AModelLoaderLoadTRBCallback(TTRB& pTRB, TModel* pModel)
{
	return TFALSE;
}

TMaterial* AModelLoader::GetCreatedMaterial(const TString8& name, TTRB* pTRB)
{
	for (auto it = ms_oMaterialList.Begin(); it != ms_oMaterialList.End(); it++)
	{
		if (name == it->m_Name)
		{
			return it->m_pMaterial;
		}
	}

	return TNULL;
}

TBOOL AModelLoader::LoadTRBCallback2(TModel* pModel, TTMDWin::TTRBWinHeader* pHeader)
{
	pModel->m_iLODCount = pHeader->m_iLODCount;
	pModel->m_fUnknown = pHeader->m_fUnknown;

	for (size_t i = 0; i < pHeader->m_iLODCount; i++)
	{
		auto& lod = pModel->m_LODLevels[i];

		TTODO("...");
	}

	return TTRUE;
}

TBOOL AModelLoader::AModelLoaderLoadTRBCallback2(TModel* pModel, TTMDWin::TTRBWinHeader* pHeader)
{
	return LoadTRBCallback2(pModel, pHeader);
}