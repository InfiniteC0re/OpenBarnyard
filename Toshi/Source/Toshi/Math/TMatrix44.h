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

		TVector3 GetTranslation3() const
		{
			return { m, n, o };
		}

		TVector4 GetTranslation4() const
		{
			return { m, n, o, p };
		}

		void SetTranslation(const TVector3& translation)
		{
			m = translation.x;
			n = translation.y;
			o = translation.z;
		}

		void SetTranslation(const TVector4& translation)
		{
			m = translation.x;
			n = translation.y;
			o = translation.z;
			p = translation.w;
		}

		void LookAtTarget(const TVector3& vec, const TVector3& vec2)
		{
			i = vec.x - vec2.x;
			j = vec.y - vec2.y;
			k = vec.z - vec2.z;
		}

		void LookAtDirection(const Toshi::TVector4& vec, const Toshi::TVector4& vec2);

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
			Set(DirectX::XMMatrixMultiply(this->XMM(), a.XMM()));
		}

	private:
		constexpr static float s_Identity[16] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	};

	inline TMatrix44 operator*(const TMatrix44& a, const TMatrix44& b)
	{
		return DirectX::XMMatrixMultiply(a.XMM(), b.XMM());
	}
}
