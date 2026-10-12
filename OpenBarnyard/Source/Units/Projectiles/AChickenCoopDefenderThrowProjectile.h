#pragma once
#include "AProjectileWithUnit.h"

//-----------------------------------------------------------------------------
// A hen thrown by the player into the coop in Chicken Coop Defender
// (projectile type 5). Doesn't hit anything, it only follows the hen.
// TClass 007095a0 (parent AProjectileWithUnit, size 0x60), vtable 00722c18.
//-----------------------------------------------------------------------------
class AChickenCoopDefenderThrowProjectile
    : public AProjectileWithUnit
{
public:
	TDECLARE_CLASS( AChickenCoopDefenderThrowProjectile, AProjectileWithUnit );

public:
	AChickenCoopDefenderThrowProjectile();
	~AChickenCoopDefenderThrowProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual TBOOL  Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                               // +0x10
	virtual void   Kill() OVERRIDE;                                                                      // +0x14
	virtual void   OnLaunched() OVERRIDE;                                                                // +0x1c
	virtual TINT   GetType() OVERRIDE;                                                                   // +0x28
	virtual TBOOL  CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit ) OVERRIDE { return TFALSE; } // +0x30, NOTE: folded with other functions returning TFALSE (006ce000)
	virtual TFLOAT GetRadius() OVERRIDE { return m_fCollisionRadius; }                                   // +0x34, NOTE: folded with other float getters of a field at +0x5c (00572580)

private:
	TFLOAT m_fCollisionRadius; // 0x5c, of the hen's AUnitCollisionComponentData
};

TSTATICASSERT( sizeof( AChickenCoopDefenderThrowProjectile ) == 0x60 );
