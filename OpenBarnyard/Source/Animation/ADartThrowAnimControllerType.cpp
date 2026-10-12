#include "pch.h"
#include "ADartThrowAnimControllerType.h"
#include "ADartThrowAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058a0a0, CreateObject 0058a170, CreateObjectInPlace 0058a1e0
TDEFINE_CLASS( ADartThrowAnimControllerType );

ADartThrowAnimControllerType::ADartThrowAnimControllerType()
{
}

// $Barnyard: FUNCTION 0058a310
// $Barnyard: FUNCTION 0058a0c0
ADartThrowAnimControllerType::~ADartThrowAnimControllerType()
{
}

// NOTE: folded with ADartThrowAnimController::GetClass
// $Barnyard: FUNCTION 0058a0b0
TClass* ADartThrowAnimControllerType::GetControllerClass()
{
	return &TGetClass( ADartThrowAnimController );
}
