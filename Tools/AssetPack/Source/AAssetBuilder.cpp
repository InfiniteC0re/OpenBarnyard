#include "pch.h"
#include "AAssetBuilder.h"

TOSHI_NAMESPACE_USING

void AAssetBuilder::Add(const TString8& a_FileName)
{
	auto pTRB = m_Files.Push();
	
	if (pTRB->ReadFromFile(a_FileName.GetString()))
	{
		TASSERT(pTRB->GetSECT()->GetStackCount() > 0);

		auto pSYMB = pTRB->GetSYMB();

		TString8 symbolDirName = a_FileName.GetString(a_FileName.FindReverse('\\', -1) + 1);
		symbolDirName.Truncate(symbolDirName.FindReverse('.', -1));

		auto pAsset = m_AssetPack.GetAssets().Push();
		pAsset->Name = symbolDirName.MakeLower();
		pAsset->pTRB = pTRB;
		pAsset->pStack = pTRB->GetSECT()->GetStack(0);
		
		for (TUINT i = 0; i < pSYMB->GetCount(); i++)
		{
			pAsset->RelatedSymbols.Push(pSYMB->GetName(i).GetString());
		}

		TStringManager::String8Copy(pAsset->Format, a_FileName.GetString(a_FileName.Length() - 3), 4);
	}
	else
	{
		TERROR("Unable to read file '%s'\n", a_FileName.GetString());
	}
}

void AAssetBuilder::Save(const char* a_szFileName, TBOOL a_bCompress)
{
	m_AssetPack.Save(a_szFileName, a_bCompress);
}
