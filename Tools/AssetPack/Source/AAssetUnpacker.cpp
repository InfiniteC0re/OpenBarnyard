#include "pch.h"
#include "AAssetUnpacker.h"

TOSHI_NAMESPACE_USING

TBOOL AAssetUnpacker::Unpack(AAssetPack& a_rAssetPack, const Toshi::TString8& a_rOutPath, TBOOL a_bUseBTEC)
{
	if (a_rAssetPack.IsLoaded())
	{
		auto pTRB = a_rAssetPack.GetTRBFile();
		auto& assets = a_rAssetPack.GetAssets();

		for (TUINT i = 0; i < assets.Size(); i++)
		{
			auto pAsset = &assets[i];

			auto pInSECT = pTRB->GetSECT();
			auto pInSYMB = pTRB->GetSYMB();

			auto pRelatedSymbols = &pAsset->RelatedSymbols;

			PTRB::TRBF outTrbFile;
			auto pOutSECT = outTrbFile.GetSECT();
			auto pOutSYMB = outTrbFile.GetSYMB();

			auto pOutStack = pOutSECT->CreateStack(pAsset->pStack);

			for (auto it = pRelatedSymbols->Begin(); !it.IsOver(); it++)
			{
				auto pInSymbolAddress = pInSYMB->Find<char>(pInSECT, it->GetString8());
				TString8 outSymbolName = it->GetString8().GetString(pAsset->Name.GetString8().Length() + 1);

				pOutSYMB->Add(
					pOutStack,
					outSymbolName,
					pOutStack->GetBuffer() + (pInSymbolAddress.get() - pInSymbolAddress.stack()->GetBuffer())
				);
			}

			TString8 outPath = a_rOutPath;
			outPath += "\\";
			outPath += pAsset->Name.GetString8();
			outPath += ".";
			outPath += pAsset->Format;
			outTrbFile.WriteToFile(outPath.GetString(), a_bUseBTEC);
		}
	}
	
	return TFALSE;
}
