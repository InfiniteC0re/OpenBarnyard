#pragma once
#include "AProjectileLauncher.h"
#include "ALaunchAmmo.h"

class AMilkProjectile;

// The player's milk squirt: fires a short stream of milk projectiles from the
// udder (cb_udder bone) and spawns the stream's particles while it lasts.
// NOTE: AAimedSquirtProjectileLauncher (TClass 0x007cd0c0) derives from this class.
class AMilkSquirtProjectileLauncher : public AProjectileLauncher
{
public:
	TDECLARE_CLASS( AMilkSquirtProjectileLauncher, AProjectileLauncher );

	// State of one of the four particle trails of the stream.
	struct MilkStream
	{
		Toshi::TVector4 vPosition         = Toshi::TVector4::VEC_ZERO; // 0x00, spawn point of the last update
		Toshi::TVector4 vDirection        = Toshi::TVector4::VEC_POSX; // 0x10, launch direction of the last update
		TFLOAT          fSpawnAccumulator = 0.0f;                      // 0x20, particles to spawn (whole part)
	};

public:
	AMilkSquirtProjectileLauncher();
	~AMilkSquirtProjectileLauncher();

	//-----------------------------------------------------------------------------
	// AProjectileLauncher
	//-----------------------------------------------------------------------------
	virtual void            Activate() OVERRIDE;                               // +0x0c
	virtual void            Deactivate() OVERRIDE;                             // +0x10
	virtual TBOOL           Update( TFLOAT a_fDeltaTime ) OVERRIDE;            // +0x18, keeps the stream going and spawns its particles
	virtual void            OnLaunched( AProjectile* a_pProjectile ) OVERRIDE; // +0x20
	virtual void            OnThrown( AProjectile* a_pProjectile ) OVERRIDE;   // +0x24
	virtual Toshi::TVector4 GetLaunchPosition() OVERRIDE;                      // +0x28, owner's cb_udder bone
	virtual Toshi::TVector4 GetLaunchDirection() OVERRIDE;                     // +0x2c, owner's horizontal direction tilted down by m_fVerticalAngle
	virtual void            Fire() OVERRIDE;                                   // +0x44
	virtual TBOOL           CanFire() OVERRIDE;                                // +0x5c, also needs at least one unit of ammo

	// Plays the (gender specific) squirt sound at the udder.
	void PlaySquirtSound();

	// Restarts the stream ids (the Sharp Squirter combos compare them)
	static void ResetLastStreamId() { ms_iLastStreamId = 0; }

private:
	// Starts a new projectile of the stream (giving back the ammo it uses up) and makes it m_pStreamProjectile.
	// NOTE: inlined in retail
	TBOOL ContinueStream();

private:
	static TINT ms_iLastStreamId; // 0x007cd1b4, given to every new stream's projectile (AMilkProjectile::SetStreamId)

private:
	ALaunchAmmo      m_oMilkAmmo;          // 0x44
	TFLOAT           m_fSquirtTimeLeft;    // 0x5c
	AMilkProjectile* m_pStreamProjectile;  // 0x60, owns the particles spawned by Update
	MilkStream       m_aStreams[ 4 ];      // 0x64
	TBOOL            m_bNewStream;         // 0xf4, TTRUE until the stream's first update
	TINT             m_iOutOfMilkEffect;   // 0xf8, "outofmilksquirt"
	TINT             m_iMuzzleFlashEffect; // 0xfc, "milkmussleflash"
};

TSTATICASSERT( sizeof( AMilkSquirtProjectileLauncher::MilkStream ) == 0x24 );
TSTATICASSERT( sizeof( AMilkSquirtProjectileLauncher ) == 0x100 );
