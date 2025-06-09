#include "ToshiPCH.h"

#include "TMath.h"
#include "TVector3.h"
#include "TMathInline.h"

TOSHI_NAMESPACE_START

TBOOL TMath::ConeVsSphere( const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, TFLOAT a_fConeFOV, TFLOAT a_fSphereRadius, const TVector4& a_rvSpherePosition )
{
	TASSERT( a_fConeFOV < TMath::HALF_PI );
	TASSERT( a_rvConeDirection.isNormalised() );

	TVector3 direction   = a_rvSpherePosition.AsVector3() - a_rvConePosition.AsVector3();
	TFLOAT   fDotProduct = TVector3::DotProduct( a_rvConeDirection.AsVector3(), direction );

	if ( fDotProduct > 0.0f && fDotProduct <= a_fSphereRadius )
	{
		direction.Normalize();

		fDotProduct = TVector3::DotProduct( a_rvConeDirection.AsVector3(), direction );
		TMath::Clip( fDotProduct, -1.0f, 1.0f );

		return ACos( fDotProduct ) <= a_fConeFOV;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006b6080
TMath::VSRESULT TMath::RayVsCircle3D( const TVector4& a_rcStartPos, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfRayDistance, TFLOAT& a_rfUnknown )
{
	TVector4 vecRayToCenter  = a_rcCenter - a_rcStartPos;
	TFLOAT   fRadiusSq       = a_fRadius * a_fRadius;
	TFLOAT   fDistToCenterSq = vecRayToCenter.MagnitudeSq();

	if ( fRadiusSq <= fDistToCenterSq )
	{
		TFLOAT fDot = TVector4::DotProduct3( vecRayToCenter, a_rcDirection );

		if ( fDot < 0.0f )
		{
			// The ray direction is pointing against the center
			return VSRESULT_OUTSIDE;
		}

		if ( fRadiusSq < fDistToCenterSq - fDot * fDot )
		{
			// The ray won't hit the circle
			return VSRESULT_OUTSIDE;
		}
	}

	// Negate to make it point from the center to the ray
	vecRayToCenter *= -1.0f;
	TFLOAT fDot = TVector4::DotProduct3( vecRayToCenter, a_rcDirection ) * 2;

	fRadiusSq = fDot * fDot - ( fDistToCenterSq - fRadiusSq ) * 4.0f;

	if ( fRadiusSq < 0.0f )
		return VSRESULT_OUTSIDE;

	TFLOAT fRadius  = TMath::Sqrt( fRadiusSq );
	a_rfRayDistance = ( -fDot - fRadiusSq ) * 0.5f;
	a_rfUnknown     = ( fRadiusSq - fDot ) * 0.5f;

	return VSRESULT_INSIDE;
}

TOSHI_NAMESPACE_END
