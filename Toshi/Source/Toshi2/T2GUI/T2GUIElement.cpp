#include "ToshiPCH.h"
#include "T2GUIElement.h"
#include "T2GUI.h"

namespace Toshi
{
	constinit uint32_t T2GUIElement::s_uiGlobalVisMask = 0xFF;
	
	void T2GUITransform::Rotate(float angle)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;
		float cos;
		float sin;

		TMath::SinCos(angle, sin, cos);
		
		fVar1 = m_Rot[0].GetX();
		fVar2 = m_Rot[1].GetX();
		fVar3 = m_Rot[0].GetY();
		fVar4 = m_Rot[1].GetY();
		m_Rot[0].SetY(fVar1 * cos + fVar2 * sin);
		m_Rot[0].SetX(fVar3 * cos + fVar4 * sin);
		m_Rot[1].SetY(fVar1 * sin + -fVar2 * cos);
		m_Rot[1].SetX(fVar3 * sin + -fVar4 * cos);
	}

	void T2GUITransform::RotateTo(float angle)
	{
		TIMPLEMENT();
	}

	void T2GUITransform::PreMultiply(const T2GUITransform& transform)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;
		float fVar5;
		float fVar6;
		float fVar7;
		float fVar8;

		fVar1 = m_Rot[0].GetX();
		fVar6 = m_Rot[0].GetY();
		fVar2 = m_Rot[1].GetX();
		fVar7 = m_Rot[1].GetY();
		fVar4 = transform.m_Rot[0].GetX();
		fVar5 = transform.m_Rot[0].GetY();
		fVar8 = transform.m_Rot[1].GetX();
		fVar3 = transform.m_Rot[1].GetY();
		m_Rot[0].SetX(fVar8 * fVar6 + fVar4 * fVar1);
		m_Rot[0].SetY(fVar3 * fVar6 + fVar5 * fVar1);
		m_Rot[1].SetX(fVar8 * fVar7 + fVar4 * fVar2);
		m_Rot[1].SetY(fVar3 * fVar7 + fVar5 * fVar2);
	}

	void T2GUITransform::PostMultiply(const T2GUITransform& transform)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;
		float fVar5;
		float fVar6;
		float fVar7;
		float fVar8;

		fVar1 = m_Rot[0].GetX();
		fVar6 = m_Rot[0].GetY();
		fVar2 = m_Rot[1].GetX();
		fVar7 = m_Rot[1].GetY();
		fVar4 = transform.m_Rot[0].GetX();
		fVar5 = transform.m_Rot[0].GetY();
		fVar8 = transform.m_Rot[1].GetX();
		fVar3 = transform.m_Rot[1].GetY();
		m_Rot[0].SetX(fVar8 * fVar6 + fVar4 * fVar1);
		m_Rot[0].SetY(fVar3 * fVar6 + fVar5 * fVar1);
		m_Rot[1].SetX(fVar8 * fVar7 + fVar4 * fVar2);
		m_Rot[1].SetY(fVar3 * fVar7 + fVar5 * fVar2);
	}

	void T2GUITransform::Matrix44(TMatrix44& outMatrix)
	{
		outMatrix.a = m_Rot[0].GetX();
		outMatrix.b = m_Rot[0].GetY();
		outMatrix.c = 0.0;
		outMatrix.d = 0.0;

		outMatrix.e = m_Rot[1].GetX();
		outMatrix.f = m_Rot[1].GetY();
		outMatrix.g = 0.0;
		outMatrix.h = 0.0;

		outMatrix.i = 0.0;
		outMatrix.j = 0.0;
		outMatrix.k = 1.0;
		outMatrix.l = 0.0;

		outMatrix.m = m_Vec.x;
		outMatrix.n = m_Vec.y;
		outMatrix.o = 0.0;
		outMatrix.p = 1.0;
	}

	void T2GUITransform::Multiply(T2GUITransform& outTransform, const T2GUITransform& a, const T2GUITransform& b)
	{
		float a1 = a.m_Rot[0].GetX(); float b1 = a.m_Rot[0].GetY();
		float c1 = a.m_Rot[1].GetX(); float d1 = a.m_Rot[1].GetY();
		
		float a2 = b.m_Rot[0].GetX(); float b2 = b.m_Rot[0].GetY();
		float c2 = b.m_Rot[1].GetX(); float d2 = b.m_Rot[1].GetY();
		
		outTransform.m_Rot[0].SetX(a1 * a2 + b1 * c2);
		outTransform.m_Rot[0].SetY(a1 * b2 + b1 * d2);
		outTransform.m_Rot[1].SetX(c1 * a2 + d1 * c2);
		outTransform.m_Rot[1].SetY(c1 * b2 + d1 * d2);

		a.Transform(outTransform.m_Vec, b.m_Vec);
	}

	T2GUIElement::T2GUIElement()
	{
		m_pParent = TNULL;
		m_Width = 0;
		m_Height = 0;
		m_Color = 0xFFFFFFFF;
		m_cbPostRender = TNULL;
		m_Flags1 = 255;
		m_Flags2 = 4;
		m_Flags3 = 4;
		m_State = State_Focused | State_Visible;
	}

	T2GUIElement::~T2GUIElement()
	{
		m_Children.DeleteAll();
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

	bool T2GUIElement::IsPaused() const
	{
		return TFALSE;
	}

	bool T2GUIElement::SkipRender()
	{
		if (HASFLAG(m_Flags1 & 1) && HASFLAG(m_Flags1 & (uint8_t)s_uiGlobalVisMask))
		{
			return TFALSE;
		}

		return TTRUE;
	}

	void T2GUIElement::PreRender()
	{
		T2GUIRenderer* pRenderer;
		TVector2 vec1;
		TVector2 vec2;
		float fHeight;
		float fWidth;

		vec1.x = 0.0;
		vec1.y = 0.0;
		vec2.x = 0.0;
		vec2.y = 0.0;
		if (this->m_pParent == (T2GUIElement*)0x0) goto switchD_006f62da_caseD_4;
		GetDimensions(fWidth, fHeight);
		if (false) goto switchD_006f62da_caseD_4;
		
		switch (this->m_Flags2) {
		case 0:
			vec1.x = vec1.x - fWidth * 0.5;
		case 1:
		switchD_006f62da_caseD_1:
			vec1.y = vec1.y - fHeight * 0.5;
			break;
		case 2:
			vec1.x = fWidth * 0.5 + vec1.x;
			goto switchD_006f62da_caseD_1;
		case 3:
			vec1.x = vec1.x - fWidth * 0.5;
			break;
		case 5:
			vec1.x = fWidth * 0.5 + vec1.x;
			break;
		case 6:
			vec1.x = vec1.x - fWidth * 0.5;
		case 7:
			goto switchD_006f62da_caseD_7;
		case 8:
			vec1.x = fWidth * 0.5 + vec1.x;
		switchD_006f62da_caseD_7:
			vec1.y = fHeight * 0.5 + vec1.y;
		}
	switchD_006f62da_caseD_4:
		GetDimensions(fWidth, fHeight);
		if (false) goto switchD_006f63c3_caseD_4;
		switch (this->m_Flags3) {
		case 0:
			vec2.x = fWidth * 0.5 + vec2.x;
			break;
		case 1:
			break;
		case 2:
			vec2.x = vec2.x - fWidth * 0.5;
			break;
		case 3:
			vec2.x = fWidth * 0.5 + vec2.x;
		default:
			goto switchD_006f63c3_caseD_4;
		case 5:
			vec2.x = vec2.x - fWidth * 0.5;
			goto switchD_006f63c3_caseD_4;
		case 6:
			vec2.x = fWidth * 0.5 + vec2.x;
		case 7:
			goto switchD_006f63c3_caseD_7;
		case 8:
			vec2.x = vec2.x - fWidth * 0.5;
			goto switchD_006f63c3_caseD_7;
		}
		vec2.y = fHeight * 0.5 + vec2.y;
	switchD_006f63c3_caseD_4:
		if ((m_State & 4) != 0) {
			auto pivot = GetPivot();
			vec2.x = pivot.x * -1.0 + vec2.x;
			vec2.y = pivot.y * -1.0 + vec2.y;
		}
		pRenderer = T2GUI::GetSingletonWeak()->GetRenderer();
		pRenderer->PushTransform(m_Transform, vec1, vec2);
		return;
	switchD_006f63c3_caseD_7:
		vec2.y = vec2.y - fHeight * 0.5;
		goto switchD_006f63c3_caseD_4;
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

	void T2GUIElement::SetVisible(bool visible)
	{
		if (visible)
		{
			m_State |= State_Visible;
		}
		else
		{
			m_State &= ~State_Visible;
		}
	}

	void T2GUIElement::SetFocus(bool focused)
	{
		if (focused)
		{
			m_State |= State_Focused;
		}
		else
		{
			m_State &= ~State_Focused;
		}
	}

	void T2GUIElement::Unk(void* unk)
	{
		TIMPLEMENT();
	}

	bool T2GUIElement::IsPointInside(const TVector2& point)
	{
		float minX, minY;
		float maxX, maxY;

		GetMins(minX, minY);
		GetMaxs(maxX, maxY);

		if (minX <= point.x && point.x <= maxX && minY <= point.y)
		{
			return point.y <= maxY;
		}

		return false;
	}

	const TVector2& T2GUIElement::GetPivot()
	{
		return TVector2::VEC_ZERO;
	}
}