#include "ToshiPCH.h"
#include "TQuaternion.h"
#include "TMatrix44.h"

namespace Toshi {

	const TQuaternion TQuaternion::IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

	void TQuaternion::SetFromEulerYX(const TFLOAT* fVal)
	{
		TFLOAT f1;
		TFLOAT f2;

		TMath::SinCos(fVal[0] * 0.5f, f1, f2);
		TQuaternion quat = TQuaternion(f1, 0, 0, f2);
		TMath::SinCos(fVal[1] * 0.5f, f1, f2);
		Set(0, f1, 0, f2);
		*this *= quat;
	}

	void TQuaternion::SetFromEulerRollPitchYaw(TFLOAT a_fRoll, TFLOAT a_fPitch, TFLOAT a_fYaw)
	{
		TFLOAT fCosRoll = cos(a_fRoll * 0.5f);
		TFLOAT fSinRoll = sin(a_fRoll * 0.5f);
		TFLOAT fCosPitch = cos(a_fPitch * 0.5f);
		TFLOAT fSinPitch = sin(a_fPitch * 0.5f);
		TFLOAT fCosYaw = cos(a_fYaw * 0.5f);
		TFLOAT fSinYaw = sin(a_fYaw * 0.5f);

		TFLOAT fX = fCosYaw * fCosPitch * fSinRoll - fSinYaw * fSinPitch * fCosRoll;
		TFLOAT fY = fCosPitch * fSinRoll * fSinYaw + fSinPitch * fCosRoll * fCosYaw;
		TFLOAT fZ = fCosRoll * fCosPitch * fSinYaw - fCosYaw * fSinPitch * fSinRoll;
		TFLOAT fW = fSinYaw * fSinPitch * fSinRoll + fCosYaw * fCosPitch * fCosRoll;

		Set(fX, fY, fZ, fW);
	}

	void TQuaternion::SetRotation(const TVector3& a_rVec3, TFLOAT a_fVal)
	{
		TFLOAT fVal;
		TFLOAT fVal2;

		TMath::SinCos(a_fVal * 0.5f, fVal, fVal2);
		Set(a_rVec3.x * fVal, a_rVec3.y * fVal, a_rVec3.z * fVal, fVal2);
	}

	void TQuaternion::SetVectorDirection(const TVector3& a_rVec3, const TVector3& a_rVec3_2)
	{
		TVector3 vec3;
		vec3.CrossProduct(a_rVec3_2, a_rVec3);

		TFLOAT mag = vec3.Magnitude();

		if (mag > TMath::TFLOAT_EPSILON)
		{
			vec3.Divide(mag);
			TFLOAT dotProduct = TVector3::DotProduct(a_rVec3_2, a_rVec3);
			TFLOAT rot = 0.0f;
			if ((dotProduct <= 1.0f - TMath::TFLOAT_EPSILON) && (rot = TMath::PI, -1.0f + TMath::TFLOAT_EPSILON <= dotProduct))
			{
				rot = TMath::ACos(dotProduct);
			}
			SetRotation(vec3, rot);
			return;
		}

		TFLOAT dotProduct = TVector3::DotProduct(a_rVec3_2, a_rVec3);

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

	void TQuaternion::RotateAroundAxis(const TVector3& param_1, TFLOAT param_2)
	{
		TFLOAT fVal;
		TFLOAT fVal2;

		TMath::SinCos(param_2 * 0.5f, fVal, fVal2);
		*this *= TQuaternion(param_1.x * fVal, param_1.y * fVal, param_1.z * fVal, fVal2);
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
		TFLOAT fX = ((x * a_Quat.w + a_Quat.z * y) - a_Quat.y * z) + a_Quat.x * w;
		TFLOAT fY = a_Quat.x * z + (y * a_Quat.w - a_Quat.z * x) + a_Quat.y * w;
		TFLOAT fZ = w * a_Quat.z + (a_Quat.y * x - a_Quat.x * y) + z * a_Quat.w;
		TFLOAT fW = (-(a_Quat.y * y + a_Quat.x * x) - a_Quat.z * z) + a_Quat.w * w;
		Set(fX, fY, fZ, fW);
		return *this;
	}

	void TQuaternion::Nlerp(TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, float a_fProgress)
	{
		float fDot = TVector4::DotProduct4(a_rStart.AsVector4(), a_rEnd.AsVector4());

		if (0.0f <= fDot)
		{
			TVector4 vec = a_rEnd.AsVector4() - a_rStart.AsVector4();
			vec.Progress(vec, a_fProgress);

			a_rOut.x = vec.x + a_rStart.x;
			a_rOut.y = vec.y + a_rStart.y;
			a_rOut.z = vec.z + a_rStart.z;
			a_rOut.w = vec.w + a_rStart.w;
		}
		else
		{
			TVector4 vec = a_rEnd.AsVector4() + a_rStart.AsVector4();
			vec.Progress(vec, a_fProgress);

			a_rOut.x = vec.x - a_rStart.x;
			a_rOut.y = vec.y - a_rStart.y;
			a_rOut.z = vec.z - a_rStart.z;
			a_rOut.w = vec.w - a_rStart.w;
		}

		float fMag = TMath::Sqrt(a_rOut.x * a_rOut.x + a_rOut.y * a_rOut.y + a_rOut.z * a_rOut.z + a_rOut.w * a_rOut.w);
		
		if (fMag != 0.0f)
		{
			float fOneOverMag = 1.0f / fMag;

			a_rOut.x /= fOneOverMag;
			a_rOut.y /= fOneOverMag;
			a_rOut.z /= fOneOverMag;
			a_rOut.w /= fOneOverMag;
		}
	}

	void TQuaternion::Slerp(TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, float a_fProgress)
	{
		if (a_fProgress == 0.0f)
		{
			a_rOut = a_rStart;
			return;
		}

		if (a_fProgress == 1.0f)
		{
			a_rOut = a_rEnd;
			return;
		}

		float fDot = TVector4::DotProduct4(a_rStart.AsVector4(), a_rEnd.AsVector4());
		TBOOL bNegativeDot = fDot < 0.0f;

		if (bNegativeDot)
			fDot *= -1;

		float fProg1 = a_fProgress;
		float fProg2;

		if (1.0f - fDot <= 0.001f)
		{
			fProg2 = 1.0f - a_fProgress;
		}
		else
		{
			float fAcos = TMath::ACos(fDot);
			float fSin = TMath::Sin(fAcos);

			fProg1 = TMath::Sin(fAcos * a_fProgress) * (1.0f / fSin);
			fProg2 = TMath::Sin((1.0f - a_fProgress) * fAcos) * (1.0f / fSin);
		}

		if (bNegativeDot)
		{
			a_rOut.x = a_rStart.x * fProg2 - a_rEnd.x * fProg1;
			a_rOut.y = a_rStart.y * fProg2 - a_rEnd.y * fProg1;
			a_rOut.z = a_rStart.z * fProg2 - a_rEnd.z * fProg1;
			a_rOut.w = a_rStart.w * fProg2 - a_rEnd.w * fProg1;
		}
		else
		{
			a_rOut.x = a_rStart.x * fProg2 + a_rEnd.x * fProg1;
			a_rOut.y = a_rStart.y * fProg2 + a_rEnd.y * fProg1;
			a_rOut.z = a_rStart.z * fProg2 + a_rEnd.z * fProg1;
			a_rOut.w = a_rStart.w * fProg2 + a_rEnd.w * fProg1;
		}
	}

}
