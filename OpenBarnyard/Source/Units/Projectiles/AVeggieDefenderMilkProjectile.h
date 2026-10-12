#pragma once
#include "AMilkProjectile.h"

//-----------------------------------------------------------------------------
// Milk squirted by the player in the Veggie Patch minigame (projectile type 1).
// Milks the raccoons (AVeggiePatchThief) it hits.
// TClass 00708de0 (parent AMilkProjectile, size 0x108), vtable 0071fdf0.
//-----------------------------------------------------------------------------
class AVeggieDefenderMilkProjectile : public AMilkProjectile
{
public:
	TDECLARE_CLASS( AVeggieDefenderMilkProjectile, AMilkProjectile );

public:
	AVeggieDefenderMilkProjectile();
	~AVeggieDefenderMilkProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual void  OnLaunched() OVERRIDE {}                                            // +0x1c, DOES_NOTHING (j_nullsub_1 0050de10)
	virtual void  OnHit() OVERRIDE;                                                   // +0x24
	virtual TINT  GetType() OVERRIDE;                                                 // +0x28
	virtual TBOOL CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit ) OVERRIDE; // +0x30
};

TSTATICASSERT( sizeof( AVeggieDefenderMilkProjectile ) == 0x108 );
