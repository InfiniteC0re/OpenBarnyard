#pragma once
#include "AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// The golf ball of the Gopher Golf minigame (unit type "golfball"). An animated
// unit that only enables the think pass on creation.
//
// TClass 0077e8b8 (parent AAnimatedUnit, size 0x98), vtables 00718970,
// 00718960 (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AUnitGolfProjectile
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AUnitGolfProjectile, AAnimatedUnit );

public:
	// NOTE: the retail ctor only exists inlined into the TClass create functions
	AUnitGolfProjectile() = default;
	~AUnitGolfProjectile();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
};

TSTATICASSERT( sizeof( AUnitGolfProjectile ) == 0x98 );
