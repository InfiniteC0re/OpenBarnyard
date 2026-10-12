#pragma once
#include "Units/Projectiles/ALaunchAmmo.h"

//-----------------------------------------------------------------------------
// ALaunchAmmo that doesn't recharge (recharge rate 0). Same vtable slots as
// ALaunchAmmo, only the class differs. vtable 0073b164, size 0x18.
// Used by AVeggiePatchDefenderGame, AChasingChicksMiniGame and AChickenCoopDefender.
//-----------------------------------------------------------------------------
class ALaunchUnitAmmo : public ALaunchAmmo
{
public:
	TDECLARE_CLASS( ALaunchUnitAmmo, ALaunchAmmo );

public:
	ALaunchUnitAmmo();
};

TSTATICASSERT( sizeof( ALaunchUnitAmmo ) == 0x18 );
