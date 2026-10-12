#pragma once
#include "AUnitStaticData.h"

// Gives every unit of a type an AUnitGameEventComponent ("AUnitGameEventData" block).
//
// TClass 0x0078bed0 (version 1.0, size 0xc), vtable 0x0072cb10.
class AUnitGameEventData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AUnitGameEventData, AUnitStaticData );

public:
	// NOTE: inlined into CreateObject 005c79b0 / CreateObjectInPlace 005c7870
	AUnitGameEventData() {}

	// NOTE: the deleting destructor (0061fa90) is shared with other static data classes
	~AUnitGameEventData() {}

	virtual void OnUnitCreate( AUnit* a_pUnit ) OVERRIDE; // +0x10
};

TSTATICASSERT( sizeof( AUnitGameEventData ) == 0xc );
