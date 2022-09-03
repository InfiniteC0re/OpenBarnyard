#pragma once
namespace Toshi
{
	class TVector3
	{
		float x, y, z;

	public:

		const Toshi::TVector3 VEC_ZERO = Toshi::TVector3(0.0f, 0.0f, 0.0f);

		void Set(const float* const a_pFloat);
		void Set(float x, float y, float z);

		void Abs(const TVector3& a_Vec3);

		void RotateX(float a_fRotation);
		void RotateY(float a_fRotation);
		void RotateZ(float a_fRotation);

		TVector3(float x, float y, float z);
		TVector3(const float* const a_pFloat);
		TVector3(const TVector3& a_Vec3);
		TVector3() {};
	};
}


