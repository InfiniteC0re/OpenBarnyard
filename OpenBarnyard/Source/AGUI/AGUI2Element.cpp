#include "pch.h"
#include "AGUI2.h"
#include "AGUI2Element.h"

AGUI2Element::AGUI2Element()
{
	m_pParent = TNULL;
	m_fWidth = 0.0f;
	m_fHeight = 0.0f;
	m_uiColour = 0xFFFFFFFF;
	m_uiVisibilityMask = 0xFFFFFFFF;
	m_cbPostRender = TNULL;
	m_ePivot = Pivot::MiddleCenter;
	m_eAnchor = Anchor::MiddleCenter;
	m_eFlags = 3;
}

void AGUI2Element::Tick(TFLOAT a_fDeltaTime)
{
	for (auto it = m_Children.Back(); it != m_Children.End(); it--)
	{
		it->Element()->Tick(a_fDeltaTime);
	}
}

void AGUI2Element::PreRender()
{
	Toshi::TVector2 vec1 = { 0.0f, 0.0f };
	Toshi::TVector2 vec2 = { 0.0f, 0.0f };

	if (m_pParent != TNULL)
	{
		float fParentWidth, fParentHeight;
		m_pParent->GetDimensions(fParentWidth, fParentHeight);

		switch (m_eAnchor)
		{
		case Anchor::TopRight:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			vec1.y = vec1.y - fParentHeight * 0.5f;
			break;
		case Anchor::TopCenter:
			vec1.y = vec1.y - fParentHeight * 0.5f;
			break;
		case Anchor::TopLeft:
			vec1.y = vec1.y - fParentHeight * 0.5f;
			vec1.x = fParentWidth * 0.5f + vec1.x;
			break;
		case Anchor::MiddleRight:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			break;
		case Anchor::MiddleLeft:
			vec1.x = fParentWidth * 0.5f + vec1.x;
			break;
		case Anchor::BottomRight:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		case Anchor::BottomCenter:
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		case Anchor::BottomLeft:
			vec1.x = fParentWidth * 0.5f + vec1.x;
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		}
	}

	float fWidth, fHeight;
	GetDimensions(fWidth, fHeight);

	switch (m_ePivot)
	{
	case Pivot::TopRight:
		vec2.x = fWidth * 0.5f + vec2.x;
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot::TopCenter:
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot::TopLeft:
		vec2.x = vec2.x - fWidth * 0.5f;
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot::MiddleRight:
		vec2.x = fWidth * 0.5f + vec2.x;
		break;
	case Pivot::MiddleLeft:
		vec2.x = vec2.x - fWidth * 0.5f;
		break;
	case Pivot::BottomRight:
		vec2.x = fWidth * 0.5f + vec2.x;
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	case Pivot::BottomCenter:
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	case Pivot::BottomLeft:
		vec2.x = vec2.x - fWidth * 0.5f;
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	}

	AGUI2::GetRenderer()->PushTransform(m_oTransform, vec1, vec2);
}

void AGUI2Element::Render()
{
	for (auto it = m_Children.Back(); it != m_Children.End(); it--)
	{
		if (it->Element()->IsVisible() && (it->Element()->m_uiVisibilityMask & s_uiVisibilityMask) != 0)
		{
			it->Element()->PreRender();
			it->Element()->Render();
			it->Element()->PostRender();
		}
	}

	if (m_cbPostRender)
	{
		m_cbPostRender();
	}
}

void AGUI2Element::PostRender()
{
	AGUI2::GetRenderer()->PopTransform();
}

void AGUI2Element::GetDimensions(TFLOAT& a_rWidth, TFLOAT& a_rHeight)
{
	a_rWidth = m_fWidth;
	a_rHeight = m_fHeight;
}

TFLOAT AGUI2Element::GetWidth()
{
	return m_fWidth;
}

TFLOAT AGUI2Element::GetHeight()
{
	return m_fHeight;
}

void AGUI2Element::SetWidth(TFLOAT a_fWidth)
{
	m_fWidth = a_fWidth;
}

void AGUI2Element::SetHeight(TFLOAT a_fHeight)
{
	m_fHeight = a_fHeight;
}

void AGUI2Element::SetDimensions(TFLOAT a_fWidth, TFLOAT a_fHeight)
{
	m_fWidth = a_fWidth;
	m_fHeight = a_fHeight;
}

void AGUI2Element::GetMins(TFLOAT& a_fX, TFLOAT& a_fY)
{
	float fWidth, fHeight;
	GetDimensions(fWidth, fHeight);
	a_fX = fWidth * -0.5f;
	a_fY = fHeight * -0.5f;
}

void AGUI2Element::GetMaxs(TFLOAT& a_fX, TFLOAT& a_fY)
{
	float fWidth, fHeight;
	GetDimensions(fWidth, fHeight);
	a_fX = fWidth * 0.5f;
	a_fY = fHeight * 0.5f;
}

void AGUI2Element::SetColour(TUINT32 a_uiColour)
{
	m_uiColour = a_uiColour;
}

void AGUI2Element::SetAlpha(TFLOAT a_fAlpha)
{
	TUINT8 alpha8 = (TUINT8)(a_fAlpha * 255.0f);
	m_uiColour = (m_uiColour & 0x00FFFFFF) | alpha8 << 24;
}

void AGUI2Element::SetUnknown(TFLOAT a_fValue)
{

}

void AGUI2Element::SetFocus(TBOOL a_bFocused)
{
	if (a_bFocused)
	{
		m_eFlags |= 2;
	}
	else
	{
		m_eFlags &= ~2;
	}
}

TBOOL AGUI2Element::IsPointInside(const Toshi::TVector2& a_rPoint)
{
	float minX, minY;
	float maxX, maxY;

	GetMins(minX, minY);
	GetMaxs(maxX, maxY);

	if (minX <= a_rPoint.x && a_rPoint.x <= maxX && minY <= a_rPoint.y)
	{
		return a_rPoint.y <= maxY;
	}

	return TFALSE;
}
