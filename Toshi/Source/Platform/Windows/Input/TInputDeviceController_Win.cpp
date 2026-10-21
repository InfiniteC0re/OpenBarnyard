#include "ToshiPCH.h"
#include "TInputDeviceController_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: GetClass 006d4560, CreateObject 006d4940, CreateObjectInPlace 006d49a0
TDEFINE_CLASS_NORUNTIME( TInputDXDeviceController );

// $Barnyard: FUNCTION 006d4570
TInputDXDeviceController::TInputDXDeviceController()
{
	m_poDXInputDevice = TNULL;
	TUtil::MemClear( &m_oJoyState, sizeof( m_oJoyState ) );
	TUtil::MemClear( &m_oOldJoyState, sizeof( m_oOldJoyState ) );
	m_bPolled = TFALSE;
}

// $Barnyard: FUNCTION 006d49f0
TInputDXDeviceController::~TInputDXDeviceController()
{
}

// $Barnyard: FUNCTION 006d4aa0
TBOOL TInputDXDeviceController::Acquire()
{
	m_bPolled = TFALSE;

	if ( FAILED( m_poDXInputDevice->Acquire() ) )
	{
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
	}

	SetAcquired( TTRUE );
	return TTRUE;
}

// $Barnyard: FUNCTION 006d4b20
TBOOL TInputDXDeviceController::Unacquire()
{
	SetAcquired( TFALSE );
	m_bPolled = TFALSE;

	return SUCCEEDED( m_poDXInputDevice->Unacquire() );
}

// $Barnyard: FUNCTION 006d45c0
void TInputDXDeviceController::Release()
{
	if ( m_poDXInputDevice )
	{
		m_poDXInputDevice->Release();
		m_poDXInputDevice = TNULL;
	}
}

// $Barnyard: FUNCTION 006d4cf0
void TInputDXDeviceController::Update( TFLOAT a_fDeltaTime )
{
	if ( FAILED( m_poDXInputDevice->Poll() ) )
		return;

	m_oOldJoyState = m_oJoyState;

	if ( FAILED( m_poDXInputDevice->GetDeviceState( sizeof( DIJOYSTATE ), &m_oJoyState ) ) )
		return;

	m_afSticks[ 0 ] = NormaliseStick( m_oJoyState.lX );
	m_afSticks[ 1 ] = NormaliseStick( m_oJoyState.lY );
	m_afSticks[ 2 ] = NormaliseStick( m_oJoyState.lZ );
	m_afSticks[ 3 ] = NormaliseStick( m_oJoyState.lRz );

	UpdateVirtualButtons();
	m_bPolled = TTRUE;
}

// $Barnyard: FUNCTION 006d4be0
TINT TInputDXDeviceController::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	if ( !m_bPolled )
		return 0;

	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );
	iNumProcessed += ProcessVirtualButtons( a_rEmitter, a_fDeltaTime );

	for ( TINT i = 0; i < NUM_DX_BUTTONS; i++ )
	{
		if ( ( m_oJoyState.rgbButtons[ i ] & 0x80 ) && !( m_oOldJoyState.rgbButtons[ i ] & 0x80 ) )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, TranslateDXButtonToDoodad( i ), TInputInterface::EVENT_TYPE_GONE_DOWN ) );
			iNumProcessed++;
		}
		else if ( !( m_oJoyState.rgbButtons[ i ] & 0x80 ) && ( m_oOldJoyState.rgbButtons[ i ] & 0x80 ) )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, TranslateDXButtonToDoodad( i ), TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed++;
		}
	}

	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_LEFT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afSticks[ 0 ], m_afSticks[ 1 ] ) );
	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_RIGHT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afSticks[ 2 ], m_afSticks[ 3 ] ) );

	return iNumProcessed + 2;
}

// $Barnyard: FUNCTION 006d47c0
TBOOL TInputDXDeviceController::IsDown( Doodad a_iDoodad ) const
{
	TINT iButton = TranslateDoodadToDXButton( a_iDoodad );

	if ( iButton < 0 )
		return TInputDeviceController::IsDown( a_iDoodad );

	return m_oJoyState.rgbButtons[ iButton ] >> 7;
}

// $Barnyard: FUNCTION 006d47f0
TBOOL TInputDXDeviceController::WasDown( Doodad a_iDoodad ) const
{
	TINT iButton = TranslateDoodadToDXButton( a_iDoodad );

	if ( iButton < 0 )
		return TInputDeviceController::WasDown( a_iDoodad );

	return m_oOldJoyState.rgbButtons[ iButton ] >> 7;
}

// $Barnyard: FUNCTION 006d4820
TINT TInputDXDeviceController::GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const
{
	TINT iValue;

	if ( a_iDoodad == STICK_LEFT )
	{
		if ( a_iAxis == 0 )
			iValue = m_oJoyState.lX;
		else if ( a_iAxis == 1 )
			iValue = m_oJoyState.lY;
		else
			return 0;
	}
	else if ( a_iDoodad == STICK_RIGHT )
	{
		if ( a_iAxis == 0 )
			iValue = m_oJoyState.lZ;
		else if ( a_iAxis == 1 )
			iValue = m_oJoyState.lRz;
		else
			return 0;
	}
	else
	{
		return 0;
	}

	// NOTE: retail centres the axes on STICK_SATURATE, not STICK_CENTRE
	TINT iResult = iValue - STICK_SATURATE;

	if ( iResult <= STICK_DEADZONE && iResult >= -STICK_DEADZONE )
		return 0;

	return iResult;
}

// $Barnyard: FUNCTION 006d48b0
TINT TInputDXDeviceController::GetAxisInt2( Doodad a_iDoodad, TINT a_iAxis ) const
{
	if ( a_iDoodad == STICK_LEFT )
	{
		if ( a_iAxis == 0 )
			return m_oJoyState.lX - STICK_SATURATE;

		if ( a_iAxis == 1 )
			return m_oJoyState.lY - STICK_SATURATE;
	}
	else if ( a_iDoodad == STICK_RIGHT )
	{
		if ( a_iAxis == 0 )
			return m_oJoyState.lZ - STICK_SATURATE;

		if ( a_iAxis == 1 )
			return m_oJoyState.lRz - STICK_SATURATE;
	}

	return 0;
}

// $Barnyard: FUNCTION 006d4890
TFLOAT TInputDXDeviceController::GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const
{
	return GetAxisInt( a_iDoodad, a_iAxis ) * ( 1.0f / STICK_SATURATE );
}

// $Barnyard: FUNCTION 006d4920
TFLOAT TInputDXDeviceController::GetAxisFloat2( Doodad a_iDoodad, TINT a_iAxis ) const
{
	return GetAxisInt2( a_iDoodad, a_iAxis ) * ( 1.0f / STICK_SATURATE );
}

// $Barnyard: FUNCTION 006d4a20
TBOOL const TInputDXDeviceController::BindToDIDevice( HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice )
{
	Initialise();

	TUtil::MemCopy( &m_oDeviceInstance, a_poDeviceInstance, sizeof( DIDEVICEINSTANCEA ) );
	m_poDXInputDevice = a_poDXInputDevice;

	m_oDIDevCaps.dwSize = sizeof( DIDEVCAPS );
	a_poDXInputDevice->GetCapabilities( &m_oDIDevCaps );

	if ( m_poDXInputDevice->SetDataFormat( &c_dfDIJoystick ) != DI_OK )
		return TFALSE;

	m_poDXInputDevice->EnumObjects( EnumerateObjectCallback, this, DIDFT_ALL );
	m_bPolled = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 006d45e0
TInputDevice::Doodad TInputDXDeviceController::TranslateDXButtonToDoodad( TINT a_iButton )
{
	switch ( a_iButton )
	{
		case 0: return BUTTON_CONTEXT_1;
		case 1: return BUTTON_CONTEXT_2;
		case 2: return BUTTON_CONTEXT_3;
		case 3: return BUTTON_CONTEXT_4;
		case 4: return BUTTON_SHOULDER_LEFT_2;
		case 5: return BUTTON_SHOULDER_RIGHT_2;
		case 6: return BUTTON_SHOULDER_LEFT_1;
		case 7: return BUTTON_SHOULDER_RIGHT_1;
		case 8: return BUTTON_SELECT;
		case 9: return BUTTON_STICK_LEFT;
		case 10: return BUTTON_STICK_RIGHT;
		case 11: return BUTTON_START;
		case 12: return BUTTON_DPAD_UP;
		case 13: return BUTTON_DPAD_RIGHT;
		case 14: return BUTTON_DPAD_DOWN;
		case 15: return BUTTON_DPAD_LEFT;
		default: return 0;
	}
}

// $Barnyard: FUNCTION 006d46c0
TINT TInputDXDeviceController::TranslateDoodadToDXButton( Doodad a_iDoodad )
{
	switch ( a_iDoodad )
	{
		case BUTTON_DPAD_UP: return 12;
		case BUTTON_DPAD_RIGHT: return 13;
		case BUTTON_DPAD_DOWN: return 14;
		case BUTTON_DPAD_LEFT: return 15;
		case BUTTON_SELECT: return 8;
		case BUTTON_START: return 11;
		case BUTTON_STICK_LEFT: return 9;
		case BUTTON_STICK_RIGHT: return 10;
		case BUTTON_CONTEXT_1: return 0;
		case BUTTON_CONTEXT_2: return 1;
		case BUTTON_CONTEXT_3: return 2;
		case BUTTON_CONTEXT_4: return 3;
		case BUTTON_SHOULDER_LEFT_1: return 6;
		case BUTTON_SHOULDER_LEFT_2: return 4;
		case BUTTON_SHOULDER_RIGHT_1: return 7;
		case BUTTON_SHOULDER_RIGHT_2: return 5;
		default: return -1;
	}
}

// $Barnyard: FUNCTION 006d4b40
TFLOAT TInputDXDeviceController::NormaliseStick( TINT a_iValue )
{
	a_iValue -= STICK_CENTRE;

	if ( a_iValue < 0 )
	{
		a_iValue = TMath::Min( a_iValue, -STICK_DEADZONE );
		a_iValue = TMath::Max( a_iValue, -STICK_SATURATE );
		return ( a_iValue + STICK_DEADZONE ) * ( 1.0f / ( STICK_SATURATE - STICK_DEADZONE ) );
	}

	a_iValue = TMath::Max( a_iValue, STICK_DEADZONE );
	a_iValue = TMath::Min( a_iValue, STICK_SATURATE );
	return ( a_iValue - STICK_DEADZONE ) * ( 1.0f / ( STICK_SATURATE - STICK_DEADZONE ) );
}

// $Barnyard: FUNCTION 006d45b0
BOOL CALLBACK TInputDXDeviceController::EnumerateObjectCallback( LPCDIDEVICEOBJECTINSTANCEA a_poDeviceInstance, LPVOID a_pData )
{
	return DIENUM_CONTINUE;
}

TOSHI_NAMESPACE_END
