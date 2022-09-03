#pragma once
#include <Toshi/Math/TVector3.h>
namespace Toshi
{
	class TVector4
	{
	public:
		float x, y, z, w;
	
		void Negate3();
		void Set(const TVector3& a_rVec3);

		
	};
}


