#pragma once

namespace Toshi
{
	class TQuaternion
	{
	public:
		TQuaternion();
		TQuaternion(const TQuaternion& a_pQuaternion);
		TQuaternion(TFloat x, TFloat y, TFloat z, TFloat w);

		void Set(TFloat x, TFloat y, TFloat z, TFloat w);
		void SetIdentity();
		void SetFromEulerYX(const TFloat* fVal);
		void SetFromEulerRollPitchYaw(TFloat a_fRoll, TFloat a_fPitch, TFloat a_fYaw);

		inline void SetConjugate() { AsVector4().Negate3(); }
		inline void SetInverse() { SetConjugate(); }
		void SetRotation(const TVector3& a_rVec3, TFloat a_fVal);
		void SetVectorDirection(const TVector3& a_rVec3, const TVector3& a_rVec3_2);

		static void RotateVector(TVector3& param_1, const TQuaternion& param_2, const TVector3 param_3);
		void RotateAroundAxis(const TVector3& param_1, TFloat param_2);

		void Negate();

		inline TFloat Magnitude() const { return TMath::Sqrt(w * w + z * z + y * y + x * x); }
		inline TFloat MagnitudeSq() const { return w * w + z * z + y * y + x * x; }

		inline const TVector4& AsVector4() const { return reinterpret_cast<const TVector4&>(*this); }
		inline TVector4& AsVector4() { return reinterpret_cast<TVector4&>(*this); }

		TQuaternion& operator*=(const TQuaternion& a_Quat);

	public:
		static const TQuaternion IDENTITY;
		static inline constexpr TFloat DELTA = 0.001f;
	
	public:
		TFloat x, y, z, w;
	};
}