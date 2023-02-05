#pragma once

#include "Toshi2/T2GUI/T2GUIPolygon.h"
#include "Toshi/File/TTRB.h"

class A2GUIBackground : public Toshi::T2GUIElement
{
	static const uint32_t MAX_FILENAME_LENGTH = 128;
	static const uint32_t MAX_FILEPATH_LENGTH = 256;
	static const uint32_t MAX_TILES           =   6;

	static const uint32_t s_aTextureXSizes    = 256;
	static const uint32_t s_aTextureYSizes    = 256;

	static int s_OffsetX;
	static int s_OffsetY;

public:

	A2GUIBackground()
	{
		m_pTrb = TNULL;
		Toshi::TUtil::MemClear(m_Poly, MAX_TILES * sizeof(Toshi::T2GUIPolygon*));
	}

	void Create(const char* fileName);
	void LoadMaterialLibrary(const char* fileName);
	void SetupBackground();
	void Destroy();

private:
	char m_pFileName[MAX_FILENAME_LENGTH];
	Toshi::TTRB* m_pTrb;
	Toshi::T2GUIPolygon* m_Poly[MAX_TILES];
};

