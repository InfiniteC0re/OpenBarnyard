#include "pch.h"
#include "AChasingChicksAnimControllerType.h"
#include "AChasingChicksAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005896a0, CreateObject 005896e0, CreateObjectInPlace 00589750
TDEFINE_CLASS( AChasingChicksAnimControllerType );

AChasingChicksAnimControllerType::AChasingChicksAnimControllerType()
{
}

// $Barnyard: FUNCTION 005897a0
// $Barnyard: FUNCTION 005896c0
AChasingChicksAnimControllerType::~AChasingChicksAnimControllerType()
{
}

// NOTE: folded with AChasingChicksAnimController::GetClass
// $Barnyard: FUNCTION 005896b0
TClass* AChasingChicksAnimControllerType::GetControllerClass()
{
	return &TGetClass( AChasingChicksAnimController );
}
