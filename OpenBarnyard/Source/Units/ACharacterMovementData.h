#pragma once
#include "AUnitStaticData.h"

// Gives every unit of a type an ACharacterMovementComponent ("ACharacterMovementData" block).
//
// TClass 0x00784068 (version 0.1, size 0xc, runtime creatable), vtable 0x007249fc.
class ACharacterMovementData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( ACharacterMovementData, AUnitStaticData );

public:
	// NOTE: inlined into CreateObject 0054f2e0 / CreateObjectInPlace 0054f1c0
	ACharacterMovementData() {}

	// NOTE: the deleting destructor (0061fa90) is shared with other static data classes
	// $Barnyard: FUNCTION 0061fa90
	~ACharacterMovementData() {}

	virtual void OnUnitCreate( AUnit* a_pUnit ) OVERRIDE; // +0x10
};

TSTATICASSERT( sizeof( ACharacterMovementData ) == 0xc );
