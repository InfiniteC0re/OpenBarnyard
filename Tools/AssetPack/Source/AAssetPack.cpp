#include "pch.h"
#include "AAssetPack.h"

TOSHI_NAMESPACE_USING

TBOOL AAssetPack::Load(const char* a_szFileName)
{
	m_FileName = a_szFileName;
	TBOOL bReaded = m_TRBFile.ReadFromFile(a_szFileName);

	m_bLoaded = TFALSE;

	if (bReaded)
	{
		auto pInSECT = m_TRBFile.GetSECT();
		auto pInSYMB = m_TRBFile.GetSYMB();

		for (TUINT i = 0; i < pInSYMB->GetCount(); i++)
		{
			TString8 name = pInSYMB->GetName(i);
			auto pStack = pInSYMB->GetStack(pInSECT, i);

			char szAssetFileName[MAX_PATH];
			auto iUnderlinePos = name.FindReverse('_', -1);

			TStringManager::String8Copy(szAssetFileName, name, iUnderlinePos);

			if (TINT32 iLODPos = name.Find("LOD"))
			{
				szAssetFileName[iLODPos >= 0 ? iLODPos - 1 : iUnderlinePos] = 0;
			}

			Asset_t* pFound = TNULL;

			for (TINT k = m_Assets.Size() - 1; k >= 0; k--)
			{
				if (m_Assets[k].pStack == pStack)
				{
					pFound = &m_Assets[k];
					break;
				}
			}

			if (!pFound)
			{
				m_Assets.PushBack({ .Name = szAssetFileName, .Format = "trb", .pStack = pStack });
				auto pAsset = &m_Assets.Back();

				TINT iAssetFileNameLength = TStringManager::String8Length(szAssetFileName);

				for (TUINT k = i; k < pInSYMB->GetCount(); k++)
				{
					TString8 inSymbolName = pInSYMB->GetName(k);
					auto pInSymbolAddress = pInSYMB->GetByIndex<char>(pInSECT, k);

					if (pInSymbolAddress.stack() == pAsset->pStack)
					{
						if (inSymbolName.Find("_keylib") >= 0)
						{
							pAsset->Format[0] = 't';
							pAsset->Format[1] = 'k';
							pAsset->Format[2] = 'l';
						}
						else if (inSymbolName.Find("_TTL") >= 0)
						{
							pAsset->Format[0] = 't';
							pAsset->Format[1] = 't';
							pAsset->Format[2] = 'l';
						}

						if (inSymbolName.Compare(szAssetFileName, iAssetFileNameLength) == 0)
						{
							auto name = TPString8(inSymbolName);
							pAsset->RelatedSymbols.Push(name);
							m_SymbolToAsset.Insert(name, pAsset);
						}
					}
					else
					{
						break;
					}
				}
			}
		}

		for (auto it = m_SymbolToAsset.Begin(); it != m_SymbolToAsset.End(); it++)
		{
			continue;
		}

		m_bLoaded = TTRUE;
	}

	return m_bLoaded;
}

void AAssetPack::Save(const char* a_szFileName)
{
	PTRB::TRBF outFile;

	auto pSECT = outFile.GetSECT();
	auto pSYMB = outFile.GetSYMB();

	for (auto it = m_Assets.Begin(); it != m_Assets.End(); it++)
	{
		auto pInStack = it->pStack;
		auto pOutStack = pSECT->CreateStack(pInStack);

		for (auto symbol = it->RelatedSymbols.Begin(); !symbol.IsOver(); symbol++)
		{
			auto pInPtr = m_TRBFile.GetSYMB()->Find<char>(m_TRBFile.GetSECT(), symbol->GetString8());

			pSYMB->Add(pOutStack, symbol->GetString8(), pOutStack->GetBuffer() + (pInPtr.get() - pInStack->GetBuffer()));
		}
	}

	outFile.WriteToFile(a_szFileName);
}

AAssetPack::Asset_t* AAssetPack::GetAssetFromSymbol(const Toshi::TPString8& a_rSymbolName)
{
	auto ppResult = m_SymbolToAsset.Find(a_rSymbolName);

	if (*ppResult != m_SymbolToAsset.End()->GetSecond())
	{
		return *ppResult;
	}

	return TNULL;
}
