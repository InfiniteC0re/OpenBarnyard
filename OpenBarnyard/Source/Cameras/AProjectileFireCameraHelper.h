#pragma once
#include "ACameraHelper.h"

#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Purpose: Over the shoulder camera used while aiming a projectile launcher
// (ACameraManager::CAMERAHELPER_PROJECTILE_FIRE and SPLITSCREEN_CAMERAHELPER_PROJECTILE_1..4).
//
// TClass 0x00782578 (size 0x4c), vtable 0x0071d088.
//-----------------------------------------------------------------------------
class AProjectileFireCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( AProjectileFireCameraHelper, ACameraHelper );

public:
	AProjectileFireCameraHelper();
	~AProjectileFireCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                       // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                       // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void*, void* ) override { return TFALSE; } // +0x18, RETURNS_FALSE 00556a00
	virtual void  OnCreate() override;                                            // +0x1c
	virtual void  OnDestroy() override {}                                         // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override {}                                          // +0x24, DOES_NOTHING 005acd80

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	void SetSpeed( TFLOAT a_fSpeed ) { m_fSpeed = a_fSpeed; }

	void                   SetAim( const Toshi::TVector2& a_rcAim ) { m_vAim = a_rcAim; }
	const Toshi::TVector2& GetAim() const { return m_vAim; }

	TBOOL IsLookingAtTarget() const { return m_bLookingAtTarget; }

private:
	// Computes where the camera goes and where it looks
	void GetCameraTransform( Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection, TFLOAT a_fDeltaTime );

	// Returns the XZ distance the target moved since m_vLastTargetPos was stored (0 without a target)
	TFLOAT GetTargetMovedDistanceXZ() const;

private:
	TFLOAT          m_fUnk10;           // 0x10, not initialised by the ctor
	TFLOAT          m_fUnk14;           // 0x14
	Toshi::TVector4 m_vLastTargetPos;   // 0x18
	Toshi::TVector2 m_vAim;             // 0x28, x - yaw, y - pitch (scaled by PI/2)
	TFLOAT          m_fSpeed;           // 0x30
	TBOOL           m_bLookingAtTarget; // 0x34
	TFLOAT          m_fDistance;        // 0x38
	TFLOAT          m_fSideOffset;      // 0x3c
	TFLOAT          m_fHeight;          // 0x40
	TFLOAT          m_fTargetHeight;    // 0x44
	TUINT           m_uiCollisionMask;  // 0x48
};

TSTATICASSERT( sizeof( AProjectileFireCameraHelper ) == 0x4c );
