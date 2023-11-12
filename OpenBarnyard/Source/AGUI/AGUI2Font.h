#pragma once
#include "AGUI2Material.h"
#include "AGUI2FontResource.h"

struct AGUI2FontDef
{
	void* Unk1;
	const char* szTextureNames;
	TUINT8 Unk2;
	TUINT8 Unk3;
	TUINT8 Unk4;
	TUINT8 uiNumMaterials;
};

class AGUI2Font
{
public:
	AGUI2Font() = default;

	TBOOL Create(AGUI2FontDef* a_pFontDef);

private:
	AGUI2FontDef* m_pFontDef = TNULL;
	AGUI2Material** m_ppMaterials = TNULL;
	TBOOL m_bFlag = TFALSE;
};

struct AGUI2FontRef
{
	char* szName;
	AGUI2Font* pFont;
	void* Unk1;
	AGUI2FontResource* pFontResource;
	void* Unk2;
};