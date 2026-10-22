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

class MODAPI TInputDeviceMouse : public TInputDevice
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

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	// Development: the buttons -devkeys holds down count as down (IsDown), so their repeats are thrown
	static void Dev_SetButtonDown( Doodad a_iDoodad, TBOOL a_bDown )
	{
		if ( a_iDoodad >= BUTTON_1 && a_iDoodad <= BUTTON_8 )
		{
			TUINT32 uiBit = 1U << ( a_iDoodad - BUTTON_1 );
			ms_uiDevButtons = a_bDown ? ( ms_uiDevButtons | uiBit ) : ( ms_uiDevButtons & ~uiBit );
		}
	}

	static TBOOL Dev_IsButtonDown( Doodad a_iDoodad ) { return a_iDoodad >= BUTTON_1 && a_iDoodad <= BUTTON_8 && ( ms_uiDevButtons & ( 1U << ( a_iDoodad - BUTTON_1 ) ) ); }
	static TBOOL Dev_HasButtonsDown() { return ms_uiDevButtons != 0; }

	// The repeats of the held buttons while the device isn't acquired (unfocused runs)
	TINT Dev_ProcessRepeats( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) { return ProcessRepeats( a_rEmitter, a_fDeltaTime ); }

protected:
	inline static TUINT32 ms_uiDevButtons = 0;
#endif
};

TOSHI_NAMESPACE_END
