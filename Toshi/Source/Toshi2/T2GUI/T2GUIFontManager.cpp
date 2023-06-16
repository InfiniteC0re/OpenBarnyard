#include "ToshiPCH.h"
#include "T2GUIFontManager.h"

TBOOL Toshi::T2GUIFontDef2::Init()
{
    FindFontRef(m_font);
    return TFALSE;
}

int* Toshi::FindFontRef(const char* a_font)
{
    return TNULL;
}

int* Toshi::FindFreeFontRef()
{
    return TNULL;
}

