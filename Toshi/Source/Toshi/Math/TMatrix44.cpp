#include "ToshiPCH.h"
#include "TMatrix44.h"

namespace Toshi
{
	TMatrix44::TMatrix44(const TMatrix44& a_rMatrix)
	{
		Set(a_rMatrix.m_f11, a_rMatrix.m_f12, a_rMatrix.m_f13, a_rMatrix.m_f14,
			a_rMatrix.m_f21, a_rMatrix.m_f22, a_rMatrix.m_f23, a_rMatrix.m_f24,
			a_rMatrix.m_f31, a_rMatrix.m_f32, a_rMatrix.m_f33, a_rMatrix.m_f34,
			a_rMatrix.m_f41, a_rMatrix.m_f42, a_rMatrix.m_f43, a_rMatrix.m_f44);
	}

	void TMatrix44::Set(float a_f11, float a_f12, float a_f13, float a_f14, 
		float a_f21, float a_f22, float a_f23, float a_f24, 
		float a_f31, float a_f32, float a_f33, float a_f34, 
		float a_f41, float a_f42, float a_f43, float a_f44)
	{
		m_f11 = a_f11;
		m_f12 = a_f12;
		m_f13 = a_f13;
		m_f14 = a_f14;

		m_f21 = a_f21;
		m_f22 = a_f22;
		m_f23 = a_f23;
		m_f24 = a_f24;

		m_f31 = a_f31;
		m_f32 = a_f32;
		m_f33 = a_f33;
		m_f34 = a_f34;

		m_f41 = a_f41;
		m_f42 = a_f42;
		m_f43 = a_f43;
		m_f44 = a_f44;
	}

	TMatrix44::TMatrix44(float a_f11, float a_f12, float a_f13, float a_f14,
		float a_f21, float a_f22, float a_f23, float a_f24,
		float a_f31, float a_f32, float a_f33, float a_f34,
		float a_f41, float a_f42, float a_f43, float a_f44)
	{
		Set(a_f11, a_f12, a_f13, a_f14,
			a_f21, a_f22, a_f23, a_f24,
			a_f31, a_f32, a_f33, a_f34,
			a_f41, a_f42, a_f43, a_f44);
	}
}