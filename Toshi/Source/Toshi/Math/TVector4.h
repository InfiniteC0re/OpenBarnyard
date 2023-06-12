#pragma once
#include "Toshi/Math/TVector3.h"

namespace Toshi
{
	class TVector4
	{
	public:
		TVector4() { Set(0, 0, 0, 1); }
		TVector4(TFloat x, TFloat y, TFloat z, TFloat w) { Set(x, y, z, w); }
		TVector4(TFloat floats[4]) { Set(floats); }
		TVector4(const TVector4& other) { Set(other.x, other.y, other.z, other.w); }

		void Set(const TVector3& vec) { TVector4::x = vec.x; TVector4::y = vec.y; TVector4::z = vec.z; TVector4::w = 1.0f; }
		void Set(const TVector4& vec) { TVector4::x = vec.x; TVector4::y = vec.y; TVector4::z = vec.z; TVector4::w = vec.w; }
		void Set(TFloat floats[4]) { TVector4::x = floats[0]; TVector4::y = floats[1]; TVector4::z = floats[2]; TVector4::w = floats[3]; }
		void Set(TFloat x, TFloat y, TFloat z, TFloat w) { TVector4::x = x; TVector4::y = y; TVector4::z = z; TVector4::w = w; }

		void Normalize();
		bool isNormalised(float fVal = 0.05f) const { return (((1.0f - fVal) * (1.0f - fVal)) < MagnitudeSq()) && (((1.0f + fVal) * (1.0f + fVal)) >= MagnitudeSq()); }

		void ProjectNormalToPlane(const TVector4& vec, const TVector4& vec2);

		void Add(const TVector4& vec)
		{
			x += vec.x;
			y += vec.y;
			z += vec.z;
		}

		void Add(const TVector4& a, const TVector4& b)
		{
			Set(a + b);
			w = a.w;
		}

		void Divide(const TVector4& vec)
		{
			x /= vec.x;
			y /= vec.y;
			z /= vec.z;
		}

		void Divide(TFloat scalar)
		{
			TFloat ratio = 1.0f / scalar;
			x *= ratio;
			y *= ratio;
			z *= ratio;
		}

		void Divide(const TVector4& vec1, const TVector4& vec2)
		{
			Set(vec1 / vec2);
		}

		void Divide(const TVector4& vec, TFloat scalar)
		{
			Set(vec);
			Divide(scalar);
		}

		void Multiply(const TVector4& vec)
		{
			x *= vec.x;
			y *= vec.y;
			z *= vec.z;
		}

		void Multiply(TFloat scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}

		void Multiply(const TVector4& vec1, const TVector4& vec2)
		{
			Set(vec1 * vec2);
		}

		void Multiply(const TVector4& vec, TFloat scalar)
		{
			Set(vec);
			Multiply(scalar);
		}

		void Cos(const TVector4& vec)
		{
			x = TMath::Cos(vec.x);
			y = TMath::Cos(vec.y);
			z = TMath::Cos(vec.z);
			w = TMath::Cos(vec.w);
		}

		void CrossProduct(const TVector4& vec1, const TVector4& vec2)
		{
			x = vec2.z * vec1.y - vec2.y * vec1.z;
			y = vec1.z * vec2.x - vec2.z * vec1.x;
			z = vec1.x * vec2.y - vec1.y * vec2.x;
			w = vec1.w;
		}

		void Lerp3(const TVector4& finish, float t) { Lerp3(*this, finish, t); }
		void Lerp3(const TVector4& start, const TVector4& finish, float t)
		{
			TVector4 progress = finish - start;
			progress.Multiply(t);

			TVector4::x = progress.x + start.x;
			TVector4::y = progress.y + start.y;
			TVector4::z = progress.z + start.z;
			TVector4::w = start.w;
		}

		double GetScalarProjectionOnUnit(const TVector4& vec)
		{
			return (double)(TVector4::z * vec.z + TVector4::x * vec.x + TVector4::y * vec.y);
		}

		void Abs(const TVector4& vec4) { Set(TMath::Abs(vec4.x), TMath::Abs(vec4.y), TMath::Abs(vec4.z), w); }
		void Abs() { Set(TMath::Abs(x), TMath::Abs(y), TMath::Abs(z), w); }
		void Negate3() { Set(-x, -y, -z, w); }

		TFloat Magnitude() const { return TMath::Sqrt(x * x + y * y + z * z); }
		TFloat MagnitudeSq() const { return x * x + y * y + z * z; }
		TFloat MagnitudeXZ() const { return TMath::Sqrt(x * x + z * z); }
		TFloat MagnitudeSqXZ() const { return x * x + z * z; }

		TVector4 operator+(const TVector4& other) const { return { x + other.x, y + other.y, z + other.z, other.w }; }
		TVector4 operator-(const TVector4& other) const { return { x - other.x, y - other.y, z - other.z, other.w }; }
		TVector4 operator*(const TVector4& other) const { return { x * other.x, y * other.y, z * other.z, other.w }; }
		TVector4 operator/(const TVector4& other) const { return { x / other.x, y / other.y, z / other.z, other.w }; }

		void operator=(const TVector4& other) { Set(other); }
		void operator+=(const TVector4& other) { Add(other); }
		void operator/=(const TVector4& other) { Divide(other); }
		void operator*=(const TVector4& other) { Multiply(other); }

		TVector3& AsVector3() { return reinterpret_cast<TVector3&>(*this); }
		const TVector3& AsVector3() const { return reinterpret_cast<const TVector3&>(*this); }

	public:
		static float Distance(const TVector4& vec1, const TVector4& vec2) { return (vec2 - vec1).Magnitude(); }
		static float DistanceSq(const TVector4& vec1, const TVector4& vec2) { return (vec2 - vec1).MagnitudeSq(); }
		static float DistanceXZ(const TVector4& vec1, const TVector4& vec2) { return (vec2 - vec1).MagnitudeXZ(); }
		static float DistanceSqXZ(const TVector4& vec1, const TVector4& vec2) { return (vec2 - vec1).MagnitudeSqXZ(); }

	public:
		static const TVector4 VEC_ZERO;
		static const TVector4 VEC_POSX;
		static const TVector4 VEC_POSY;
		static const TVector4 VEC_POSZ;
		static const TVector4 VEC_NEGX;
		static const TVector4 VEC_NEGY;
		static const TVector4 VEC_NEGZ;

	public:
		TFloat x, y, z, w;
	};
}