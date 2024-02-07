#pragma once

namespace Toshi
{
	class TVector3
	{
	public:
		TVector3() = default;
		TVector3(TFLOAT x, TFLOAT y, TFLOAT z) { Set(x, y, z); }
		TVector3(TFLOAT floats[3]) { Set(floats); }
		TVector3(const TVector3& other) { Set(other.x, other.y, other.z); }

		void Clip(TFLOAT fVal, TFLOAT fVal2);
		void CrossProduct(const TVector3&, const TVector3&);

		void RotateX(TFLOAT a_fRotation);
		void RotateY(TFLOAT a_fRotation);
		void RotateZ(TFLOAT a_fRotation);

		void Normalize();

		void Set(const TVector3& vec) { TVector3::x = vec.x; TVector3::y = vec.y; TVector3::z = vec.z; }
		void Set(TFLOAT floats[3]) { TVector3::x = floats[0]; TVector3::y = floats[1]; TVector3::z = floats[2]; }
		void Set(TFLOAT x, TFLOAT y, TFLOAT z) { TVector3::x = x; TVector3::y = y; TVector3::z = z; }

		void Lerp(const TVector3& finish, float t) { Lerp(*this, finish, t); }
		void Lerp(const TVector3& start, const TVector3& finish, float t)
		{
			TVector3 progress = finish - start;
			progress.Multiply(t);

			Set(start + progress);
		}

		void Add(const TVector3& vec)
		{
			x += vec.x;
			y += vec.y;
			z += vec.z;
		}

		void Add(const TVector3& a, const TVector3& b)
		{
			Set(a + b);
		}

		void Divide(const TVector3& vec)
		{
			x /= vec.x;
			y /= vec.y;
			z /= vec.z;
		}

		void Divide(TFLOAT scalar)
		{
			TFLOAT ratio = 1.0f / scalar;
			x *= ratio;
			y *= ratio;
			z *= ratio;
		}

		void Divide(const TVector3& vec1, const TVector3& vec2)
		{
			Set(vec1 / vec2);
		}

		void Divide(const TVector3& vec, TFLOAT scalar)
		{
			Set(vec);
			Divide(scalar);
		}

		void Multiply(const TVector3& vec)
		{
			x *= vec.x;
			y *= vec.y;
			z *= vec.z;
		}

		void Multiply(TFLOAT scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}

		void Multiply(const TVector3& vec1, const TVector3& vec2)
		{
			Set(vec1 * vec2);
		}

		void Multiply(const TVector3& vec, TFLOAT scalar)
		{
			Set(vec);
			Multiply(scalar);
		}

		void Abs(const TVector3& vec3) { Set(TMath::Abs(vec3.x), TMath::Abs(vec3.y), TMath::Abs(vec3.z)); }
		void Abs() { Set(TMath::Abs(x), TMath::Abs(y), TMath::Abs(z)); }

		TFLOAT Magnitude() const { return TMath::Sqrt(x * x + y * y + z * z); }
		TFLOAT MagnitudeSq() const { return x * x + y * y + z * z; }

		TVector3 operator+(const TVector3& other) const { return { x + other.x, y + other.y, z + other.z }; }
		TVector3 operator-(const TVector3& other) const { return { x - other.x, y - other.y, z - other.z }; }
		TVector3 operator*(const TVector3& other) const { return { x * other.x, y * other.y, z * other.z }; }
		TVector3 operator/(const TVector3& other) const { return { x / other.x, y / other.y, z / other.z }; }

		void operator=(const TVector3& other)  { Set(other); }
		void operator+=(const TVector3& other) { Add(other); }
		void operator/=(const TVector3& other) { Divide(other); }

	public:
		static TFLOAT Distance(const TVector3& vec1, const TVector3& vec2) { return (vec2 - vec1).Magnitude(); }
		static TFLOAT DistanceSq(const TVector3& vec1, const TVector3& vec2) { return (vec2 - vec1).MagnitudeSq(); }
		static TFLOAT DotProduct(const TVector3& vec1, const TVector3& vec2) { return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z; }

	public:
		static const TVector3 VEC_ZERO;
		static const TVector3 VEC_POSX;
		static const TVector3 VEC_POSY;
		static const TVector3 VEC_POSZ;
		static const TVector3 VEC_NEGX;
		static const TVector3 VEC_NEGY;
		static const TVector3 VEC_NEGZ;
		static const TVector3 VEC_NEGXPOSZ;
		static const TVector3 VEC_NEGXNEGZ;
		static const TVector3 VEC_POSXPOSZ;
		static const TVector3 VEC_POSXNEGZ;

	public:
		TFLOAT x, y, z;
	};
}