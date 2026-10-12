#pragma once
#include "AMilkProjectile.h"

// Projectile of the player's milk squirt (projectile type 0): triggers the
// reactions of the units hit by the milk. Its radius grows with its age.
class ASimMilkProjectile : public AMilkProjectile
{
public:
	TDECLARE_CLASS( ASimMilkProjectile, AMilkProjectile );

public:
	ASimMilkProjectile();
	~ASimMilkProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual void OnHit() OVERRIDE; // +0x24
	// $Barnyard: FUNCTION 0066c6b0
	virtual TFLOAT GetRadius() OVERRIDE { return m_fAge * 0.58f + 0.5f; } // +0x34
};

TSTATICASSERT( sizeof( ASimMilkProjectile ) == 0x108 );
