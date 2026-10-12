#pragma once
#include <Toshi/TObject.h>

class AProjectile;

// Ammunition of an AProjectileLauncher: which projectile type is fired and a
// counter that recharges over time (AMilkComponent feeds the player's values).
class ALaunchAmmo : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ALaunchAmmo, Toshi::TObject );

public:
	ALaunchAmmo();
	~ALaunchAmmo();

	virtual void OnLaunch( AProjectile* a_pProjectile ) {}         // +0x0c, DOES_NOTHING, called by AProjectileLauncher::Launch
	virtual TINT GetProjectileType() { return m_iProjectileType; } // +0x10, NOTE: folded with other getters of a field at +0x4 (006d9d10)

	// Recharges the ammo by a_fDeltaTime * m_fRechargeRate, up to m_iMaxAmmo.
	void Update( TFLOAT a_fDeltaTime );

	// Sets the ammo count, clamped to m_iMaxAmmo.
	void SetAmmo( TINT a_iAmmo );

	// Adds one unit of ammo, clamped to m_iMaxAmmo.
	void AddAmmo();

	// Removes one unit of ammo, clamped to 0.
	void UseAmmo();

	TFLOAT GetAmmo() const { return m_fAmmo; }
	TINT   GetMaxAmmo() const { return m_iMaxAmmo; }
	void   SetMaxAmmo( TINT a_iMaxAmmo ) { m_iMaxAmmo = a_iMaxAmmo; }
	TFLOAT GetRechargeRate() const { return m_fRechargeRate; }
	void   SetRechargeRate( TFLOAT a_fRechargeRate ) { m_fRechargeRate = a_fRechargeRate; }
	void   SetProjectileType( TINT a_iType ) { m_iProjectileType = a_iType; }

private:
	TINT   m_iProjectileType; // 0x04, index passed to ASimpleProjectileManager (ASimpleProjectileManager::GetProjectile)
	TINT   m_iMaxAmmo;        // 0x08
	TFLOAT m_fRechargeRate;   // 0x0c, ammo per second
	TFLOAT m_fAmmo;           // 0x10
	TFLOAT m_fUnk14;          // 0x14
};

TSTATICASSERT( sizeof( ALaunchAmmo ) == 0x18 );
