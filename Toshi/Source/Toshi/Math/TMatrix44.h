#pragma once
#include "TQuaternion.h"

#include <DirectXMath.h>

namespace Toshi
{
	class TMatrix44
	{
	public:
		TFloat
			m_f11, m_f12, m_f13, m_f14,
			m_f21, m_f22, m_f23, m_f24,
			m_f31, m_f32, m_f33, m_f34,
			m_f41, m_f42, m_f43, m_f44;

	public:
		TMatrix44() = default;

		TMatrix44(const TMatrix44& a_rMatrix) :
			m_f11(a_rMatrix.m_f11), m_f12(a_rMatrix.m_f12), m_f13(a_rMatrix.m_f13), m_f14(a_rMatrix.m_f14),
			m_f21(a_rMatrix.m_f21), m_f22(a_rMatrix.m_f22), m_f23(a_rMatrix.m_f23), m_f24(a_rMatrix.m_f24),
			m_f31(a_rMatrix.m_f31), m_f32(a_rMatrix.m_f32), m_f33(a_rMatrix.m_f33), m_f34(a_rMatrix.m_f34),
			m_f41(a_rMatrix.m_f41), m_f42(a_rMatrix.m_f42), m_f43(a_rMatrix.m_f43), m_f44(a_rMatrix.m_f44)
		{ }

		TMatrix44(
			TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14,
			TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24,
			TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34,
			TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44
		) :
			m_f11(a_f11), m_f12(a_f12), m_f13(a_f13), m_f14(a_f14),
			m_f21(a_f21), m_f22(a_f22), m_f23(a_f23), m_f24(a_f24),
			m_f31(a_f31), m_f32(a_f32), m_f33(a_f33), m_f34(a_f34),
			m_f41(a_f41), m_f42(a_f42), m_f43(a_f43), m_f44(a_f44)
		{ }

		void Set(
			TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14,
			TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24,
			TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34,
			TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44
		)
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

		void Identity()
		{
			m_f11 = IDENTITY.m_f11;
			m_f12 = IDENTITY.m_f12;
			m_f13 = IDENTITY.m_f13;
			m_f14 = IDENTITY.m_f14;
			m_f21 = IDENTITY.m_f21;
			m_f22 = IDENTITY.m_f22;
			m_f23 = IDENTITY.m_f23;
			m_f24 = IDENTITY.m_f24;
			m_f31 = IDENTITY.m_f31;
			m_f32 = IDENTITY.m_f32;
			m_f33 = IDENTITY.m_f33;
			m_f34 = IDENTITY.m_f34;
			m_f41 = IDENTITY.m_f41;
			m_f42 = IDENTITY.m_f42;
			m_f43 = IDENTITY.m_f43;
			m_f44 = IDENTITY.m_f44;
		}

		TVector3 GetTranslation3() const
		{
			return { m_f41, m_f42, m_f43 };
		}

		TVector4 GetTranslation4() const
		{
			return { m_f41, m_f42, m_f43, m_f44 };
		}

		const TVector3& AsBasisVector3(int a_iIndex = 0) const
		{
			return *TREINTERPRETCAST(
				TVector3*,
				TREINTERPRETCAST(uintptr_t, this) + a_iIndex * sizeof(TVector4)
			);
		}

		TVector3& AsBasisVector3(int a_iIndex = 0)
		{
			return *TREINTERPRETCAST(
				TVector3*,
				TREINTERPRETCAST(uintptr_t, this) + a_iIndex * sizeof(TVector4)
			);
		}

		const TVector4& AsBasisVector4(int a_iIndex = 0) const
		{
			return *TREINTERPRETCAST(
				TVector4*,
				TREINTERPRETCAST(uintptr_t, this) + a_iIndex * sizeof(TVector4)
			);
		}

		TVector4& AsBasisVector4(int a_iIndex = 0)
		{
			return *TREINTERPRETCAST(
				TVector4*,
				TREINTERPRETCAST(uintptr_t, this) + a_iIndex * sizeof(TVector4)
			);
		}

		TVector4& GetTranslation()
		{
			return AsBasisVector4(3);
		}

		const TVector4& GetTranslation() const
		{
			return AsBasisVector4(3);
		}

		void SetTranslation(const TVector3& a_rTranslation)
		{
			GetTranslation() = a_rTranslation;
		}

		void SetTranslation(const TVector4& a_rTranslation)
		{
			GetTranslation() = a_rTranslation;
		}

		void LookAtTarget(const TVector4& a_rTarget, const TVector4& a_rUp);
		void LookAtDirection(const TVector4& a_rVec, const TVector4& a_rVec2);

		void Scale(float a_fScalar1, float a_fScalar2, float a_fScalar3)
		{
			m_f11 *= a_fScalar1;
			m_f12 *= a_fScalar1;
			m_f13 *= a_fScalar1;
			m_f21 *= a_fScalar2;
			m_f22 *= a_fScalar2;
			m_f23 *= a_fScalar2;
			m_f31 *= a_fScalar3;
			m_f32 *= a_fScalar3;
			m_f33 *= a_fScalar3;
		}

		TBOOL IsOrthonormal() const
		{
			float fVar1 = (m_f32 * m_f32 + m_f31 * m_f31 + m_f33 * m_f33) - 1.0f;
			float fVar2 = (m_f22 * m_f22 + m_f21 * m_f21 + m_f23 * m_f23) - 1.0f;
			float fVar3 = m_f22 * m_f32 + m_f31 * m_f21 + m_f23 * m_f33;
			float fVar4 = m_f32 * m_f12 + m_f11 * m_f31 + m_f33 * m_f13;
			float fVar5 = m_f22 * m_f12 + m_f11 * m_f21 + m_f23 * m_f13;
			float fVar6 = (m_f11 * m_f11 + m_f12 * m_f12 + m_f13 * m_f13) - 1.0f;

			return !(((0.01 <= fVar6 * fVar6 + fVar2 * fVar2 + fVar1 * fVar1 + fVar4 * fVar4 + fVar5 * fVar5 + fVar3 * fVar3) || (m_f14 != 0.0)) || (m_f24 != 0.0)) || ((m_f34 != 0.0 || (m_f44 != 1.0)));
		}

		TBOOL Invert(TMatrix44& a_rRight);
		void InvertOrthogonal();

		TMatrix44& SetFromQuaternion(const TQuaternion& a_rQuaternion);
		void RotateX(float a_fAngle);
		void RotateY(float a_fAngle);
		void RotateZ(float a_fAngle);

		static void TransformVector(TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector);

		void operator=(const TMatrix44& a_rMatrix)
		{
			Set(
				a_rMatrix.m_f11, a_rMatrix.m_f12, a_rMatrix.m_f13, a_rMatrix.m_f14,
				a_rMatrix.m_f21, a_rMatrix.m_f22, a_rMatrix.m_f23, a_rMatrix.m_f24,
				a_rMatrix.m_f31, a_rMatrix.m_f32, a_rMatrix.m_f33, a_rMatrix.m_f34,
				a_rMatrix.m_f41, a_rMatrix.m_f42, a_rMatrix.m_f43, a_rMatrix.m_f44
			);
		}

		// DirectX Math

		TMatrix44(const DirectX::XMMATRIX& a_rMatrix)
		{
			*(DirectX::XMMATRIX*)this = a_rMatrix;
		}

		void Set(const DirectX::XMMATRIX& a_rMatrix)
		{
			*(DirectX::XMMATRIX*)this = a_rMatrix;
		}

		const DirectX::XMMATRIX& XMM() const
		{
			return *(DirectX::XMMATRIX*)this;
		}

		TMatrix44 operator*=(const TMatrix44& a_rMatrix)
		{
			Set(DirectX::XMMatrixMultiply(XMM(), a_rMatrix.XMM()));
		}

	public:
		static TMatrix44 IDENTITY;
	};

	inline TMatrix44 operator*(const TMatrix44& a_rMatrix1, const TMatrix44& a_rMatrix2)
	{
		return DirectX::XMMatrixMultiply(a_rMatrix1.XMM(), a_rMatrix2.XMM());
	}
}
