#pragma once
#include "ACameraHelper.h"

//-----------------------------------------------------------------------------
// Purpose: Camera that stays where it is and turns to look at the target
// (ACameraManager::CAMERAHELPER_TRACKING).
//
// TClass 0x0078260c (size 0x2c), vtable 0x0071d358.
//-----------------------------------------------------------------------------
class ATrackingCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( ATrackingCameraHelper, ACameraHelper );

public:
	ATrackingCameraHelper();
	~ATrackingCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                       // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                       // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void*, void* ) override { return TFALSE; } // +0x18, RETURNS_FALSE 00556a00
	virtual void  OnCreate() override {}                                          // +0x1c, DOES_NOTHING 005acd80
	virtual void  OnDestroy() override;                                           // +0x20
	virtual void  OnAttach() override {}                                          // +0x24, DOES_NOTHING 005acd80
	virtual void  SetTarget( Toshi::TRef<AUnitProxy> a_pTarget ) override;        // +0x30

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	void SetSmoothTracking( TBOOL a_bSmooth ) { m_bSmooth = a_bSmooth; }
	void SetSpeed( TFLOAT a_fSpeed ) { m_fSpeed = a_fSpeed; }
	void SetHeightOffset( TFLOAT a_fHeightOffset ) { m_fHeightOffset = a_fHeightOffset; }

private:
	TBOOL           m_bSmooth;       // 0x10, the look at point follows the target smoothly
	Toshi::TVector4 m_vLookAt;       // 0x14
	TFLOAT          m_fSpeed;        // 0x24
	TFLOAT          m_fHeightOffset; // 0x28
};

TSTATICASSERT( sizeof( ATrackingCameraHelper ) == 0x2c );
