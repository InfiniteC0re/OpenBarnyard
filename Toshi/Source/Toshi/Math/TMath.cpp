#include "ToshiPCH.h"
#include "TMath.h"
#include "TVector3.h"
#include "TMathInline.h"

namespace Toshi {

	TBOOL TMath::ConeVsSphere(const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, float a_fConeFOV, float a_fSphereRadius, const TVector4& a_rvSpherePosition)
	{
		TASSERT(a_fConeFOV < TMath::HALF_PI);
		TASSERT(a_rvConeDirection.isNormalised());

		TVector3 direction = a_rvSpherePosition.AsVector3() - a_rvConePosition.AsVector3();
		float fDotProduct = TVector3::DotProduct(a_rvConeDirection.AsVector3(), direction);

		if (fDotProduct > 0.0f && fDotProduct <= a_fSphereRadius)
		{
			direction.Normalize();
			
			fDotProduct = TVector3::DotProduct(a_rvConeDirection.AsVector3(), direction);
			TMath::Clip(fDotProduct, -1.0f, 1.0f);

			return ACos(fDotProduct) <= a_fConeFOV;
		}

		return TFALSE;
	}

}