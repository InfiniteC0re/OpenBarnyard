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
TINT TMath::RayVsCircle3D( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_pNearIntersection, TFLOAT& a_pFarIntersection )
{
	TVector4 vecRayToCenter  = a_rcCenter - a_rcRayOrigin;
	TFLOAT   fRadiusSq       = a_fRadius * a_fRadius;
	TFLOAT   fDistToCenterSq = vecRayToCenter.MagnitudeSq();

	if ( fRadiusSq <= fDistToCenterSq )
	{
		TFLOAT fDot = TVector4::DotProduct3( vecRayToCenter, a_rcRayDir );

		if ( fDot < 0.0f )
		{
			// The ray direction is pointing against the center
			return 0;
		}

		if ( fRadiusSq < fDistToCenterSq - fDot * fDot )
		{
			// The ray won't hit the circle
			return 0;
		}
	}

	// Negate to make it point from the center to the ray
	vecRayToCenter *= -1.0f;
	TFLOAT fDot = TVector4::DotProduct3( vecRayToCenter, a_rcRayDir ) * 2;

	fRadiusSq = fDot * fDot - ( fDistToCenterSq - fRadiusSq ) * 4.0f;

	if ( fRadiusSq < 0.0f )
		return 0;

	TFLOAT fRadius      = TMath::Sqrt( fRadiusSq );
	a_pNearIntersection = ( -fDot - fRadius ) * 0.5f;
	a_pFarIntersection  = ( fRadius - fDot ) * 0.5f;

	return 2;
}

// $Barnyard: FUNCTION 006b7c40
TBOOL TMath::SphereVsLine( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TINT* a_pNumHits, TFLOAT* a_pNearIntersection, TFLOAT* a_pFarIntersection )
{
	TVector4 vecStartToEnd = a_rcLineEnd - a_rcLineStart;

	TFLOAT a = vecStartToEnd.MagnitudeSq();
	TFLOAT b = ( ( a_rcLineStart.y - a_rcCenter.y ) * vecStartToEnd.y + ( a_rcLineStart.z - a_rcCenter.z ) * vecStartToEnd.z + ( a_rcLineStart.x - a_rcCenter.x ) * vecStartToEnd.x ) * 2.0f;

	TFLOAT fDot          = TVector4::DotProduct3( a_rcCenter, a_rcLineStart );
	TFLOAT fDiscriminant = b * b - ( ( ( a_rcCenter.MagnitudeSq() + a_rcLineStart.MagnitudeSq() ) - ( fDot * 2.0f ) ) - a_fRadius * a_fRadius ) * a * 4.0f;

	if ( fDiscriminant < 0.0f )
		return TFALSE;

	if ( fDiscriminant == 0.0f )
	{
		if ( a_pNumHits )
			*a_pNumHits = 1;

		if ( a_pNearIntersection )
			*a_pNearIntersection = -( b / ( a * 2.0f ) );
	}
	else
	{
		if ( a_pNumHits )
			*a_pNumHits = 2;

		if ( a_pNearIntersection )
			*a_pNearIntersection = ( TMath::Sqrt( fDiscriminant ) - b ) / ( a * 2.0f );

		if ( a_pFarIntersection )
			*a_pFarIntersection = ( -b - TMath::Sqrt( fDiscriminant ) ) / ( a * 2.0f );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006b7dc0
TFLOAT TMath::DistancePointToLineSq( const TVector4& a_rcPoint, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TVector4& a_rClosestPoint )
{
	TVector4 vecStartToPoint = a_rcPoint - a_rcLineStart;
	TVector4 vecStartToEnd   = a_rcLineEnd - a_rcLineStart;

	TFLOAT fLineLengthSq = vecStartToEnd.MagnitudeSq();
	TFLOAT fDot          = TVector4::DotProduct3( vecStartToPoint, vecStartToEnd );
	
	TVector4 vecProjection;
	vecProjection.Multiply( vecStartToEnd, fDot / fLineLengthSq );

	a_rClosestPoint = vecProjection + a_rcLineStart;

	TVector4 vecDiff = vecStartToPoint - vecProjection;
	return vecDiff.MagnitudeSq();
}

TOSHI_NAMESPACE_END
