#pragma once
#include "AAssetPack.h"

class AAssetBuilder
{
public:
	AAssetBuilder()
	{

	}

	~AAssetBuilder()
	{
		m_Files.Clear();
	}

	void Add(const Toshi::TString8& a_FileName);
	void Save(const char* a_szFileName, TBOOL a_bCompress = TFALSE);

private:
	AAssetPack m_AssetPack;
	Toshi::T2Vector<PTRBWriter, 1024> m_Files;
};
