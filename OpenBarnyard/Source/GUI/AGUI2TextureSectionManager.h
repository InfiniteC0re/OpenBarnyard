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

	inline static TBOOL                g_providedtrb = TFALSE;
	inline static Toshi::TTRB*         g_trb         = TNULL;
	inline static TRBHeader*           g_defblock    = TNULL;
	inline static AGUI2TextureSection* g_sections    = TNULL;

public:
	static TBOOL Open( const TCHAR* a_szFileName, Toshi::TTRB* a_pTRB );
	static void  Close();

	static AGUI2TextureSection* GetTextureSection( const TCHAR* a_szSectionName );

	static void CreateMaterials();
	static void DestroyMaterials();
	static void UpdateMaterials();
};
