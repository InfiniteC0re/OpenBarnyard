#pragma once
namespace Toshi
{
	class TQuaternion
	{
		float x, y, z, w;

		

	public:
		const float DELTA = 0.001f;
		const Toshi::TQuaternion IDENTITY = Toshi::TQuaternion(0.0f,0.0f,0.0f,1.0f);

		void Set(float x, float y, float z, float w);
		void SetIdentity();
		void SetFromEulerYX(const float* fVal);
		void SetFromEulerRollPitchYaw(float a_fRoll, float a_fPitch, float a_fYaw);


		void Negate();

		inline float Magnitude() const { return TMath::Sqrt(w * w + z * z + y * y + x * x); }
		inline float MagnitudeSq() const { return w * w + z * z + y * y + x * x; }


		TQuaternion& operator*=(const TQuaternion& a_Quat);

		TQuaternion(const TQuaternion& a_pQuaternion);
		TQuaternion(float x, float y, float z, float w);
		TQuaternion();
	};

}

