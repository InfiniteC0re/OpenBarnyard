#include "pch.h"
#include <Toshi/Render/TRenderInterface.h>
#include <Toshi/File/TTRB.h>

#include "AGUI/AGUISystem.h"
#include "AGUI/AGUI2TextureSectionManager.h"
#include "AMaterialLibraryManager.h"
#include "Locale/ALocaleManager.h"

#include TOSHI_MULTIRENDER(TRenderInterface)

TOSHI_NAMESPACE_USING

AMaterialLibraryManager::AMaterialLibraryManager() :
	m_NumRefLibraries(AMemory::GetAllocator(AMemory::POOL_StringPool)),
	m_LoadedLibraries(AMemory::GetAllocator(AMemory::POOL_StringPool))
{
	List::CreateSingleton();

	m_iNumUsedTextures = 0;
	m_iNumFreeTextures = 0;

	for (TUINT i = 0; i < NUM_SLOTS; i++)
	{
		m_FreeTextures.PushFront(&m_aSlots[i]);
		m_iNumFreeTextures++;
	}
}

void AMaterialLibraryManager::LoadFromProperties(const PPropertyValue* a_pProperty, Toshi::TTRB* a_pTRB, TBOOL a_bUpdateGUIMaterials)
{
	if (a_pProperty)
	{
		auto pArray = a_pProperty->GetArray();

		for (TUINT i = 0; i < pArray->GetSize(); i++)
		{
			auto matlibName = pArray->GetValue(i)->GetTPString8();
			auto eLangId = ALocaleManager::GetSingleton()->GetLangId();

			TBOOL bLocaliseAsset;

			if (eLangId == ALocaleManager::Lang_English || eLangId == ALocaleManager::Lang_EnglishUK)
			{
				bLocaliseAsset = TFALSE;
			}
			else
			{
				bLocaliseAsset = TPString8("gui_loc") == matlibName;
			}

			if (bLocaliseAsset)
			{
				TString8 localisedAssetName = matlibName.GetString8();
				localisedAssetName += "_";
				localisedAssetName += ALocaleManager::Interface()->GetCurrentLanguageName();
				localisedAssetName.MakeLower();

				matlibName = TPString8(localisedAssetName);
			}

			LoadLibrary(matlibName, a_pTRB, TFALSE);
			TTODO("ALoadingScreen::Update");
		}

		TRenderInterface::GetSingleton()->FlushDyingResources();
		TRenderInterface::GetSingleton()->FlushDyingResources();

		if (a_bUpdateGUIMaterials && AGUISystem::GetSingleton())
		{
			AGUI2TextureSectionManager::UpdateMaterials();
		}

		TTODO("ALoadingScreen::Update");
	}
}

void AMaterialLibraryManager::LoadLibrary(const Toshi::TPString8& a_rLibName, Toshi::TTRB* a_pTRB, TBOOL a_bIsGUI)
{
	auto pRenderer = TRenderInterface::GetSingleton();

	if (m_LoadedLibraries.Find(a_rLibName) == &m_LoadedLibraries.End()->GetSecond())
	{
		Toshi::TString8 fileMiddle = "";
		Toshi::TString8 fileFormat = "";

		if (!pRenderer->Supports32BitTextures())
		{
			fileFormat = "16";
		}

		char matlibPath[128];
		TStringManager::String8Format(
			matlibPath,
			sizeof(matlibPath),
			"data/matlibs/%s%s%s.ttl",
			a_rLibName.GetPooledString()->GetString8().GetString(),
			fileMiddle.GetString(),
			fileFormat.GetString()
		);

		auto pLibrary = List::GetSingleton()->CreateLibraryFromAsset(matlibPath, a_pTRB);

		if (pLibrary)
		{
			m_LoadedLibraries.Insert(a_rLibName, pLibrary);
			CreateTextures(pLibrary);
			m_NumRefLibraries.Insert(a_rLibName, 1);

			if (a_rLibName.GetString8().Find("gui_") == 0)
			{
				a_bIsGUI = TTRUE;
			}

			OnLibraryLoaded(a_bIsGUI);
		}
	}
	else
	{
		auto pRefCount = m_NumRefLibraries.Find(a_rLibName);
		*pRefCount = *pRefCount + 1;
	}
}

void AMaterialLibraryManager::UnloadLibrary(const TPString8& a_rLibName, TBOOL a_bUnused)
{
	TPString8 matlibName = a_rLibName;
	auto eLangId = ALocaleManager::GetSingleton()->GetLangId();

	TBOOL bLocaliseAsset;

	if (eLangId == ALocaleManager::Lang_English || eLangId == ALocaleManager::Lang_EnglishUK)
	{
		bLocaliseAsset = TFALSE;
	}
	else
	{
		bLocaliseAsset = TPString8("gui_loc") == matlibName;
	}

	if (bLocaliseAsset)
	{
		TString8 localisedAssetName = matlibName.GetString8();
		localisedAssetName += "_";
		localisedAssetName += ALocaleManager::Interface()->GetCurrentLanguageName();
		localisedAssetName.MakeLower();

		matlibName = TPString8(localisedAssetName);
	}

	auto pRenderer = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	pRenderer->BeginEndScene();

	auto pMatlibNode = m_LoadedLibraries.FindNode(matlibName);
	
	if (pMatlibNode != m_LoadedLibraries.End())
	{
		auto pRefCountNode = m_NumRefLibraries.FindNode(matlibName);

		if (pRefCountNode != m_NumRefLibraries.End())
		{
			pRefCountNode->GetValue()->GetSecond() -= 1;

			if (pRefCountNode->GetValue()->GetSecond() == 0)
			{
				DestroyLibrary(pMatlibNode, a_bUnused);
				m_NumRefLibraries.RemoveNode(pRefCountNode);
			}
		}
	}
}

void AMaterialLibraryManager::DestroyLibrary(LibrariesMap::Node*& a_rpMaterialLibraryNode, TBOOL a_bUpdateGUIMaterials)
{
	auto pMaterialLibrary = a_rpMaterialLibraryNode->GetValue()->GetSecond();
	UnloadTexturesOfLibrary(pMaterialLibrary);
	
	pMaterialLibrary->Destroy();
	m_LoadedLibraries.RemoveNode(a_rpMaterialLibraryNode);

	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	if (a_bUpdateGUIMaterials && AGUISystem::GetSingleton())
	{
		AGUI2TextureSectionManager::UpdateMaterials();
	}
}

void AMaterialLibraryManager::UnloadTexturesOfLibrary(AMaterialLibrary* a_pMaterialLibrary)
{
	for (auto it = m_UsedTextures.Begin(); it != m_UsedTextures.End(); it = it->GetNext())
	{
		if (it->GetLibrary() == a_pMaterialLibrary)
		{
			AMaterialLibraryManager::TextureSlot* pRemovedSlot;
			m_UsedTextures.Remove(pRemovedSlot, it);
			m_iNumUsedTextures -= 1;

			pRemovedSlot->GetTexture()->DestroyResource();
			pRemovedSlot->SetTexture(TNULL);
			pRemovedSlot->ResetName();

			m_FreeTextures.PushFront(pRemovedSlot);
			m_iNumFreeTextures += 1;
		}
	}

	TRenderInterface::GetSingleton()->FlushDyingResources();
	TTODO("FUN_006120e0");
}

void AMaterialLibraryManager::CreateTextures(AMaterialLibrary* a_pMatLibrary)
{
	auto pTextureFactory = TSTATICCAST(
		TTextureFactory*,
		TRenderInterface::GetSingleton()->GetSystemResource(TRenderInterface::SYSRESOURCE_TEXTUREFACTORY)
	);
	
	for (TINT i = 0; i < a_pMatLibrary->GetNumTextures(); i++)
	{
		auto pMatlibTexture = a_pMatLibrary->GetTexture(i);
		
		auto pTexSlot = m_FreeTextures.PopFront();
		m_iNumFreeTextures -= 1;

		pTexSlot->SetName(pMatlibTexture->Name);
		pTexSlot->SetLibrary(a_pMatLibrary);
		pTexSlot->SetTexture(pTextureFactory->CreateFromT2Texture(pMatlibTexture->pTexture));

		m_UsedTextures.PushFront(pTexSlot);
		m_iNumUsedTextures += 1;
	}
}

void AMaterialLibraryManager::OnLibraryLoaded(TBOOL a_bUpdateGUIMaterials)
{
	TRenderInterface::GetSingleton()->FlushDyingResources();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	if (a_bUpdateGUIMaterials && AGUISystem::GetSingleton())
	{
		AGUI2TextureSectionManager::UpdateMaterials();
	}
}

Toshi::TTexture* AMaterialLibraryManager::FindTexture(const char* a_szTextureName)
{
	for (auto it = m_UsedTextures.Begin(); it != m_UsedTextures.End(); it = it->GetNext())
	{
		if (Toshi::TStringManager::String8CompareNoCase(it->GetName(), a_szTextureName) == 0)
		{
			return it->GetTexture();
		}
	}

	return TNULL;
}

AMaterialLibrary* AMaterialLibraryManager::List::CreateLibraryFromAsset(const char* a_szFilePath, Toshi::TTRB* a_pTRB)
{
	auto pLibrary = new AMaterialLibrary;
	pLibrary->SetPath(a_szFilePath);

	TBOOL bSuccess;

	if (a_pTRB)
	{
		auto iFilePathLength = TStringManager::String8Length(a_szFilePath);
		auto iFileNamePos = iFilePathLength - 1;

		while (a_szFilePath[iFileNamePos] != '\\' && a_szFilePath[iFileNamePos] != '/')
			iFileNamePos--;

		iFileNamePos++;

		char symbolName[132];
		auto iLen = iFilePathLength - iFileNamePos - 4;
		TStringManager::String8Copy(symbolName, a_szFilePath + iFileNamePos, iFilePathLength - iFileNamePos - 4);

		symbolName[iLen] = '_';
		symbolName[iLen + 1] = '\0';
		TStringManager::String8ToLowerCase(symbolName);
		TStringManager::String8Copy(symbolName + iLen + 1, "TTL");

		auto pTTLData = a_pTRB->GetSymbolAddress(symbolName);

		if (pTTLData)
		{
			bSuccess = pLibrary->LoadTTLData(pTTLData);
		}
		else
		{
			bSuccess = pLibrary->LoadTTLFile(a_szFilePath);
		}
	}
	else
	{
		bSuccess = pLibrary->LoadTTLFile(a_szFilePath);
	}

	if (!bSuccess)
	{
		if (pLibrary)
		{
			delete pLibrary;
			pLibrary = TNULL;
		}
	}
	else
	{
		m_Libraries.PushBack(pLibrary);
	}

	return pLibrary;
}

ATexture* AMaterialLibraryManager::List::FindTexture(const char* a_szTextureName, AMaterialLibrary** a_ppMaterialLibrary, TINT* a_pTextureIndex)
{
	TINT iIndex = -1;
	auto it = m_Libraries.Begin();

	while (true)
	{
		if (it == m_Libraries.End())
		{
			return TNULL;
		}

		iIndex = it->FindTextureIndex(a_szTextureName);
		if (iIndex != -1) break;

		it++;
	}

	if (a_ppMaterialLibrary)
	{
		*a_ppMaterialLibrary = it;
	}

	if (a_pTextureIndex)
	{
		*a_pTextureIndex = iIndex;
	}

	return it->GetTexture(iIndex);
}
