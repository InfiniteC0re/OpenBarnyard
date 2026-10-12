#pragma once
#include "AProjectile.h"

// Projectile of the milk squirt (type 0 projectiles and the minigame variants
// derive from it). Owns the AMilkManager particles that visualise its part of
// the stream and splashes once it hits something.
class AMilkProjectile : public AProjectile
{
public:
	TDECLARE_CLASS( AMilkProjectile, AProjectile );

	static constexpr TINT MAX_NUM_PARTICLES = 40;

public:
	AMilkProjectile();
	~AMilkProjectile();

	//-----------------------------------------------------------------------------
	// AProjectile
	//-----------------------------------------------------------------------------
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x10, returns TTRUE once the milk has hit something
	virtual void  Kill() OVERRIDE;                        // +0x14
	virtual void  Reset() OVERRIDE;                       // +0x20
	virtual void  OnHit() OVERRIDE;                       // +0x24, splashes and kills the projectile

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL IsAboutToHitCamera(); // +0x38, flying towards the camera and close to it
	virtual void  SplashCamera();       // +0x3c, flashes the screen white

	// Adds a particle of AMilkManager to the ones owned by this projectile (up to MAX_NUM_PARTICLES).
	void AddParticle( TINT a_iParticle );

	TINT GetNumParticles() const { return m_iNumParticles; }

	void SetStreamId( TINT a_iStreamId ) { m_iStreamId = a_iStreamId; }
	TINT GetStreamId() const { return m_iStreamId; }

protected:
	// Plays the (gender specific) splash sound at the projectile.
	void PlaySplashSound();

	// Gives the particles back to AMilkManager.
	void ReleaseParticles();

protected:
	TINT  m_iUnk50;                          // 0x50, not initialised by the constructor, -1 after Reset
	TINT  m_iNumParticles;                   // 0x54
	TINT  m_aParticles[ MAX_NUM_PARTICLES ]; // 0x58, AMilkManager particle indices
	TBOOL m_bHit;                            // 0xf8
	TBOOL m_bSplashedCamera;                 // 0xf9
	TINT  m_iSplatEffect;                    // 0xfc, "milksplat"
	TINT  m_iSplashEffect;                   // 0x100, "milksplash"
	TINT  m_iStreamId;                       // 0x104, projectiles of the same squirt share it (AMilkSquirtProjectileLauncher)
};

TSTATICASSERT( sizeof( AMilkProjectile ) == 0x108 );
