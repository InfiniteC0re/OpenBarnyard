#include "pch.h"
#include "ACharacterMovementData.h"
#include "ACharacterMovementComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054f1b0, CreateObject 0054f2e0, CreateObjectInPlace 0054f1c0 (the constructor is inlined in both)
TDEFINE_CLASS( ACharacterMovementData );

// $Barnyard: FUNCTION 0054f5a0
void ACharacterMovementData::OnUnitCreate( AUnit* a_pUnit )
{
	a_pUnit->AddComponent( new ACharacterMovementComponent() );
}
