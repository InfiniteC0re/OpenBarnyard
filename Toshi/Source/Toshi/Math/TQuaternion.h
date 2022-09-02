#pragma once
namespace Toshi
{
	class TQuaternion
	{
		float x, y, z, w;

		

	public:
		const float DELTA = 0.001f;
		const Toshi::TQuaternion* IDENTITY = new Toshi::TQuaternion(0.0f,0.0f,0.0f,1.0f);

		void Set(float x, float y, float z, float w);
		void SetIdentity();
		void SetFromEulerYX(const float* fVal);

		TQuaternion(const TQuaternion& a_pQuaternion);
		TQuaternion(float x, float y, float z, float w);
		TQuaternion();
	};

}

