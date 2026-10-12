/**
 * @file TInputDeviceController.h
 * Controller input device interface for the Toshi engine
 * 
 * This file defines the controller input device that provides:
 * - Game controller input handling
 * - Button and axis state management
 * - Controller-specific input mapping
 * - Force feedback support
 */

#pragma once
#include "Input/TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputDeviceController : public TInputDevice
{
public:
	TDECLARE_CLASS( TInputDeviceController, TInputDevice );

public:
	constexpr static TUINT NUM_BUTTONS = 8;

	enum : Doodad
	{
		BUTTON_DPAD_UP            = 0x10001,
		BUTTON_DPAD_RIGHT         = 0x10002,
		BUTTON_DPAD_DOWN          = 0x10003,
		BUTTON_DPAD_LEFT          = 0x10004,
		BUTTON_SELECT             = 0x10005,
		BUTTON_START              = 0x10006,
		BUTTON_STICK_LEFT         = 0x10007,
		BUTTON_STICK_RIGHT        = 0x10008,
		BUTTON_CONTEXT_1          = 0x10009,
		BUTTON_CONTEXT_2          = 0x1000a,
		BUTTON_CONTEXT_3          = 0x1000b,
		BUTTON_CONTEXT_4          = 0x1000c,
		BUTTON_SHOULDER_LEFT_1    = 0x1000d,
		BUTTON_SHOULDER_LEFT_2    = 0x1000e,
		BUTTON_SHOULDER_RIGHT_1   = 0x1000f,
		BUTTON_SHOULDER_RIGHT_2   = 0x10010,
		VIRTBTN_STICK_LEFT_UP     = 0x10011,
		VIRTBTN_STICK_LEFT_RIGHT  = 0x10012,
		VIRTBTN_STICK_LEFT_DOWN   = 0x10013,
		VIRTBTN_STICK_LEFT_LEFT   = 0x10014,
		VIRTBTN_STICK_RIGHT_UP    = 0x10015,
		VIRTBTN_STICK_RIGHT_RIGHT = 0x10016,
		VIRTBTN_STICK_RIGHT_DOWN  = 0x10017,
		VIRTBTN_STICK_RIGHT_LEFT  = 0x10018,
		STICK_LEFT                = 0x10019,
		STICK_RIGHT               = 0x1001a,
		VIRTSTK_DPAD              = 0x1001b,
	};

	enum VIBRATION_ACTUATOR
	{
		VIBRATION_ACTUATOR_0, // the two motors, AInputHandler's positional rumble uses the second one
		VIBRATION_ACTUATOR_1,
	};

public:
	TInputDeviceController();
	~TInputDeviceController();

	virtual TBOOL        GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& a_rProperties ) const override;
	virtual const TCHAR* GetButtonFromDoodad( Doodad a_iDoodad ) const override;
	virtual TBOOL        IsDown( Doodad a_iDoodad ) const override;

	virtual void  SetVibration( VIBRATION_ACTUATOR a_eActuator, TFLOAT a_fValue, TBOOL a_bUnknown ); // +0x60, does nothing on PC (00413930)
	virtual TBOOL WasDown( Doodad a_iDoodad ) const;
	virtual TINT  GetPortIndex() = 0; // +0x68, the DX controller returns -1 (folded stub 004ee130)
	virtual TBOOL Unknown2();
	virtual TINT  ProcessVirtualButtons( EventEmitter& a_rEmitter, float a_fDeltaTime );

protected:
	// Turns the sticks into the virtual direction buttons (0x10011-0x10018) and the D-pad into m_fMag1/m_fMag2
	void UpdateVirtualButtons();

private:
	TBOOL* m_CurrentState; // 0x3C
	TBOOL* m_OldState;     // 0x40
	TFLOAT m_fMag1;        // 0x44
	TFLOAT m_fMag2;        // 0x48
	TFLOAT m_fOldMag1;     // 0x4C
	TFLOAT m_fOldMag2;     // 0x50
};

TOSHI_NAMESPACE_END
