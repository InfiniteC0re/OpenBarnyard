#pragma once
#include "SDK_T2GUIMaterial.h"

#include <GUI/T2GUIFontResource.h>

struct AGUI2FontDefCharacterData
{
	TINT8   Unk1;
	TINT8   Unk2;
	TUINT16 ui16PosX;
	TUINT16 ui16PosY;
	TUINT8  ui8CharWidth;
	TUINT8  ui8CharHeight;
	TINT8   Unk3;
	TINT8   Unk4;
	TINT8   Unk5;
	TINT8   Unk6;
};

struct SDK_T2GUIFontDef
{
	void*                      Unk1;
	const char*                szTextureNames;
	TINT8                      Unk2;
	TUINT8                     uiLinesMargin;
	TUINT8                     uiLineHeight;
	TUINT8                     uiNumMaterials;
	TUINT16                    ui16MaxCharacter;
	TUINT16                    ui16MinCharacter;
	TINT32                     Unk3;
	TINT32*                    pCharactersMap;
	AGUI2FontDefCharacterData* pCharactersData;
};

class SDK_T2GUIFont
{
public:
	enum TextAlign : TUINT32
	{
		TextAlign_Left,
		TextAlign_Center,
		TextAlign_Right,
	};

public:
	SDK_T2GUIFont() = default;

	TBOOL Create( SDK_T2GUIFontDef* a_pFontDef );

	void DrawTextWrapped( const wchar_t* a_wszText, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour, TFLOAT a_fScale, TextAlign a_eAlign, void* a_fnCallback = TNULL );
	void DrawTextSingleLine( const wchar_t* a_wszText, TINT a_iTextLength, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, void* a_fnCallback = TNULL );

	TFLOAT GetTextHeightWrapped( const wchar_t* a_wszText, TFLOAT a_fMaxWidth, TFLOAT a_fScale );
	TFLOAT GetTextWidth( const wchar_t* a_wszText, TFLOAT a_fScale );
	TFLOAT GetTextWidth( const wchar_t* a_wszText, TINT a_iTextLength, TFLOAT a_fScale );

	SDK_T2GUIFontDef* GetFontDef() const { return m_pFontDef; }

protected:
	SDK_T2GUIFontDef*   m_pFontDef    = TNULL;
	SDK_T2GUIMaterial** m_ppMaterials = TNULL;
	TBOOL               m_bFlag       = TFALSE;
};

struct SDK_T2GUIFontRef
{
	char*                     szName;
	SDK_T2GUIFont*            pFont;
	void*                     Unk1;
	Toshi::T2GUIFontResource* pFontResource;
	TUINT                     uiNumRefs;
};
