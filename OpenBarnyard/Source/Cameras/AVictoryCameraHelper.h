#pragma once
#include "ACameraHelper.h"

//-----------------------------------------------------------------------------
// Purpose: Camera that circles around the target (ACameraManager::CAMERAHELPER_VICTORY).
//
// TClass 0x0078263c (size 0x3c), vtable 0x0071d3b8.
//-----------------------------------------------------------------------------
class AVictoryCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( AVictoryCameraHelper, ACameraHelper );

public:
	AVictoryCameraHelper();
	~AVictoryCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                         // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                         // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void* a_pCommand, void* a_pEvent ) override; // +0x18
	virtual void  OnCreate() override;                                              // +0x1c
	virtual void  OnDestroy() override {}                                           // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override {}                                            // +0x24, DOES_NOTHING 005acd80

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void VictoryHelper_Unknown() {} // +0x44, DOES_NOTHING 005acd80

private:
	Toshi::TVector4 m_vUnk10;    // 0x10
	Toshi::TVector4 m_vPosition; // 0x20, where the camera goes
	TFLOAT          m_fSpeed;    // 0x30
	TFLOAT          m_fUnk34;    // 0x34
	TFLOAT          m_fAngle;    // 0x38, degrees
};

TSTATICASSERT( sizeof( AVictoryCameraHelper ) == 0x3c );
