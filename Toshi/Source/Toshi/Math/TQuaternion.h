#pragma once
#include "Toshi/Math/TVector4.h"

namespace Toshi
{
	class TQuaternion;

	namespace Props
	{
		struct Quaternion
		{
			TVectorNum x, y, z, w;
			operator Toshi::TVector4& () { return reinterpret_cast<Toshi::TVector4&>(*this); }
			operator Toshi::TQuaternion& () { return reinterpret_cast<Toshi::TQuaternion&>(*this); }
		};
	}

	class TQuaternion : Props::Quaternion
	{
	public:
		TQuaternion();
		TQuaternion(const TQuaternion& a_pQuaternion);
		TQuaternion(float x, float y, float z, float w);

		void Set(float x, float y, float z, float w);
		void SetIdentity();
		void SetFromEulerYX(const float* fVal);
		void SetFromEulerRollPitchYaw(float a_fRoll, float a_fPitch, float a_fYaw);

		inline void SetConjugate() { AsVector4().Negate3(); }
		inline void SetInverse() { SetConjugate(); }
		void SetRotation(const TVector3& a_rVec3, float a_fVal);
		void SetVectorDirection(const TVector3& a_rVec3, const TVector3& a_rVec3_2);

		static void RotateVector(TVector3& param_1, const TQuaternion& param_2, const TVector3 param_3);
		void RotateAroundAxis(const TVector3& param_1, float param_2);

		void Negate();

		inline float Magnitude() const { return TMath::Sqrt(w * w + z * z + y * y + x * x); }
		inline float MagnitudeSq() const { return w * w + z * z + y * y + x * x; }

		inline const TVector4& AsVector4() const { return reinterpret_cast<const TVector4&>(*this); }
		inline TVector4& AsVector4() { return reinterpret_cast<TVector4&>(*this); }

		TQuaternion& operator*=(const TQuaternion& a_Quat);

	public:
		static inline constexpr float DELTA = 0.001f;
		static inline constexpr Props::Quaternion IDENTITY{ 0.0f, 0.0f, 0.0f, 1.0f };
	};

	static_assert(sizeof(TQuaternion) == sizeof(Props::Quaternion));
}