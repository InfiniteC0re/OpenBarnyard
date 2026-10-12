#include "pch.h"
#include "AAlienAnimControllerType.h"
#include "AAlienAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00585520, CreateObject 00585680, CreateObjectInPlace 005856f0
TDEFINE_CLASS( AAlienAnimControllerType );

AAlienAnimControllerType::AAlienAnimControllerType()
{
}

// $Barnyard: FUNCTION 005857e0
// $Barnyard: FUNCTION 00585530
AAlienAnimControllerType::~AAlienAnimControllerType()
{
}

// NOTE: folded with AAlienAnimController::GetClass
// $Barnyard: FUNCTION 00585540
TClass* AAlienAnimControllerType::GetControllerClass()
{
	return &TGetClass( AAlienAnimController );
}
