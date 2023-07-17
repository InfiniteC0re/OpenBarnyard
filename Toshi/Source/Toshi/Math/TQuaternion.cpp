#include "ToshiPCH.h"
#include "TQuaternion.h"

namespace Toshi
{
	const TQuaternion TQuaternion::IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

	TQuaternion::TQuaternion()
	{
		Set(0, 0, 0, 1);
	}

	TQuaternion::TQuaternion(const TQuaternion& a_pQuaternion)
	{
		Set(a_pQuaternion.x, a_pQuaternion.y, a_pQuaternion.z, a_pQuaternion.w);
	}

	TQuaternion::TQuaternion(TFloat x, TFloat y, TFloat z, TFloat w)
	{
		Set(x, y, z, w);
	}

	void TQuaternion::Set(TFloat x, TFloat y, TFloat z, TFloat w)
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

	void TQuaternion::SetFromEulerYX(const TFloat* fVal)
	{
		TFloat f1;
		TFloat f2;

		TMath::SinCos(fVal[0] * 0.5f, f1, f2);
		TQuaternion quat = TQuaternion(f1, 0, 0, f2);
		TMath::SinCos(fVal[1] * 0.5f, f1, f2);
		Set(0, f1, 0, f2);
		*this *= quat;
	}

	void TQuaternion::SetFromEulerRollPitchYaw(TFloat a_fRoll, TFloat a_fPitch, TFloat a_fYaw)
	{
		TFloat fCosRoll = cos(a_fRoll * 0.5f);
		TFloat fSinRoll = sin(a_fRoll * 0.5f);
		TFloat fCosPitch = cos(a_fPitch * 0.5f);
		TFloat fSinPitch = sin(a_fPitch * 0.5f);
		TFloat fCosYaw = cos(a_fYaw * 0.5f);
		TFloat fSinYaw = sin(a_fYaw * 0.5f);

		TFloat fX = fCosYaw * fCosPitch * fSinRoll - fSinYaw * fSinPitch * fCosRoll;
		TFloat fY = fCosPitch * fSinRoll * fSinYaw + fSinPitch * fCosRoll * fCosYaw;
		TFloat fZ = fCosRoll * fCosPitch * fSinYaw - fCosYaw * fSinPitch * fSinRoll;
		TFloat fW = fSinYaw * fSinPitch * fSinRoll + fCosYaw * fCosPitch * fCosRoll;

		Set(fX, fY, fZ, fW);
	}

	void TQuaternion::SetRotation(const TVector3& a_rVec3, TFloat a_fVal)
	{
		TFloat fVal;
		TFloat fVal2;

		TMath::SinCos(a_fVal * 0.5f, fVal, fVal2);
		Set(a_rVec3.x * fVal, a_rVec3.y * fVal, a_rVec3.z * fVal, fVal2);
	}

	void TQuaternion::SetVectorDirection(const TVector3& a_rVec3, const TVector3& a_rVec3_2)
	{
		TVector3 vec3;
		vec3.CrossProduct(a_rVec3_2, a_rVec3);

		TFloat mag = vec3.Magnitude();

		if (mag > TMath::TFLOAT_EPSILON)
		{
			vec3.Divide(mag);
			TFloat dotProduct = TVector3::DotProduct(a_rVec3_2, a_rVec3);
			TFloat rot = 0.0f;
			if ((dotProduct <= 1.0f - TMath::TFLOAT_EPSILON) && (rot = TMath::PI, -1.0f + TMath::TFLOAT_EPSILON <= dotProduct))
			{
				rot = TMath::ACos(dotProduct);
			}
			SetRotation(vec3, rot);
			return;
		}

		TFloat dotProduct = TVector3::DotProduct(a_rVec3_2, a_rVec3);

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

	void TQuaternion::RotateAroundAxis(const TVector3& param_1, TFloat param_2)
	{
		TFloat fVal;
		TFloat fVal2;

		TMath::SinCos(param_2 * 0.5f, fVal, fVal2);
		*this *= TQuaternion(param_1.x * fVal, param_1.y * fVal, param_1.z * fVal, fVal2);
	}

	void TQuaternion::Negate()
	{
		Set(-x, -y, -z, -w);
	}

	void TQuaternion::GetEulerXYZ(TVector3& outVec) const
	{
		TMatrix44 matrix;
		matrix.SetFromQuaternion(*this);

		float fVal1 = matrix.m_f31;
		TMath::Clip(fVal1, -1.0f, 1.0f);
		
		float fVal2 = TMath::Sqrt(1.0f - fVal1 * fVal1);

		if (matrix.m_f11 < 0 && matrix.m_f33 < 0)
			fVal2 *= -1;

		if (0.001f < fVal2 || fVal2 < -0.001)
		{
			float fVal3 = 1.0f / fVal2;

			outVec.x = TMath::ATan2(matrix.m_f32 * fVal3, matrix.m_f33 * fVal3);
			outVec.y = TMath::ATan2(fVal1, fVal2);
			outVec.z = TMath::ATan2(matrix.m_f21 * fVal3, matrix.m_f11 * fVal3);
		}
		else
		{
			outVec.x = TMath::ATan2(-matrix.m_f32, matrix.m_f22);
			outVec.y = -TMath::ASin(fVal1);
			outVec.z = 0.0f;
		}
	}

	TQuaternion& TQuaternion::operator*=(const TQuaternion& a_Quat)
	{
		TFloat fX = ((x * a_Quat.w + a_Quat.z * y) - a_Quat.y * z) + a_Quat.x * w;
		TFloat fY = a_Quat.x * z + (y * a_Quat.w - a_Quat.z * x) + a_Quat.y * w;
		TFloat fZ = w * a_Quat.z + (a_Quat.y * x - a_Quat.x * y) + z * a_Quat.w;
		TFloat fW = (-(a_Quat.y * y + a_Quat.x * x) - a_Quat.z * z) + a_Quat.w * w;
		Set(fX, fY, fZ, fW);
		return *this;
	}
}