#pragma once
#include "ACameraHelper.h"

//-----------------------------------------------------------------------------
// Purpose: Camera behind and above the target looking at it (ACameraManager::CAMERAHELPER_GOLF).
//
// TClass 0x00782480 (size 0x40), vtable 0x0071ce28.
//-----------------------------------------------------------------------------
class AGolfCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( AGolfCameraHelper, ACameraHelper );

	// 0x00773800
	static constexpr TFLOAT DEFAULT_LOOKAT_HEIGHT = 2.0f;
	static constexpr TFLOAT DEFAULT_DISTANCE      = 5.0f;
	static constexpr TFLOAT DEFAULT_HEIGHT        = 4.0f;

public:
	AGolfCameraHelper();
	~AGolfCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                       // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                       // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void*, void* ) override { return TFALSE; } // +0x18, RETURNS_FALSE 00556a00
	virtual void  OnCreate() override;                                            // +0x1c
	virtual void  OnDestroy() override {}                                         // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override;                                            // +0x24

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void GolfHelper_Unknown() {} // +0x44, DOES_NOTHING 005acd80

private:
	Toshi::TVector4 m_vPosition;     // 0x10, where the camera goes
	Toshi::TVector4 m_vLookAt;       // 0x20
	TFLOAT          m_fSpeed;        // 0x30
	TFLOAT          m_fLookAtHeight; // 0x34
	TFLOAT          m_fDistance;     // 0x38
	TFLOAT          m_fHeight;       // 0x3c
};

TSTATICASSERT( sizeof( AGolfCameraHelper ) == 0x40 );
