#pragma once
#include "T2GUITextureSection.h"

TOSHI_NAMESPACE_START

class T2GUITextureSectionManager
{
public:
	struct TRBHeader
	{
		T2GUITextureSection* m_pSections;
		TUINT32              m_uiNumSections;
	};

	inline static TBOOL                g_providedtrb = TFALSE;
	inline static TTRB*                g_trb         = TNULL;
	inline static TRBHeader*           g_defblock    = TNULL;
	inline static T2GUITextureSection* g_sections    = TNULL;

public:
	static TBOOL Open( const TCHAR* a_szFileName, TTRB* a_pTRB );
	static void  Close();

	static T2GUITextureSection* GetTextureSection( const TCHAR* a_szSectionName );

	static void CreateMaterials();
	static void DestroyMaterials();
	static void UpdateMaterials();
};

TOSHI_NAMESPACE_END
