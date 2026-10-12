#pragma once
#include "ACameraHelper.h"
#include "ACameraHelperSpline.h"

#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Purpose: Camera that follows the target and can be rotated, elevated and zoomed
// with the camera commands (ACameraManager::CAMERAHELPER_FOLLOW).
//
// TClass 0x00782450 (size 0x10c, runtime creatable), vtable 0x0071cdc8.
//-----------------------------------------------------------------------------
class AFollowCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( AFollowCameraHelper, ACameraHelper );

public:
	AFollowCameraHelper();
	~AFollowCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                            // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                            // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void* a_pCommand, void* a_pEvent ) override;    // +0x18, processes the camera input commands
	virtual void  OnCreate() override;                                                 // +0x1c
	virtual void  OnDestroy() override;                                                // +0x20
	virtual void  OnAttach() override {}                                               // +0x24, DOES_NOTHING 005acd80
	virtual void  GetCameraRotation( Toshi::TVector4& a_rOutRotation ) const override; // +0x28

	// Attaches the camera to a_pTarget and places it at a_fDistance, a_fPitch above the target and a_fYaw around its facing
	void Setup( Toshi::TRef<AUnitProxy> a_pTarget, ACamera* a_pCamera, TFLOAT a_fPitch, TFLOAT a_fYaw, TFLOAT a_fDistance, TFLOAT a_fZoomSpeed, TFLOAT a_fYawSpeed, TFLOAT a_fPitchSpeed );

private:
	// Moves the camera
	void UpdateCamera( TFLOAT a_fDeltaTime );

	// Rotates the camera up/down by a_fAngle, keeping it between acos( -0.8 ) and acos( -0.2 ) from the down axis
	void Elevate( TFLOAT a_fAngle );

	// Scales the camera distance (relative to a_fAmount), keeping it in the ( 3; 100 ) range
	void Zoom( TFLOAT a_fAmount );

	// Sets the elevation of the camera direction to a_fPitch
	void SetPitch( TFLOAT a_fPitch );

	// Sets the angle between the camera direction and the facing of the target to a_fYaw
	void SetYaw( TFLOAT a_fYaw );

	// Returns the speed of a released rotation that slows down over *a_pTime, resets *a_pTime once it's below 0.1
	static TFLOAT GetReleaseSpeed( TFLOAT a_fSpeed, TFLOAT* a_pTime );

	// Returns the angle (0..2PI) between a_rcVec1 and a_rcVec2 around the Y axis
	static TFLOAT GetAngleAroundY( const Toshi::TVector4& a_rcVec1, const Toshi::TVector4& a_rcVec2 );

private:
	Toshi::TVector4     m_vDirection;             // 0x10
	TFLOAT              m_fYawSpeed;              // 0x20
	TFLOAT              m_fPitchSpeed;            // 0x24
	TFLOAT              m_fZoomSpeed;             // 0x28, not initialised by the ctor
	ACamera*            m_pFollowCamera;          // 0x2c
	TBOOL               m_bKeepRotatingOnRelease; // 0x30
	TBOOL               m_bYawReleased;           // 0x31
	TBOOL               m_bPitchReleased;         // 0x32
	TINT                m_iYaw;                   // 0x34
	TINT                m_iLastYaw;               // 0x38
	TINT                m_iPitch;                 // 0x3c
	TINT                m_iLastPitch;             // 0x40
	TINT                m_iZoom;                  // 0x44
	TINT                m_iUnk48;                 // 0x48
	TBOOL               m_bFixedPosition;         // 0x4c
	Toshi::TVector4     m_vFixedPosition;         // 0x50
	Toshi::TVector2     m_vUnk60;                 // 0x60
	Toshi::TVector2     m_vStickInput;            // 0x68
	Toshi::TVector4     m_vRotation;              // 0x70
	TINT                m_iYawInput;              // 0x80
	TINT                m_iPitchInput;            // 0x84
	TINT                m_iZoomInput;             // 0x88
	ACameraHelperSpline m_oSpline;                // 0x8c
};

TSTATICASSERT( sizeof( AFollowCameraHelper ) == 0x10c );
