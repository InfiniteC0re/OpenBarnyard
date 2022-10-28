#pragma once

namespace Toshi
{
	class TVector2
	{
	public:
		TVector2() { Set(0, 0); }
		TVector2(TFloat x, TFloat y) { Set(x, y); }
		TVector2(TFloat coords[2]) { Set(coords[0], coords[1]); }
		TVector2(const TVector2& other) { Set(other); }

		void Set(TFloat x, TFloat y) { TVector2::x = x; TVector2::y = y; }
		void Set(TFloat coords[2]) { TVector2::x = coords[0]; TVector2::y = coords[1]; }
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

		void Divide(TFloat scalar)
		{
			TFloat ratio = 1.0f / scalar;
			x *= ratio;
			y *= ratio;
		}

		void Divide(const TVector2& vec1, const TVector2& vec2)
		{
			Set(vec1 / vec2);
		}

		void Divide(const TVector2& vec, TFloat scalar)
		{
			Set(vec);
			Divide(scalar);
		}

		void Multiply(const TVector2& vec)
		{
			x *= vec.x;
			y *= vec.y;
		}

		void Multiply(TFloat scalar)
		{
			x *= scalar;
			y *= scalar;
		}

		void Multiply(const TVector2& vec1, const TVector2& vec2)
		{
			Set(vec1 * vec2);
		}

		void Multiply(const TVector2& vec, TFloat scalar)
		{
			Set(vec);
			Multiply(scalar);
		}

		void Abs(const TVector2& vec3) { Set(TMath::Abs(vec3.x), TMath::Abs(vec3.y)); }
		void Abs() { Set(TMath::Abs(x), TMath::Abs(y)); }

		TFloat Magnitude() const { return TMath::Sqrt(x * x + y * y); }
		TFloat MagnitudeSq() const { return x * x + y * y; }

		TVector2 operator+(const TVector2& other) const { return { x + other.x, y + other.y }; }
		TVector2 operator-(const TVector2& other) const { return { x - other.x, y - other.y }; }
		TVector2 operator*(const TVector2& other) const { return { x * other.x, y * other.y }; }
		TVector2 operator/(const TVector2& other) const { return { x / other.x, y / other.y }; }

		void operator=(const TVector2& other) { Set(other); }
		void operator+=(const TVector2& other) { Add(other); }
		void operator/=(const TVector2& other) { Divide(other); }

	public:
		static TFloat Distance(const TVector2& vec1, const TVector2& vec2) { return (vec2 - vec1).Magnitude(); }
		static TFloat DistanceSq(const TVector2& vec1, const TVector2& vec2) { return (vec2 - vec1).MagnitudeSq(); }
		static TFloat DotProduct(const TVector2& vec1, const TVector2& vec2) { return vec1.x * vec2.x + vec1.y * vec2.y; }

	public:
		float x, y;
	};
}