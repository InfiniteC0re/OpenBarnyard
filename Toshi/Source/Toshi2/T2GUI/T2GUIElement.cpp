#include "ToshiPCH.h"
#include "T2GUIElement.h"

namespace Toshi
{
	constinit uint32_t T2GUIElement::s_uiGlobalVisMask = 0xFF;
	
	void T2GUITransform::Rotate(float angle)
	{
		float v4; // xmm6_4
		float v7; // xmm0_4
		float v11; // xmm1_4
		float cos; // [esp+10h] [ebp-8h] BYREF
		float sin; // [esp+14h] [ebp-4h] BYREF
		float a2a; // [esp+20h] [ebp+8h]

		TMath::SinCos(angle, sin, cos);
		a2a = m_Rot[0].GetX();
		v7 = m_Rot[0].GetY();
		v4 = m_Rot[1].GetX();
		v11 = m_Rot[1].GetY();

		m_Rot[0].SetX((v4 * sin) + (a2a * cos));
		m_Rot[0].SetY((v11 * sin) + (v7 * cos));
		m_Rot[1].SetX((v4 * cos) - (a2a * sin));
		m_Rot[1].SetY((v11 * cos) - (v7 * sin));
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
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;
		float fVar5;
		float fVar6;
		float fVar7;
		float fVar8;

		fVar3 = a.m_Rot[0].GetX();
		fVar1 = a.m_Rot[0].GetY();
		fVar8 = a.m_Rot[1].GetX();
		fVar2 = a.m_Rot[1].GetY();

		fVar5 = b.m_Rot[0].GetX();
		fVar4 = b.m_Rot[0].GetY();
		fVar7 = b.m_Rot[1].GetX();
		fVar6 = b.m_Rot[1].GetY();
		
		outTransform.m_Rot[0].SetX(fVar4 * fVar8 + fVar5 * fVar3);
		outTransform.m_Rot[0].SetY(fVar4 * fVar2 + fVar5 * fVar1);
		outTransform.m_Rot[1].SetX(fVar6 * fVar8 + fVar7 * fVar3);
		outTransform.m_Rot[1].SetY(fVar6 * fVar2 + fVar7 * fVar1);

		a.Transform(outTransform.m_Vec, b.m_Vec);
	}

	T2GUIElement::T2GUIElement()
	{
		m_Unk1 = TNULL;
		m_fX = 0;
		m_fY = 0;
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
		TIMPLEMENT();

		if (m_Unk1 != TNULL)
		{

		}
		else
		{

		}
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
		TIMPLEMENT();
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