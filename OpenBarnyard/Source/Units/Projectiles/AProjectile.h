#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>

#include "Units/AUnitProxy.h"

class AProjectileLauncher;

// A pooled projectile of ASimpleProjectileManager: a point moving along a
// ballistic trajectory until it hits something or runs out of lifetime.
// Projectiles are fetched with ASimpleProjectileManager::GetProjectile and
// given back to the pool by Kill.
class AProjectile : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AProjectile, Toshi::TObject );

public:
	AProjectile();
	~AProjectile();

	// Velocity that throws a projectile from a_rcOrigin to a_rcTarget (gravity 9.8), faster for further targets (a_fSpeed scales it)
	static void CalculateThrowVelocity( const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcOrigin, TFLOAT a_fSpeed, Toshi::TVector4& a_rOutVelocity );

	virtual void  Restart();                                                 // +0x0c, restarts the age and the trajectory's previous position
	virtual TBOOL Update( TFLOAT a_fDeltaTime );                             // +0x10, moves the projectile, returns TTRUE once it has hit something
	virtual void  Kill();                                                    // +0x14, gives the projectile back to ASimpleProjectileManager
	virtual void  SetVelocity( const Toshi::TVector4& a_rcVelocity );        // +0x18
	virtual void  OnLaunched() {}                                            // +0x1c, DOES_NOTHING (005acd80), called by the launcher once it's on its way
	virtual void  Reset();                                                   // +0x20, called when the projectile is taken from or given back to the pool
	virtual void  OnHit() {}                                                 // +0x24, DOES_NOTHING (005acd80)
	virtual TINT  GetType() { return 0; }                                    // +0x28, RETURNS_ZERO (0054d8b0), index of the pool in ASimpleProjectileManager
	virtual TBOOL CheckWorldCollision();                                     // +0x2c
	virtual TBOOL CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit ); // +0x30, stores the unit that was hit in a_rHitUnit
	// $Barnyard: FUNCTION 004c4a10
	virtual TFLOAT GetRadius() { return 0.5f; } // +0x34

	// Seconds since the projectile was launched.
	// NOTE: folded with other float getters of a field at +0xc (0066a1d0)
	TFLOAT GetAge() const { return m_fAge; }

	// The projectile is killed once it's older than this (<= 0: never).
	// NOTE: folded with other setters of a pointer at +0x8 (0066a1c0)
	void SetLifetime( TFLOAT a_fLifetime ) { m_fLifetime = a_fLifetime; }

	// NOTE: folded with other setters of a pointer at +0x4 (0066a1e0)
	void                 SetLauncher( AProjectileLauncher* a_pLauncher ) { m_pLauncher = a_pLauncher; }
	AProjectileLauncher* GetLauncher() const { return m_pLauncher; }

	// NOTE: folded with AStationarySteer::SetDirection (00570490)
	void SetPosition( const Toshi::TVector4& a_rcPosition ) { m_vPosition = a_rcPosition; }

	// NOTE: folded with other getters of a vector at +0x30 (00577260)
	const Toshi::TVector4& GetPosition() const { return m_vPosition; }

	// NOTE: folded with other getters of a vector at +0x10 (006d8130)
	const Toshi::TVector4& GetVelocity() const { return m_vVelocity; }

	// Normalised velocity.
	Toshi::TVector4 GetDirection() const;

	// Index in ASimpleProjectileManager's pool, -1 when the projectile is free.
	// NOTE: folded with other getters/setters of a field at +0x4c (0066a280 / 0066a270)
	TINT GetPoolIndex() const { return m_iPoolIndex; }

	// Gravity of the trajectories computed by AProjectile::CalculateThrowVelocity (minigames change it)
	// $Barnyard: FUNCTION 0066a900
	static void SetTrajectoryGravity( TFLOAT a_fGravity ) { ms_fTrajectoryGravity = a_fGravity; }

	// $Barnyard: FUNCTION 0066a910
	static void ResetTrajectoryGravity() { ms_fTrajectoryGravity = 9.8f; }

	inline static TFLOAT ms_fTrajectoryGravity = 9.8f; // 00779a14
	void                 SetPoolIndex( TINT a_iIndex ) { m_iPoolIndex = a_iIndex; }

protected:
	AProjectileLauncher*    m_pLauncher;       // 0x04
	TFLOAT                  m_fLifetime;       // 0x08
	TFLOAT                  m_fAge;            // 0x0c
	Toshi::TVector4         m_vVelocity;       // 0x10
	Toshi::TVector4         m_vPrevPosition;   // 0x20, position before the last update
	Toshi::TVector4         m_vPosition;       // 0x30
	TBOOL                   m_bUnk40;          // 0x40
	Toshi::TRef<AUnitProxy> m_oHitUnit;        // 0x44, filled by CheckUnitCollision
	TUINT                   m_uiCollisionMask; // 0x48, passed to the world collision test
	TINT                    m_iPoolIndex;      // 0x4c
};

TSTATICASSERT( sizeof( AProjectile ) == 0x50 );
