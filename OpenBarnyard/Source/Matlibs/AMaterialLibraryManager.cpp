#include "pch.h"
#include <Toshi/Render/TRenderInterface.h>
#include <Toshi/File/TTRB.h>

#include "AMaterialLibraryManager.h"
#include "Locale/ALocaleManager.h"

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
		}
	}
}

void AMaterialLibraryManager::LoadLibrary(const Toshi::TPString8& a_rLibName, Toshi::TTRB* a_pTRB, TBOOL a_bIsGUI)
{
	TIMPLEMENT();

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

		auto pLibrary = List::GetSingleton()->CreateLibrary(matlibPath, a_pTRB);

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

void AMaterialLibraryManager::CreateTextures(AMaterialLibrary* a_pMatLibrary)
{
	TIMPLEMENT();
}

void AMaterialLibraryManager::OnLibraryLoaded(TBOOL a_bIsGUI)
{
	auto pRenderer = TRenderInterface::GetSingleton();

	pRenderer->FlushDyingResources();
	pRenderer->FlushDyingResources();

	if (a_bIsGUI)
	{
		TTODO("Recreate GUI materials");
	}
}

AMaterialLibrary* AMaterialLibraryManager::List::CreateLibrary(const char* a_szFilePath, Toshi::TTRB* a_pTRB)
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
			bSuccess = LoadTTLData(pTTLData);
		}
		else
		{
			bSuccess = LoadTTLFile(a_szFilePath);
		}
	}
	else
	{
		bSuccess = LoadTTLFile(a_szFilePath);
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

TBOOL AMaterialLibraryManager::List::LoadTTLData(void* a_pTTLData)
{
	TIMPLEMENT();
	return TTRUE;
}

TBOOL AMaterialLibraryManager::List::LoadTTLFile(const char* a_szFilePath)
{
	TIMPLEMENT();
	return TTRUE;
}

AMaterialLibrary::AMaterialLibrary()
{
	m_Unk1 = 0;
	m_Unk2 = 0;
	m_iNumTextures = 0;
}
