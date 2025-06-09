/**
 * @file TMath.h
 * Core math utilities for the Toshi engine
 */

#pragma once
#include "TVector4.h"

TOSHI_NAMESPACE_START

namespace TMath
{

enum VSRESULT
{
	VSRESULT_OUTSIDE,
	VSRESULT_HIT,
	VSRESULT_INSIDE,
};

TBOOL    ConeVsSphere( const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, float a_fConeFOV, float a_fSphereRadius, const TVector4& a_rvSpherePosition );
VSRESULT RayVsCircle3D( const TVector4& a_rcStartPos, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfRayDistance, TFLOAT& a_rfUnknown );

}

TOSHI_NAMESPACE_END
