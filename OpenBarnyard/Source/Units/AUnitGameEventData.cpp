#include "pch.h"
#include "AUnitGameEventData.h"
#include "AUnitGameEventComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c7860
// NOTE: GetClass 005c7860, CreateObject 005c79b0, CreateObjectInPlace 005c7870
TDEFINE_CLASS( AUnitGameEventData, 1, 0 );

// $Barnyard: FUNCTION 005c8000
void AUnitGameEventData::OnUnitCreate( AUnit* a_pUnit )
{
	a_pUnit->AddComponent( new AUnitGameEventComponent() );
}
