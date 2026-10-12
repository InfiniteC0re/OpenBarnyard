#pragma once
#include "ACameraHelper.h"

//-----------------------------------------------------------------------------
// Purpose: Camera of the car chase minigames, one per player
// (ACameraManager::SPLITSCREEN_CAMERAHELPER_CAR_CHASE_1..4).
//
// TClass 0x007823e8 (size 0x200), vtable 0x0071cce0.
//-----------------------------------------------------------------------------
class ACarChaseCamera
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( ACarChaseCamera, ACameraHelper );

public:
	ACarChaseCamera();
	~ACarChaseCamera();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                       // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                       // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void*, void* ) override { return TFALSE; } // +0x18, RETURNS_FALSE 00556a00
	virtual void  OnCreate() override {}                                          // +0x1c, DOES_NOTHING 005acd80
	virtual void  OnDestroy() override {}                                         // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override {}                                          // +0x24, DOES_NOTHING 005acd80

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void CarChaseCamera_Unknown() {} // +0x44, DOES_NOTHING 005acd80

private:
	// Converts a_vOffset (x right, y up, z along the heading of the car) to world space
	Toshi::TVector4 TransformOffset( Toshi::TVector4 a_vOffset );

private:
	static constexpr TINT NUM_VIEWS = 9;

	Toshi::TVector4 m_avPositionOffsets[ NUM_VIEWS ]; // 0x10, of the camera per view, not initialised by the ctor
	Toshi::TVector4 m_avTargetOffsets[ NUM_VIEWS ];   // 0xa0, of the look-at point per view, not initialised by the ctor
	Toshi::TVector4 m_vUnk130;                        // 0x130, the heading of the car is m_vUnk130 - m_vUnk140
	Toshi::TVector4 m_vUnk140;                        // 0x140
	Toshi::TVector4 m_vCarPosition;                   // 0x150
	TUINT8          m_aUnk160[ 0x24 ];                // 0x160
	TINT            m_iViewIndex;                     // 0x184
	TINT            m_iUnk188;                        // 0x188
	TFLOAT          m_afHoldDistancesSq[ NUM_VIEWS ]; // 0x18c, camera stays in place while the car is closer than this
	TFLOAT          m_fOffsetSpeed;                   // 0x1b0
	TFLOAT          m_fCameraSpeed;                   // 0x1b4
	TBOOL           m_bHolding;                       // 0x1b8, camera stays in place and only turns to the car
	Toshi::TVector4 m_vPositionOffset;                // 0x1bc, smoothed m_avPositionOffsets[ m_iViewIndex ]
	Toshi::TVector4 m_vTargetOffset;                  // 0x1cc, smoothed m_avTargetOffsets[ m_iViewIndex ]
	Toshi::TVector4 m_vCameraTarget;                  // 0x1dc
	Toshi::TVector4 m_vCameraPosition;                // 0x1ec
	TINT            m_iLastViewIndex;                 // 0x1fc
};

TSTATICASSERT( sizeof( ACarChaseCamera ) == 0x200 );
