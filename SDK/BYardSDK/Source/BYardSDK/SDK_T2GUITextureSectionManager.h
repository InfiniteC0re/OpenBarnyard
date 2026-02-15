#pragma once
#include <GUI/T2GUITextureSection.h>

class SDK_T2GUITextureSectionManager
{
public:
	struct TRBHeader
	{
		Toshi::T2GUITextureSection* m_pSections;
		TUINT32                     m_uiNumSections;
	};

	inline static TBOOL*                       g_providedtrb = (TBOOL*)0x007ce700;
	inline static Toshi::TTRB**                g_trb         = (Toshi::TTRB**)0x007ce6fc;
	inline static TRBHeader**                  g_defblock    = (TRBHeader**)0x007ce704;
	inline static Toshi::T2GUITextureSection** g_sections    = (Toshi::T2GUITextureSection**)0x007ce708;

public:
	static TBOOL Open( const char* a_szFileName, Toshi::TTRB* a_pTRB );
	static void  Close();

	static Toshi::T2GUITextureSection* GetTextureSection( const char* a_szSectionName );

	static void CreateMaterials();
	static void DestroyMaterials();
	static void UpdateMaterials();
};
