#include "ToshiPCH.h"

#include "TMath.h"
#include "TVector3.h"
#include "TMathInline.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006b7670
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

	a_rClosestPoint.Set( vecProjection.x + a_rcLineStart.x, vecProjection.y + a_rcLineStart.y, vecProjection.z + a_rcLineStart.z, a_rcLineStart.w );

	TVector4 vecDiff = vecStartToPoint - vecProjection;
	return vecDiff.MagnitudeSq();
}

// $Barnyard: FUNCTION 006b7770
void TMath::TriangleNormalCounterClockwise( TVector3& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 )
{
	TVector3 vecVertex1To2 = a_rcVertex2 - a_rcVertex1;
	TVector3 vecVertex1To3 = a_rcVertex3 - a_rcVertex1;

	a_rNormal.CrossProduct( vecVertex1To2, vecVertex1To3 );
	a_rNormal.Normalize();
}

// $Barnyard: FUNCTION 006b77f0
void TMath::TriangleUnscaledNormalCounterClockwise( TVector4& a_rNormal, const TVector3* const* a_ppVertices )
{
	TVector4 vecEdge1( a_ppVertices[ 1 ]->x - a_ppVertices[ 0 ]->x, a_ppVertices[ 1 ]->y - a_ppVertices[ 0 ]->y, a_ppVertices[ 1 ]->z - a_ppVertices[ 0 ]->z, 1.0f );
	TVector4 vecEdge2( a_ppVertices[ 2 ]->x - a_ppVertices[ 0 ]->x, a_ppVertices[ 2 ]->y - a_ppVertices[ 0 ]->y, a_ppVertices[ 2 ]->z - a_ppVertices[ 0 ]->z, 1.0f );

	a_rNormal.CrossProduct( vecEdge1, vecEdge2 );
}

// $Barnyard: FUNCTION 006d8490
void TMath::TrianglePlane( TPlane& a_rOutPlane, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 )
{
	TriangleNormalCounterClockwise( a_rOutPlane.AsNormal(), a_rcVertex1, a_rcVertex2, a_rcVertex3 );
	a_rOutPlane.SetD( TVector3::DotProduct( a_rcVertex1, a_rOutPlane.GetNormal() ) );
}

// $Barnyard: FUNCTION 006b7860
TBOOL TMath::IsPointInTriangle( const TVector4& a_rcVertex1, const TVector4& a_rcVertex2, const TVector4& a_rcVertex3, const TVector4& a_rcPoint )
{
	// Barycentric coordinates of the point
	TVector4 vecEdge1 = a_rcVertex2 - a_rcVertex1;
	TVector4 vecEdge2 = a_rcVertex3 - a_rcVertex1;
	TVector4 vecPoint = a_rcPoint - a_rcVertex1;

	TFLOAT fDot11 = TVector4::DotProduct3( vecEdge1, vecEdge1 );
	TFLOAT fDot12 = TVector4::DotProduct3( vecEdge1, vecEdge2 );
	TFLOAT fDot22 = TVector4::DotProduct3( vecEdge2, vecEdge2 );
	TFLOAT fDotP1 = TVector4::DotProduct3( vecPoint, vecEdge1 );
	TFLOAT fDotP2 = TVector4::DotProduct3( vecPoint, vecEdge2 );

	TFLOAT fInvDenom = 1.0f / ( fDot12 * fDot12 - fDot22 * fDot11 );
	TFLOAT fU        = ( fDotP2 * fDot12 - fDotP1 * fDot22 ) * fInvDenom;

	if ( fU >= 0.0f && fU <= 1.0f )
	{
		TFLOAT fV = ( fDotP1 * fDot12 - fDotP2 * fDot11 ) * fInvDenom;

		if ( fV >= 0.0f && fU + fV <= 1.0f )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006b7b80
TBOOL TMath::IsPointInPolygon( TINT a_iNumVertices, const TVector3* const* a_ppVertices, const TVector4& a_rcPoint )
{
	TVector4 vecFirst( *a_ppVertices[ 0 ] );

	// Test every triangle of the fan
	for ( TINT i = 0; i < a_iNumVertices - 2; i++ )
	{
		TVector4 vecSecond( *a_ppVertices[ i + 1 ] );
		TVector4 vecThird( *a_ppVertices[ i + 2 ] );

		if ( IsPointInTriangle( vecFirst, vecSecond, vecThird, a_rcPoint ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006b7a40
TBOOL TMath::IsPointInTriangle2D( const TVector2& a_rcVertex1, const TVector2& a_rcVertex2, const TVector2& a_rcVertex3, const TVector2& a_rcPoint )
{
	TFLOAT fEdge1X = a_rcVertex2.x - a_rcVertex1.x;
	TFLOAT fEdge1Y = a_rcVertex2.y - a_rcVertex1.y;
	TFLOAT fEdge2X = a_rcVertex3.x - a_rcVertex1.x;
	TFLOAT fEdge2Y = a_rcVertex3.y - a_rcVertex1.y;
	TFLOAT fPointX = a_rcPoint.x - a_rcVertex1.x;
	TFLOAT fPointY = a_rcPoint.y - a_rcVertex1.y;

	TFLOAT fDot11 = fEdge1X * fEdge1X + fEdge1Y * fEdge1Y;
	TFLOAT fDot12 = fEdge2X * fEdge1X + fEdge1Y * fEdge2Y;
	TFLOAT fDot22 = fEdge2X * fEdge2X + fEdge2Y * fEdge2Y;
	TFLOAT fDot1P = fPointX * fEdge1X + fPointY * fEdge1Y;
	TFLOAT fDot2P = fPointX * fEdge2X + fPointY * fEdge2Y;

	TFLOAT fInvDenom = 1.0f / ( fDot12 * fDot12 - fDot22 * fDot11 );
	TFLOAT fU        = ( fDot2P * fDot12 - fDot1P * fDot22 ) * fInvDenom;

	// NOTE: written as negated comparisons because the original's x87 compares let NaN through
	// (a degenerate triangle gives NaN and counts as containing the point)
	if ( !( fU < 0.0f ) && !( fU > 1.0f ) )
	{
		TFLOAT fV = ( fDot1P * fDot12 - fDot2P * fDot11 ) * fInvDenom;

		if ( !( fV < 0.0f ) && !( fU + fV > 1.0f ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006b8bc0
TFLOAT TMath::DistancePointToSegmentSq( const TVector4& a_rcPoint, const TVector3& a_rcStart, const TVector3& a_rcEnd )
{
	TVector3 vecSegment = a_rcEnd - a_rcStart;

	TFLOAT fT = ( vecSegment.z * ( a_rcPoint.z - a_rcStart.z ) + vecSegment.x * ( a_rcPoint.x - a_rcStart.x ) + vecSegment.y * ( a_rcPoint.y - a_rcStart.y ) ) /
	    ( vecSegment.z * vecSegment.z + vecSegment.y * vecSegment.y + vecSegment.x * vecSegment.x );

	if ( fT > 1.0f )
		fT = 1.0f;
	else if ( fT < 0.0f )
		fT = 0.0f;

	TFLOAT fDX = vecSegment.x * fT + a_rcStart.x - a_rcPoint.x;
	TFLOAT fDY = vecSegment.y * fT + a_rcStart.y - a_rcPoint.y;
	TFLOAT fDZ = vecSegment.z * fT + a_rcStart.z - a_rcPoint.z;

	return fDZ * fDZ + fDY * fDY + fDX * fDX;
}

// $Barnyard: FUNCTION 006b8d00
TFLOAT TMath::DistancePointToSegmentSq( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcEnd, TFLOAT& a_rfT )
{
	TVector4 vecStartToPoint = a_rcPoint;
	vecStartToPoint.Substract( a_rcStart );

	TVector4 vecSegment = a_rcEnd;
	vecSegment.Substract( a_rcStart );

	a_rfT = TVector4::DotProduct3( vecStartToPoint, vecSegment ) / vecSegment.MagnitudeSq();

	if ( a_rfT > 1.0f )
		a_rfT = 1.0f;
	else if ( a_rfT < 0.0f )
		a_rfT = 0.0f;

	TVector4 vecClosest = vecSegment;
	vecClosest.Multiply( a_rfT );
	vecClosest.Add( a_rcStart );
	vecClosest.Substract( a_rcPoint );

	return vecClosest.MagnitudeSq();
}

// $Barnyard: FUNCTION 006b6470
TFLOAT TMath::ClosestPointOnLine2D( const TVector2& a_rcLineStart, const TVector2& a_rcLineEnd, const TVector2& a_rcPoint )
{
	TFLOAT fDirX = a_rcLineEnd.x - a_rcLineStart.x;
	TFLOAT fDirY = a_rcLineEnd.y - a_rcLineStart.y;

	return ( ( a_rcPoint.x - a_rcLineStart.x ) * fDirX + fDirY * ( a_rcPoint.y - a_rcLineStart.y ) ) / ( fDirX * fDirX + fDirY * fDirY );
}

// $Barnyard: FUNCTION 006b6b20
TBOOL TMath::CircleVsSegment2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius, const TVector2& a_rcCenter, TFLOAT* a_pOutPenetration )
{
	TFLOAT fT = ClosestPointOnLine2D( a_rcStart, a_rcEnd, a_rcCenter );

	if ( fT > 1.0f )
		fT = 1.0f;
	else if ( fT < 0.0f )
		fT = 0.0f;

	TVector2 vecClosest;
	vecClosest.Lerp( a_rcStart, a_rcEnd, fT );

	TFLOAT fDistanceSq = ( vecClosest.x - a_rcCenter.x ) * ( vecClosest.x - a_rcCenter.x ) + ( vecClosest.y - a_rcCenter.y ) * ( vecClosest.y - a_rcCenter.y );

	if ( a_fRadius * a_fRadius < fDistanceSq )
		return TFALSE;

	if ( a_pOutPenetration )
		*a_pOutPenetration = a_fRadius - sqrtf( fDistanceSq );

	return TTRUE;
}

// $Barnyard: FUNCTION 006b61a0
TBOOL TMath::LineVsLine2D( const TFLOAT* a_pLine1Start, const TFLOAT* a_pLine1End, const TFLOAT* a_pLine2Start, const TFLOAT* a_pLine2End, TFLOAT& a_rfT )
{
	TFLOAT fLine2X = a_pLine2End[ 0 ] - a_pLine2Start[ 0 ];
	TFLOAT fLine2Y = a_pLine2End[ 1 ] - a_pLine2Start[ 1 ];
	TFLOAT fDenom  = ( a_pLine1End[ 0 ] - a_pLine1Start[ 0 ] ) * fLine2Y - ( a_pLine1End[ 1 ] - a_pLine1Start[ 1 ] ) * fLine2X;

	if ( fDenom == 0.0f )
		return TFALSE;

	a_rfT = ( ( a_pLine1Start[ 1 ] - a_pLine2Start[ 1 ] ) * fLine2X - ( a_pLine1Start[ 0 ] - a_pLine2Start[ 0 ] ) * fLine2Y ) / fDenom;
	return TTRUE;
}

// $Barnyard: FUNCTION 006b6250
TBOOL TMath::LineVsLine2D( const TVector2& a_rcLine1Start, const TVector2& a_rcLine1End, const TVector2& a_rcLine2Start, const TVector2& a_rcLine2End, TFLOAT& a_rfT1, TFLOAT& a_rfT2 )
{
	TFLOAT fLine1X = a_rcLine1End.x - a_rcLine1Start.x;
	TFLOAT fLine1Y = a_rcLine1End.y - a_rcLine1Start.y;
	TFLOAT fLine2X = a_rcLine2End.x - a_rcLine2Start.x;
	TFLOAT fLine2Y = a_rcLine2End.y - a_rcLine2Start.y;
	TFLOAT fDenom  = fLine1X * fLine2Y - fLine1Y * fLine2X;

	if ( fDenom == 0.0f )
		return TFALSE;

	TFLOAT fDiffX    = a_rcLine1Start.x - a_rcLine2Start.x;
	TFLOAT fDiffY    = a_rcLine1Start.y - a_rcLine2Start.y;
	TFLOAT fInvDenom = 1.0f / fDenom;

	a_rfT1 = ( fDiffY * fLine2X - fDiffX * fLine2Y ) * fInvDenom;
	a_rfT2 = ( fDiffY * fLine1X - fDiffX * fLine1Y ) * fInvDenom;
	return TTRUE;
}

// $Barnyard: FUNCTION 006b6330
TBOOL TMath::SegmentVsSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2, TVector2& a_rOutPoint )
{
	TFLOAT fT1;
	TFLOAT fT2;

	if ( LineVsLine2D( a_rcStart1, a_rcEnd1, a_rcStart2, a_rcEnd2, fT1, fT2 ) != TTRUE ||
	     fT1 < 0.0f || fT2 < 0.0f || fT1 > 1.0f || fT2 > 1.0f )
	{
		return TFALSE;
	}

	a_rOutPoint = a_rcEnd1;
	a_rOutPoint.x -= a_rcStart1.x;
	a_rOutPoint.y -= a_rcStart1.y;
	a_rOutPoint.Multiply( fT1 );
	a_rOutPoint.x += a_rcStart1.x;
	a_rOutPoint.y += a_rcStart1.y;
	return TTRUE;
}

// $Barnyard: FUNCTION 006b63f0
TBOOL TMath::SegmentVsSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 )
{
	TFLOAT fT1;
	TFLOAT fT2;

	return LineVsLine2D( a_rcStart1, a_rcEnd1, a_rcStart2, a_rcEnd2, fT1, fT2 ) &&
	    fT1 >= 0.0f && fT2 >= 0.0f && fT1 <= 1.0f && fT2 <= 1.0f;
}

// $Barnyard: FUNCTION 006b8e50
TFLOAT TMath::DistanceAlongSegment( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcEnd )
{
	TVector4 vecToPoint;
	vecToPoint.Substract( a_rcPoint, a_rcStart );

	TVector4 vecSegment;
	vecSegment.Substract( a_rcEnd, a_rcStart );

	TFLOAT fLength   = vecSegment.Magnitude();
	TFLOAT fDistance = TVector4::DotProduct3( vecToPoint, vecSegment ) / fLength;

	if ( fDistance > fLength )
		return fLength;

	return ( fDistance >= 0.0f ) ? fDistance : 0.0f;
}

// $Barnyard: FUNCTION 006b6ca0
TFLOAT TMath::DistanceSegmentToSegmentSq( const TVector3& a_rcStart1, const TVector3& a_rcEnd1, const TVector3& a_rcStart2, const TVector3& a_rcEnd2 )
{
	TVector3 vecDir1( a_rcEnd1.x - a_rcStart1.x, a_rcEnd1.y - a_rcStart1.y, a_rcEnd1.z - a_rcStart1.z );
	TVector3 vecDir2( a_rcEnd2.x - a_rcStart2.x, a_rcEnd2.y - a_rcStart2.y, a_rcEnd2.z - a_rcStart2.z );
	TVector3 vecDiff( a_rcStart1.x - a_rcStart2.x, a_rcStart1.y - a_rcStart2.y, a_rcStart1.z - a_rcStart2.z );

	TFLOAT fA = vecDir1.x * vecDir1.x + vecDir1.z * vecDir1.z + vecDir1.y * vecDir1.y;
	TFLOAT fB = vecDir2.x * vecDir1.x + vecDir2.z * vecDir1.z + vecDir2.y * vecDir1.y;
	TFLOAT fC = vecDir2.y * vecDir2.y + vecDir2.z * vecDir2.z + vecDir2.x * vecDir2.x;
	TFLOAT fD = vecDiff.x * vecDir1.x + vecDiff.z * vecDir1.z + vecDiff.y * vecDir1.y;
	TFLOAT fE = vecDiff.y * vecDir2.y + vecDiff.z * vecDir2.z + vecDiff.x * vecDir2.x;

	TFLOAT fDenom  = fC * fA - fB * fB;
	TFLOAT fSDenom = fDenom;
	TFLOAT fTDenom = fDenom;
	TFLOAT fSNumer;
	TFLOAT fTNumer;

	if ( fDenom < 1e-9f )
	{
		// The segments are parallel
		fSNumer = 0.0f;
		fSDenom = 1.0f;
		fTNumer = fE;
		fTDenom = fC;
	}
	else
	{
		fSNumer = fE * fB - fD * fC;
		fTNumer = fE * fA - fD * fB;

		if ( fSNumer < 0.0f )
		{
			fSNumer = 0.0f;
			fTNumer = fE;
			fTDenom = fC;
		}
		else if ( fSNumer > fDenom )
		{
			fSNumer = fDenom;
			fTNumer = fE + fB;
			fTDenom = fC;
		}
	}

	if ( fTNumer < 0.0f )
	{
		fTNumer = 0.0f;
		fSNumer = -fD;

		if ( fSNumer < 0.0f )
			fSNumer = 0.0f;
		else if ( fSNumer > fA )
			fSNumer = fSDenom;
		else
			fSDenom = fA;
	}
	else if ( fTNumer > fTDenom )
	{
		fTNumer = fTDenom;
		fSNumer = fB - fD;

		if ( fSNumer < 0.0f )
			fSNumer = 0.0f;
		else if ( fSNumer > fA )
			fSNumer = fSDenom;
		else
			fSDenom = fA;
	}

	TFLOAT fS = ( TMath::Abs( fSNumer ) >= 1e-9f ) ? fSNumer / fSDenom : 0.0f;
	TFLOAT fT = ( TMath::Abs( fTNumer ) >= 1e-9f ) ? fTNumer / fTDenom : 0.0f;

	TFLOAT fX = ( vecDir1.x * fS + vecDiff.x ) - vecDir2.x * fT;
	TFLOAT fY = ( vecDir1.y * fS + vecDiff.y ) - vecDir2.y * fT;
	TFLOAT fZ = ( vecDiff.z + vecDir1.z * fS ) - vecDir2.z * fT;

	return fZ * fZ + fY * fY + fX * fX;
}

// $Barnyard: FUNCTION 006b9170
TFLOAT TMath::DistanceSegmentToSegmentSq2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 )
{
	return DistanceSegmentToSegmentSq(
	    TVector3( a_rcStart1.x, a_rcStart1.y, 0.0f ),
	    TVector3( a_rcEnd1.x, a_rcEnd1.y, 0.0f ),
	    TVector3( a_rcStart2.x, a_rcStart2.y, 0.0f ),
	    TVector3( a_rcEnd2.x, a_rcEnd2.y, 0.0f )
	);
}

// $Barnyard: FUNCTION 006b9200
TFLOAT TMath::DistanceSegmentToSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 )
{
	return TMath::Sqrt( DistanceSegmentToSegmentSq(
	    TVector3( a_rcStart1.x, a_rcStart1.y, 0.0f ),
	    TVector3( a_rcEnd1.x, a_rcEnd1.y, 0.0f ),
	    TVector3( a_rcStart2.x, a_rcStart2.y, 0.0f ),
	    TVector3( a_rcEnd2.x, a_rcEnd2.y, 0.0f )
	) );
}

// $Barnyard: FUNCTION 006b9290
TBOOL TMath::IsSegmentNearSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2, TFLOAT a_fDistance )
{
	return a_fDistance * a_fDistance >= DistanceSegmentToSegmentSq2D( a_rcStart1, a_rcEnd1, a_rcStart2, a_rcEnd2 );
}

// $Barnyard: FUNCTION 006b6be0
TBOOL TMath::CircleVsCapsule2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fCapsuleRadius, const TVector2& a_rcCenter, TFLOAT a_fRadius, TFLOAT* a_pOutPenetration )
{
	TFLOAT fT = ClosestPointOnLine2D( a_rcStart, a_rcEnd, a_rcCenter );

	if ( fT > 1.0f )
		fT = 1.0f;
	else if ( fT < 0.0f )
		fT = 0.0f;

	TVector2 vecClosest;
	vecClosest.Lerp( a_rcStart, a_rcEnd, fT );

	TFLOAT fDistanceSq = ( vecClosest.x - a_rcCenter.x ) * ( vecClosest.x - a_rcCenter.x ) + ( vecClosest.y - a_rcCenter.y ) * ( vecClosest.y - a_rcCenter.y );
	TFLOAT fRadius     = a_fCapsuleRadius + a_fRadius;

	if ( fRadius * fRadius < fDistanceSq )
		return TFALSE;

	if ( a_pOutPenetration )
		*a_pOutPenetration = fRadius - sqrtf( fDistanceSq );

	return TTRUE;
}

// $Barnyard: FUNCTION 006d84d0
TBOOL TMath::SphereVsSphere( const TVector4& a_rcSphere1, const TVector4& a_rcSphere2 )
{
	TFLOAT fDX     = a_rcSphere1.x - a_rcSphere2.x;
	TFLOAT fDY     = a_rcSphere1.y - a_rcSphere2.y;
	TFLOAT fDZ     = a_rcSphere1.z - a_rcSphere2.z;
	TFLOAT fRadius = a_rcSphere1.w + a_rcSphere2.w;

	return fRadius * fRadius > fDX * fDX + fDZ * fDZ + fDY * fDY;
}

// $Barnyard: FUNCTION 006b5d30
TBOOL TMath::SphereVsSphere( const TVector4& a_rcCenter1, TFLOAT a_fRadius1, const TVector4& a_rcCenter2, TFLOAT a_fRadius2 )
{
	TFLOAT fRadius = a_fRadius1 + a_fRadius2;
	return fRadius * fRadius >= TVector4::DistanceSq( a_rcCenter1, a_rcCenter2 );
}

// $Barnyard: FUNCTION 006b89c0
TBOOL TMath::SweptSphereVsSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4& a_rcOtherCenter, TFLOAT a_fOtherRadius, TFLOAT& a_rfDistance, TVector4& a_rOutNormal, TFLOAT a_fEpsilon )
{
	TFLOAT fMaxDistance = a_rfDistance;

	TVector4 vecMove;
	vecMove.Multiply( a_rcDirection, fMaxDistance );

	TVector4 vecEnd( a_rcCenter.x + vecMove.x, a_rcCenter.y + vecMove.y, a_rcCenter.z + vecMove.z, a_rcCenter.w );

	TFLOAT fT;
	TFLOAT fDistanceSq = DistancePointToLineSq( a_rcOtherCenter, a_rcCenter, vecEnd, fT );

	if ( fT < 0.0f )
	{
		// Moving away from the other sphere
		a_rfDistance = fMaxDistance;
		return TFALSE;
	}

	TFLOAT fRadiusSq = ( a_fRadius + a_fOtherRadius ) * ( a_fRadius + a_fOtherRadius );

	// NOTE: written as !( < ) so that a NaN distance (a_rfDistance of 0, the line has no length) returns
	// TFALSE like the x87 compare of the original does
	if ( !( fDistanceSq - fRadiusSq < 0.0f ) )
		return TFALSE;

	TFLOAT fDistance = fT * fMaxDistance - ( TMath::Sqrt( fRadiusSq - fDistanceSq ) + a_fEpsilon );
	a_rfDistance     = fDistance;

	if ( fDistance < 0.01f )
	{
		a_rfDistance = 0.0f;
		return TTRUE;
	}

	if ( fDistance > fMaxDistance || -a_fRadius > fDistance )
	{
		a_rfDistance = fMaxDistance;
		return TFALSE;
	}

	TVector4 vecContactMove;
	vecContactMove.Multiply( a_rcDirection, fDistance );

	TVector4 vecContact( a_rcCenter.x + vecContactMove.x, a_rcCenter.y + vecContactMove.y, a_rcCenter.z + vecContactMove.z, a_rcCenter.w );

	a_rOutNormal.Set( vecContact.x - a_rcOtherCenter.x, vecContact.y - a_rcOtherCenter.y, vecContact.z - a_rcOtherCenter.z, vecContact.w );
	a_rOutNormal.Normalise();

	return TTRUE;
}

// $Barnyard: FUNCTION 006b5f30
TINT TMath::RayVsSphere( const TVector3& a_rcOrigin, const TVector3& a_rcDirection, const TVector3& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNear, TFLOAT& a_rfFar )
{
	TVector3 vecToCenter( a_rcCenter.x - a_rcOrigin.x, a_rcCenter.y - a_rcOrigin.y, a_rcCenter.z - a_rcOrigin.z );

	TFLOAT fRadiusSq   = a_fRadius * a_fRadius;
	TFLOAT fDistanceSq = vecToCenter.x * vecToCenter.x + vecToCenter.z * vecToCenter.z + vecToCenter.y * vecToCenter.y;

	if ( fDistanceSq >= fRadiusSq )
	{
		// Outside: the sphere must be in front of the ray and close enough to it
		TFLOAT fProjection = vecToCenter.z * a_rcDirection.z + vecToCenter.y * a_rcDirection.y + vecToCenter.x * a_rcDirection.x;

		if ( fProjection < 0.0f )
			return 0;

		if ( fDistanceSq - fProjection * fProjection > fRadiusSq )
			return 0;
	}

	TFLOAT fB = ( vecToCenter.z * -1.0f * a_rcDirection.z + vecToCenter.y * -1.0f * a_rcDirection.y + vecToCenter.x * -1.0f * a_rcDirection.x );
	fB        = fB + fB;

	TFLOAT fDiscriminant = fB * fB - ( fDistanceSq - fRadiusSq ) * 4.0f;

	if ( fDiscriminant < 0.0f )
		return 0;

	TFLOAT fRoot = TMath::Sqrt( fDiscriminant );
	a_rfNear     = ( -fB - fRoot ) * 0.5f;
	a_rfFar      = ( fRoot - fB ) * 0.5f;

	return 2;
}

// $Barnyard: FUNCTION 006b5d70
TBOOL TMath::SweptSphereVsPlane( const TVector4& a_rcCenter, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4 a_vPlanePoint, TVector4 a_vPlaneNormal )
{
	TVector4 vecToCenter( a_rcCenter.x - a_vPlanePoint.x, a_rcCenter.y - a_vPlanePoint.y, a_rcCenter.z - a_vPlanePoint.z, a_rcCenter.w );

	// Make the normal face the sphere
	if ( TVector4::DotProduct3( a_vPlaneNormal, vecToCenter ) < 0.0f )
	{
		a_vPlaneNormal.x = -a_vPlaneNormal.x;
		a_vPlaneNormal.y = -a_vPlaneNormal.y;
		a_vPlaneNormal.z = -a_vPlaneNormal.z;
	}

	TFLOAT fDot = TVector4::DotProduct3( a_rcDirection, a_vPlaneNormal );

	if ( fDot == 0.0f )
		return TFALSE;

	// The plane moved towards the sphere by its radius, relative to the centre
	TVector4 vecOffset;
	vecOffset.Multiply( a_vPlaneNormal, a_fRadius );

	a_vPlanePoint.x = vecOffset.x + a_vPlanePoint.x - a_rcCenter.x;
	a_vPlanePoint.y = vecOffset.y + a_vPlanePoint.y - a_rcCenter.y;
	a_vPlanePoint.z = vecOffset.z + a_vPlanePoint.z - a_rcCenter.z;

	TFLOAT fDistance = TVector4::DotProduct3( a_vPlaneNormal, a_vPlanePoint ) / fDot;

	if ( fDistance < 0.0f || fDistance > a_rfDistance )
		return TFALSE;

	a_rfDistance = ( fDistance <= 0.1f ) ? 0.0f : fDistance - 0.1f;
	return TTRUE;
}

// $Barnyard: FUNCTION 006b92d0
TBOOL TMath::IsPointInCapsule( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength )
{
	TFLOAT fDistanceSq;

	if ( a_fLength > 0.0f )
	{
		TVector4 vecEnd;
		vecEnd.Multiply( a_rcDirection, a_fLength );
		vecEnd.Add( a_rcStart );
		vecEnd.w = a_rcStart.w;

		fDistanceSq = DistancePointToSegmentSq( a_rcPoint, a_rcStart, vecEnd, a_fLength );
	}
	else
	{
		fDistanceSq = TVector4::DistanceSq( a_rcPoint, a_rcStart );
	}

	return a_fRadius * a_fRadius >= fDistanceSq;
}

// $Barnyard: FUNCTION 006ba210
TBOOL TMath::SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector3* const* a_ppVertices, const TPlane& a_rcPlane, TINT a_iNumVertices, TFLOAT a_fEpsilon )
{
	const TVector3& rcNormal = a_rcPlane.GetNormal();

	TFLOAT fPlaneDist    = rcNormal.x * a_rcCenter.x + rcNormal.z * a_rcCenter.z + rcNormal.y * a_rcCenter.y - a_rcPlane.GetD();
	TFLOAT fAbsPlaneDist = TMath::Abs( fPlaneDist );

	if ( a_fRadius > a_rfDistance )
		a_fRadius = a_rfDistance;

	if ( fAbsPlaneDist >= a_fRadius - a_fEpsilon )
		return TFALSE;

	// NOTE: the point is moved along the normal (not projected back on the plane)
	TVector4 vecPoint( fPlaneDist * rcNormal.x + a_rcCenter.x, fPlaneDist * rcNormal.y + a_rcCenter.y, fPlaneDist * rcNormal.z + a_rcCenter.z, 1.0f );

	if ( IsPointInPolygon( a_iNumVertices, a_ppVertices, vecPoint ) )
	{
		a_rfDistance = fAbsPlaneDist;
		a_rNormal.Set( rcNormal.x, rcNormal.y, rcNormal.z, 1.0f );
		return TTRUE;
	}

	// Find the closest edge
	TINT   iClosestEdge = -1;
	TFLOAT fClosestSq   = a_fRadius * a_fRadius;

	for ( TINT i = a_iNumVertices - 1, k = a_iNumVertices; i >= 0; i--, k-- )
	{
		TFLOAT fDistanceSq = DistancePointToSegmentSq( a_rcCenter, *a_ppVertices[ i ], *a_ppVertices[ k % a_iNumVertices ] );

		if ( fClosestSq - a_fEpsilon > fDistanceSq )
		{
			fClosestSq   = fDistanceSq;
			iClosestEdge = i;
		}
	}

	if ( iClosestEdge == -1 )
		return TFALSE;

	a_rfDistance = sqrtf( fClosestSq );

	// NOTE: the normal points from the first vertex of the edge, not from the closest point
	const TVector3* pVertex = a_ppVertices[ iClosestEdge ];
	a_rNormal.Set( a_rcCenter.x - pVertex->x, a_rcCenter.y - pVertex->y, a_rcCenter.z - pVertex->z, 1.0f );
	a_rNormal.Normalise();

	return TTRUE;
}

// $Barnyard: FUNCTION 006b7f30
TINT TMath::SweptSphereVsTriangle( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance, const TPlane& a_rcPlane, TVector4& a_rNormal )
{
	constexpr TFLOAT EPSILON     = 0.001f;
	static TFLOAT    s_fPushBack = EPSILON * 4.0f;

	const TVector3& rcPlaneNormal = a_rcPlane.GetNormal();
	TVector4        vecDirection  = a_rcDirection;

	TFLOAT fDirDot = vecDirection.z * rcPlaneNormal.z + vecDirection.x * rcPlaneNormal.x + vecDirection.y * rcPlaneNormal.y;

	// Moving away from the plane
	if ( fDirDot >= 0.1f )
		return 0;

	TFLOAT fPlaneDist = a_rcCenter.y * rcPlaneNormal.y + a_rcCenter.x * rcPlaneNormal.x + a_rcCenter.z * rcPlaneNormal.z - a_rcPlane.GetD();
	TFLOAT fNegRadius = -a_fRadius;

	// Behind the plane
	if ( fPlaneDist < fNegRadius )
		return 0;

	TFLOAT   fOriginalDistance = a_rfDistance;
	TVector4 vecOriginalNormal = a_rNormal;
	a_rfDistance               = EPSILON + a_rfDistance;

	TINT iResult = 0;

	// Face
	if ( fDirDot < 0.0f && a_fRadius - EPSILON < fPlaneDist )
	{
		TFLOAT fGap = fPlaneDist - a_fRadius;
		if ( fGap < 0.0f ) fGap = 0.0f;

		TFLOAT fT = -( fGap / fDirDot );

		TVector4 vecPoint;
		vecPoint.Multiply( vecDirection, fT );
		vecPoint = a_rcCenter + vecPoint;

		if ( fT < a_rfDistance && IsPointInTriangle( *a_ppVertices[ 0 ], *a_ppVertices[ 1 ], *a_ppVertices[ 2 ], vecPoint ) )
		{
			a_rfDistance = fT;
			a_rNormal.Set( rcPlaneNormal.x, rcPlaneNormal.y, rcPlaneNormal.z, 1.0f );
			iResult = 1;
		}
	}

	// Vertices
	for ( TINT i = 0; i < 3; i++ )
	{
		TVector4 vecVertex = *a_ppVertices[ i ];
		TVector4 vecVertexEnd( vecVertex.x - vecDirection.x, vecVertex.y - vecDirection.y, vecVertex.z - vecDirection.z, vecVertex.w );

		TINT   iNumHits;
		TFLOAT fHit1 = TMath::MAXFLOAT;
		TFLOAT fHit2 = TMath::MAXFLOAT;

		if ( !SphereVsLine( a_rcCenter, a_fRadius, vecVertex, vecVertexEnd, &iNumHits, &fHit1, &fHit2 ) )
			continue;

		TFLOAT fT;

		if ( fHit2 < -EPSILON || fHit2 >= a_rfDistance )
			fT = fHit1;
		else if ( fHit1 < -EPSILON || fHit1 >= a_rfDistance )
			fT = fHit2;
		else
			fT = ( fHit2 < fHit1 ) ? fHit2 : fHit1;

		if ( fT >= -EPSILON && fT < a_rfDistance )
		{
			if ( fT < 0.0f ) fT = 0.0f;

			TVector4 vecOffset;
			vecOffset.Multiply( vecDirection, fT );

			TVector4 vecHitPoint( vecVertex.x - vecOffset.x, vecVertex.y - vecOffset.y, vecVertex.z - vecOffset.z, vecVertex.w );
			a_rNormal.Set( a_rcCenter.x - vecHitPoint.x, a_rcCenter.y - vecHitPoint.y, a_rcCenter.z - vecHitPoint.z, a_rcCenter.w );

			iResult += 2;
			a_rfDistance = fT;
		}
	}

	// Edges
	for ( TINT i = 0; i < 3; i++ )
	{
		TVector4 vecStart = *a_ppVertices[ i ];
		TVector4 vecEnd   = *a_ppVertices[ ( i + 1 == 3 ) ? 0 : i + 1 ];
		TVector3 vecEndBack( vecEnd.x - vecDirection.x, vecEnd.y - vecDirection.y, vecEnd.z - vecDirection.z );

		// Plane containing the edge and the movement direction
		TPlane oEdgePlane;
		TrianglePlane( oEdgePlane, vecStart.AsVector3(), vecEnd.AsVector3(), vecEndBack );

		const TVector3& rcEdgeNormal = oEdgePlane.GetNormal();
		TFLOAT          fEdgeDist    = rcEdgeNormal.y * a_rcCenter.y + rcEdgeNormal.z * a_rcCenter.z + rcEdgeNormal.x * a_rcCenter.x - oEdgePlane.GetD();

		if ( fEdgeDist > a_fRadius || fEdgeDist < fNegRadius )
			continue;

		// Circle where the sphere cuts the plane
		TVector4 vecCircleCenter( a_rcCenter.x - rcEdgeNormal.x * fEdgeDist, a_rcCenter.y - rcEdgeNormal.y * fEdgeDist, a_rcCenter.z - rcEdgeNormal.z * fEdgeDist, 1.0f );

		TVector4 vecClosest;
		DistancePointToLineSq( vecCircleCenter, vecStart, vecEnd, vecClosest );

		TVector4 vecToEdge( vecClosest.x - vecCircleCenter.x, vecClosest.y - vecCircleCenter.y, vecClosest.z - vecCircleCenter.z, vecClosest.w );
		vecToEdge.Normalise();

		TVector4 vecTouch;
		vecTouch.Multiply( vecToEdge, sqrtf( a_fRadius * a_fRadius - fEdgeDist * fEdgeDist ) );
		vecTouch.x += vecCircleCenter.x;
		vecTouch.y += vecCircleCenter.y;
		vecTouch.z += vecCircleCenter.z;

		// Drop the dominant axis of the plane normal and intersect the lines in 2D
		TFLOAT fAbsX = TMath::Abs( rcEdgeNormal.x );
		TFLOAT fAbsY = TMath::Abs( rcEdgeNormal.y );
		TFLOAT fAbsZ = TMath::Abs( rcEdgeNormal.z );

		TINT iAxis1 = 0;
		TINT iAxis2 = 1;

		if ( fAbsX > fAbsY && fAbsX > fAbsZ )
		{
			iAxis1 = 1;
			iAxis2 = 2;
		}
		else if ( fAbsY > fAbsZ )
		{
			iAxis1 = 0;
			iAxis2 = 2;
		}

		TVector4 vecTouchEnd( vecTouch.x + vecDirection.x, vecTouch.y + vecDirection.y, vecTouch.z + vecDirection.z, vecTouch.w );

		const TFLOAT* pTouch    = &vecTouch.x;
		const TFLOAT* pTouchEnd = &vecTouchEnd.x;
		const TFLOAT* pStart    = &vecStart.x;
		const TFLOAT* pEnd      = &vecEnd.x;

		TFLOAT afLine1Start[ 2 ] = { pTouch[ iAxis1 ], pTouch[ iAxis2 ] };
		TFLOAT afLine1End[ 2 ]   = { pTouchEnd[ iAxis1 ], pTouchEnd[ iAxis2 ] };
		TFLOAT afLine2Start[ 2 ] = { pStart[ iAxis1 ], pStart[ iAxis2 ] };
		TFLOAT afLine2End[ 2 ]   = { pEnd[ iAxis1 ], pEnd[ iAxis2 ] };

		TFLOAT fT;
		if ( LineVsLine2D( afLine1Start, afLine1End, afLine2Start, afLine2End, fT ) && fT >= 0.0f && fT < a_rfDistance )
		{
			TVector4 vecHit;
			vecHit.Multiply( vecDirection, fT );
			vecHit = vecTouch + vecHit;

			TVector4 vecToStart = vecStart - vecHit;
			TVector4 vecToEnd   = vecEnd - vecHit;

			// The hit point should be between the edge vertices
			if ( TVector4::DotProduct3( vecToStart, vecToEnd ) <= 0.0f )
			{
				a_rfDistance = fT;
				a_rNormal.Set( a_rcCenter.x - vecTouch.x, a_rcCenter.y - vecTouch.y, a_rcCenter.z - vecTouch.z, a_rcCenter.w );
				iResult += 16;
			}
		}
	}

	a_rfDistance = a_rfDistance - EPSILON;

	if ( iResult != 0 )
	{
		a_rNormal.Normalise();
		TFLOAT fNormalDot = TVector4::DotProduct3( a_rNormal, vecDirection );

		if ( a_rfDistance < 0.0f && fNormalDot >= -0.0001f )
		{
			// Already touching and not moving into it
			a_rfDistance = fOriginalDistance;
			a_rNormal    = vecOriginalNormal;
			iResult      = 0;
		}

		if ( fNormalDot <= -0.0001f )
			a_rfDistance = a_rfDistance - s_fPushBack;

		if ( a_rfDistance < 0.0f )
			a_rfDistance = 0.0f;
	}

	return iResult;
}

// $Barnyard: FUNCTION 006b93b0
TINT TMath::SweptSphereVsTriangle( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector3* const* a_ppVertices, TFLOAT& a_rfDistance, const TPlane& a_rcPlane, TVector4& a_rNormal )
{
	static TVector4 s_aVertices[ 3 ];

	s_aVertices[ 0 ] = TVector4( *a_ppVertices[ 0 ] );
	s_aVertices[ 1 ] = TVector4( *a_ppVertices[ 1 ] );
	s_aVertices[ 2 ] = TVector4( *a_ppVertices[ 2 ] );

	const TVector4* apVertices[ 3 ] = { &s_aVertices[ 0 ], &s_aVertices[ 1 ], &s_aVertices[ 2 ] };
	return SweptSphereVsTriangle( a_rcCenter, a_fRadius, a_rcDirection, apVertices, a_rfDistance, a_rcPlane, a_rNormal );
}

// $Barnyard: FUNCTION 006ba3c0
void TMath::SlideAlongPlane( const TVector4& a_rcDirection, const TVector4& a_rcNormal, TFLOAT a_fSpeedScale, TFLOAT a_fTotalDistance, TFLOAT a_fTravelledDistance, TFLOAT& a_rfOutDistance, TVector4& a_rOutDirection, TFLOAT a_fPushOut, TFLOAT a_fFriction, TFLOAT a_fMaxDot )
{
	TFLOAT fRemaining = a_fTotalDistance - a_fTravelledDistance;
	TFLOAT fDot       = -TVector4::DotProduct3( a_rcNormal, a_rcDirection );

	if ( fDot <= 0.0f )
		fDot = 0.0f;

	if ( fDot < a_fMaxDot )
	{
		// Push the direction off the plane
		TVector4 vecPush;
		vecPush.Multiply( a_rcNormal, fDot + a_fPushOut );

		a_rOutDirection.Set( vecPush.x + a_rcDirection.x, vecPush.y + a_rcDirection.y, vecPush.z + a_rcDirection.z, a_rcDirection.w );
		a_rOutDirection.Normalise();

		TFLOAT fFactor = fDot + a_fFriction;

		if ( fFactor > 1.0f )
			fFactor = 1.0f;
		else if ( fFactor < 0.0f )
			fFactor = 0.0f;

		TFLOAT fDistance = ( 1.0f - fFactor * fFactor ) * fRemaining * a_fSpeedScale;
		a_rfOutDistance  = ( fDistance > 0.0f ) ? fDistance : 0.0f;
	}
	else
	{
		a_rfOutDistance = 0.0f;
	}
}

// $Barnyard: FUNCTION 006b64e0
TFLOAT TMath::DistancePointToLineSq( const TVector4& a_rcPoint, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TFLOAT& a_rfT )
{
	TFLOAT fLineX = a_rcLineEnd.x - a_rcLineStart.x;
	TFLOAT fLineY = a_rcLineEnd.y - a_rcLineStart.y;
	TFLOAT fLineZ = a_rcLineEnd.z - a_rcLineStart.z;

	a_rfT = ( fLineZ * ( a_rcPoint.z - a_rcLineStart.z ) + fLineX * ( a_rcPoint.x - a_rcLineStart.x ) + fLineY * ( a_rcPoint.y - a_rcLineStart.y ) ) /
	    ( fLineZ * fLineZ + fLineY * fLineY + fLineX * fLineX );

	TFLOAT fDX = fLineX * a_rfT + a_rcLineStart.x - a_rcPoint.x;
	TFLOAT fDY = fLineY * a_rfT + a_rcLineStart.y - a_rcPoint.y;
	TFLOAT fDZ = fLineZ * a_rfT + a_rcLineStart.z - a_rcPoint.z;

	return fDZ * fDZ + fDY * fDY + fDX * fDX;
}

// $Barnyard: FUNCTION 006b7000
TFLOAT TMath::DistanceSegmentToSegmentSq( const TVector4& a_rcStart1, const TVector4& a_rcEnd1, const TVector4& a_rcStart2, const TVector4& a_rcEnd2, TFLOAT& a_rfT1, TFLOAT& a_rfT2 )
{
	TVector3 vecDir1( a_rcEnd1.x - a_rcStart1.x, a_rcEnd1.y - a_rcStart1.y, a_rcEnd1.z - a_rcStart1.z );
	TVector3 vecDir2( a_rcEnd2.x - a_rcStart2.x, a_rcEnd2.y - a_rcStart2.y, a_rcEnd2.z - a_rcStart2.z );
	TVector3 vecDiff( a_rcStart1.x - a_rcStart2.x, a_rcStart1.y - a_rcStart2.y, a_rcStart1.z - a_rcStart2.z );

	TFLOAT fA = vecDir1.x * vecDir1.x + vecDir1.z * vecDir1.z + vecDir1.y * vecDir1.y;
	TFLOAT fB = vecDir2.x * vecDir1.x + vecDir2.z * vecDir1.z + vecDir2.y * vecDir1.y;
	TFLOAT fE = vecDir2.y * vecDir2.y + vecDir2.z * vecDir2.z + vecDir2.x * vecDir2.x;
	TFLOAT fC = vecDiff.x * vecDir1.x + vecDiff.z * vecDir1.z + vecDiff.y * vecDir1.y;
	TFLOAT fF = vecDiff.y * vecDir2.y + vecDiff.z * vecDir2.z + vecDiff.x * vecDir2.x;

	TFLOAT fAE    = fE * fA;
	TFLOAT fDenom = fAE - fB * fB;

	TFLOAT fParallel = fDenom;
	if ( TMath::Abs( fAE ) > 0.01f )
		fParallel = fDenom / fAE;

	TFLOAT fNum1, fDenom1;
	TFLOAT fNum2, fDenom2;

	if ( fParallel < 0.00001f )
	{
		// The segments are parallel
		fNum1   = 0.0f;
		fDenom1 = 1.0f;
		fNum2   = fF;
		fDenom2 = fE;
	}
	else
	{
		fNum1   = fF * fB - fC * fE;
		fNum2   = fF * fA - fC * fB;
		fDenom1 = fDenom;
		fDenom2 = fDenom;

		if ( fNum1 < 0.0f )
		{
			fNum1   = 0.0f;
			fNum2   = fF;
			fDenom2 = fE;
		}
		else if ( fNum1 > fDenom1 )
		{
			fNum1   = fDenom1;
			fNum2   = fF + fB;
			fDenom2 = fE;
		}
	}

	if ( fNum2 < 0.0f )
	{
		fNum2 = 0.0f;

		if ( -fC < 0.0f )
		{
			fNum1 = 0.0f;
		}
		else if ( -fC > fA )
		{
			fNum1 = fDenom1;
		}
		else
		{
			fNum1   = -fC;
			fDenom1 = fA;
		}
	}
	else if ( fNum2 > fDenom2 )
	{
		fNum2 = fDenom2;

		if ( fB - fC < 0.0f )
		{
			fNum1 = 0.0f;
		}
		else if ( fB - fC > fA )
		{
			fNum1 = fDenom1;
		}
		else
		{
			fNum1   = fB - fC;
			fDenom1 = fA;
		}
	}

	a_rfT1 = ( TMath::Abs( fNum1 ) < 0.00001f ) ? 0.0f : fNum1 / fDenom1;
	a_rfT2 = ( TMath::Abs( fNum2 ) < 0.00001f ) ? 0.0f : fNum2 / fDenom2;

	TFLOAT fDX = vecDir1.x * a_rfT1 + vecDiff.x - vecDir2.x * a_rfT2;
	TFLOAT fDY = vecDir1.y * a_rfT1 + vecDiff.y - vecDir2.y * a_rfT2;
	TFLOAT fDZ = vecDir1.z * a_rfT1 + vecDiff.z - vecDir2.z * a_rfT2;

	return fDZ * fDZ + fDY * fDY + fDX * fDX;
}

// $Barnyard: FUNCTION 006b9c30
TBOOL TMath::RayVsCapsuleOutside( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon )
{
	TFLOAT fMaxDistance = a_rfDistance;

	TVector4 vecFar;
	vecFar.Multiply( a_rcDirection, 10000.0f );
	vecFar.Set( vecFar.x + a_rcOrigin.x, vecFar.y + a_rcOrigin.y, vecFar.z + a_rcOrigin.z, a_rcOrigin.w );

	if ( a_fHeight == 0.0f )
	{
		// Sphere
		TFLOAT fT;
		TFLOAT fDistanceSq = DistancePointToSegmentSq( a_rcBase, a_rcOrigin, vecFar, fT );

		if ( fDistanceSq - a_fRadius * a_fRadius >= 0.0f )
			return TFALSE;

		TFLOAT fHit  = fT * 10000.0f - sqrtf( ( a_fRadius + a_fEpsilon ) * ( a_fRadius + a_fEpsilon ) - fDistanceSq );
		a_rfDistance = fHit;

		if ( fHit > fMaxDistance || fHit < 0.0f )
		{
			a_rfDistance = fMaxDistance;
			return TFALSE;
		}

		TVector4 vecHit;
		vecHit.Multiply( a_rcDirection, fHit );
		vecHit = a_rcOrigin + vecHit;

		a_rNormal.Set( vecHit.x - a_rcBase.x, vecHit.y - a_rcBase.y, vecHit.z - a_rcBase.z, vecHit.w );
		a_rNormal.Normalise();
		return TTRUE;
	}

	// Capsule
	TVector4 vecTop;
	vecTop.Multiply( a_rcUp, a_fHeight );
	vecTop.Set( vecTop.x + a_rcBase.x, vecTop.y + a_rcBase.y, vecTop.z + a_rcBase.z, a_rcBase.w );

	TFLOAT fRayT;
	TFLOAT fCapsuleT;
	TFLOAT fDistanceSq = DistanceSegmentToSegmentSq( a_rcOrigin, vecFar, a_rcBase, vecTop, fRayT, fCapsuleT );

	if ( fDistanceSq - a_fRadius * a_fRadius >= 0.0f )
		return TFALSE;

	TFLOAT fHit  = fRayT * 10000.0f - sqrtf( ( a_fRadius + a_fEpsilon ) * ( a_fRadius + a_fEpsilon ) - fDistanceSq );
	a_rfDistance = fHit;

	if ( fHit > fMaxDistance || fHit < 0.0f )
	{
		a_rfDistance = fMaxDistance;
		return TFALSE;
	}

	TVector4 vecHit;
	vecHit.Multiply( a_rcDirection, fHit );
	vecHit = a_rcOrigin + vecHit;

	// Closest point on the axis
	TVector4 vecAxisPoint;
	vecAxisPoint.Multiply( a_rcUp, a_fHeight );
	vecAxisPoint.Multiply( fCapsuleT );
	vecAxisPoint = a_rcBase + vecAxisPoint;

	TVector4 vecFromBase = vecHit - a_rcBase;
	TVector4 vecFromTop  = vecHit - vecTop;

	if ( TVector4::DotProduct3( vecFromBase, a_rcUp ) > 0.0f && TVector4::DotProduct3( vecFromTop, a_rcUp ) < 0.0f )
	{
		// Hit the cylinder, remove the vertical part of the normal
		TVector4 vecVertical;
		vecVertical.Multiply( a_rcUp, TVector4::DotProduct3( vecFromBase, a_rcUp ) );

		a_rNormal.Set( vecFromBase.x - vecVertical.x, vecFromBase.y - vecVertical.y, vecFromBase.z - vecVertical.z, vecFromBase.w );
		a_rNormal.Normalise();
		return TTRUE;
	}

	a_rNormal.Set( vecHit.x - vecAxisPoint.x, vecHit.y - vecAxisPoint.y, vecHit.z - vecAxisPoint.z, vecHit.w );
	a_rNormal.Normalise();
	return TTRUE;
}

// $Barnyard: FUNCTION 006ba760
TBOOL TMath::RayVsCapsule( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon )
{
	if ( !IsPointInCapsule( a_rcOrigin, a_rcBase, a_rcUp, a_fRadius, a_fHeight ) )
		return RayVsCapsuleOutside( a_rcOrigin, a_rcDirection, a_rcBase, a_fRadius, a_rcUp, a_fHeight, a_rfDistance, a_rNormal, a_fEpsilon );

	TVector4 vecEnd( a_rcOrigin.x + a_rcDirection.x, a_rcOrigin.y + a_rcDirection.y, a_rcOrigin.z + a_rcDirection.z, a_rcOrigin.w );

	// The whole ray is inside of the capsule
	if ( IsPointInCapsule( vecEnd, a_rcBase, a_rcUp, a_fRadius, a_fHeight ) )
		return TFALSE;

	// Cast the ray back from its end
	TVector4 vecBackDirection;
	vecBackDirection.Multiply( a_rcDirection, -1.0f );

	TVector4 vecStart = a_rcOrigin + a_rcDirection;
	TBOOL    bResult  = RayVsCapsuleOutside( vecStart, vecBackDirection, a_rcBase, a_fRadius, a_rcUp, a_fHeight, a_rfDistance, a_rNormal, a_fEpsilon );

	if ( bResult == TTRUE )
		a_rfDistance = a_rcDirection.Magnitude() - a_rfDistance;

	return bResult;
}

// $Barnyard: FUNCTION 006ba090
TBOOL TMath::SphereVsCapsule( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcBase, TFLOAT a_fCapsuleRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon )
{
	TFLOAT fOriginalDistance = a_rfDistance;

	TVector4 vecTop;
	vecTop.Multiply( a_rcUp, a_fHeight );
	vecTop.Set( vecTop.x + a_rcBase.x, vecTop.y + a_rcBase.y, vecTop.z + a_rcBase.z, a_rcBase.w );

	TFLOAT fT;
	TFLOAT fDistanceSq = DistancePointToSegmentSq( a_rcCenter, a_rcBase, vecTop, fT );
	TFLOAT fRadius     = a_fRadius + a_fCapsuleRadius;

	// NOTE: negated so that NaN (a zero height capsule, i.e. a sphere, makes a degenerate segment) returns TFALSE like the original's x87 compare
	if ( !( fDistanceSq - fRadius * fRadius < 0.0f ) )
		return TFALSE;

	// Penetration depth
	TFLOAT fDepth = fRadius - sqrtf( fDistanceSq ) + a_fEpsilon;
	a_rfDistance  = fDepth;

	if ( fDepth > fOriginalDistance || fDepth < -a_fRadius )
	{
		a_rfDistance = fOriginalDistance;
		return TFALSE;
	}

	TVector4 vecAxisPoint;
	vecAxisPoint.Multiply( a_rcUp, fT * a_fHeight );
	vecAxisPoint = a_rcBase + vecAxisPoint;

	a_rNormal.Set( a_rcCenter.x - vecAxisPoint.x, a_rcCenter.y - vecAxisPoint.y, a_rcCenter.z - vecAxisPoint.z, a_rcCenter.w );
	a_rNormal.Normalise();
	return TTRUE;
}

// $Barnyard: FUNCTION 006b9490
TBOOL TMath::SweptSphereVsCapsule( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fCapsuleRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon )
{
	TFLOAT fMaxDistance = a_rfDistance;
	TFLOAT fRadius      = a_fRadius + a_fCapsuleRadius;

	if ( a_fHeight == 0.0f )
	{
		// Sphere
		TVector4 vecEnd;
		vecEnd.Multiply( a_rcDirection, fMaxDistance );
		vecEnd.Set( vecEnd.x + a_rcCenter.x, vecEnd.y + a_rcCenter.y, vecEnd.z + a_rcCenter.z, a_rcCenter.w );

		TFLOAT fT;
		TFLOAT fDistanceSq = DistancePointToLineSq( a_rcBase, a_rcCenter, vecEnd, fT );
		TFLOAT fRadiusSq   = fRadius * fRadius;

		if ( !( fDistanceSq - fRadiusSq < 0.0f ) )
			return TFALSE;

		TFLOAT fHit  = fT * fMaxDistance - ( sqrtf( fRadiusSq - fDistanceSq ) + a_fEpsilon );
		a_rfDistance = fHit;

		if ( fHit > fMaxDistance )
		{
			// Only check if the spheres are already touching on the XZ plane
			TVector4 vecToCenter( a_rcCenter.x - a_rcBase.x, 0.0f, a_rcCenter.z - a_rcBase.z, a_rcCenter.w );
			TFLOAT   fDistSq = vecToCenter.MagnitudeSq();

			if ( !( fDistSq > 0.0f ) || !( a_fEpsilon * a_fEpsilon + fRadiusSq > fDistSq ) )
			{
				a_rfDistance = fMaxDistance;
				return TFALSE;
			}

			if ( !( TVector4::DotProduct3( vecToCenter, a_rcDirection ) < 0.0f ) )
			{
				a_rfDistance = fMaxDistance;
				return TTRUE;
			}

			TFLOAT fDist = sqrtf( fDistSq );
			vecToCenter.Divide( fDist );

			a_rNormal    = vecToCenter;
			a_rfDistance = -fRadius + fDist;
			return TTRUE;
		}

		if ( fHit < 0.0f )
		{
			// Already overlapping
			if ( !( TVector4::DistanceSq( a_rcCenter, a_rcBase ) - fRadiusSq - a_fEpsilon * a_fEpsilon < 0.0f ) )
			{
				a_rfDistance = fMaxDistance;
				return TFALSE;
			}

			// NOTE: the vector to the base is not normalised
			TVector4 vecToBase = a_rcBase - a_rcCenter;
			TVector4 vecOffset;
			vecOffset.Multiply( vecToBase, fRadius );

			TVector4 vecPoint  = a_rcCenter + vecOffset;
			TVector4 vecNormal = a_rcCenter - vecPoint;

			if ( !( TVector4::DotProduct3( vecNormal, a_rcDirection ) < 0.0f ) )
			{
				a_rfDistance = fMaxDistance;
				return TTRUE;
			}

			a_rNormal    = vecNormal;
			a_rfDistance = a_rNormal.NormaliseMag() - fRadius - a_fEpsilon;

			if ( a_rfDistance >= 0.0f )
				a_rfDistance = -a_fEpsilon;

			return TTRUE;
		}

		TVector4 vecHit;
		vecHit.Multiply( a_rcDirection, fHit );
		vecHit = a_rcCenter + vecHit;

		a_rNormal = vecHit - a_rcBase;

		if ( a_rfDistance < 0.0f )
		{
			a_rfDistance = a_rNormal.NormaliseMag() - fRadius - a_fEpsilon;
			return TTRUE;
		}

		a_rNormal.Normalise();
		return TTRUE;
	}

	// Capsule
	TFLOAT fLength = ( fRadius + fMaxDistance + a_fHeight ) * 3.0f;

	TVector4 vecOffset;
	vecOffset.Multiply( a_rcDirection, fLength );

	TVector4 vecFar( vecOffset.x + a_rcCenter.x, vecOffset.y + a_rcCenter.y, vecOffset.z + a_rcCenter.z, a_rcCenter.w );

	TVector4 vecTop;
	vecTop.Multiply( a_rcUp, a_fHeight );
	vecTop.Set( vecTop.x + a_rcBase.x, vecTop.y + a_rcBase.y, vecTop.z + a_rcBase.z, a_rcBase.w );

	TVector4 vecNear( a_rcCenter.x - vecOffset.x, a_rcCenter.y - vecOffset.y, a_rcCenter.z - vecOffset.z, a_rcCenter.w );

	TFLOAT fMoveT;
	TFLOAT fCapsuleT;
	TFLOAT fDistanceSq = DistanceSegmentToSegmentSq( vecNear, vecFar, a_rcBase, vecTop, fMoveT, fCapsuleT );
	TFLOAT fRadiusSq   = fRadius * fRadius;

	if ( !( fDistanceSq - fRadiusSq <= 0.0f ) )
		return TFALSE;

	TFLOAT fHit  = ( fMoveT * fLength + fMoveT * fLength - sqrtf( fRadiusSq - fDistanceSq + a_fEpsilon ) ) - fLength;
	a_rfDistance = fHit;

	if ( fHit > fMaxDistance )
	{
		a_rfDistance = fMaxDistance;
		return TFALSE;
	}

	if ( fHit < 0.0f )
	{
		// Already overlapping
		TFLOAT fT;

		if ( !( DistancePointToSegmentSq( a_rcCenter, a_rcBase, vecTop, fT ) - fRadiusSq - a_fEpsilon * a_fEpsilon < 0.0f ) )
		{
			a_rfDistance = fMaxDistance;
			return TFALSE;
		}

		TVector4 vecAxis = vecTop - a_rcBase;
		TVector4 vecAxisOffset;
		vecAxisOffset.Multiply( vecAxis, fT );

		TVector4 vecAxisPoint = a_rcBase + vecAxisOffset;
		TVector4 vecNormal    = a_rcCenter - vecAxisPoint;

		if ( !( TVector4::DotProduct3( vecNormal, a_rcDirection ) < 0.0f ) )
		{
			a_rfDistance = fMaxDistance;
			return TTRUE;
		}

		a_rNormal    = vecNormal;
		a_rfDistance = a_rNormal.NormaliseMag() - fRadius - a_fEpsilon;

		if ( a_rfDistance >= 0.0f )
			a_rfDistance = -a_fEpsilon;

		return TTRUE;
	}

	fHit         = fHit - a_fEpsilon * 10.0f;
	a_rfDistance = fHit;

	TVector4 vecHit;
	vecHit.Multiply( a_rcDirection, fHit );
	vecHit = a_rcCenter + vecHit;

	TVector4 vecAxisPoint;
	vecAxisPoint.Multiply( a_rcUp, a_fHeight );
	vecAxisPoint.Multiply( fCapsuleT );
	vecAxisPoint = a_rcBase + vecAxisPoint;

	a_rNormal = vecHit - vecAxisPoint;

	if ( a_rfDistance < 0.0f )
	{
		a_rfDistance = a_rNormal.NormaliseMag() - fRadius - a_fEpsilon;
		return TTRUE;
	}

	a_rNormal.Normalise();
	return TTRUE;
}

// $Barnyard: FUNCTION 006ba530
void TMath::SlideAlongPlane( const TVector4& a_rcDirection, const TVector4& a_rcNormal, TFLOAT a_fSpeedScale, TFLOAT a_fTotalDistance, TFLOAT a_fTravelledDistance, TFLOAT& a_rfOutDistance, TVector4& a_rOutDirection, TBOOL a_bLinearFriction, TFLOAT a_fPushOut, TFLOAT a_fFriction, TFLOAT a_fMaxDot )
{
	TFLOAT fRemaining = a_fTotalDistance - a_fTravelledDistance;
	TFLOAT fDot       = -TVector4::DotProduct3( a_rcDirection, a_rcNormal );

	if ( fDot <= 0.0f )
		fDot = 0.0f;

	if ( fDot < a_fMaxDot )
	{
		// Push the direction off the plane
		TVector4 vecPush;
		vecPush.Multiply( a_rcNormal, fDot + a_fPushOut );

		a_rOutDirection.Set( vecPush.x + a_rcDirection.x, vecPush.y + a_rcDirection.y, vecPush.z + a_rcDirection.z, a_rcDirection.w );
		a_rOutDirection.Normalise();

		TFLOAT fFactor = fDot + a_fFriction;

		if ( fFactor > 1.0f )
			fFactor = 1.0f;
		else if ( fFactor < 0.0f )
			fFactor = 0.0f;

		TFLOAT fDistance = a_bLinearFriction ? ( 1.0f - fFactor ) * fRemaining * a_fSpeedScale : ( 1.0f - fFactor * fFactor ) * fRemaining * a_fSpeedScale;
		a_rfOutDistance  = ( fDistance > 0.0f ) ? fDistance : 0.0f;
	}
	else
	{
		a_rfOutDistance = 0.0f;
	}
}

TOSHI_NAMESPACE_END
