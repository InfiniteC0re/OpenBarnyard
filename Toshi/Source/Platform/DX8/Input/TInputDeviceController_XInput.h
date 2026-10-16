#pragma once
#include "Includes.h"
#include "Input/TInputDeviceController.h"

#ifdef BARNYARD_COMMUNITY_PATCH

#  include <Xinput.h>

TOSHI_NAMESPACE_START

class TInputXInputDeviceController : public TInputDeviceController
{
public:
	TDECLARE_CLASS( TInputXInputDeviceController, TInputDeviceController );

	static constexpr TINT STICK_CENTRE   = 32768;
	static constexpr TINT STICK_DEADZONE = 7500;
	static constexpr TINT STICK_SATURATE = 32000;

	static constexpr TINT TRIGGER_THRESHOLD = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

	static constexpr DWORD RECONNECT_INTERVAL_MS = 1000;

public:
	TInputXInputDeviceController();
	~TInputXInputDeviceController();

	//-----------------------------------------------------------------------------
	// TInputDevice
	//-----------------------------------------------------------------------------
	virtual TBOOL  Acquire() override;
	virtual TBOOL  Unacquire() override;
	virtual void   Release() override;
	virtual void   Update( TFLOAT a_fDeltaTime ) override;
	virtual TBOOL  Flush() override { return TTRUE; }
	virtual TINT   ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) override;
	virtual TINT   GetButtonCount() const override { return 12; }
	virtual TINT   GetAxisCount() const override { return 2; }
	virtual TBOOL  IsForceFeedbackDevice() override { return TTRUE; }
	virtual TBOOL  IsDown( Doodad a_iDoodad ) const override;
	virtual TINT   GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TINT   GetAxisInt2( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TFLOAT GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TFLOAT GetAxisFloat2( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TBOOL  IsEnabled() const override { return TTRUE; }

	//-----------------------------------------------------------------------------
	// TInputDeviceController
	//-----------------------------------------------------------------------------
	virtual void  SetVibration( VIBRATION_ACTUATOR a_eActuator, TFLOAT a_fValue, TBOOL a_bUnknown ) override;
	virtual TBOOL WasDown( Doodad a_iDoodad ) const override;
	virtual TINT  GetPortIndex() override { return TINT( m_uiUserIndex ); }

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	void  BindToUserIndex( DWORD a_uiUserIndex );
	DWORD GetUserIndex() const { return m_uiUserIndex; }

	// Loads XInput, returns TFALSE if none is available
	static TBOOL LoadXInput();
	static void  UnloadXInput();
	static TBOOL IsXInputLoaded();

	// Returns TTRUE if the user index has a pad connected
	static TBOOL IsUserConnected( DWORD a_uiUserIndex );

	// Returns TTRUE if a DirectInput device (by its product GUID) is an XInput pad
	static TBOOL IsXInputDevice( const GUID& a_rcProductGuid );

private:
	static TBOOL IsGamepadDown( const XINPUT_GAMEPAD& a_rcGamepad, Doodad a_iDoodad );

	// Returns the DirectInput equivalent (0..65535, Y down) of an axis
	TINT GetDXAxis( Doodad a_iDoodad, TINT a_iAxis ) const;

private:
	DWORD        m_uiUserIndex;
	XINPUT_STATE m_oState;
	XINPUT_STATE m_oOldState;
	XINPUT_STATE m_oPendingState; // read by Acquire, used by the next Update
	TFLOAT       m_afSticks[ 4 ]; // normalised X/Y of the left stick and X/Y of the right stick
	TFLOAT       m_afVibration[ 2 ];
	DWORD        m_uiLastRetryTime;
	TBOOL        m_bPolled;
};

TOSHI_NAMESPACE_END

#endif // BARNYARD_COMMUNITY_PATCH
