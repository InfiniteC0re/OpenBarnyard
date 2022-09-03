#pragma once
namespace Toshi
{
	class TVector3
	{
	public:
		float x, y, z;

	

		const Toshi::TVector3 VEC_ZERO = Toshi::TVector3(0.0f, 0.0f, 0.0f);

		void Set(const float* const a_pFloat);
		void Set(float x, float y, float z);

		void Abs(const TVector3& a_Vec3);
		void Abs();

		void Clip(float fVal, float fVal2);
		void CrossProduct(const TVector3&, const TVector3&);
		inline static float DotProduct(const TVector3& param_1, const TVector3& param_2) { return param_1.x * param_2.x + param_1.y * param_2.y + param_1.z * param_2.z; }

		static float Distance(const TVector3&, const TVector3&);
		static float DistanceSq(const TVector3&, const TVector3&);

		inline float Magnitude() const { return TMath::Sqrt(z * z + y * y + x * x); }
		inline float MagnitudeSq() const { return z * z + y * y + x * x; }

		void Divide(const TVector3& param_1, const TVector3& param_2);
		void Divide(const TVector3& param_1);
		void Divide(float param_1);
		void Divide(const TVector3& param_1, float param_2);

		void RotateX(float a_fRotation);
		void RotateY(float a_fRotation);
		void RotateZ(float a_fRotation);

		TVector3(float x, float y, float z);
		TVector3(const float* const a_pFloat);
		TVector3(const TVector3& a_Vec3);
		TVector3() {};
	};
}


