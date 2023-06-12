#include "ToshiPCH.h"
#include "TMatrix44.h"

#include <DirectXMath.h>

namespace Toshi
{
	TMatrix44::TMatrix44(const TMatrix44& a_rMatrix)
	{
		Set(a_rMatrix.a, a_rMatrix.b, a_rMatrix.c, a_rMatrix.d,
			a_rMatrix.e, a_rMatrix.f, a_rMatrix.g, a_rMatrix.h,
			a_rMatrix.i, a_rMatrix.j, a_rMatrix.k, a_rMatrix.l,
			a_rMatrix.m, a_rMatrix.n, a_rMatrix.o, a_rMatrix.p);
	}

	void TMatrix44::LookAtDirection(const Toshi::TVector4& vec, const Toshi::TVector4& vec2)
	{
		TVector4 v;
		TVector4 v2;
		v.ProjectNormalToPlane(vec2, vec);
		v.Multiply(-1.0f);
		v2.CrossProduct(v, vec);
		Set(v2.x, v2.y, v2.z, 0.0f, v.x, v.y, v.z, 0.0f, vec.x, vec.y, vec.z, vec.w, m, n, o, p);
	}

	void TMatrix44::InvertOrthogonal()
	{
		TASSERT(IsOrthonormal());

		std::swap(b, e);
		std::swap(c, i);
		std::swap(g, j);

		n *= -1;
		m *= -1;
		o *= -1;
		n = f * n + m * b + o * j;
		o = n * g + m * c + k * o;
		m = n * e + m * a + o * i;
	}

	void TMatrix44::SetFromQuaternion(const TQuaternion& quaternion)
	{
		auto matrix = (DirectX::XMMATRIX*)this;
		auto quat = (DirectX::XMVECTOR*)&quaternion;
		*matrix = DirectX::XMMatrixRotationQuaternion(*quat);
	}

	void TMatrix44::RotateX(float angle)
	{
		float fVar1;
		float fVar2;
		float fCos;
		float fSin;

		TMath::SinCos(angle, fSin, fCos);
		fVar1 = i;
		i = fVar1 * fCos - e * fSin;
		fVar2 = j;
		e = fVar1 * fSin + e * fCos;
		fVar1 = g;
		j = fVar2 * fCos - f * fSin;
		f = fVar2 * fSin + f * fCos;
		g = k * fSin + fVar1 * fCos;
		k = k * fCos - fVar1 * fSin;
	}

	void TMatrix44::RotateY(float angle)
	{
		float fVar1;
		float fVar2;
		float fSin;
		float fCos;

		TMath::SinCos(angle, fSin, fCos);
		fVar1 = i;
		i = fVar1 * fCos + a * fSin;
		fVar2 = j;
		a = a * fCos - fVar1 * fSin;
		fVar1 = c;
		j = fVar2 * fCos + b * fSin;
		b = b * fCos - fVar2 * fSin;
		c = fVar1 * fCos - k * fSin;
		k = k * fCos + fVar1 * fSin;
	}

	void TMatrix44::RotateZ(float angle)
	{
		float fVar1;
		float fVar2;
		float fCos;
		float fSin;

		TMath::SinCos(angle, fSin, fCos);
		fVar1 = e;
		e = fVar1 * fCos - a * fSin;
		fVar2 = f;
		a = fVar1 * fSin + a * fCos;
		fVar1 = c;
		f = fVar2 * fCos - b * fSin;
		b = fVar2 * fSin + b * fCos;
		c = g * fSin + fVar1 * fCos;
		g = g * fCos - fVar1 * fSin;
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

	void TMatrix44::TransformVector(TVector4& outVector, const TMatrix44& matrix, const TVector4& vector)
	{
		outVector.z = matrix.c * vector.x + matrix.g * vector.y + matrix.k * vector.z + matrix.o * vector.w;
		outVector.w = matrix.d * vector.x + matrix.h * vector.y + matrix.l * vector.z + matrix.p * vector.w;
		outVector.y = matrix.b * vector.x + matrix.f * vector.y + matrix.j * vector.z + matrix.n * vector.w;
		outVector.x = matrix.e * vector.y + matrix.a * vector.x + matrix.i * vector.z + matrix.m * vector.w;
	}

}