#pragma once
#include "AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// An animated unit without any behaviour of its own (e.g. "player_golf", the
// player of the Gopher Golf minigame).
//
// TClass 0077e564 (parent AAnimatedUnit, size 0x98), vtables 00717b18,
// 00717b0c (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AUnitAesthetic
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AUnitAesthetic, AAnimatedUnit );

public:
	// NOTE: the retail ctor only exists inlined into the TClass create function
	AUnitAesthetic() = default;
	~AUnitAesthetic();
};

TSTATICASSERT( sizeof( AUnitAesthetic ) == 0x98 );
