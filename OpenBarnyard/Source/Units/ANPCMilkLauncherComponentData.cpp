#include "pch.h"
#include "ANPCMilkLauncherComponentData.h"
#include "ANPCMilkLauncherComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 00669980, CreateObjectInPlace 00669750
// NOTE: GetClass 00669740, CreateObject 00669980, CreateObjectInPlace 00669750
TDEFINE_CLASS( ANPCMilkLauncherComponentData );

// $Barnyard: FUNCTION 00669ad0
void ANPCMilkLauncherComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	ANPCMilkLauncherComponent* pComponent = new ANPCMilkLauncherComponent();
	pComponent->Create( a_pUnit, this );

	a_pUnit->AddComponent( pComponent );
}
