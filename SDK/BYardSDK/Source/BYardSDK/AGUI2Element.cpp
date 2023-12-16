#include "pch.h"
#include "SDKHooks.h"
#include "AGUI2Element.h"

#include <Toshi/Render/TRenderInterface.h>

AGUI2Element::AGUI2Element()
{
	
}

void AGUI2Element::Tick(TFLOAT a_fDeltaTime)
{
	CALL_THIS(0x006c22c0, AGUI2Element*, AGUI2Element*, this, TFLOAT, a_fDeltaTime);
}

void AGUI2Element::PreRender()
{
	CALL_THIS(0x006c2300, AGUI2Element*, AGUI2Element*, this);
}

void AGUI2Element::Render()
{
	CALL_THIS(0x006c2510, AGUI2Element*, AGUI2Element*, this);
}

void AGUI2Element::PostRender()
{
	CALL_THIS(0x006c2570, AGUI2Element*, AGUI2Element*, this);
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

void AGUI2Element::SetShadowAlpha(TFLOAT a_fAlpha)
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

void AGUI2Element::GetScreenTransform(AGUI2Transform& a_rOutTransform)
{
	CALL_THIS(0x006c2690, AGUI2Element*, AGUI2Element*, this, AGUI2Transform&, a_rOutTransform);
}

void AGUI2Element::GetInvScreenTransform(AGUI2Transform& a_rOutTransform)
{
	CALL_THIS(0x006c29d0, AGUI2Element*, AGUI2Element*, this, AGUI2Transform&, a_rOutTransform);
}

AGUI2Element* AGUI2Element::CreateFromEngine()
{
	return CALL_THIS(0x006c2b10, AGUI2Element*, AGUI2Element*, new AGUI2Element);
}
