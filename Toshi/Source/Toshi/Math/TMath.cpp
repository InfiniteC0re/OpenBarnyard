#include "ToshiPCH.h"
#include "TMath.h"

using namespace Toshi;

void TMath::ConeVsSphere(const TVector4& a_rvSphereDirection, const TVector4& a_rvConeDirection, float a_fConeFOV, float a_fSphereFOV, const TVector4& a_vVec)
{
	TASSERT(a_fConeFOV < TMath::HALF_PI);
	TASSERT(a_rvConeDirection.isNormalised());
	TVector4 vec = a_vVec - a_rvConeDirection;
	float mag = vec.MagnitudeSq();
	if (mag >= 0.0f && mag <= a_fSphereFOV)
	{
		TTODO("...");
	}
}
