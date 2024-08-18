#pragma once
#include "AGUI2TextureSection.h"

class AGUI2TextureSectionManager
{
public:
	struct TRBHeader
	{
		AGUI2TextureSection* m_pSections;
		TUINT32              m_uiNumSections;
	};

	inline static TBOOL*                g_providedtrb = (TBOOL*)0x007ce700;
	inline static Toshi::TTRB**         g_trb         = (Toshi::TTRB**)0x007ce6fc;
	inline static TRBHeader**           g_defblock    = (TRBHeader**)0x007ce704;
	inline static AGUI2TextureSection** g_sections    = (AGUI2TextureSection**)0x007ce708;

public:
	static TBOOL Open( const char* a_szFileName, Toshi::TTRB* a_pTRB );
	static void  Close();

	static AGUI2TextureSection* GetTextureSection( const char* a_szSectionName );

	static void CreateMaterials();
	static void DestroyMaterials();
	static void UpdateMaterials();
};
