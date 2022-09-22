#include "ToshiPCH.h"
#include "TQuaternion.h"

namespace Toshi
{
	TQuaternion::TQuaternion()
	{
		
	}

	TQuaternion::TQuaternion(const TQuaternion& a_pQuaternion)
	{
		Set(a_pQuaternion.x, a_pQuaternion.y, a_pQuaternion.z, a_pQuaternion.w);
	}

	TQuaternion::TQuaternion(TMathFloating x, TMathFloating y, TMathFloating z, TMathFloating w)
	{
		Set(x, y, z, w);
	}

	void TQuaternion::Set(TMathFloating x, TMathFloating y, TMathFloating z, TMathFloating w)
	{
		TQuaternion::x = x;
		TQuaternion::y = y;
		TQuaternion::z = z;
		TQuaternion::w = w;
	}

	void TQuaternion::SetIdentity()
	{
		Set(TQuaternion::IDENTITY.x, TQuaternion::IDENTITY.y, TQuaternion::IDENTITY.z, TQuaternion::IDENTITY.w);
	}

	void TQuaternion::SetFromEulerYX(const TMathFloating* fVal)
	{
		TMathFloating f1;
		TMathFloating f2;

		TMath::SinCos(fVal[0] * 0.5f, f1, f2);
		TQuaternion quat = TQuaternion(f1, 0, 0, f2);
		TMath::SinCos(fVal[1] * 0.5f, f1, f2);
		Set(0, f1, 0, f2);
		*this *= quat;
	}

	void TQuaternion::SetFromEulerRollPitchYaw(TMathFloating a_fRoll, TMathFloating a_fPitch, TMathFloating a_fYaw)
	{
		TMathFloating fCosRoll = cos(a_fRoll * 0.5f);
		TMathFloating fSinRoll = sin(a_fRoll * 0.5f);
		TMathFloating fCosPitch = cos(a_fPitch * 0.5f);
		TMathFloating fSinPitch = sin(a_fPitch * 0.5f);
		TMathFloating fCosYaw = cos(a_fYaw * 0.5f);
		TMathFloating fSinYaw = sin(a_fYaw * 0.5f);

		TMathFloating fX = fCosYaw * fCosPitch * fSinRoll - fSinYaw * fSinPitch * fCosRoll;
		TMathFloating fY = fCosPitch * fSinRoll * fSinYaw + fSinPitch * fCosRoll * fCosYaw;
		TMathFloating fZ = fCosRoll * fCosPitch * fSinYaw - fCosYaw * fSinPitch * fSinRoll;
		TMathFloating fW = fSinYaw * fSinPitch * fSinRoll + fCosYaw * fCosPitch * fCosRoll;

		Set(fX, fY, fZ, fW);
	}

	void TQuaternion::SetRotation(const TVector3& a_rVec3, TMathFloating a_fVal)
	{
		TMathFloating fVal;
		TMathFloating fVal2;

		TMath::SinCos(a_fVal * 0.5f, fVal, fVal2);
		Set(a_rVec3.x * fVal, a_rVec3.y * fVal, a_rVec3.z * fVal, fVal2);
	}

	void TQuaternion::SetVectorDirection(const TVector3& a_rVec3, const TVector3& a_rVec3_2)
	{
		TVector3 vec3;
		vec3.CrossProduct(a_rVec3_2, a_rVec3);

		TMathFloating mag = vec3.Magnitude();

		if (mag > TMath::TFLOAT_EPSILON)
		{
			vec3.Divide(mag);
			TMathFloating dotProduct = TVector3::DotProduct(a_rVec3_2, a_rVec3);
			TMathFloating rot = 0.0f;
			if ((dotProduct <= 1.0f - TMath::TFLOAT_EPSILON) && (rot = TMath::PI, -1.0f + TMath::TFLOAT_EPSILON <= dotProduct))
			{
				rot = TMath::ACos(dotProduct);
			}
			SetRotation(vec3, rot);
			return;
		}

		TMathFloating dotProduct = TVector3::DotProduct(a_rVec3_2, a_rVec3);

		if (dotProduct < 0.0f)
		{
			TVector3 vec = a_rVec3_2;
			if (TMath::Abs(a_rVec3_2.y) < 0.95f)
			{
				vec.RotateY(TMath::PI * 0.0055555557f * 90.0f);
			}
			else
			{
				vec.RotateX(TMath::PI * 0.0055555557f * 90.0f);
			}
			SetIdentity();
			RotateAroundAxis(vec, TMath::PI * 0.0055555557f * 180.0f);
		}
	}

	void TQuaternion::RotateVector(TVector3& param_1, const TQuaternion& param_2, const TVector3 param_3)
	{
		TQuaternion quat = param_2;
		quat.Negate();
		TQuaternion quat2 = TQuaternion(param_3.x, param_3.y, param_3.z, 0.0f);
		TQuaternion quat3 = param_2;
		quat3 *= quat2;
		quat3 *= quat;

		param_1.Set(quat3.x, quat3.y, quat3.z);
	}

	void TQuaternion::RotateAroundAxis(const TVector3& param_1, TMathFloating param_2)
	{
		TMathFloating fVal;
		TMathFloating fVal2;

		TMath::SinCos(param_2 * 0.5f, fVal, fVal2);
		*this *= TQuaternion(param_1.x * fVal, param_1.y * fVal, param_1.z * fVal, fVal2);
	}

	void TQuaternion::Negate()
	{
		Set(-x, -y, -z, -w);
	}

	TQuaternion& TQuaternion::operator*=(const TQuaternion& a_Quat)
	{
		TMathFloating fX = ((x * a_Quat.w + a_Quat.z * y) - a_Quat.y * z) + a_Quat.x * w;
		TMathFloating fY = a_Quat.x * z + (y * a_Quat.w - a_Quat.z * x) + a_Quat.y * w;
		TMathFloating fZ = w * a_Quat.z + (a_Quat.y * x - a_Quat.x * y) + z * a_Quat.w;
		TMathFloating fW = (-(a_Quat.y * y + a_Quat.x * x) - a_Quat.z * z) + a_Quat.w * w;
		Set(fX, fY, fZ, fW);
		return *this;
	}
}