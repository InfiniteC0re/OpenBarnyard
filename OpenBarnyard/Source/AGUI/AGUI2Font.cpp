#include "pch.h"
#include "AGUI2Font.h"
#include "AGUI2.h"

TBOOL AGUI2Font::Create(AGUI2FontDef* a_pFontDef)
{
	TASSERT(a_pFontDef->uiNumMaterials != 0);

	m_pFontDef = a_pFontDef;
	m_ppMaterials = new AGUI2Material*[a_pFontDef->uiNumMaterials];

	auto pRenderer = AGUI2::GetRenderer();
	auto pTextureName = a_pFontDef->szTextureNames;

	for (TUINT8 i = 0; i < a_pFontDef->uiNumMaterials; i++)
	{
		m_ppMaterials[i] = pRenderer->CreateMaterial(pTextureName);

		while (*pTextureName != '\0') pTextureName++;
		pTextureName++;
	}

	return TTRUE;
}
