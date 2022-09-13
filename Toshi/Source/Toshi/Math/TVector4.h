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
		void Abs(const TVector4& a_rVec4);
		void Abs();
		void Add(const TVector4& a_rVec4, const TVector4& a_rVec4_2);
		void Add(const TVector4& a_rVec4);
		inline const TVector3& AsVector3() const { return reinterpret_cast<const TVector3&>(*this); }
		inline TVector3& AsVector3() { return reinterpret_cast<TVector3&>(*this); }
		void Cos(const TVector4& a_rVec4);
		void CrossProduct(const TVector4& a_rVec4, const TVector4& a_rVec4_2);
		static float Distance(const TVector4& a_rVec4, const TVector4& a_rVec4_2);
		static float DistanceSq(const TVector4& a_rVec4, const TVector4& a_rVec4_2);
		void Negate3();
		void Set(const TVector3& a_rVec3);
		void Set(float a_fVal1, float a_fVal2, float a_fVal3, float a_fVal4);
		void Set(float a_pfVal[4]);
	};

	static_assert(sizeof(TVector4) == sizeof(Props::Vec4));
}