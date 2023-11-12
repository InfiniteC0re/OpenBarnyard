#pragma once
#include "AGUI2Element.h"
#include "AGUI2Font.h"

class AGUI2TextBox : public AGUI2Element
{
public:
	AGUI2TextBox();

	void Create(AGUI2Font* a_pFont, TFLOAT a_fWidth);

private:
	TBOOL m_bRenderShadow;
	TFLOAT m_fShadowOffsetX;
	TFLOAT m_fShadowOffsetY;
	TUINT32 m_uiShadowColor;
	wchar_t* m_wzText;
	AGUI2Font* m_pFont;
	TFLOAT m_fScale;
	TUINT m_uiUnknown2;
	TBOOL m_bIsCreated;
	TBOOL m_bUnkFlag2;
};
