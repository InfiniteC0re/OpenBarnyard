#pragma once
#include "AMilkProjectile.h"
#include "MiniGame/AChasingChicksGame.h"

//-----------------------------------------------------------------------------
// Milk squirted in the Chasing Chicks minigame (projectile type 9).
// Only hits the players (AUnitPlayer) and stuns the ones it splashes.
// TClass 007093a0 (parent AMilkProjectile, size 0x108), vtable 007222f8.
//-----------------------------------------------------------------------------
class AChasingChicksMilkProjectile
    : public AMilkProjectile
{
public:
	TDECLARE_CLASS( AChasingChicksMilkProjectile, AMilkProjectile );

public:
	AChasingChicksMilkProjectile();
	~AChasingChicksMilkProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual void   OnLaunched() OVERRIDE {}                                            // +0x1c, DOES_NOTHING (j_nullsub_1 0050de10)
	virtual void   OnHit() OVERRIDE;                                                   // +0x24
	virtual TINT   GetType() OVERRIDE;                                                 // +0x28
	virtual TBOOL  CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit ) OVERRIDE; // +0x30
	virtual TFLOAT GetRadius() OVERRIDE { return 0.75f; }                              // +0x34, NOTE: folded with other functions returning 0.75f (00515830)
};

TSTATICASSERT( sizeof( AChasingChicksMilkProjectile ) == 0x108 );
