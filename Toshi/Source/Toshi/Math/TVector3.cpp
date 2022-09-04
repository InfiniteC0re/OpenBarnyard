#include "ToshiPCH.h"
#include "TVector3.h"

namespace Toshi
{
	void TVector3::Set(TMathFloating a_pTMathFloating[3])
	{
		x = a_pTMathFloating[0];
		y = a_pTMathFloating[1];
		z = a_pTMathFloating[2];
	}

	void TVector3::Set(TMathFloating x, TMathFloating y, TMathFloating z)
	{
		TVector3::x = x;
		TVector3::y = y;
		TVector3::z = z;
	}

	void TVector3::Abs(const TVector3& a_Vec3)
	{
		Set(TMath::Abs(a_Vec3.x), TMath::Abs(a_Vec3.y), TMath::Abs(a_Vec3.z));
	}

	void TVector3::Abs()
	{
		Set(TMath::Abs(x), TMath::Abs(y), TMath::Abs(z));
	}

	void TVector3::Clip(TMathFloating fVal, TMathFloating fVal2)
	{
		TMath::Clip(x, fVal, fVal2);
		TMath::Clip(y, fVal, fVal2);
		TMath::Clip(z, fVal, fVal2);
	}

	void TVector3::CrossProduct(const TVector3& param_1, const TVector3& param_2)
	{
		TMathFloating fX = param_2.z * param_1.y - param_2.y * param_1.z;
		TMathFloating fY = param_1.z * param_2.x - param_2.z * param_1.x;
		TMathFloating fZ = param_1.x * param_2.y - param_1.y * param_2.x;
		Set(fX, fY, fZ);
	}

	float TVector3::Distance(const TVector3& param_1, const TVector3& param_2)
	{
		TMathFloating fVar1 = param_2.x - param_1.x;
		TMathFloating fVar2 = param_2.y - param_1.y;
		TMathFloating fVar3 = param_2.z - param_1.z;

		return TMath::Sqrt(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3);
	}

	float TVector3::DistanceSq(const TVector3& param_1, const TVector3& param_2)
	{
		TMathFloating fVar1 = param_2.x - param_1.x;
		TMathFloating fVar2 = param_2.y - param_1.y;
		TMathFloating fVar3 = param_2.z - param_1.z;

		return fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3;
	}

	void TVector3::Divide(const TVector3& param_1, const TVector3& param_2)
	{
		x = param_1.x / param_2.x;
		y = param_1.y / param_2.y;
		z = param_1.z / param_2.z;
	}

	void TVector3::Divide(const TVector3& param_1)
	{
		x /= param_1.x;
		y /= param_1.y;
		z /= param_1.z;
	}

	void TVector3::Divide(TMathFloating param_1)
	{
		TMathFloating ratio = 1.0f / param_1;
		x *= ratio;
		y *= ratio;
		z *= ratio;
	}

	void TVector3::Divide(const TVector3& param_1, TMathFloating param_2)
	{
		TMathFloating ratio = 1.0f / param_2;
		x = param_1.x * ratio;
		y = param_1.y * ratio;
		z = param_1.z * ratio;
	}

	void TVector3::RotateX(TMathFloating a_fRotation)
	{
		TMathFloating f1;
		TMathFloating f2;
		TMathFloating oldY = y;

		TMath::SinCos(a_fRotation, f1, f2);

		y = -f1 * z + f2 * oldY;
		z = oldY * f1 + z * f2;
	}

	void TVector3::RotateY(TMathFloating a_fRotation)
	{
		TMathFloating f1;
		TMathFloating f2;
		TMathFloating oldX = x;

		TMath::SinCos(a_fRotation, f1, f2);

		x = f1 * z + f2 * oldX;
		z = z * f2 + -f1 * oldX;
	}

	void TVector3::RotateZ(TMathFloating a_fRotation)
	{
		TMathFloating f1;
		TMathFloating f2;
		TMathFloating oldX = x;

		TMath::SinCos(a_fRotation, f1, f2);

		x = -f1 * y + f2 * oldX;
		y = f1 * oldX + y * f2;
	}
}