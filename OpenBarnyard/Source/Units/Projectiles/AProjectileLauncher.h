#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TEvent.h>
#include <Input/TInputInterface.h>
#include <Math/TVector4.h>

#include "Input/AInputMap.h"
#include "Units/AUnitProxy.h"

class AUnit;
class AProjectile;
class ALaunchAmmo;

// Weapon of a unit (the player's milk squirt and aimed squirt): fires projectiles
// of its ALaunchAmmo's type and throws the unit held at the owner's cb_rlocator bone.
class AProjectileLauncher : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AProjectileLauncher, Toshi::TObject );

public:
	AProjectileLauncher();
	~AProjectileLauncher();

	virtual void            Activate();                                                                                                                 // +0x0c, makes the launcher ready to fire
	virtual void            Deactivate();                                                                                                               // +0x10
	virtual TBOOL           ProcessInput( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent, TBOOL a_bUnk1, TBOOL a_bUnk2 ); // +0x14, AInputCommand_Shoot fires, AInputCommand_ShootAlt throws
	virtual TBOOL           Update( TFLOAT a_fDeltaTime );                                                                                              // +0x18
	virtual Toshi::TVector4 GetLaunchVelocity();                                                                                                        // +0x1c, aim velocity plus part of the owner's velocity
	virtual void            OnLaunched( AProjectile* a_pProjectile ) {}                                                                                 // +0x20, DOES_NOTHING
	virtual void            OnThrown( AProjectile* a_pProjectile ) {}                                                                                   // +0x24, DOES_NOTHING
	virtual Toshi::TVector4 GetLaunchPosition();                                                                                                        // +0x28, owner's steer position
	virtual Toshi::TVector4 GetLaunchDirection();                                                                                                       // +0x2c, owner's steer direction
	virtual Toshi::TVector4 GetAimVelocity();                                                                                                           // +0x30, GetLaunchDirection() * m_fLaunchSpeed
	virtual Toshi::TVector4 GetThrowPosition();                                                                                                         // +0x34, owner's cb_rlocator bone
	virtual Toshi::TVector4 GetThrowDirection( AUnit* a_pObject );                                                                                      // +0x38
	virtual TFLOAT          GetThrowSpeed( AUnit* a_pObject );                                                                                          // +0x3c
	virtual Toshi::TVector4 GetThrowVelocity( AUnit* a_pObject );                                                                                       // +0x40
	virtual void            Fire();                                                                                                                     // +0x44
	virtual void            StopFiring() {}                                                                                                             // +0x48, DOES_NOTHING
	virtual TBOOL           Throw();                                                                                                                    // +0x4c, throws the unit held at the owner's cb_rlocator bone
	virtual void            StopThrowing() {}                                                                                                           // +0x50, DOES_NOTHING
	virtual TFLOAT          GetFireInterval() { return m_fFireInterval; }                                                                               // +0x54, NOTE: folded with other float getters of a field at +0x10 (0058ac00)
	virtual void            SetFireInterval( TFLOAT a_fFireInterval ) { m_fFireInterval = a_fFireInterval; }                                            // +0x58, NOTE: folded with other setters of a field at +0x10 (006639a0)
	virtual TBOOL           CanFire();                                                                                                                  // +0x5c
	virtual TBOOL           CanThrow();                                                                                                                 // +0x60
	virtual void            OnFired();                                                                                                                  // +0x64, plays the owner's squirt animation
	virtual void            Unk_0x68() {}                                                                                                               // +0x68, DOES_NOTHING

	// Points m_oOwner at a_pUnit's proxy.
	void SetOwner( AUnit* a_pUnit );

	// Copies the main player profile's gender into m_iGender.
	void UpdateGender();

	// Gender of the player's character, 1 = female (see UpdateGender).
	TINT GetGender() const { return m_iGender; }

	// NOTE: folded with other getters of a pointer at +0x28 (0066b130)
	ALaunchAmmo* GetAmmo() { return m_pAmmo; }

	// Thrown with the TRef<AUnitProxy> of a unit hit by a projectile of the launcher
	Toshi::TGenericEmitter& GetHitEmitter() { return m_oEmitter; }

	// Throws the hit emitter with a_pUnit (a projectile of the launcher has hit it)
	void OnUnitHit( AUnit* a_pUnit );

	void SetAmmo( ALaunchAmmo* a_pAmmo );

	// Ammo of the thrown projectile (type 2 when there is none).
	// NOTE: folded with other getters/setters of a pointer at +0x2c (0066b150 / 0066b160)
	ALaunchAmmo* GetThrowAmmo() { return m_pThrowAmmo; }
	void         SetThrowAmmo( ALaunchAmmo* a_pAmmo ) { m_pThrowAmmo = a_pAmmo; }

	void   SetLaunchSpeed( TFLOAT a_fSpeed ) { m_fLaunchSpeed = a_fSpeed; }
	TFLOAT GetLaunchSpeed() const { return m_fLaunchSpeed; }
	void   SetVerticalAngle( TFLOAT a_fAngle ) { m_fVerticalAngle = a_fAngle; }
	TFLOAT GetVerticalAngle() const { return m_fVerticalAngle; }
	void   SetInfiniteAmmo( TBOOL a_bInfinite ) { m_bInfiniteAmmo = a_bInfinite; }

	AUnit* GetOwnerUnit() { return m_oOwner->GetUnit(); }

	// $Barnyard: FUNCTION 004c4b30
	Toshi::TRef<AUnitProxy> GetOwner() const { return m_oOwner; }

protected:
	// Creates a projectile of the ammo's type and launches it.
	AProjectile* LaunchProjectile();

	// Uses up ammo and sends a_pProjectile off from GetLaunchPosition() with GetLaunchVelocity().
	AProjectile* Launch( AProjectile* a_pProjectile );

	// Throws a_pObject (the held unit) with a_pProjectile from GetThrowPosition() with GetThrowVelocity().
	AProjectile* ThrowUnit( AProjectile* a_pProjectile, AUnit* a_pObject );

	// Detaches the unit held at the owner's cb_rlocator bone and throws it.
	void ThrowHeldUnit();

protected:
	Toshi::TRef<AUnitProxy> m_oOwner;                // 0x04
	TFLOAT                  m_fOwnerVelocityScale;   // 0x08, part of the owner's velocity added to the launch velocity
	TFLOAT                  m_fLaunchSpeed;          // 0x0c, not initialised by the constructor
	TFLOAT                  m_fFireInterval;         // 0x10
	TFLOAT                  m_fFireTimer;            // 0x14, time since the last shot
	TFLOAT                  m_fUnk18;                // 0x18
	TFLOAT                  m_fHorizontalSpeedScale; // 0x1c, scale of the horizontal part of the aim velocity
	TFLOAT                  m_fThrowSpeedScale;      // 0x20
	TINT                    m_iGender;               // 0x24, from the main player profile, 1 = female
	ALaunchAmmo*            m_pAmmo;                 // 0x28
	ALaunchAmmo*            m_pThrowAmmo;            // 0x2c
	TBOOL                   m_bInfiniteAmmo;         // 0x30
	TFLOAT                  m_fVerticalAngle;        // 0x34, radians
	Toshi::TGenericEmitter  m_oEmitter;              // 0x38
};

TSTATICASSERT( sizeof( AProjectileLauncher ) == 0x44 );
