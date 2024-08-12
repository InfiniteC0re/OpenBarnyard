#pragma once
#include "AGUI2Material.h"
#include "AGUI2FontResource.h"

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

struct AGUI2FontDef
{
    void*                      Unk1;
    const TCHAR*               szTextureNames;
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

class AGUI2Font
{
public:
    enum TextAlign : TUINT32
    {
        TextAlign_Left,
        TextAlign_Center,
        TextAlign_Right,
    };

public:
    AGUI2Font() = default;

    TBOOL Create( AGUI2FontDef* a_pFontDef );

    void DrawTextWrapped( const TWCHAR* a_wszText, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour, TFLOAT a_fScale, TextAlign a_eAlign, void* a_fnCallback = TNULL );
    void DrawTextSingleLine( const TWCHAR* a_wszText, TINT a_iTextLength, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, void* a_fnCallback = TNULL );

    TFLOAT GetTextHeightWrapped( const TWCHAR* a_wszText, TFLOAT a_fMaxWidth, TFLOAT a_fScale );
    TFLOAT GetTextWidth( const TWCHAR* a_wszText, TFLOAT a_fScale );
    TFLOAT GetTextWidth( const TWCHAR* a_wszText, TINT a_iTextLength, TFLOAT a_fScale );

private:
    AGUI2FontDef*   m_pFontDef    = TNULL;
    AGUI2Material** m_ppMaterials = TNULL;
    TBOOL           m_bFlag       = TFALSE;
};

struct AGUI2FontRef
{
    TCHAR*             szName;
    AGUI2Font*         pFont;
    void*              Unk1;
    AGUI2FontResource* pFontResource;
    TUINT              uiNumRefs;
};