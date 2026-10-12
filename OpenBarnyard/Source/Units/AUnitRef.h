#pragma once
#include <Toshi/TRefCounted.h>

#include "AUnitProxy.h"

class AUnit;
class AUnitType;

// Unit reference held by FSM code: a live proxy, or a type/script name to resolve later.
// NOTE: name made up, 0xc bytes.
struct AUnitRef
{
	// $Barnyard: FUNCTION 005c52f0
	AUnitRef()
	    : pType( TNULL ), szName( TNULL )
	{}

	// Resolves the reference, re-binding the proxy by script name if needed.
	AUnit* GetUnit();

	Toshi::TRef<AUnitProxy> oProxy; // 0x00
	AUnitType*              pType;  // 0x04
	const TCHAR*            szName; // 0x08, script name (AUnitNameManager)
};
