#pragma once

namespace Toshi
{

	class TQuaternion;

	namespace Props
	{
		struct Quaternion
		{
			TMathFloating x, y, z, w;
			operator Toshi::TVector4& () { return reinterpret_cast<Toshi::TVector4&>(*this); }
			operator Toshi::TQuaternion& () { return reinterpret_cast<Toshi::TQuaternion&>(*this); }
		};
	}

	class TQuaternion : Props::Quaternion
	{
	public:
		TQuaternion();
		TQuaternion(const TQuaternion& a_pQuaternion);
		TQuaternion(TMathFloating x, TMathFloating y, TMathFloating z, TMathFloating w);

		void Set(TMathFloating x, TMathFloating y, TMathFloating z, TMathFloating w);
		void SetIdentity();
		void SetFromEulerYX(const TMathFloating* fVal);
		void SetFromEulerRollPitchYaw(TMathFloating a_fRoll, TMathFloating a_fPitch, TMathFloating a_fYaw);

		inline void SetConjugate() { AsVector4().Negate3(); }
		inline void SetInverse() { SetConjugate(); }
		void SetRotation(const TVector3& a_rVec3, TMathFloating a_fVal);
		void SetVectorDirection(const TVector3& a_rVec3, const TVector3& a_rVec3_2);

		static void RotateVector(TVector3& param_1, const TQuaternion& param_2, const TVector3 param_3);
		void RotateAroundAxis(const TVector3& param_1, TMathFloating param_2);

		void Negate();

		inline TMathFloating Magnitude() const { return TMath::Sqrt(w * w + z * z + y * y + x * x); }
		inline TMathFloating MagnitudeSq() const { return w * w + z * z + y * y + x * x; }

		inline const TVector4& AsVector4() const { return reinterpret_cast<const TVector4&>(*this); }
		inline TVector4& AsVector4() { return reinterpret_cast<TVector4&>(*this); }

		TQuaternion& operator*=(const TQuaternion& a_Quat);

	public:
		static inline constexpr TMathFloating DELTA = 0.001f;
		static inline constexpr Props::Quaternion IDENTITY{ 0.0f, 0.0f, 0.0f, 1.0f };
	};

	static_assert(sizeof(TQuaternion) == sizeof(Props::Quaternion));
}