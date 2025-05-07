/**
 * @file TInputDeviceMouse.h
 * Mouse input device for the Toshi engine
 * 
 * This file defines the mouse input device that provides:
 * - Mouse position tracking
 * - Button state management
 * - Mouse wheel handling
 * - Mouse event processing
 */

#pragma once
#include "TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputDeviceMouse : public TInputDevice
{
public:
	TDECLARE_CLASS( TInputDeviceMouse, TInputDevice );

public:
	enum : Doodad
	{
		BUTTON_1              = 0x30001,
		BUTTON_2              = 0x30002,
		BUTTON_3              = 0x30003,
		BUTTON_4              = 0x30004,
		BUTTON_5              = 0x30005,
		BUTTON_6              = 0x30006,
		BUTTON_7              = 0x30007,
		BUTTON_8              = 0x30008,
		BUTTON_WHEEL_FORWARD  = 0x30009,
		BUTTON_WHEEL_BACKWARD = 0x3000A,
		AXIS_CURSOR           = 0x3000B,
		AXIS_WHEEL            = 0x3000C,
	};

public:
	TInputDeviceMouse();
	~TInputDeviceMouse();

	virtual TBOOL        GetDoodadProperties( Doodad a_iDoodad, TInputDevice::DoodadProperties& props ) const override;
	virtual const TCHAR* GetButtonFromDoodad( Doodad a_iDoodad ) const override;
};

TOSHI_NAMESPACE_END
