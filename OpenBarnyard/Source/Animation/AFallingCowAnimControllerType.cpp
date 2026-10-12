#include "pch.h"
#include "AFallingCowAnimControllerType.h"
#include "AFallingCowAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058a4b0, CreateObject 0058a4f0, CreateObjectInPlace 0058a560
TDEFINE_CLASS( AFallingCowAnimControllerType );

AFallingCowAnimControllerType::AFallingCowAnimControllerType()
{
}

// $Barnyard: FUNCTION 0058a6a0
// $Barnyard: FUNCTION 0058a4d0
AFallingCowAnimControllerType::~AFallingCowAnimControllerType()
{
}

// NOTE: folded with AFallingCowAnimController::GetClass
// $Barnyard: FUNCTION 0058a4c0
TClass* AFallingCowAnimControllerType::GetControllerClass()
{
	return &TGetClass( AFallingCowAnimController );
}
