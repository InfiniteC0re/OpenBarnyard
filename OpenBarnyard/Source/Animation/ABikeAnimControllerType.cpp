#include "pch.h"
#include "ABikeAnimControllerType.h"
#include "ABikeAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00586990, CreateObject 00586ae0, CreateObjectInPlace 00586b50
TDEFINE_CLASS( ABikeAnimControllerType );

ABikeAnimControllerType::ABikeAnimControllerType()
{
}

// $Barnyard: FUNCTION 00586de0
// $Barnyard: FUNCTION 005869b0
ABikeAnimControllerType::~ABikeAnimControllerType()
{
}

// NOTE: folded with ABikeAnimController::GetClass
// $Barnyard: FUNCTION 005869c0
TClass* ABikeAnimControllerType::GetControllerClass()
{
	return &TGetClass( ABikeAnimController );
}
