#include "pch.h"
#include "ABasicAnimControllerType.h"
#include "ABasicAnimController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00586560, CreateObject 005865a0, CreateObjectInPlace 00586610
TDEFINE_CLASS( ABasicAnimControllerType );

ABasicAnimControllerType::ABasicAnimControllerType()
{
}

// $Barnyard: FUNCTION 00586720
// $Barnyard: FUNCTION 00586570
ABasicAnimControllerType::~ABasicAnimControllerType()
{
}

// NOTE: folded with ABasicAnimController::GetClass
// $Barnyard: FUNCTION 00586580
TClass* ABasicAnimControllerType::GetControllerClass()
{
	return &TGetClass( ABasicAnimController );
}
