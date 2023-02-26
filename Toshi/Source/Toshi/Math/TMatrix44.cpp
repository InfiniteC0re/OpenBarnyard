#include "ToshiPCH.h"
#include "TMatrix44.h"

namespace Toshi
{
	TMatrix44::TMatrix44(const TMatrix44& a_rMatrix)
	{
		Set(a_rMatrix.a, a_rMatrix.b, a_rMatrix.c, a_rMatrix.d,
			a_rMatrix.e, a_rMatrix.f, a_rMatrix.g, a_rMatrix.h,
			a_rMatrix.i, a_rMatrix.j, a_rMatrix.k, a_rMatrix.l,
			a_rMatrix.m, a_rMatrix.n, a_rMatrix.o, a_rMatrix.p);
	}

	void TMatrix44::Identity()
	{
		a = s_Identity[0];
		b = s_Identity[1];
		c = s_Identity[2];
		d = s_Identity[3];
		e = s_Identity[4];
		f = s_Identity[5];
		g = s_Identity[6];
		h = s_Identity[7];
		i = s_Identity[8];
		j = s_Identity[9];
		k = s_Identity[10];
		l = s_Identity[11];
		m = s_Identity[12];
		n = s_Identity[13];
		o = s_Identity[14];
		p = s_Identity[15];
	}

	void TMatrix44::LookAtTarget(const TVector3& vec, const TVector3& vec2)
	{
		i = vec.x - vec2.x;
		j = vec.y - vec2.y;
		k = vec.z - vec2.z;
		
	}

	void TMatrix44::Set(TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14,
		TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24, 
		TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34, 
		TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44)
	{
		a = a_f11;
		b = a_f12;
		c = a_f13;
		d = a_f14;

		e = a_f21;
		f = a_f22;
		g = a_f23;
		h = a_f24;

		i = a_f31;
		j = a_f32;
		k = a_f33;
		l = a_f34;

		m = a_f41;
		n = a_f42;
		o = a_f43;
		p = a_f44;
	}
}