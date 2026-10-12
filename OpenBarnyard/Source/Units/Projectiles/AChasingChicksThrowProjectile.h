#pragma once
#include "AProjectileWithUnit.h"
#include "MiniGame/AChasingChicksGame.h"

//-----------------------------------------------------------------------------
// A chick thrown by a player into its pen in the Chasing Chicks minigame
// (projectile type 4). Doesn't hit anything, it only follows the chick.
// TClass 00709430 (parent AProjectileWithUnit, size 0x64), vtable 00722578.
//-----------------------------------------------------------------------------
class AChasingChicksThrowProjectile
    : public AProjectileWithUnit
{
public:
	TDECLARE_CLASS( AChasingChicksThrowProjectile, AProjectileWithUnit );

public:
	AChasingChicksThrowProjectile();
	~AChasingChicksThrowProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual TBOOL  Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                               // +0x10
	virtual void   Kill() OVERRIDE;                                                                      // +0x14
	virtual void   OnLaunched() OVERRIDE;                                                                // +0x1c
	virtual TINT   GetType() OVERRIDE;                                                                   // +0x28
	virtual TBOOL  CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit ) OVERRIDE { return TFALSE; } // +0x30, NOTE: folded with other functions returning TFALSE (006ce000)
	virtual TFLOAT GetRadius() OVERRIDE { return m_fCollisionRadius; }                                   // +0x34, NOTE: folded with other float getters of a field at +0x60 (0061dea0)

private:
	TINT   m_iPlayer;          // 0x5c, index of the player (AChasingChicksGame::aPlayers) who threw the chick, -1
	TFLOAT m_fCollisionRadius; // 0x60, of the chick's AUnitCollisionComponentData
};

TSTATICASSERT( sizeof( AChasingChicksThrowProjectile ) == 0x64 );
