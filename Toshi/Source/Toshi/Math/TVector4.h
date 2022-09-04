#pragma once
#include "Toshi/Math/TVector3.h"

namespace Toshi
{
	class TVector4;

	namespace Props
	{
		struct Vec4
		{
			TMathFloating x, y, z, w;
			operator Toshi::TVector4& () { return reinterpret_cast<Toshi::TVector4&>(*this); }
		};
	}

	class TVector4 : public Props::Vec4
	{
	public:
		void Negate3();
		void Set(const TVector3& a_rVec3);
	};

	static_assert(sizeof(TVector4) == sizeof(Props::Vec4));
}