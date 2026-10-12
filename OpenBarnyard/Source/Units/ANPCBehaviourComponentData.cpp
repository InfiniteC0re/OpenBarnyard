#include "pch.h"
#include "ANPCBehaviourComponentData.h"
#include "ANPCBehaviourComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00633810
// NOTE: GetClass 00633810, CreateObject 00633930, CreateObjectInPlace 00633820
TDEFINE_CLASS( ANPCBehaviourComponentData );

// NOTE: a thunk to AUnitStaticData::~AUnitStaticData
// $Barnyard: FUNCTION 00633990
ANPCBehaviourComponentData::~ANPCBehaviourComponentData()
{
}

// $Barnyard: FUNCTION 00633a60
void ANPCBehaviourComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	a_pUnit->AddComponent( new ANPCBehaviourComponent() );
}
