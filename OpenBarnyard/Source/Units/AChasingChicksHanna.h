#pragma once
#include "AAnimatedUnit.h"

//-----------------------------------------------------------------------------
// Hanna in the centre of the Chasing Chicks farmyard (unit type
// "chasingchicks_hanna"). The chicks are thrown into her basket; she plays
// "distress_pleased" when one lands there (see AChasingChicksChick).
// No behaviour of its own.
//
// TClass 00709360 (parent AAnimatedUnit, size 0x98), vtables 00722208,
// 007221fc (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AChasingChicksHanna
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AChasingChicksHanna, AAnimatedUnit );

	AChasingChicksHanna();
	~AChasingChicksHanna();
};

TSTATICASSERT( sizeof( AChasingChicksHanna ) == 0x98 );
