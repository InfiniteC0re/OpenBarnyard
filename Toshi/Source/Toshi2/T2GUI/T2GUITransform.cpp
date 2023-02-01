#include "ToshiPCH.h"
#include "T2GUITransform.h"

namespace Toshi
{
	void T2GUITransform::Rotate(float angle)
	{
		float cos;
		float sin;

		// at the moment it resets the previous angle
		// but originally it seems to take it into account
		TMath::SinCos(angle, sin, cos);
		m_Rot[0].SetX(cos);
		m_Rot[0].SetY(-sin);
		m_Rot[1].SetX(sin);
		m_Rot[1].SetY(cos);
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
}