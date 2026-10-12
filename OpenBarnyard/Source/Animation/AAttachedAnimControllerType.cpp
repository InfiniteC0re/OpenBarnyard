#include "pch.h"
#include "AAttachedAnimControllerType.h"
#include "AAttachedAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00585f00, CreateObject 00585f80, CreateObjectInPlace 00585ff0
TDEFINE_CLASS( AAttachedAnimControllerType );

AAttachedAnimControllerType::AAttachedAnimControllerType()
{
}

// $Barnyard: FUNCTION 00586100
// $Barnyard: FUNCTION 00585f10
AAttachedAnimControllerType::~AAttachedAnimControllerType()
{
}

// NOTE: folded with AAttachedAnimController::GetClass
// $Barnyard: FUNCTION 00585f20
TClass* AAttachedAnimControllerType::GetControllerClass()
{
	return &TGetClass( AAttachedAnimController );
}
