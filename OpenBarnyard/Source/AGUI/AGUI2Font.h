#pragma once
#include "AGUI2Material.h"
#include "AGUI2FontResource.h"

struct AGUI2FontDefCharacterData
{
	TINT8 Unk1;
	TINT8 Unk2;
	TINT16 ui16PosX;
	TINT16 ui16PosY;
	TINT8 ui8CharWidth;
	TINT8 ui8CharHeight;
	TINT8 Unk3;
	TINT8 Unk4;
	TINT8 Unk5;
	TINT8 Unk6;
};

struct AGUI2FontDef
{
	void* Unk1;
	const char* szTextureNames;
	TINT8 Unk2;
	TINT8 uiLinesMargin;
	TINT8 uiLineHeight;
	TINT8 uiNumMaterials;
	TUINT16 ui16MaxCharacter;
	TUINT16 ui16MinCharacter;
	TINT32 Unk3;
	TINT32* pCharactersMap;
	AGUI2FontDefCharacterData* pCharactersData;
};

class AGUI2Font
{
public:
	AGUI2Font() = default;

	TBOOL Create(AGUI2FontDef* a_pFontDef);

	void DrawTextSingleLine(const wchar_t* a_wszText, TINT a_iTextSize, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, void* a_fnCallback = TNULL);

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
	TUINT uiNumRefs;
};