#pragma once

namespace Toshi
{
	class TVector2
	{
	public:
		TVector2() { Set(0, 0); }
		TVector2(TFLOAT x, TFLOAT y) { Set(x, y); }
		TVector2(TFLOAT coords[2]) { Set(coords[0], coords[1]); }
		TVector2(const TVector2& other) { Set(other); }

		void Set(TFLOAT x, TFLOAT y) { TVector2::x = x; TVector2::y = y; }
		void Set(TFLOAT coords[2]) { TVector2::x = coords[0]; TVector2::y = coords[1]; }
		void Set(const TVector2& other) { TVector2::x = other.x; TVector2::y = other.y; }

		void Add(const TVector2& vec)
		{
			x += vec.x;
			y += vec.y;
		}

		void Add(const TVector2& a, const TVector2& b)
		{
			Set(a + b);
		}

		void Divide(const TVector2& vec)
		{
			x /= vec.x;
			y /= vec.y;
		}

		void Divide(TFLOAT scalar)
		{
			TFLOAT ratio = 1.0f / scalar;
			x *= ratio;
			y *= ratio;
		}

		void Divide(const TVector2& vec1, const TVector2& vec2)
		{
			Set(vec1 / vec2);
		}

		void Divide(const TVector2& vec, TFLOAT scalar)
		{
			Set(vec);
			Divide(scalar);
		}

		void Multiply(const TVector2& vec)
		{
			x *= vec.x;
			y *= vec.y;
		}

		void Multiply(TFLOAT scalar)
		{
			x *= scalar;
			y *= scalar;
		}

		void Multiply(const TVector2& vec1, const TVector2& vec2)
		{
			Set(vec1 * vec2);
		}

		void Multiply(const TVector2& vec, TFLOAT scalar)
		{
			Set(vec);
			Multiply(scalar);
		}

		void Abs(const TVector2& vec3) { Set(TMath::Abs(vec3.x), TMath::Abs(vec3.y)); }
		void Abs() { Set(TMath::Abs(x), TMath::Abs(y)); }

		TFLOAT Magnitude() const { return TMath::Sqrt(x * x + y * y); }
		TFLOAT MagnitudeSq() const { return x * x + y * y; }

		TBOOL IsEqual(const TVector2& vec) { return TMath::Abs(x - x) < 0.00001 && TMath::Abs(y - y) < 0.00001; }

		TVector2 operator+(const TVector2& other) const { return { x + other.x, y + other.y }; }
		TVector2 operator-(const TVector2& other) const { return { x - other.x, y - other.y }; }
		TVector2 operator*(const TVector2& other) const { return { x * other.x, y * other.y }; }
		TVector2 operator/(const TVector2& other) const { return { x / other.x, y / other.y }; }

		void operator=(const TVector2& other) { Set(other); }
		void operator+=(const TVector2& other) { Add(other); }
		void operator/=(const TVector2& other) { Divide(other); }

	public:
		static TFLOAT Distance(const TVector2& vec1, const TVector2& vec2) { return (vec2 - vec1).Magnitude(); }
		static TFLOAT DistanceSq(const TVector2& vec1, const TVector2& vec2) { return (vec2 - vec1).MagnitudeSq(); }
		static TFLOAT DotProduct(const TVector2& vec1, const TVector2& vec2) { return vec1.x * vec2.x + vec1.y * vec2.y; }

	public:
		const static TVector2 VEC_ZERO;
		const static TVector2 VEC_POSX;
		const static TVector2 VEC_POSY;
		const static TVector2 VEC_NEGX;
		const static TVector2 VEC_NEGY;

	public:
		float x, y;
	};
}