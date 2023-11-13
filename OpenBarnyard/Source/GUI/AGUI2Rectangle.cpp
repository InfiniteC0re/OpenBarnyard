#include "pch.h"
#include "AGUI2Rectangle.h"
#include "AGUI2.h"

AGUI2Rectangle::AGUI2Rectangle()
{
	m_pMaterial = TNULL;
}

void AGUI2Rectangle::Render()
{
	auto pRenderer = AGUI2::GetRenderer();

	pRenderer->SetMaterial(m_pMaterial);
	pRenderer->SetColour(m_uiColour);
	
	if (ShoudResetZCoordinate())
	{
		pRenderer->ResetZCoordinate();
	}

	Toshi::TVector2 pos1 = { -m_fWidth / 2, -m_fHeight / 2 };
	Toshi::TVector2 pos2 = { pos1.x + m_fWidth, pos1.y + m_fHeight };
	pRenderer->RenderRectangle(pos1, pos2, m_UV1.AsVector2(), m_UV2.AsVector2());

	AGUI2Element::Render();
}

void AGUI2Rectangle::SetTextureSection(AGUI2TextureSection* a_pTexSection)
{
	if (a_pTexSection)
	{
		m_pMaterial = a_pTexSection->GetMaterial();
		m_UV1 = a_pTexSection->GetUVPoint1();
		m_UV2 = a_pTexSection->GetUVPoint2();
	}
	else
	{
		m_pMaterial = TNULL;
	}
}
