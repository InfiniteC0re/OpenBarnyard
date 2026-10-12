#pragma once
#include "AUnitStaticData.h"

//-----------------------------------------------------------------------------
// Static data of the units that squirt back ("ANPCMilkLauncherComponentData"
// block, no properties). Creates an ANPCMilkLauncherComponent for every unit.
//
// TClass 0x007cd218 (version 1.0, size 0xc, runtime creatable), vtable 0x0073b518.
//-----------------------------------------------------------------------------
class ANPCMilkLauncherComponentData
    : public AUnitStaticData
{
public:
	TDECLARE_CLASS( ANPCMilkLauncherComponentData, AUnitStaticData );

public:
	virtual void OnUnitCreate( AUnit* a_pUnit ) OVERRIDE; // +0x10
};

TSTATICASSERT( sizeof( ANPCMilkLauncherComponentData ) == 0xc );
