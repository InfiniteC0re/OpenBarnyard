#pragma once
#include "TVector.h"
#include "Toshi/Math/TMathInline.h"

namespace Toshi
{
	class TVector3;

	namespace Props
	{
		struct Vec3
		{
			TVectorNum x, y, z;
			operator Toshi::TVector3& () { return reinterpret_cast<Toshi::TVector3&>(*this); }
		};
	}

	class TVector3 : public Props::Vec3
	{
	public:
		TVector3() { Set(0, 0, 0); }
		TVector3(TVectorNum x, TVectorNum y, TVectorNum z) { Set(x, y, z); }
		TVector3(TVectorNum a_pTVectorNum[3]) { Set(a_pTVectorNum); }
		TVector3(const TVector3& a_Vec3) { Set(a_Vec3.x, a_Vec3.y, a_Vec3.z); }

		void Set(TVectorNum a_pTVectorNum[3]);
		void Set(TVectorNum x, TVectorNum y, TVectorNum z);

		void Abs(const TVector3& a_Vec3);
		void Abs();

		void Clip(TVectorNum fVal, TVectorNum fVal2);
		void CrossProduct(const TVector3&, const TVector3&);
		inline static TVectorNum DotProduct(const TVector3& param_1, const TVector3& param_2) { return param_1.x * param_2.x + param_1.y * param_2.y + param_1.z * param_2.z; }

		static float Distance(const TVector3&, const TVector3&);
		static float DistanceSq(const TVector3&, const TVector3&);

		inline float Magnitude() const { return TMath::Sqrt(z * z + y * y + x * x); }
		inline float MagnitudeSq() const { return z * z + y * y + x * x; }

		void Divide(const TVector3& param_1, const TVector3& param_2);
		void Divide(const TVector3& param_1);
		void Divide(TVectorNum param_1);
		void Divide(const TVector3& param_1, TVectorNum param_2);

		void RotateX(TVectorNum a_fRotation);
		void RotateY(TVectorNum a_fRotation);
		void RotateZ(TVectorNum a_fRotation);

	public:
		static inline constexpr Props::Vec3 VEC_ZERO{ 0, 0, 0 };
	};

	static_assert(sizeof(TVector3) == sizeof(Props::Vec3));
}