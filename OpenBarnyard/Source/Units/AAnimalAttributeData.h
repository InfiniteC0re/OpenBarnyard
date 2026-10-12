#pragma once
#include "AUnitStaticData.h"

// Gives every unit of a type an AAnimalAttributes ("AAnimalAttributeData" block).
//
// TClass 0x00789fc4 (version 0.1, size 0xc), vtable 0x0072850c.
class AAnimalAttributeData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AAnimalAttributeData, AUnitStaticData );

public:
	// NOTE: inlined into CreateObject 0058aba0 / CreateObjectInPlace 0058aa20
	AAnimalAttributeData() {}

	// NOTE: the deleting destructor (0061fa90) is shared with other static data classes
	~AAnimalAttributeData() {}

	virtual void OnUnitCreate( AUnit* a_pUnit ) OVERRIDE; // +0x10
};

TSTATICASSERT( sizeof( AAnimalAttributeData ) == 0xc );
