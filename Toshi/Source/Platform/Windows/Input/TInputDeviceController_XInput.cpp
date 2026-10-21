#include "ToshiPCH.h"
#include "TInputDeviceController_XInput.h"
#include "TInputDeviceController_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

#ifdef BARNYARD_COMMUNITY_PATCH

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TInputXInputDeviceController );

using t_XInputGetState = DWORD( WINAPI* )( DWORD a_uiUserIndex, XINPUT_STATE* a_pState );
using t_XInputSetState = DWORD( WINAPI* )( DWORD a_uiUserIndex, XINPUT_VIBRATION* a_pVibration );

static HMODULE          s_hXInputModule    = NULL;
static t_XInputGetState s_fnXInputGetState = TNULL;
static t_XInputSetState s_fnXInputSetState = TNULL;

TInputXInputDeviceController::TInputXInputDeviceController()
{
	m_uiUserIndex     = 0;
	m_uiLastRetryTime = 0;
	m_bPolled         = TFALSE;
	TUtil::MemClear( &m_oState, sizeof( m_oState ) );
	TUtil::MemClear( &m_oOldState, sizeof( m_oOldState ) );
	TUtil::MemClear( &m_oPendingState, sizeof( m_oPendingState ) );
	TUtil::MemClear( m_afSticks, sizeof( m_afSticks ) );
	TUtil::MemClear( m_afVibration, sizeof( m_afVibration ) );
}

TInputXInputDeviceController::~TInputXInputDeviceController()
{
}

TBOOL TInputXInputDeviceController::LoadXInput()
{
	if ( s_hXInputModule )
		return TTRUE;

	static const TCHAR* s_aszDLLs[] = { "xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll" };

	for ( TINT i = 0; i < TARRAYSIZE( s_aszDLLs ); i++ )
	{
		HMODULE hModule = LoadLibraryA( s_aszDLLs[ i ] );

		if ( !hModule )
			continue;

		s_fnXInputGetState = (t_XInputGetState)GetProcAddress( hModule, "XInputGetState" );
		s_fnXInputSetState = (t_XInputSetState)GetProcAddress( hModule, "XInputSetState" );

		if ( s_fnXInputGetState && s_fnXInputSetState )
		{
			s_hXInputModule = hModule;
			TINFO( "TInputXInputDeviceController: loaded %s\n", s_aszDLLs[ i ] );
			return TTRUE;
		}

		s_fnXInputGetState = TNULL;
		s_fnXInputSetState = TNULL;
		FreeLibrary( hModule );
	}

	TWARN( "TInputXInputDeviceController: XInput is not available\n" );
	return TFALSE;
}

void TInputXInputDeviceController::UnloadXInput()
{
	if ( s_hXInputModule )
	{
		FreeLibrary( s_hXInputModule );
		s_hXInputModule    = NULL;
		s_fnXInputGetState = TNULL;
		s_fnXInputSetState = TNULL;
	}
}

TBOOL TInputXInputDeviceController::IsXInputLoaded()
{
	return s_hXInputModule != NULL;
}

TBOOL TInputXInputDeviceController::IsUserConnected( DWORD a_uiUserIndex )
{
	if ( !s_fnXInputGetState )
		return TFALSE;

	XINPUT_STATE oState;
	return s_fnXInputGetState( a_uiUserIndex, &oState ) == ERROR_SUCCESS;
}

TBOOL TInputXInputDeviceController::IsXInputDevice( const GUID& a_rcProductGuid )
{
	// The product GUID of DirectInput devices holds MAKELONG( VID, PID ) in Data1.
	// XInput pads are the HID devices with "IG_" in their device path.
	UINT uiNumDevices = 0;

	if ( GetRawInputDeviceList( NULL, &uiNumDevices, sizeof( RAWINPUTDEVICELIST ) ) != 0 || uiNumDevices == 0 )
		return TFALSE;

	RAWINPUTDEVICELIST* pDevices  = new RAWINPUTDEVICELIST[ uiNumDevices ];
	UINT                uiResult  = GetRawInputDeviceList( pDevices, &uiNumDevices, sizeof( RAWINPUTDEVICELIST ) );
	TBOOL               bIsXInput = TFALSE;

	for ( UINT i = 0; i < uiResult && uiResult != UINT( -1 ) && !bIsXInput; i++ )
	{
		if ( pDevices[ i ].dwType != RIM_TYPEHID )
			continue;

		RID_DEVICE_INFO oInfo;
		UINT            uiSize = sizeof( oInfo );
		oInfo.cbSize           = sizeof( oInfo );

		if ( GetRawInputDeviceInfoA( pDevices[ i ].hDevice, RIDI_DEVICEINFO, &oInfo, &uiSize ) == UINT( -1 ) )
			continue;

		if ( MAKELONG( oInfo.hid.dwVendorId, oInfo.hid.dwProductId ) != LONG( a_rcProductGuid.Data1 ) )
			continue;

		TCHAR szName[ 256 ];
		UINT  uiNameSize = TARRAYSIZE( szName );

		if ( GetRawInputDeviceInfoA( pDevices[ i ].hDevice, RIDI_DEVICENAME, szName, &uiNameSize ) == UINT( -1 ) )
			continue;

		szName[ TARRAYSIZE( szName ) - 1 ] = '\0';

		if ( strstr( szName, "IG_" ) || strstr( szName, "ig_" ) )
			bIsXInput = TTRUE;
	}

	delete[] pDevices;
	return bIsXInput;
}

void TInputXInputDeviceController::BindToUserIndex( DWORD a_uiUserIndex )
{
	m_uiUserIndex = a_uiUserIndex;
	m_bPolled     = TFALSE;
}

TBOOL TInputXInputDeviceController::Acquire()
{
	m_bPolled = TFALSE;

	// Don't poll an empty slot every frame, XInputGetState is slow when nothing is connected
	DWORD uiTime = GetTickCount();

	if ( !IsAcquired() && m_uiLastRetryTime != 0 && uiTime - m_uiLastRetryTime < RECONNECT_INTERVAL_MS )
		return TFALSE;

	if ( !s_fnXInputGetState || s_fnXInputGetState( m_uiUserIndex, &m_oPendingState ) != ERROR_SUCCESS )
	{
		m_uiLastRetryTime = uiTime;

		if ( IsAcquired() )
		{
			TInputInterface::DeviceStatusEvent oEvent = { this, TInputInterface::DEVICESTATUS_DISCONNECTED };
			GetInputInterface()->GetDeviceStatusEmitter().Throw( &oEvent );
		}

		SetAcquired( TFALSE );
		return TFALSE;
	}

	if ( !IsAcquired() )
	{
		TInputInterface::DeviceStatusEvent oEvent = { this, TInputInterface::DEVICESTATUS_CONNECTED };
		GetInputInterface()->GetDeviceStatusEmitter().Throw( &oEvent );

		// Don't report the buttons that were held while the pad was away as new presses
		m_oState = m_oPendingState;
	}

	SetAcquired( TTRUE );
	return TTRUE;
}

TBOOL TInputXInputDeviceController::Unacquire()
{
	SetAcquired( TFALSE );
	m_bPolled = TFALSE;

	return TTRUE;
}

void TInputXInputDeviceController::Release()
{
	if ( s_fnXInputSetState && ( m_afVibration[ 0 ] != 0.0f || m_afVibration[ 1 ] != 0.0f ) )
	{
		XINPUT_VIBRATION oVibration = { 0, 0 };
		s_fnXInputSetState( m_uiUserIndex, &oVibration );
	}

	m_afVibration[ 0 ] = 0.0f;
	m_afVibration[ 1 ] = 0.0f;
}

void TInputXInputDeviceController::Update( TFLOAT a_fDeltaTime )
{
	m_oOldState = m_oState;
	m_oState    = m_oPendingState;

	m_afSticks[ 0 ] = TInputDXDeviceController::NormaliseStick( GetDXAxis( STICK_LEFT, 0 ) );
	m_afSticks[ 1 ] = TInputDXDeviceController::NormaliseStick( GetDXAxis( STICK_LEFT, 1 ) );
	m_afSticks[ 2 ] = TInputDXDeviceController::NormaliseStick( GetDXAxis( STICK_RIGHT, 0 ) );
	m_afSticks[ 3 ] = TInputDXDeviceController::NormaliseStick( GetDXAxis( STICK_RIGHT, 1 ) );

	UpdateVirtualButtons();
	m_bPolled = TTRUE;
}

TINT TInputXInputDeviceController::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	if ( !m_bPolled )
		return 0;

	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );
	iNumProcessed += ProcessVirtualButtons( a_rEmitter, a_fDeltaTime );

	// Same buttons in the same order as TInputDXDeviceController::ProcessEvents
	for ( TINT i = 0; i < TInputDXDeviceController::NUM_DX_BUTTONS; i++ )
	{
		Doodad iDoodad  = TInputDXDeviceController::TranslateDXButtonToDoodad( i );
		TBOOL  bIsDown  = IsGamepadDown( m_oState.Gamepad, iDoodad );
		TBOOL  bWasDown = IsGamepadDown( m_oOldState.Gamepad, iDoodad );

		if ( bIsDown && !bWasDown )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, iDoodad, TInputInterface::EVENT_TYPE_GONE_DOWN ) );
			iNumProcessed++;
		}
		else if ( !bIsDown && bWasDown )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, iDoodad, TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed++;
		}
	}

	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_LEFT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afSticks[ 0 ], m_afSticks[ 1 ] ) );
	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_RIGHT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afSticks[ 2 ], m_afSticks[ 3 ] ) );

	return iNumProcessed + 2;
}

TBOOL TInputXInputDeviceController::IsGamepadDown( const XINPUT_GAMEPAD& a_rcGamepad, Doodad a_iDoodad )
{
	// The game expects a PS2 pad: Context1-4 are triangle, circle, cross and square
	switch ( a_iDoodad )
	{
		case BUTTON_DPAD_UP: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP ) != 0;
		case BUTTON_DPAD_RIGHT: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT ) != 0;
		case BUTTON_DPAD_DOWN: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ) != 0;
		case BUTTON_DPAD_LEFT: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ) != 0;
		case BUTTON_SELECT: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_BACK ) != 0;
		case BUTTON_START: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_START ) != 0;
		case BUTTON_STICK_LEFT: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ) != 0;
		case BUTTON_STICK_RIGHT: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ) != 0;
		case BUTTON_CONTEXT_1: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_Y ) != 0;
		case BUTTON_CONTEXT_2: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_B ) != 0;
		case BUTTON_CONTEXT_3: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_A ) != 0;
		case BUTTON_CONTEXT_4: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_X ) != 0;
		case BUTTON_SHOULDER_LEFT_1: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ) != 0;
		case BUTTON_SHOULDER_RIGHT_1: return ( a_rcGamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ) != 0;
		case BUTTON_SHOULDER_LEFT_2: return a_rcGamepad.bLeftTrigger > TRIGGER_THRESHOLD;
		case BUTTON_SHOULDER_RIGHT_2: return a_rcGamepad.bRightTrigger > TRIGGER_THRESHOLD;
		default: return TFALSE;
	}
}

TBOOL TInputXInputDeviceController::IsDown( Doodad a_iDoodad ) const
{
	if ( TInputDXDeviceController::TranslateDoodadToDXButton( a_iDoodad ) < 0 )
		return TInputDeviceController::IsDown( a_iDoodad );

	return IsGamepadDown( m_oState.Gamepad, a_iDoodad );
}

TBOOL TInputXInputDeviceController::WasDown( Doodad a_iDoodad ) const
{
	if ( TInputDXDeviceController::TranslateDoodadToDXButton( a_iDoodad ) < 0 )
		return TInputDeviceController::WasDown( a_iDoodad );

	return IsGamepadDown( m_oOldState.Gamepad, a_iDoodad );
}

TINT TInputXInputDeviceController::GetDXAxis( Doodad a_iDoodad, TINT a_iAxis ) const
{
	// XInput: -32768..32767 with Y up, DirectInput: 0..65535 with Y down
	const XINPUT_GAMEPAD& rcGamepad = m_oState.Gamepad;

	if ( a_iDoodad == STICK_LEFT )
	{
		if ( a_iAxis == 0 )
			return rcGamepad.sThumbLX + STICK_CENTRE;

		if ( a_iAxis == 1 )
			return ( STICK_CENTRE - 1 ) - rcGamepad.sThumbLY;
	}
	else if ( a_iDoodad == STICK_RIGHT )
	{
		if ( a_iAxis == 0 )
			return rcGamepad.sThumbRX + STICK_CENTRE;

		if ( a_iAxis == 1 )
			return ( STICK_CENTRE - 1 ) - rcGamepad.sThumbRY;
	}

	return STICK_CENTRE;
}

TINT TInputXInputDeviceController::GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const
{
	TINT iResult = GetAxisInt2( a_iDoodad, a_iAxis );

	if ( iResult <= STICK_DEADZONE && iResult >= -STICK_DEADZONE )
		return 0;

	return iResult;
}

TINT TInputXInputDeviceController::GetAxisInt2( Doodad a_iDoodad, TINT a_iAxis ) const
{
	if ( ( a_iDoodad != STICK_LEFT && a_iDoodad != STICK_RIGHT ) || ( a_iAxis != 0 && a_iAxis != 1 ) )
		return 0;

	TINT iResult = GetDXAxis( a_iDoodad, a_iAxis ) - STICK_CENTRE;
	TMath::Clip( iResult, -STICK_SATURATE, STICK_SATURATE );

	return iResult;
}

TFLOAT TInputXInputDeviceController::GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const
{
	return GetAxisInt( a_iDoodad, a_iAxis ) * ( 1.0f / STICK_SATURATE );
}

TFLOAT TInputXInputDeviceController::GetAxisFloat2( Doodad a_iDoodad, TINT a_iAxis ) const
{
	return GetAxisInt2( a_iDoodad, a_iAxis ) * ( 1.0f / STICK_SATURATE );
}

void TInputXInputDeviceController::SetVibration( VIBRATION_ACTUATOR a_eActuator, TFLOAT a_fValue, TBOOL a_bUnknown )
{
	if ( !s_fnXInputSetState || ( a_eActuator != VIBRATION_ACTUATOR_0 && a_eActuator != VIBRATION_ACTUATOR_1 ) )
		return;

	TMath::Clip( a_fValue, 0.0f, 1.0f );

	if ( m_afVibration[ a_eActuator ] == a_fValue )
		return;

	m_afVibration[ a_eActuator ] = a_fValue;

	// Actuator 0 is the small motor of the PS2 pad and actuator 1 the large one,
	// XInput has the large (low frequency) motor on the left
	XINPUT_VIBRATION oVibration;
	oVibration.wLeftMotorSpeed  = WORD( m_afVibration[ VIBRATION_ACTUATOR_1 ] * 65535.0f );
	oVibration.wRightMotorSpeed = WORD( m_afVibration[ VIBRATION_ACTUATOR_0 ] * 65535.0f );
	s_fnXInputSetState( m_uiUserIndex, &oVibration );
}

TOSHI_NAMESPACE_END

#endif // BARNYARD_COMMUNITY_PATCH
