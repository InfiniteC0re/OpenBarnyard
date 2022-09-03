#include "pch.h"
#include "TVector4.h"

namespace Toshi
{
	void TVector4::Negate3()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	void TVector4::Set(const TVector3& a_rVec3)
	{
		x = a_rVec3.x;
		y = a_rVec3.y;
		z = a_rVec3.z;
		w = 1.0f;
	}
}
