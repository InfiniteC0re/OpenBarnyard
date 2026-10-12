#pragma once
#include "AUnitComponent.h"
#include "FSM/AFunctionTable.h"

class AAimedSquirtProjectileLauncher;
class ANPCMilkLauncherComponentData;

//-----------------------------------------------------------------------------
// Lets an NPC squirt milk back (e.g. Bud, Eddie and Igg when the player squirts
// them from behind, see ASimMilkProjectile::OnHit). Owns an aimed squirt launcher
// that's activated for 2 seconds by FireAtTarget.
//
// TClass 0x007cd1e8 (version 1.0, size 0x1c, runtime creatable), vtable 0x0073b530.
//-----------------------------------------------------------------------------
class ANPCMilkLauncherComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( ANPCMilkLauncherComponent, AUnitComponent );

public:
	ANPCMilkLauncherComponent();
	~ANPCMilkLauncherComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual AFunctionTable*        GetFunctionTable() OVERRIDE;              // +0x0c
	virtual AUnitComponentManager* GetManager() OVERRIDE;                    // +0x10
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c

	// Creates the launcher of a_pOwner.
	TBOOL Create( AUnit* a_pOwner, ANPCMilkLauncherComponentData* a_pData );

	// FSM native: squirts at a_pTarget's position, raised by a_fHeightOffset.
	TBOOL FireAtTarget( AUnit* a_pTarget, TFLOAT a_fHeightOffset );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	AAimedSquirtProjectileLauncher* m_pLauncher;  // 0x10
	TBOOL                           m_bFiring;    // 0x14, the launcher is active
	TFLOAT                          m_fFireTimer; // 0x18, time left until the launcher is deactivated
};

TSTATICASSERT( sizeof( ANPCMilkLauncherComponent ) == 0x1c );
