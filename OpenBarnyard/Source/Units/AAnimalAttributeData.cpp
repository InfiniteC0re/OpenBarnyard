#include "pch.h"
#include "AAnimalAttributeData.h"
#include "AAnimalAttributes.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058aa10, CreateObject 0058aba0, CreateObjectInPlace 0058aa20
TDEFINE_CLASS( AAnimalAttributeData );

// $Barnyard: FUNCTION 0058b280
void AAnimalAttributeData::OnUnitCreate( AUnit* a_pUnit )
{
	a_pUnit->AddComponent( new AAnimalAttributes() );
}
