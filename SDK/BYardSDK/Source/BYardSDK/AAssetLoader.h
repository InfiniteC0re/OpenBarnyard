#pragma once
#include <Toshi/File/TTRB.h>

using AAssetType = TUINT;
enum AAssetType_ : AAssetType
{
	AAssetType_ModelLib,
	AAssetType_Startup,
	AAssetType_WaveBank,
	AAssetType_NUMOF,
};

// TODO: use addresses of the game!!!

class AAssetLoader
{
public:
	AAssetLoader() = delete;

	static TBOOL Load(const char* a_szFileName, AAssetType a_eAssetType, TBOOL a_bUseStreaming);
	static void Close(AAssetType a_eAssetType);
	static void* GetSymbolAddress(const char* a_szFileName, const char* a_szSymbolName, AAssetType a_eAssetType);

	template <class T>
	static T* CastSymbol(const char* a_szFileName, const char* a_szSymbolName, AAssetType a_eAssetType)
	{
		return TSTATICCAST(
			T*,
			GetSymbolAddress(a_szFileName, a_szSymbolName, a_eAssetType)
		);
	}

	static Toshi::TTRB* GetAssetTRB(AAssetType a_eAssetType)
	{
		TASSERT(a_eAssetType < AAssetType_NUMOF);
		return ms_pTRBFiles[a_eAssetType];
	}

private:
	inline static Toshi::TTRB* ms_pTRBFiles[AAssetType_NUMOF];
};