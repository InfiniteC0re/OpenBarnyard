#include "pch.h"
#include "AUnitDummy.h"
#include "UnitMovement/Steering/AStationarySteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004144b0, CreateObject 00414530, CreateObjectInPlace 00414590
TDEFINE_CLASS( AUnitDummy );

// $Barnyard: FUNCTION 004144c0
AUnitDummy::AUnitDummy()
{
	SetHashed( TFALSE );
}

// $Barnyard: FUNCTION 00414510
// $Barnyard: FUNCTION 004145d0
AUnitDummy::~AUnitDummy()
{
}

// $Barnyard: FUNCTION 00414520
TBOOL AUnitDummy::Create( AUnitTransform* a_pTransform )
{
	return AUnit::Create( a_pTransform );
}

// NOTE: folded with AStationarySteer::GetClass (00570220) in retail
TClass* AUnitDummy::GetSteerClass()
{
	return &TGetClass( AStationarySteer );
}
