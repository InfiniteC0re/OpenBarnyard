#include "ToshiPCH.h"
#include "TVector4.h"

namespace Toshi
{
	const TVector4 TVector4::VEC_ZERO = { 0, 0, 0, 1 };
	const TVector4 TVector4::VEC_POSX = { 1, 0, 0, 1 };
	const TVector4 TVector4::VEC_POSY = { 0, 1, 0, 1 };
	const TVector4 TVector4::VEC_POSZ = { 0, 0, 1, 1 };
	const TVector4 TVector4::VEC_NEGX = { -1, 0, 0, 1 };
	const TVector4 TVector4::VEC_NEGY = { 0, -1, 0, 1 };
	const TVector4 TVector4::VEC_NEGZ = { 0, 0, -1, 1 };

	void TVector4::Normalize()
	{
		if (MagnitudeSq() != 0.0f)
		{
			float magnitude = TMath::OneOverSqrt(MagnitudeSq());
			Set({ x * magnitude, y * magnitude, z * magnitude });
		}
		else
		{
			Set({ 0.0f, 0.0f, 1.0f });
		}
	}

	void TVector4::ProjectNormalToPlane(const TVector4& vec, const TVector4& vec2)
	{
		TVector4 crossProduct;
		crossProduct.CrossProduct(vec, vec2);
		CrossProduct(crossProduct, vec2);
		Normalize();
	}
}
