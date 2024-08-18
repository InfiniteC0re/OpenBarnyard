#pragma once
#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>
#include <Toshi/TArray.h>
#include <Toshi/T2Vector.h>

#include <File/TTRB.h>
#include <Plugins/PTRB.h>

class AAssetPack
{
public:
	struct Asset_t
	{
		Toshi::TPString8                Name;
		char                            Format[ 4 ];
		PTRBWriter*                     pTRB;
		PTRBSections::MemoryStream*     pStack;
		Toshi::TArray<Toshi::TPString8> RelatedSymbols;
	};

public:
	AAssetPack()
	{
		m_bLoaded = TTRUE;
	}

	TBOOL Load( const char* a_szFileName );
	void  Save( const char* a_szFileName, TBOOL a_bCompress = TFALSE );

	Asset_t* GetAssetFromSymbol( const Toshi::TPString8& a_rSymbolName );

	PTRBWriter* GetTRBFile() { return &m_TRBFile; }

	Toshi::T2Vector<Asset_t, 1024>& GetAssets() { return m_Assets; }

	TBOOL IsLoaded() const { return m_bLoaded; }

private:
	PTRBWriter                                                             m_TRBFile;
	TBOOL                                                                  m_bLoaded;
	Toshi::TPString8                                                       m_FileName;
	Toshi::T2Vector<Asset_t, 1024>                                         m_Assets;
	Toshi::T2Map<Toshi::TPString8, Asset_t*, Toshi::TPString8::Comparator> m_SymbolToAsset;
};
