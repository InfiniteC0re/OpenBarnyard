#include "ToshiPCH.h"
#include "T2GUIElement.h"
#include "T2GUI.h"

namespace Toshi
{
	constinit uint32_t T2GUIElement::s_uiGlobalVisMask = 0xFF;

	T2GUIElement::T2GUIElement()
	{
		m_pParent = TNULL;
		m_Width = 0;
		m_Height = 0;
		m_Color = 0xFFFFFFFF;
		m_cbPostRender = TNULL;
		m_State = 255;
		m_Pivot = Pivot::MiddleCenter;
		m_Anchor = Anchor::MiddleCenter;
		m_Flags = 3;
	}

	T2GUIElement::~T2GUIElement()
	{
		TASSERT(!m_Children.IsLinked());
	}

	void T2GUIElement::Tick(float deltaTime)
	{
		for (auto it = m_Children.Front(); it != m_Children.End(); it++)
		{
			if (!it->IsPaused())
			{
				it->Tick(deltaTime);
			}
		}
	}

	TBOOL T2GUIElement::IsPaused() const
	{
		return TFALSE;
	}

	TBOOL T2GUIElement::SkipRender()
	{
		if (HASFLAG(m_Flags & FLAGS_VISIBLE) && HASFLAG(m_Flags & (uint8_t)s_uiGlobalVisMask))
		{
			return TFALSE;
		}

		return TTRUE;
	}

	void T2GUIElement::PreRender()
	{
		TVector2 vec1, vec2;	
		
		if (m_pParent != TNULL)
		{
			float fParentWidth, fParentHeight;
			m_pParent->GetDimensions(fParentWidth, fParentHeight);

			switch (m_Anchor)
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

		switch (m_Pivot)
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
		
		if (HASFLAG(m_Flags & FLAGS_XUIELEMENT))
		{
			auto pivot = GetPivot();
			vec2.x = -pivot.x + vec2.x;
			vec2.y = -pivot.y + vec2.y;
		}

		T2GUI::GetSingletonWeak()->GetRenderer()->PushTransform(m_Transform, vec1, vec2);
	}

	void T2GUIElement::Render()
	{
		for (auto it = m_Children.Front(); it != m_Children.End(); it++)
		{
			if (!it->SkipRender())
			{
				it->PreRender();
				it->Render();
				it->PostRender();
			}
		}

		if (m_cbPostRender)
		{
			m_cbPostRender();
		}
	}

	void T2GUIElement::PostRender()
	{
		T2GUI::GetSingletonWeak()->GetRenderer()->PopTransform();
	}

	void T2GUIElement::GetDimensions(float& width, float& height)
	{
		width = UnpackFloat(m_Width);
		height = UnpackFloat(m_Height);
	}

	float T2GUIElement::GetWidth()
	{
		return UnpackFloat(m_Width);
	}

	float T2GUIElement::GetHeight()
	{
		return UnpackFloat(m_Height);
	}

	void T2GUIElement::SetWidth(float width)
	{
		TASSERT(width >= 0.0f);
		m_Width = PackFloat(width);
	}

	void T2GUIElement::SetHeight(float height)
	{
		TASSERT(height >= 0.0f);
		m_Height = PackFloat(height);
	}

	void T2GUIElement::SetDimensions(float width, float height)
	{
		TASSERT(width >= 0.0f);
		TASSERT(height >= 0.0f);
		m_Width = PackFloat(width);
		m_Height = PackFloat(height);
	}

	void T2GUIElement::GetMins(float& x, float& y)
	{
		float width, height;
		GetDimensions(width, height);
		x = width * -0.5f;
		y = height * -0.5f;
	}

	void T2GUIElement::GetMaxs(float& x, float& y)
	{
		float width, height;
		GetDimensions(width, height);
		x = width * 0.5f;
		y = height * 0.5f;
	}

	void T2GUIElement::SetColour(uint32_t colour)
	{
		m_Color = colour;
	}

	void T2GUIElement::SetAlpha(float alpha)
	{
		uint8_t alpha8 = (uint8_t)(alpha * 255.0f);
		m_Color = (m_Color & 0x00FFFFFF) | alpha8 << 24;
	}

	float T2GUIElement::GetAlpha()
	{
		return ((m_Color & 0x00FFFFFF) >> 24) * 255.0f;
	}

	void T2GUIElement::SetVisible(TBOOL visible)
	{
		if (visible)
		{
			m_Flags |= FLAGS_VISIBLE;
		}
		else
		{
			m_Flags &= ~FLAGS_VISIBLE;
		}
	}

	void T2GUIElement::SetFocus(TBOOL focused)
	{
		if (focused)
		{
			m_Flags |= FLAGS_FOCUSED;
		}
		else
		{
			m_Flags &= ~FLAGS_FOCUSED;
		}
	}

	void T2GUIElement::GetScreenTransform(void* unk)
	{
		TIMPLEMENT();
	}

	TBOOL T2GUIElement::IsPointInside(const TVector2& point)
	{
		float minX, minY;
		float maxX, maxY;

		GetMins(minX, minY);
		GetMaxs(maxX, maxY);

		if (minX <= point.x && point.x <= maxX && minY <= point.y)
		{
			return point.y <= maxY;
		}

		return TFALSE;
	}

	const TVector2& T2GUIElement::GetPivot()
	{
		return TVector2::VEC_ZERO;
	}
}