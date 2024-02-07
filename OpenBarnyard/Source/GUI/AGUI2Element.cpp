#include "pch.h"
#include "AGUI2.h"
#include "AGUI2Element.h"

#include <Render/TRenderInterface.h>

AGUI2Element::AGUI2Element()
{
	m_pParent = TNULL;
	m_fWidth = 0.0f;
	m_fHeight = 0.0f;
	m_uiColour = 0xFFFFFFFF;
	m_uiVisibilityMask = 0xFFFFFFFF;
	m_cbPostRender = TNULL;
	m_ePivot = Pivot_MiddleCenter;
	m_eAnchor = Anchor_MiddleCenter;
	m_eFlags = 3;
}

void AGUI2Element::Tick(TFLOAT a_fDeltaTime)
{
	for (auto elem = m_Children.GetPrevElem(); elem != &m_Children; elem = elem->GetPrevElem())
	{
		elem->Tick(a_fDeltaTime);
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
		case Anchor_TopLeft:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			vec1.y = vec1.y - fParentHeight * 0.5f;
			break;
		case Anchor_TopCenter:
			vec1.y = vec1.y - fParentHeight * 0.5f;
			break;
		case Anchor_TopRight:
			vec1.y = vec1.y - fParentHeight * 0.5f;
			vec1.x = fParentWidth * 0.5f + vec1.x;
			break;
		case Anchor_MiddleLeft:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			break;
		case Anchor_MiddleRight:
			vec1.x = fParentWidth * 0.5f + vec1.x;
			break;
		case Anchor_BottomLeft:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		case Anchor_BottomCenter:
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		case Anchor_BottomRight:
			vec1.x = fParentWidth * 0.5f + vec1.x;
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		}
	}

	float fWidth, fHeight;
	GetDimensions(fWidth, fHeight);

	switch (m_ePivot)
	{
	case Pivot_TopLeft:
		vec2.x = fWidth * 0.5f + vec2.x;
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot_TopCenter:
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot_TopRight:
		vec2.x = vec2.x - fWidth * 0.5f;
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot_MiddleLeft:
		vec2.x = fWidth * 0.5f + vec2.x;
		break;
	case Pivot_MiddleRight:
		vec2.x = vec2.x - fWidth * 0.5f;
		break;
	case Pivot_BottomLeft:
		vec2.x = fWidth * 0.5f + vec2.x;
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	case Pivot_BottomCenter:
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	case Pivot_BottomRight:
		vec2.x = vec2.x - fWidth * 0.5f;
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	}

	AGUI2::GetRenderer()->PushTransform(m_oTransform, vec1, vec2);
}

void AGUI2Element::Render()
{
#if 0
	{
		TFLOAT fWidth;
		TFLOAT fHeight;
		GetDimensions(fWidth, fHeight);
		Toshi::TVector2 pos1 = { -fWidth / 2, -fHeight / 2 };
		Toshi::TVector2 pos2 = { pos1.x + fWidth - 1, pos1.y + fHeight - 1 };

		AGUI2::GetRenderer()->RenderOutlineRectangle(pos1, pos2);
	}
#endif

	for (auto elem = m_Children.GetPrevElem(); elem != &m_Children; elem = elem->GetPrevElem())
	{
		if (elem->IsVisible() && (elem->m_uiVisibilityMask & s_uiVisibilityMask) != 0)
		{
			elem->PreRender();
			elem->Render();
			elem->PostRender();
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
	Toshi::TVector2 vec1 = { 0.0f, 0.0f };
	Toshi::TVector2 vec2 = { 0.0f, 0.0f };

	if (m_pParent != TNULL)
	{
		float fParentWidth, fParentHeight;
		m_pParent->GetDimensions(fParentWidth, fParentHeight);

		switch (m_eAnchor)
		{
		case Anchor_BottomLeft:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			vec1.y = vec1.y - fParentHeight * 0.5f;
			break;
		case Anchor_BottomCenter:
			vec1.y = vec1.y - fParentHeight * 0.5f;
			break;
		case Anchor_BottomRight:
			vec1.x = vec1.x + fParentWidth * 0.5f;
			vec1.y = vec1.y - fParentHeight * 0.5f;
			break;
		case Anchor_MiddleLeft:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			break;
		case Anchor_MiddleRight:
			vec1.x = fParentWidth * 0.5f + vec1.x;
			break;
		case Anchor_TopLeft:
			vec1.x = vec1.x - fParentWidth * 0.5f;
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		case Anchor_TopCenter:
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		case Anchor_TopRight:
			vec1.x = fParentWidth * 0.5f + vec1.x;
			vec1.y = fParentHeight * 0.5f + vec1.y;
			break;
		}
	}

	float fWidth, fHeight;
	GetDimensions(fWidth, fHeight);

	switch (m_ePivot)
	{
	case Pivot_BottomLeft:
		vec2.x = fWidth * 0.5f + vec2.x;
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot_BottomCenter:
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot_BottomRight:
		vec2.x = vec2.x - fWidth * 0.5f;
		vec2.y = fHeight * 0.5f + vec2.y;
		break;
	case Pivot_MiddleLeft:
		vec2.x = fWidth * 0.5f + vec2.x;
		break;
	case Pivot_MiddleRight:
		vec2.x = vec2.x - fWidth * 0.5f;
		break;
	case Pivot_TopLeft:
		vec2.x = fWidth * 0.5f + vec2.x;
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	case Pivot_TopCenter:
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	case Pivot_TopRight:
		vec2.x = vec2.x - fWidth * 0.5f;
		vec2.y = vec2.y - fHeight * 0.5f;
		break;
	}

	AGUI2Transform transform1;
	AGUI2Transform transform2;

	if (m_pParent)
	{
		m_pParent->GetScreenTransform(transform1);
		transform2 = GetTransform();
	}
	else
	{
		auto pDisplayParams = Toshi::TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

		transform1.m_Rotation[0] = { 1.0f, 0.0f };
		transform1.m_Rotation[1] = { 0.0f, 1.0f };
		transform1.m_Position = { pDisplayParams->uiWidth / 2.0f, pDisplayParams->uiHeight / 2.0f };

		transform1.PreMultiply(pDisplayParams->uiWidth / fWidth, pDisplayParams->uiHeight / fHeight);
		transform2 = GetTransform();
	}

	Toshi::TVector2 vec;
	transform1.Transform(vec, vec1);
	transform1.m_Position = { vec.x, vec.y };

	transform2.Transform(vec, vec2);
	transform2.m_Position = { vec.x, vec.y };

	AGUI2Transform::Multiply(a_rOutTransform, transform1, transform2);
}

void AGUI2Element::GetInvScreenTransform(AGUI2Transform& a_rOutTransform)
{
	AGUI2Transform transform;
	GetScreenTransform(transform);
	transform.GetInverse(a_rOutTransform);
}
