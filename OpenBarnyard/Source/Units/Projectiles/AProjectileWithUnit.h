#pragma once
#include "AProjectile.h"

// A thrown unit (projectile type 2). While it flies the unit uses an
// AThrowItemSteer and the projectile follows the steer's position.
class AProjectileWithUnit : public AProjectile
{
public:
	TDECLARE_CLASS( AProjectileWithUnit, AProjectile );

public:
	AProjectileWithUnit();
	~AProjectileWithUnit();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;                             // +0x10
	virtual void  Kill() OVERRIDE;                                                    // +0x14, gives the unit its previous steer back
	virtual void  SetVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE;        // +0x18
	virtual void  OnLaunched() OVERRIDE;                                              // +0x1c
	virtual void  Reset() OVERRIDE;                                                   // +0x20
	virtual void  OnHit() OVERRIDE {}                                                 // +0x24, DOES_NOTHING (005acd80)
	virtual TINT  GetType() OVERRIDE { return 2; }                                    // +0x28, NOTE: folded with other functions returning 2 (006d47b0)
	virtual TBOOL CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit ) OVERRIDE; // +0x30
	// NOTE: folded with ABallSteer's getter of the float at +0x58
	// $Barnyard: FUNCTION 0066bdd0
	virtual TFLOAT GetRadius() OVERRIDE { return m_fRadius; } // +0x34

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void OnHitUnit() {} // +0x38, DOES_NOTHING (005acd80)
	virtual void OnLanded() {}  // +0x3c, DOES_NOTHING (005acd80)

	// Makes a_oUnit the thrown unit and gives it an AThrowItemSteer.
	void SetUnit( Toshi::TRef<AUnitProxy> a_oUnit );

	Toshi::TRef<AUnitProxy> GetUnit() const;

private:
	Toshi::TRef<AUnitProxy> m_oUnit;           // 0x50
	Toshi::TClass*          m_pPrevSteerClass; // 0x54, steer the unit had before it was thrown
	TFLOAT                  m_fRadius;         // 0x58, radius of the unit's steer
};

TSTATICASSERT( sizeof( AProjectileWithUnit ) == 0x5c );
