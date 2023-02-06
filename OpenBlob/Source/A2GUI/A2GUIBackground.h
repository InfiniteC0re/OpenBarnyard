#pragma once

#include "Toshi2/T2GUI/T2GUIPolygon.h"
#include "Toshi/File/TTRB.h"

class A2GUIBackground : public Toshi::T2GUIElement
{
private:
	static constexpr uint32_t MAX_FILENAME_LENGTH = 128;
	static constexpr uint32_t MAX_FILEPATH_LENGTH = 256;
	static constexpr uint32_t MAX_TILES           =   6;

	static constexpr uint32_t s_aTextureXSizes    = 256;
	static constexpr uint32_t s_aTextureYSizes    = 256;

	static constexpr int s_OffsetX = 0;
	static constexpr int s_OffsetY = 0;

public:
	A2GUIBackground()
	{
		m_pTrb = TNULL;
		Toshi::TUtil::MemClear(m_Poly, sizeof(m_Poly));
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

