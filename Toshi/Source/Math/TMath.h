/**
 * @file TMath.h
 * Core math utilities for the Toshi engine
 */

#pragma once
#include "TVector4.h"

TOSHI_NAMESPACE_START

namespace TMath
{

// Performs a cone-sphere intersection test and returns TTRUE if hit.
TBOOL ConeVsSphere( const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, float a_fConeFOV, float a_fSphereRadius, const TVector4& a_rvSpherePosition );

// Performs a ray-sphere intersection test and returns number of intersections. Always returns 0 or 2 intersections.
TINT RayVsCircle3D( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNearIntersection, TFLOAT& a_rfFarIntersection );

// Performs a sphere-line intersection test and returns TTRUE if hit anything.
TBOOL SphereVsLine( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TINT* a_pNumHits, TFLOAT* a_pNearIntersection, TFLOAT* a_pFarIntersection );

// Calculates squared distance from a point to line. Outputs position of the closest point in a_rClosestPoint.
TFLOAT DistancePointToLineSq( const TVector4& a_rcPoint, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TVector4& a_rClosestPoint );

} // namespace TMath

TOSHI_NAMESPACE_END
