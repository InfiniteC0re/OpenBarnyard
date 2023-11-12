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

void AGUI2Font::DrawTextSingleLine(const wchar_t* a_wszText, TINT a_iTextSize, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, void* a_fnCallback /*= TNULL*/)
{
	auto pRenderer = AGUI2::GetRenderer();
	pRenderer->SetColour(a_uiColour);

	TFLOAT fOffsetX = 0.0f;
	TFLOAT fOffsetY = 0.0f;
	TFLOAT fXToUV = 0.0f;
	TFLOAT fYToUV = 0.0f;
	AGUI2Material* pPrevMaterial = TNULL;

	for (TINT i = 0; i < a_iTextSize; i++)
	{
		auto pFontDef = m_pFontDef;
		auto wChar = a_wszText[i];

		if (pFontDef->ui16MinCharacter <= wChar && wChar <= pFontDef->ui16MaxCharacter)
		{
			TINT iCharIndex = pFontDef->pCharactersMap[wChar - pFontDef->ui16MinCharacter];

			if (iCharIndex >= 0)
			{
				auto& pCharData = pFontDef->pCharactersData[iCharIndex];
				TFLOAT fPos1X = pCharData.Unk3 * a_fScale + a_fX;
				
				if (pCharData.ui8CharWidth != 0 && pCharData.ui8CharHeight != 0)
				{
					AGUI2Material* pMaterial = (pFontDef->uiNumMaterials > 1) ? 
						m_ppMaterials[(wChar >> 8)] :
						m_ppMaterials[0];

					if (pMaterial != pPrevMaterial)
					{
						pPrevMaterial = pMaterial;
						pRenderer->SetMaterial(pMaterial);
						fXToUV = 1.0f / pRenderer->GetWidth(pMaterial);
						fYToUV = 1.0f / pRenderer->GetHeight(pMaterial);
					}

					TFIXME("Call a_fnCallback(i, fOffsetX, fOffsetY)");

					TFLOAT fPos1Y = ((pFontDef->uiLineHeight + pCharData.Unk5) - pCharData.ui8CharHeight) * a_fScale + a_fY;
					TFLOAT fPos2Y = (pCharData.ui8CharHeight + 1) * a_fScale + fPos1Y;
					TFLOAT fPos2X = (pCharData.ui8CharWidth + 1) * a_fScale + fPos1X;

					pRenderer->RenderRectangle(
						{ fPos1X + fOffsetX, fPos1Y + fOffsetY },
						{ fPos2X + fOffsetX, fPos2Y + fOffsetY },
						{ (pCharData.ui16PosX) * fXToUV, (pCharData.ui16PosY) * fYToUV },
						{ (pCharData.ui16PosX + pCharData.ui8CharWidth + 1) * fXToUV, (pCharData.ui16PosY + pCharData.ui8CharHeight + 1) * fYToUV }
					);

				}

				TFLOAT fPos2X = pFontDef->Unk2 + pCharData.Unk4 + pCharData.ui8CharWidth;
				a_fX = fPos2X * a_fScale + fPos1X;
			}
		}
	}
}
