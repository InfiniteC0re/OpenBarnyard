#include "ToshiPCH.h"
#include "TVector4.h"

namespace Toshi
{
	void TVector4::Abs(const TVector4& a_rVec4)
	{
		x = TMath::Abs(a_rVec4.x);
		y = TMath::Abs(a_rVec4.y);
		z = TMath::Abs(a_rVec4.z);
		w = TMath::Abs(a_rVec4.w);
	}

	void TVector4::Abs()
	{
		x = TMath::Abs(x);
		y = TMath::Abs(y);
		z = TMath::Abs(z);
		w = TMath::Abs(w);
	}

	void TVector4::Add(const TVector4& a_rVec4, const TVector4& a_rVec4_2)
	{
		x = a_rVec4.x + a_rVec4_2.x;
		y = a_rVec4.y + a_rVec4_2.y;
		z = a_rVec4.z + a_rVec4_2.z;
		y = a_rVec4.w + a_rVec4_2.w;
	}

	void TVector4::Add(const TVector4& a_rVec4)
	{
		x += a_rVec4.x;
		y += a_rVec4.y;
		z += a_rVec4.z;
	}

	void TVector4::Cos(const TVector4& a_rVec4)
	{
		x = TMath::ACos(a_rVec4.x);
		y = TMath::ACos(a_rVec4.y);
		z = TMath::ACos(a_rVec4.z);
		w = TMath::ACos(a_rVec4.w);
	}

	void TVector4::CrossProduct(const TVector4& a_rVec4, const TVector4& a_rVec4_2)
	{
		x = a_rVec4_2.z * a_rVec4.y - a_rVec4_2.y * a_rVec4.z;
		y = a_rVec4.z * a_rVec4_2.x - a_rVec4_2.z * a_rVec4.x;
		z = a_rVec4.x * a_rVec4_2.y - a_rVec4.y * a_rVec4_2.x;
		w = a_rVec4.w;
	}

	void TVector4::Negate3()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	void TVector4::Set(const TVector3& a_rVec3)
	{
		x = a_rVec3.x;
		y = a_rVec3.y;
		z = a_rVec3.z;
		w = 1.0f;
	}

	void TVector4::Set(float a_fVal1, float a_fVal2, float a_fVal3, float a_fVal4)
	{
		x = a_fVal1;
		y = a_fVal2;
		z = a_fVal3;
		w = a_fVal4;
	}

	void TVector4::Set(float a_pfVal[4])
	{
		x = a_pfVal[0];
		y = a_pfVal[1];
		z = a_pfVal[2];
		w = a_pfVal[3];
	}
}
