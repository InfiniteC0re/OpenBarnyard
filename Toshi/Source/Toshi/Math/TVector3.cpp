#include "ToshiPCH.h"
#include "TVector3.h"

namespace Toshi
{
	const TVector3 TVector3::VEC_ZERO = { 0, 0, 0 };
	const TVector3 TVector3::VEC_POSX = { 1, 0, 0 };
	const TVector3 TVector3::VEC_POSY = { 0, 1, 0 };
	const TVector3 TVector3::VEC_POSZ = { 0, 0, 1 };
	const TVector3 TVector3::VEC_NEGX = { -1, 0, 0 };
	const TVector3 TVector3::VEC_NEGY = { 0, -1, 0 };
	const TVector3 TVector3::VEC_NEGZ = { 0, 0, -1 };
	const TVector3 TVector3::VEC_POSXPOSZ = { TMath::ONEOVER_SQRT_TWO, 0, TMath::ONEOVER_SQRT_TWO };
	const TVector3 TVector3::VEC_POSXNEGZ = { TMath::ONEOVER_SQRT_TWO, 0, -TMath::ONEOVER_SQRT_TWO };
	const TVector3 TVector3::VEC_NEGXPOSZ = { TVector3::VEC_POSXNEGZ.z, 0, TMath::ONEOVER_SQRT_TWO };
	const TVector3 TVector3::VEC_NEGXNEGZ = { TVector3::VEC_POSXNEGZ.z, 0, TVector3::VEC_POSXNEGZ.z };

	void TVector3::Clip(TFloat fVal, TFloat fVal2)
	{
		TMath::Clip(x, fVal, fVal2);
		TMath::Clip(y, fVal, fVal2);
		TMath::Clip(z, fVal, fVal2);
	}

	void TVector3::CrossProduct(const TVector3& vec1, const TVector3& vec2)
	{
		TFloat fX = vec2.z * vec1.y - vec2.y * vec1.z;
		TFloat fY = vec1.z * vec2.x - vec2.z * vec1.x;
		TFloat fZ = vec1.x * vec2.y - vec1.y * vec2.x;
		Set(fX, fY, fZ);
	}

	void TVector3::RotateX(TFloat a_fRotation)
	{
		TFloat f1;
		TFloat f2;
		TFloat oldY = y;

		TMath::SinCos(a_fRotation, f1, f2);

		y = -f1 * z + f2 * oldY;
		z = oldY * f1 + z * f2;
	}

	void TVector3::RotateY(TFloat a_fRotation)
	{
		TFloat f1;
		TFloat f2;
		TFloat oldX = x;

		TMath::SinCos(a_fRotation, f1, f2);

		x = f1 * z + f2 * oldX;
		z = z * f2 + -f1 * oldX;
	}

	void TVector3::RotateZ(TFloat a_fRotation)
	{
		TFloat f1;
		TFloat f2;
		TFloat oldX = x;

		TMath::SinCos(a_fRotation, f1, f2);

		x = -f1 * y + f2 * oldX;
		y = f1 * oldX + y * f2;
	}
}