#pragma once
#include <File/TTRB.h>

using AAssetType = TUINT;
enum AAssetType_ : AAssetType
{
	AAssetType_AssetPack,
	AAssetType_Startup,
	AAssetType_WaveBank,
	AAssetType_NUMOF,
};

class AAssetLoader
{
public:
	AAssetLoader() = delete;

	static TBOOL LoadAssetPackFromLibrary(const TCHAR* a_szLibraryName, TBOOL a_bStream);
	static TBOOL CreateAssetsFromLibrary(const TCHAR* a_szLibraryName);

	// Loads asset file as a specified asset
	static TBOOL Load(const TCHAR* a_szFileName, AAssetType a_eAssetType, TBOOL a_bStream);
	
	// Closes asset file
	static void Close(AAssetType a_eAssetType);

	// Returns address of symbol of specified asset
	static void* GetSymbolAddress(const TCHAR* a_szFileName, const TCHAR* a_szSymbolName, AAssetType a_eAssetType);

	template <class T>
	static T* CastSymbol(const TCHAR* a_szFileName, const TCHAR* a_szSymbolName, AAssetType a_eAssetType)
	{
		return TSTATICCAST(
			T*,
			GetSymbolAddress(a_szFileName, a_szSymbolName, a_eAssetType)
		);
	}

	// Returns TRB file of specified asset
	static Toshi::TTRB* GetAssetTRB(AAssetType a_eAssetType)
	{
		TASSERT(a_eAssetType < AAssetType_NUMOF);
		return ms_pTRBFiles[a_eAssetType];
	}

private:
	inline static Toshi::TTRB* ms_pTRBFiles[AAssetType_NUMOF];
};