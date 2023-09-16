#pragma once
#include "TQuaternion.h"
#include "TPlane.h"

#include <DirectXMath.h>

namespace Toshi
{
	class TMatrix44
	{
	public:
		TFLOAT
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
			TFLOAT a_f11, TFLOAT a_f12, TFLOAT a_f13, TFLOAT a_f14,
			TFLOAT a_f21, TFLOAT a_f22, TFLOAT a_f23, TFLOAT a_f24,
			TFLOAT a_f31, TFLOAT a_f32, TFLOAT a_f33, TFLOAT a_f34,
			TFLOAT a_f41, TFLOAT a_f42, TFLOAT a_f43, TFLOAT a_f44
		) :
			m_f11(a_f11), m_f12(a_f12), m_f13(a_f13), m_f14(a_f14),
			m_f21(a_f21), m_f22(a_f22), m_f23(a_f23), m_f24(a_f24),
			m_f31(a_f31), m_f32(a_f32), m_f33(a_f33), m_f34(a_f34),
			m_f41(a_f41), m_f42(a_f42), m_f43(a_f43), m_f44(a_f44)
		{ }

		void Set(
			TFLOAT a_f11, TFLOAT a_f12, TFLOAT a_f13, TFLOAT a_f14,
			TFLOAT a_f21, TFLOAT a_f22, TFLOAT a_f23, TFLOAT a_f24,
			TFLOAT a_f31, TFLOAT a_f32, TFLOAT a_f33, TFLOAT a_f34,
			TFLOAT a_f41, TFLOAT a_f42, TFLOAT a_f43, TFLOAT a_f44
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

		void Scale(const TVector4& a_rScalars)
		{
			m_f11 *= a_rScalars.x;
			m_f12 *= a_rScalars.x;
			m_f13 *= a_rScalars.x;
			m_f21 *= a_rScalars.y;
			m_f22 *= a_rScalars.y;
			m_f23 *= a_rScalars.y;
			m_f31 *= a_rScalars.z;
			m_f32 *= a_rScalars.z;
			m_f33 *= a_rScalars.z;
		}

		void Scale(float a_fScale)
		{
			Scale(a_fScale, a_fScale, a_fScale);
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

		void Multiply(const TMatrix44& a_rLeft, const TMatrix44& a_rRight);

		void Multiply(const TMatrix44& a_rRight)
		{
			TMatrix44 temp;
			temp.Multiply(*this, a_rRight);
			*this = temp;
		}

		TBOOL Invert(TMatrix44& a_rRight);

		void InvertOrthogonal(const TMatrix44& a_rRight);
		void InvertOrthonormal();

		TMatrix44& SetFromQuaternion(const TQuaternion& a_rQuaternion);
		TMatrix44& PushQuaternion(const TQuaternion& a_rQuaternion, const TMatrix44& a_rMatrix, const TVector3& a_rOrigin);

		void RotateX(float a_fAngle);
		void RotateY(float a_fAngle);
		void RotateZ(float a_fAngle);

		void Transpose();
		void Transpose(const TMatrix44& a_rSource);

		static void TransformPlaneOrthogonal(TPlane& a_rOutPlane, const TMatrix44& a_rMatrix, const TPlane& a_rPlane)
		{
			RotateVector(a_rOutPlane.AsVector4(), a_rMatrix, a_rPlane.AsVector4());
			
			a_rOutPlane.SetD(
				a_rPlane.GetD() + 
				TVector4::DotProduct3(
					a_rOutPlane.AsVector4(), a_rMatrix.AsBasisVector4(3)
				)
			);
		}

		static void RotateVector(TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector)
		{
			a_rOutVector.x = a_rMatrix.m_f31 * a_rVector.z + a_rMatrix.m_f21 * a_rVector.y + a_rMatrix.m_f11 * a_rVector.x;
			a_rOutVector.y = a_rMatrix.m_f32 * a_rVector.z + a_rMatrix.m_f22 * a_rVector.y + a_rMatrix.m_f12 * a_rVector.x;
			a_rOutVector.z = a_rMatrix.m_f13 * a_rVector.x + a_rMatrix.m_f23 * a_rVector.y + a_rMatrix.m_f33 * a_rVector.z;
			a_rOutVector.w = a_rVector.w;
		}

		static void TransformVector(TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector)
		{
			a_rOutVector.x = a_rMatrix.m_f11 * a_rVector.x + a_rMatrix.m_f21 * a_rVector.y + a_rMatrix.m_f31 * a_rVector.z + a_rMatrix.m_f41 * a_rVector.w;
			a_rOutVector.y = a_rMatrix.m_f12 * a_rVector.x + a_rMatrix.m_f22 * a_rVector.y + a_rMatrix.m_f32 * a_rVector.z + a_rMatrix.m_f42 * a_rVector.w;
			a_rOutVector.z = a_rMatrix.m_f13 * a_rVector.x + a_rMatrix.m_f23 * a_rVector.y + a_rMatrix.m_f33 * a_rVector.z + a_rMatrix.m_f43 * a_rVector.w;
			a_rOutVector.w = a_rMatrix.m_f14 * a_rVector.x + a_rMatrix.m_f24 * a_rVector.y + a_rMatrix.m_f34 * a_rVector.z + a_rMatrix.m_f44 * a_rVector.w;
		}

		void operator=(const TMatrix44& a_rMatrix)
		{
			Set(
				a_rMatrix.m_f11, a_rMatrix.m_f12, a_rMatrix.m_f13, a_rMatrix.m_f14,
				a_rMatrix.m_f21, a_rMatrix.m_f22, a_rMatrix.m_f23, a_rMatrix.m_f24,
				a_rMatrix.m_f31, a_rMatrix.m_f32, a_rMatrix.m_f33, a_rMatrix.m_f34,
				a_rMatrix.m_f41, a_rMatrix.m_f42, a_rMatrix.m_f43, a_rMatrix.m_f44
			);
		}

	public:
		static TMatrix44 IDENTITY;
	};
}
