#pragma once
#include "AMilkProjectile.h"

//-----------------------------------------------------------------------------
// Milk squirted in Chicken Coop Defender (projectile type 8).
// Only hits the thieves (AChickenCoopDefenderThief) that aren't stunned yet,
// and stuns the ones around where it splashes.
// TClass 007094e0 (parent AMilkProjectile, size 0x108), vtable 007227f8.
//-----------------------------------------------------------------------------
class AChickenCoopDefenderMilkProjectile
    : public AMilkProjectile
{
public:
	TDECLARE_CLASS( AChickenCoopDefenderMilkProjectile, AMilkProjectile );

public:
	AChickenCoopDefenderMilkProjectile();
	~AChickenCoopDefenderMilkProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual void   OnLaunched() OVERRIDE {}                                            // +0x1c, DOES_NOTHING (j_nullsub_1 0050de10)
	virtual void   OnHit() OVERRIDE;                                                   // +0x24
	virtual TINT   GetType() OVERRIDE;                                                 // +0x28
	virtual TBOOL  CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit ) OVERRIDE; // +0x30
	virtual TFLOAT GetRadius() OVERRIDE;                                               // +0x34
};

TSTATICASSERT( sizeof( AChickenCoopDefenderMilkProjectile ) == 0x108 );
