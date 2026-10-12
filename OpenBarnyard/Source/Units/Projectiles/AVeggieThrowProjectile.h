#pragma once
#include "AProjectileWithUnit.h"

//-----------------------------------------------------------------------------
// A unit thrown by the player in the Veggie Patch minigame (projectile type 3).
// Milks the raccoons (AVeggiePatchThief) it hits.
// TClass 00708ed0 (parent AProjectileWithUnit, size 0x60), vtable 007203b0.
//-----------------------------------------------------------------------------
class AVeggieThrowProjectile : public AProjectileWithUnit
{
public:
	TDECLARE_CLASS( AVeggieThrowProjectile, AProjectileWithUnit );

public:
	AVeggieThrowProjectile();
	~AVeggieThrowProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual TBOOL  Update( TFLOAT a_fDeltaTime ) OVERRIDE;                             // +0x10
	virtual void   OnLaunched() OVERRIDE;                                              // +0x1c
	virtual TINT   GetType() OVERRIDE;                                                 // +0x28
	virtual TBOOL  CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit ) OVERRIDE; // +0x30
	virtual TFLOAT GetRadius() OVERRIDE { return m_fCollisionRadius; }                 // +0x34, NOTE: folded with other float getters of a field at +0x5c (00572580)

	//-----------------------------------------------------------------------------
	// AProjectileWithUnit
	//-----------------------------------------------------------------------------
	virtual void OnHitUnit() OVERRIDE; // +0x38

private:
	TFLOAT m_fCollisionRadius; // 0x5c
};

TSTATICASSERT( sizeof( AVeggieThrowProjectile ) == 0x60 );
