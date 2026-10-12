#pragma once
#include "AUnitStaticData.h"

// Gives every unit of a type an ANPCBehaviourComponent ("ANPCBehaviourComponentData" block).
//
// TClass 0x007b4f60 (version 0.1, size 0xc), vtable 0x00739740.
class ANPCBehaviourComponentData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( ANPCBehaviourComponentData, AUnitStaticData );

public:
	// NOTE: inlined into CreateObject 00633930 / CreateObjectInPlace 00633820
	ANPCBehaviourComponentData() {}

	// NOTE: the deleting destructor (0061fa90) is shared with other static data classes
	~ANPCBehaviourComponentData();

	virtual void OnUnitCreate( AUnit* a_pUnit ) OVERRIDE; // +0x10
};

TSTATICASSERT( sizeof( ANPCBehaviourComponentData ) == 0xc );
