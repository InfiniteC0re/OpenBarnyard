#include "pch.h"
#include "TVector3.h"

namespace Toshi
{
	void TVector3::Set(TVectorNum a_pTVectorNum[3])
	{
		x = a_pTVectorNum[0];
		y = a_pTVectorNum[1];
		z = a_pTVectorNum[2];
	}

	void TVector3::Set(TVectorNum x, TVectorNum y, TVectorNum z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	void TVector3::Abs(const TVector3& a_Vec3)
	{
		Set(TMath::Abs(a_Vec3.x), TMath::Abs(a_Vec3.y), TMath::Abs(a_Vec3.z));
	}

	void TVector3::Abs()
	{
		Set(TMath::Abs(x), TMath::Abs(y), TMath::Abs(z));
	}

	void TVector3::Clip(TVectorNum fVal, TVectorNum fVal2)
	{
		TMath::Clip(x, fVal, fVal2);
		TMath::Clip(y, fVal, fVal2);
		TMath::Clip(z, fVal, fVal2);
	}

	void TVector3::CrossProduct(const TVector3& param_1, const TVector3& param_2)
	{
		TVectorNum fX = param_2.z * param_1.y - param_2.y * param_1.z;
		TVectorNum fY = param_1.z * param_2.x - param_2.z * param_1.x;
		TVectorNum fZ = param_1.x * param_2.y - param_1.y * param_2.x;
		Set(fX, fY, fZ);
	}

	float TVector3::Distance(const TVector3& param_1, const TVector3& param_2)
	{
		TVectorNum fVar1 = param_2.x - param_1.x;
		TVectorNum fVar2 = param_2.y - param_1.y;
		TVectorNum fVar3 = param_2.z - param_1.z;

		return TMath::Sqrt(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3);
	}

	float TVector3::DistanceSq(const TVector3& param_1, const TVector3& param_2)
	{
		TVectorNum fVar1 = param_2.x - param_1.x;
		TVectorNum fVar2 = param_2.y - param_1.y;
		TVectorNum fVar3 = param_2.z - param_1.z;

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

	void TVector3::Divide(TVectorNum param_1)
	{
		TVectorNum ratio = 1.0f / param_1;
		x *= ratio;
		y *= ratio;
		z *= ratio;
	}

	void TVector3::Divide(const TVector3& param_1, TVectorNum param_2)
	{
		TVectorNum ratio = 1.0f / param_2;
		x = param_1.x * ratio;
		y = param_1.y * ratio;
		z = param_1.z * ratio;
	}

	void TVector3::RotateX(TVectorNum a_fRotation)
	{
		TVectorNum f1;
		TVectorNum f2;
		TVectorNum oldY = y;

		TMath::SinCos(a_fRotation, f1, f2);

		y = -f1 * z + f2 * oldY;
		z = oldY * f1 + z * f2;
	}

	void TVector3::RotateY(TVectorNum a_fRotation)
	{
		TVectorNum f1;
		TVectorNum f2;
		TVectorNum oldX = x;

		TMath::SinCos(a_fRotation, f1, f2);

		x = f1 * z + f2 * oldX;
		z = z * f2 + -f1 * oldX;
	}

	void TVector3::RotateZ(TVectorNum a_fRotation)
	{
		TVectorNum f1;
		TVectorNum f2;
		TVectorNum oldX = x;

		TMath::SinCos(a_fRotation, f1, f2);

		x = -f1 * y + f2 * oldX;
		y = f1 * oldX + y * f2;
	}
}