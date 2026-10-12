#include "pch.h"
#include "ALaunchUnitAmmo.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006664a0, CreateObject 006664d0, CreateObjectInPlace 00666540
TDEFINE_CLASS( ALaunchUnitAmmo );

// $Barnyard: FUNCTION 006664b0
ALaunchUnitAmmo::ALaunchUnitAmmo()
{
	SetRechargeRate( 0.0f );
}
