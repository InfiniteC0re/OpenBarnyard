#pragma once
#include <Toshi/File/TTRB.h>

class AAssetLoader
{
public:
	using AssetType = TUINT;
	enum AssetType_ : AssetType
	{
		AssetType_ModelLib,
		AssetType_Startup,
		AssetType_WaveBank,
		AssetType_NUMOF,
	};

public:
	static TBOOL Load(const char* a_szFileName, AssetType a_eAssetType, TBOOL a_bUseStreaming);
	static void Close(AssetType a_eAssetType);

	static Toshi::TTRB* GetAssetTRB(AssetType a_eAssetType)
	{
		TASSERT(a_eAssetType < AssetType_NUMOF);
		return ms_pTRBFiles[a_eAssetType];
	}

private:
	inline static Toshi::TTRB* ms_pTRBFiles[AssetType_NUMOF];
};