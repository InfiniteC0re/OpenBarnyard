#pragma once
#include "T2GUIMaterial.h"
#include "T2GUIFontResource.h"

TOSHI_NAMESPACE_START

struct T2GUIFontDefCharacterData
{
	TINT8   Unk1;
	TINT8   Unk2;
	TUINT16 ui16PosX;
	TUINT16 ui16PosY;
	TUINT8  ui8CharWidth;
	TUINT8  ui8CharHeight;
	TINT8   iLeftPadding;
	TINT8   iRightPadding;
	TINT8   Unk5;
	TINT8   Unk6;
};

struct T2GUIFontDef
{
	void*                      Unk1;
	const TCHAR*               szTextureNames;
	TINT8                      iLetterSpacing;
	TUINT8                     uiLinesMargin;
	TUINT8                     uiLineHeight;
	TUINT8                     uiNumMaterials;
	TUINT16                    ui16MaxCharacter;
	TUINT16                    ui16MinCharacter;
	TINT32                     Unk3;
	TINT32*                    pCharactersMap;
	T2GUIFontDefCharacterData* pCharactersData;
};

class T2GUIFont
{
public:
	enum TextAlign : TUINT32
	{
		TextAlign_Left,
		TextAlign_Center,
		TextAlign_Right
	};

public:
	T2GUIFont() = default;

	TBOOL Create( T2GUIFontDef* a_pFontDef );

	void DrawTextWrapped( const TWCHAR* a_wszText, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour, TFLOAT a_fScale, TextAlign a_eAlign, void* a_fnCallback = TNULL );
	void DrawTextSingleLine( const TWCHAR* a_wszText, TINT a_iTextLength, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, void* a_fnCallback = TNULL );

	TFLOAT GetTextHeightWrapped( const TWCHAR* a_wszText, TFLOAT a_fMaxWidth, TFLOAT a_fScale = 1.0f );
	TFLOAT GetTextWidth( const TWCHAR* a_wszText, TFLOAT a_fScale = 1.0f );
	TFLOAT GetTextWidth( const TWCHAR* a_wszText, TINT a_iTextLength, TFLOAT a_fScale = 1.0f );

	TFLOAT GetLinesMargin();

private:
	T2GUIFontDef*   m_pFontDef    = TNULL;
	T2GUIMaterial** m_ppMaterials = TNULL;
	TBOOL           m_bFlag       = TFALSE;
};

struct T2GUIFontRef
{
	TCHAR*             szName;
	T2GUIFont*         pFont;
	void*              Unk1;
	T2GUIFontResource* pFontResource;
	TUINT              uiNumRefs;
};

TOSHI_NAMESPACE_END
