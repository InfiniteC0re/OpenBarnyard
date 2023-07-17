#pragma once

namespace Toshi
{
	class TQuaternion
	{
	public:
		TQuaternion() :
			x(0.0f), y(0.0f), z(0.0f), w(1.0f)
		{ }

		TQuaternion(const TQuaternion& a_pQuaternion) :
			x(a_pQuaternion.x), y(a_pQuaternion.y), z(a_pQuaternion.z), w(a_pQuaternion.w) { }

		TQuaternion(TFloat a_fX, TFloat a_fY, TFloat a_fZ, TFloat a_fW) :
			x(a_fX), y(a_fY), z(a_fZ), w(a_fW) { }

		void Set(TFloat a_fX, TFloat a_fY, TFloat a_fZ, TFloat a_fW)
		{
			x = a_fX;
			y = a_fY;
			z = a_fZ;
			w = a_fW;
		}

		void SetIdentity()
		{
			Set(
				TQuaternion::IDENTITY.x,
				TQuaternion::IDENTITY.y,
				TQuaternion::IDENTITY.z,
				TQuaternion::IDENTITY.w
			);
		}

		void SetFromEulerYX(const TFloat* fVal);
		void SetFromEulerRollPitchYaw(TFloat a_fRoll, TFloat a_fPitch, TFloat a_fYaw);

		inline void SetConjugate() { AsVector4().Negate3(); }
		inline void SetInverse() { SetConjugate(); }
		void SetRotation(const TVector3& a_rVec3, TFloat a_fVal);
		void SetVectorDirection(const TVector3& a_rVec3, const TVector3& a_rVec3_2);

		static void RotateVector(TVector3& param_1, const TQuaternion& param_2, const TVector3 param_3);
		void RotateAroundAxis(const TVector3& param_1, TFloat param_2);

		void Negate() { Set(-x, -y, -z, -w); }
		void GetEulerXYZ(TVector3& outVec) const;

		TFloat Magnitude() const { return TMath::Sqrt(w * w + z * z + y * y + x * x); }
		TFloat MagnitudeSq() const { return w * w + z * z + y * y + x * x; }

		const TVector4& AsVector4() const { return reinterpret_cast<const TVector4&>(*this); }
		TVector4& AsVector4() { return reinterpret_cast<TVector4&>(*this); }

		void operator=(const TQuaternion& other) { Set(other.x, other.y, other.z, other.w); }
		TQuaternion& operator*=(const TQuaternion& a_rQuat);

		static void Nlerp(TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, float a_fProgress);
		static void Slerp(TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, float a_fProgress);

	public:
		static const TQuaternion IDENTITY;
		static inline constexpr TFloat DELTA = 0.001f;
	
	public:
		TFloat x, y, z, w;
	};
}