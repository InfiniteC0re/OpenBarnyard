#pragma once
#include "TQuaternion.h"
#include <DirectXMath.h>

namespace Toshi
{
	class TMatrix44
	{
	public:
		TFloat
			a, b, c, d,
			e, f, g, h,
			i, j, k, l,
			m, n, o, p;

	public:
		TMatrix44(
			TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14,
			TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24,
			TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34,
			TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44
		)
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

		TMatrix44(const TMatrix44& a_rMatrix);

		TMatrix44() { }

		void Identity()
		{
			*this = IDENTITY;
		}

		TVector3 GetTranslation3() const
		{
			return { m, n, o };
		}

		TVector4 GetTranslation4() const
		{
			return { m, n, o, p };
		}

		const TVector3& AsBasicVector3(int index = 0) const
		{
			return *TREINTERPRETCAST(
				TVector3*,
				TREINTERPRETCAST(uintptr_t, this) + index * sizeof(TVector4)
			);
		}

		TVector3& AsBasicVector3(int index = 0)
		{
			return *TREINTERPRETCAST(
				TVector3*,
				TREINTERPRETCAST(uintptr_t, this) + index * sizeof(TVector4)
			);
		}

		const TVector4& AsBasicVector4(int index = 0) const
		{
			return *TREINTERPRETCAST(
				TVector4*,
				TREINTERPRETCAST(uintptr_t, this) + index * sizeof(TVector4)
			);
		}

		TVector4& AsBasicVector4(int index = 0)
		{
			return *TREINTERPRETCAST(
				TVector4*,
				TREINTERPRETCAST(uintptr_t, this) + index * sizeof(TVector4)
			);
		}

		TVector4& GetTranslation()
		{
			return AsBasicVector4(3);
		}

		const TVector4& GetTranslation() const
		{
			return AsBasicVector4(3);
		}

		void SetTranslation(const TVector3& translation)
		{
			GetTranslation() = translation;
		}

		void SetTranslation(const TVector4& translation)
		{
			GetTranslation() = translation;
		}

		void LookAtTarget(const TVector4& target, const TVector4& up);
		void LookAtDirection(const TVector4& vec, const TVector4& vec2);

		void Scale(float scalar1, float scalar2, float scalar3)
		{
			a *= scalar1;
			b *= scalar1;
			c *= scalar1;
			e *= scalar2;
			f *= scalar2;
			g *= scalar2;
			i *= scalar3;
			j *= scalar3;
			k *= scalar3;
		}

		bool IsOrthonormal() const
		{
			float fVar1 = (j * j + i * i + k * k) - 1.0f;
			float fVar2 = (f * f + e * e + g * g) - 1.0f;
			float fVar3 = f * j + i * e + g * k;
			float fVar4 = j * b + a * i + k * c;
			float fVar5 = f * b + a * e + g * c;
			float fVar6 = (a * a + b * b + c * c) - 1.0f;

			return !(((0.01 <= fVar6 * fVar6 + fVar2 * fVar2 + fVar1 * fVar1 + fVar4 * fVar4 + fVar5 * fVar5 + fVar3 * fVar3) || (d != 0.0)) || (h != 0.0)) || ((l != 0.0 || (p != 1.0)));
		}

		void InvertOrthogonal();

		void SetFromQuaternion(const TQuaternion& quaternion);
		void RotateX(float angle);
		void RotateY(float angle);
		void RotateZ(float angle);

		void Set(
			TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14,
			TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24,
			TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34,
			TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44
		);

		static void TransformVector(TVector4& outVector, const TMatrix44& matrix, const TVector4& vector);

		void operator=(const TMatrix44& matrix)
		{
			Set(
				matrix.a, matrix.b, matrix.c, matrix.d,
				matrix.e, matrix.f, matrix.g, matrix.h,
				matrix.i, matrix.j, matrix.k, matrix.l,
				matrix.m, matrix.n, matrix.o, matrix.p
			);
		}

		// DirectX Math

		TMatrix44(const DirectX::XMMATRIX& matrix)
		{
			*(DirectX::XMMATRIX*)this = matrix;
		}

		void Set(const DirectX::XMMATRIX& matrix)
		{
			*(DirectX::XMMATRIX*)this = matrix;
		}

		const DirectX::XMMATRIX& XMM() const
		{
			return *(DirectX::XMMATRIX*)this;
		}

		TMatrix44 operator*=(const TMatrix44& a)
		{
			Set(DirectX::XMMatrixMultiply(XMM(), a.XMM()));
		}

	public:
		static TMatrix44 IDENTITY;
	};

	inline TMatrix44 operator*(const TMatrix44& a, const TMatrix44& b)
	{
		return DirectX::XMMatrixMultiply(a.XMM(), b.XMM());
	}
}
