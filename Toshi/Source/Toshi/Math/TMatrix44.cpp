#include "ToshiPCH.h"
#include "TMatrix44.h"

#include <DirectXMath.h>

namespace Toshi
{
	TMatrix44 TMatrix44::IDENTITY(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	void TMatrix44::LookAtTarget(const TVector4& target, const TVector4& up)
	{
		auto& forward = AsBasisVector3(2);

		forward = target.AsVector3() - up.AsVector3();
		forward.Normalize();

		AsBasisVector3(0) = { m_f33, 0, -m_f31 };
		AsBasisVector3(0).Normalize();

		m_f22 = m_f11 * m_f33 - m_f13 * m_f31;
		m_f23 = m_f12 * m_f31 - m_f11 * m_f32;
		m_f21 = m_f13 * m_f32 - m_f12 * m_f33;
	}

	void TMatrix44::LookAtDirection(const Toshi::TVector4& vec, const Toshi::TVector4& vec2)
	{
		TVector4 v;
		TVector4 v2;
		v.ProjectNormalToPlane(vec2, vec);
		v.Multiply(-1.0f);
		v2.CrossProduct(v, vec);
		Set(v2.x, v2.y, v2.z, 0.0f, v.x, v.y, v.z, 0.0f, vec.x, vec.y, vec.z, vec.w, m_f41, m_f42, m_f43, m_f44);
	}

	TBOOL TMatrix44::Invert(TMatrix44& a_rRight)
	{
		assert(a_rRight.m_f14 == 0.0f);
		assert(a_rRight.m_f24 == 0.0f);
		assert(a_rRight.m_f34 == 0.0f);
		assert(a_rRight.m_f44 == 1.0f);

		float fVal1 = a_rRight.m_f22 * a_rRight.m_f33 - a_rRight.m_f23 * a_rRight.m_f32;
		float fVal2 = -(a_rRight.m_f12 * a_rRight.m_f33 - a_rRight.m_f13 * a_rRight.m_f32);
		float fVal3 = a_rRight.m_f12 * a_rRight.m_f23 - a_rRight.m_f13 * a_rRight.m_f22;
		float fDet = a_rRight.m_f11 * fVal1 + a_rRight.m_f21 * fVal2 + a_rRight.m_f31 * fVal3;

		if (fDet == 0.0)
		{
			// Matrix is not invertible
			return TFALSE;
		}

		float fInvDet = 1.0f / fDet;

		m_f11 = fInvDet * fVal1;
		m_f12 = fInvDet * fVal2;
		m_f13 = fInvDet * fVal3;
		m_f14 = a_rRight.m_f14;

		m_f21 = -((a_rRight.m_f21 * a_rRight.m_f33 - a_rRight.m_f31 * a_rRight.m_f23) * fInvDet);
		m_f22 = (a_rRight.m_f11 * a_rRight.m_f33 - a_rRight.m_f31 * a_rRight.m_f13) * fInvDet;
		m_f23 = -((a_rRight.m_f11 * a_rRight.m_f23 - a_rRight.m_f21 * a_rRight.m_f13) * fInvDet);
		m_f24 = a_rRight.m_f24;

		m_f31 = (a_rRight.m_f21 * a_rRight.m_f32 - a_rRight.m_f31 * a_rRight.m_f22) * fInvDet;
		m_f32 = -((a_rRight.m_f11 * a_rRight.m_f32 - a_rRight.m_f31 * a_rRight.m_f12) * fInvDet);
		m_f33 = (a_rRight.m_f11 * a_rRight.m_f22 - a_rRight.m_f21 * a_rRight.m_f12) * fInvDet;
		m_f34 = a_rRight.m_f34;

		m_f41 = -a_rRight.m_f41 * m_f11 - a_rRight.m_f42 * m_f21 - a_rRight.m_f43 * m_f31;
		m_f42 = -a_rRight.m_f41 * m_f12 - a_rRight.m_f42 * m_f22 - a_rRight.m_f43 * m_f32;
		m_f43 = -a_rRight.m_f41 * m_f13 - a_rRight.m_f42 * m_f23 - a_rRight.m_f43 * m_f33;
		m_f44 = a_rRight.m_f44;

		return TTRUE;
	}

	void TMatrix44::InvertOrthogonal()
	{
		TASSERT(IsOrthonormal());

		std::swap(m_f12, m_f21);
		std::swap(m_f13, m_f31);
		std::swap(m_f23, m_f32);

		m_f42 *= -1;
		m_f41 *= -1;
		m_f43 *= -1;
		m_f42 = m_f22 * m_f42 + m_f41 * m_f12 + m_f43 * m_f32;
		m_f43 = m_f42 * m_f23 + m_f41 * m_f13 + m_f33 * m_f43;
		m_f41 = m_f42 * m_f21 + m_f41 * m_f11 + m_f43 * m_f31;
	}

	TMatrix44& TMatrix44::SetFromQuaternion(const TQuaternion& a_rQuaternion)
	{
		float fVal1 = a_rQuaternion.z * 2.0f;
		float fVal2 = a_rQuaternion.y * 2.0f;
		float fVal3 = a_rQuaternion.x * 2.0f * a_rQuaternion.w;
		float fVal4 = a_rQuaternion.x * 2.0f * a_rQuaternion.x;
		float fVal5 = fVal1 * a_rQuaternion.z;

		m_f14 = 0.0f;
		m_f24 = 0.0f;
		m_f34 = 0.0f;
		m_f41 = 0.0f;
		m_f42 = 0.0f;
		m_f43 = 0.0f;
		m_f44 = 1.0f;
		m_f11 = 1.0f - (fVal5 + fVal2 * a_rQuaternion.y);
		m_f21 = fVal2 * a_rQuaternion.x - fVal1 * a_rQuaternion.w;
		m_f12 = fVal2 * a_rQuaternion.x + fVal1 * a_rQuaternion.w;
		m_f31 = fVal1 * a_rQuaternion.x + fVal2 * a_rQuaternion.w;
		m_f13 = fVal1 * a_rQuaternion.x - fVal2 * a_rQuaternion.w;
		m_f22 = 1.0f - (fVal5 + fVal4);
		m_f32 = fVal1 * a_rQuaternion.y - fVal3;
		m_f23 = fVal1 * a_rQuaternion.y + fVal3;
		m_f33 = 1.0f - (fVal2 * a_rQuaternion.y + fVal4);

		return *this;
	}

	void TMatrix44::RotateX(float angle)
	{
		float fVar1;
		float fVar2;
		float fCos;
		float fSin;

		TMath::SinCos(angle, fSin, fCos);
		fVar1 = m_f31;
		m_f31 = fVar1 * fCos - m_f21 * fSin;
		fVar2 = m_f32;
		m_f21 = fVar1 * fSin + m_f21 * fCos;
		fVar1 = m_f23;
		m_f32 = fVar2 * fCos - m_f22 * fSin;
		m_f22 = fVar2 * fSin + m_f22 * fCos;
		m_f23 = m_f33 * fSin + fVar1 * fCos;
		m_f33 = m_f33 * fCos - fVar1 * fSin;
	}

	void TMatrix44::RotateY(float angle)
	{
		float fVar1;
		float fVar2;
		float fSin;
		float fCos;

		TMath::SinCos(angle, fSin, fCos);
		fVar1 = m_f31;
		m_f31 = fVar1 * fCos + m_f11 * fSin;
		fVar2 = m_f32;
		m_f11 = m_f11 * fCos - fVar1 * fSin;
		fVar1 = m_f13;
		m_f32 = fVar2 * fCos + m_f12 * fSin;
		m_f12 = m_f12 * fCos - fVar2 * fSin;
		m_f13 = fVar1 * fCos - m_f33 * fSin;
		m_f33 = m_f33 * fCos + fVar1 * fSin;
	}

	void TMatrix44::RotateZ(float angle)
	{
		float fVar1;
		float fVar2;
		float fCos;
		float fSin;

		TMath::SinCos(angle, fSin, fCos);
		fVar1 = m_f21;
		m_f21 = fVar1 * fCos - m_f11 * fSin;
		fVar2 = m_f22;
		m_f11 = fVar1 * fSin + m_f11 * fCos;
		fVar1 = m_f13;
		m_f22 = fVar2 * fCos - m_f12 * fSin;
		m_f12 = fVar2 * fSin + m_f12 * fCos;
		m_f13 = m_f23 * fSin + fVar1 * fCos;
		m_f23 = m_f23 * fCos - fVar1 * fSin;
	}

	void TMatrix44::TransformVector(TVector4& outVector, const TMatrix44& matrix, const TVector4& vector)
	{
		outVector.z = matrix.m_f13 * vector.x + matrix.m_f23 * vector.y + matrix.m_f33 * vector.z + matrix.m_f43 * vector.w;
		outVector.w = matrix.m_f14 * vector.x + matrix.m_f24 * vector.y + matrix.m_f34 * vector.z + matrix.m_f44 * vector.w;
		outVector.y = matrix.m_f12 * vector.x + matrix.m_f22 * vector.y + matrix.m_f32 * vector.z + matrix.m_f42 * vector.w;
		outVector.x = matrix.m_f21 * vector.y + matrix.m_f11 * vector.x + matrix.m_f31 * vector.z + matrix.m_f41 * vector.w;
	}

}