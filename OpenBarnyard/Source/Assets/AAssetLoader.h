#pragma once
#include <File/TTRB.h>
#include <Toshi/TString8.h>

class PBPropertyValue;

using AAssetType = TUINT;
enum AAssetType_ : AAssetType
{
	AAssetType_AssetPack,
	AAssetType_Startup,
	AAssetType_WaveBank,
	AAssetType_NUMOF,
};

class MODAPI AAssetLoader
{
public:
	AAssetLoader() = delete;

	static TBOOL LoadAssetPackOfLibrary( const TCHAR* a_szLibraryName, TBOOL a_bStream );
	static TBOOL DestroyAssetPackOfLibrary( const TCHAR* a_szLibraryName );
	static TBOOL CreateAssetsFromLibrary( const TCHAR* a_szLibraryName );

	// Loads asset file as a specified asset
	static TBOOL Load( const TCHAR* a_szFileName, AAssetType a_eAssetType, TBOOL a_bStream );

	// Closes asset file
	static void Close( AAssetType a_eAssetType );

	// Closes the files of every asset type
	static void CloseAll();

	// Returns address of symbol of specified asset
	static void* GetSymbolAddress( const TCHAR* a_szFileName, const TCHAR* a_szSymbolName, AAssetType a_eAssetType );

	template <class T>
	static T* CastSymbol( const TCHAR* a_szFileName, const TCHAR* a_szSymbolName, AAssetType a_eAssetType )
	{
		return TSTATICCAST(
		    T,
		    GetSymbolAddress( a_szFileName, a_szSymbolName, a_eAssetType )
		);
	}

	// Returns TRB file of specified asset
	static Toshi::TTRB* GetAssetTRB( AAssetType a_eAssetType )
	{
		TASSERT( a_eAssetType < AAssetType_NUMOF );
		return ms_pTRBFiles[ a_eAssetType ];
	}

private:
	// Builds Data/<library>.trb path, a_rOutLibName receives the library variant that should be used
	static void GetLibraryFileName( Toshi::TString8& a_rOutLibName, Toshi::TString8& a_rOutFileName, const TCHAR* a_szLibraryName, TINT a_iMainGameAct );

public:
	// Loads/unloads matlib, keylibs and animatable object types of the main player's gender and costume ("playercharacter" property)
	static void LoadPlayerCharacter( const PBPropertyValue* a_pProperty );
	static void UnloadPlayerCharacter( const PBPropertyValue* a_pProperty );

private:
	inline static Toshi::TTRB* ms_pTRBFiles[ AAssetType_NUMOF ];

	// Costume of the currently loaded player character (0x007727d4)
	inline static TINT ms_iPlayerCostume = -1;

	// Act of the lib_maingame variant the assets were created from (0x007727f4)
	inline static TINT ms_iMainGameAct = -1;
};
