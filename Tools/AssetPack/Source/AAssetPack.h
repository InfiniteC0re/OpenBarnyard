#pragma once
#include <Toshi/Strings/TPString8.h>
#include <Toshi/File/TTRB.h>
#include <Toshi2/T2Map.h>
#include <Toshi2/T2DynamicArray.h>
#include <Toshi2/T2Vector.h>
#include <Plugins/PTRB.h>

class AAssetPack
{
public:
	struct Asset_t
	{
		Toshi::TPString8 Name;
		char Format[4];
		PTRB::SECT::Stack* pStack;
		Toshi::T2DynamicArray<Toshi::TPString8> RelatedSymbols;
	};

public:
	AAssetPack()
	{
		m_bLoaded = TTRUE;
	}

	TBOOL Load(const char* a_szFileName);
	Asset_t* GetAssetFromSymbol(const Toshi::TPString8& a_rSymbolName);

	PTRB::TRBF* GetTRBFile() { return &m_TRBFile; }

	Toshi::T2Vector<Asset_t, 1024>& GetAssets() { return m_Assets; }

	TBOOL IsLoaded() const { return m_bLoaded; }

private:
	PTRB::TRBF m_TRBFile;
	TBOOL m_bLoaded;
	Toshi::TPString8 m_FileName;
	Toshi::T2Vector<Asset_t, 1024> m_Assets;
	Toshi::T2Map<Toshi::TPString8, Asset_t*, Toshi::TPString8::Comparator> m_SymbolToAsset;
};
