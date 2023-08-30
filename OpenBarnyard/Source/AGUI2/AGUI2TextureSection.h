#pragma once
#include <Toshi/File/TTRB.h>

class AGUI2TextureSection
{

};

class AGUI2TextureSectionManager
{
public:
	inline static TBOOL g_providedtrb = TFALSE;
	inline static Toshi::TTRB* g_trb = TNULL;
	inline static AGUI2TextureSection** g_sections = TNULL;
	inline static AGUI2TextureSection* g_defblock = TNULL;

public:
	static TBOOL Open(const char* a_szFileName, Toshi::TTRB* a_pTRB);
	static void Close();

	static void CreateMaterials();
	static void DestroyMaterials();
};
