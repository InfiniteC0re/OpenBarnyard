#include "pch.h"
#include "AKinematicsObstructionInteraction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00573650, CreateObject 005736b0, CreateObjectInPlace 00573750
TDEFINE_CLASS( AKinematicsObstructionInteraction );

// $Barnyard: FUNCTION 00573750
AKinematicsObstructionInteraction::AKinematicsObstructionInteraction()
    : m_vPosition( TVector4::VEC_ZERO )
    , m_fRadius( 0.0f )
{
}

// $Barnyard: FUNCTION 005737e0
// $Barnyard: FUNCTION 005736a0
AKinematicsObstructionInteraction::~AKinematicsObstructionInteraction()
{
}

// $Barnyard: FUNCTION 00573670
TVector4 AKinematicsObstructionInteraction::GetDirection()
{
	return TVector4::VEC_POSX;
}

TVector4 AKinematicsObstructionInteraction::GetVelocity()
{
	// NOTE: same function as GetDirection in retail (00573670)
	return TVector4::VEC_POSX;
}
