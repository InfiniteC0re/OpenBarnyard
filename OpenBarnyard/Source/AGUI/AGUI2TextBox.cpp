#include "pch.h"
#include "AGUI2TextBox.h"

AGUI2TextBox::AGUI2TextBox()
{
	m_bRenderShadow = TFALSE;
	m_wzText = TNULL;
	m_pFont = TNULL;
	m_bUnkFlag2 = TFALSE;
	m_fShadowOffsetX = 5.0f;
	m_fShadowOffsetY = 5.0f;
	m_uiShadowColor = 0x80000000;
	m_fScale = 1.0f;
	m_uiUnknown2 = 1;
}

void AGUI2TextBox::Create(AGUI2Font* a_pFont, TFLOAT a_fWidth)
{
	m_pFont = a_pFont;
	AGUI2Element::m_fWidth = a_fWidth;
	m_bIsCreated = TTRUE;
	m_bUnkFlag2 = TFALSE;
}

void AGUI2TextBox::Render()
{
	TIMPLEMENT();
	TASSERT(m_pFont != TNULL);

	TFLOAT fWidth;
	TFLOAT fHeight;
	AGUI2Element::GetDimensions(fWidth, fHeight);

	m_pFont->DrawTextSingleLine(L"Hello, world!", 13, -fWidth / 2, -fHeight / 2, TCOLOR(150, 130, 220), 0.6f, TNULL);
	AGUI2Element::Render();
}
