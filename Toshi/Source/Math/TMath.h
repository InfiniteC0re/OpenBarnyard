#pragma once
#include "TVector4.h"

namespace Toshi
{

namespace TMath
{

TBOOL ConeVsSphere( const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, float a_fConeFOV, float a_fSphereRadius, const TVector4& a_rvSpherePosition );

}

} // namespace Toshi