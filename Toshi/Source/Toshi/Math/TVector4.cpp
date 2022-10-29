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
}
