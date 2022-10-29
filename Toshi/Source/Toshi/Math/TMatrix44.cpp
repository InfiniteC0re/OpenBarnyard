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

	void TMatrix44::Set(TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14, 
		TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24, 
		TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34, 
		TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44)
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

	TMatrix44::TMatrix44(TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14,
		TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24,
		TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34,
		TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44)
	{
		Set(a_f11, a_f12, a_f13, a_f14,
			a_f21, a_f22, a_f23, a_f24,
			a_f31, a_f32, a_f33, a_f34,
			a_f41, a_f42, a_f43, a_f44);
	}
}