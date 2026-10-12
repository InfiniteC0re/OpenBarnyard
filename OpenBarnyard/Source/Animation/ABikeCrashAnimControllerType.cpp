#include "pch.h"
#include "ABikeCrashAnimControllerType.h"
#include "ABikeCrashAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005869a0, CreateObject 00586c40, CreateObjectInPlace 00586cb0
TDEFINE_CLASS( ABikeCrashAnimControllerType );

ABikeCrashAnimControllerType::ABikeCrashAnimControllerType()
{
}

// $Barnyard: FUNCTION 00586e60
// $Barnyard: FUNCTION 00586ab0
ABikeCrashAnimControllerType::~ABikeCrashAnimControllerType()
{
}

// NOTE: folded with ABikeCrashAnimController::GetClass
// $Barnyard: FUNCTION 00586ac0
TClass* ABikeCrashAnimControllerType::GetControllerClass()
{
	return &TGetClass( ABikeCrashAnimController );
}
