#pragma once
#include "AMilkProjectile.h"

//-----------------------------------------------------------------------------
// Milk squirted in the Sharp Squirter minigame (projectile type 7), by the
// player and by the Jersey cows. Knocks over the targets, coyotes, raccoons and
// scarecrows it hits, and makes the cows "yuk".
// TClass 0077... (parent AMilkProjectile, size 0x108), vtable 0073b190.
//-----------------------------------------------------------------------------
class AMilkPrecisionProjectile
    : public AMilkProjectile
{
public:
	TDECLARE_CLASS( AMilkPrecisionProjectile, AMilkProjectile );

public:
	AMilkPrecisionProjectile();
	~AMilkPrecisionProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual void   OnLaunched() OVERRIDE {}               // +0x1c, DOES_NOTHING (j_nullsub_1 0050de10)
	virtual void   OnHit() OVERRIDE;                      // +0x24
	virtual TINT   GetType() OVERRIDE;                    // +0x28
	virtual TFLOAT GetRadius() OVERRIDE { return 0.75f; } // +0x34, NOTE: folded with other functions returning 0.75f (00515830)
};

TSTATICASSERT( sizeof( AMilkPrecisionProjectile ) == 0x108 );
