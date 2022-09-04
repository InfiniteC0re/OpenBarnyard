#pragma once

namespace Toshi
{
	class TVector3;

	namespace Props
	{
		struct Vec3
		{
			TMathFloating x, y, z;
			operator Toshi::TVector3& () { return reinterpret_cast<Toshi::TVector3&>(*this); }
		};
	}

	class TVector3 : public Props::Vec3
	{
	public:
		TVector3() { Set(0, 0, 0); }
		TVector3(TMathFloating x, TMathFloating y, TMathFloating z) { Set(x, y, z); }
		TVector3(TMathFloating a_pTMathFloating[3]) { Set(a_pTMathFloating); }
		TVector3(const TVector3& a_Vec3) { Set(a_Vec3.x, a_Vec3.y, a_Vec3.z); }

		void Set(TMathFloating a_pTMathFloating[3]);
		void Set(TMathFloating x, TMathFloating y, TMathFloating z);

		void Abs(const TVector3& a_Vec3);
		void Abs();

		void Clip(TMathFloating fVal, TMathFloating fVal2);
		void CrossProduct(const TVector3&, const TVector3&);
		inline static TMathFloating DotProduct(const TVector3& param_1, const TVector3& param_2) { return param_1.x * param_2.x + param_1.y * param_2.y + param_1.z * param_2.z; }

		static float Distance(const TVector3&, const TVector3&);
		static float DistanceSq(const TVector3&, const TVector3&);

		inline float Magnitude() const { return TMath::Sqrt(z * z + y * y + x * x); }
		inline float MagnitudeSq() const { return z * z + y * y + x * x; }

		void Divide(const TVector3& param_1, const TVector3& param_2);
		void Divide(const TVector3& param_1);
		void Divide(TMathFloating param_1);
		void Divide(const TVector3& param_1, TMathFloating param_2);

		void RotateX(TMathFloating a_fRotation);
		void RotateY(TMathFloating a_fRotation);
		void RotateZ(TMathFloating a_fRotation);

	public:
		static inline constexpr Props::Vec3 VEC_ZERO{ 0, 0, 0 };
	};

	static_assert(sizeof(TVector3) == sizeof(Props::Vec3));
}