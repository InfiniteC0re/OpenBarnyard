#include "ToshiPCH.h"
#include "TInputDeviceMouse_Win.h"
#include "Render/TBackend.h"
#include "Platform/Windows/Input/TInputInterface_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: GetClass 006d3c20, CreateObject 006d4190, CreateObjectInPlace 006d41d0
TDEFINE_CLASS( TInputDXDeviceMouse );

// $Barnyard: FUNCTION 006d3dd0
TBOOL TInputDXDeviceMouse::Acquire()
{
	HRESULT hr = m_poDXInputDevice->Acquire();

	if ( FAILED( hr ) )
	{
		SetAcquired( TFALSE );
		return TFALSE;
	}

	SetAcquired( TTRUE );

	if ( hr != S_FALSE )
	{
		RefreshDirect();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006d3e00
TBOOL TInputDXDeviceMouse::Unacquire()
{
	SetAcquired( TFALSE );
	HRESULT hr = m_poDXInputDevice->Unacquire();
	return SUCCEEDED( hr );
}

// $Barnyard: FUNCTION 006d3e20
void TInputDXDeviceMouse::Release()
{
	if ( m_poDXInputDevice )
	{
		m_poDXInputDevice->Release();
		m_poDXInputDevice = NULL;
	}
}

// $Barnyard: FUNCTION 006d3f30
void TInputDXDeviceMouse::Update( TFLOAT a_fDeltaTime )
{
	if ( IsAcquired() )
	{
		m_iDeltaPositionX = 0;
		m_iDeltaPositionY = 0;
		m_iDeltaPositionZ = 0;

		HRESULT hRes = m_poDXInputDevice->Poll();

		if ( SUCCEEDED( hRes ) )
		{
			DIMOUSESTATE state;
			hRes = m_poDXInputDevice->GetDeviceState( sizeof( state ), &state );

			if ( SUCCEEDED( hRes ) )
			{
				m_iDeltaPositionX += state.lX;
				m_iPositionX += state.lX;
				m_iDeltaPositionY += state.lY;
				m_iPositionY += state.lY;
				m_iDeltaPositionZ += state.lZ;
				m_iPositionZ += state.lZ;

				if ( TRenderInterface::GetSingleton() )
				{
					auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

					if ( !pDisplayParams->bWindowed )
					{
						TMath::Clip( m_iPositionX, 0, TINT( pDisplayParams->uiWidth ) );
						TMath::Clip( m_iPositionY, 0, TINT( pDisplayParams->uiHeight ) );
					}
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 006d3e40
TBOOL TInputDXDeviceMouse::Flush()
{
	DWORD dwItems = INFINITE;

	if ( IsAcquired() )
	{
		HRESULT hr = m_poDXInputDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), NULL, &dwItems, 0 );

		if ( SUCCEEDED( hr ) )
		{
			RefreshDirect();
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006d4200
TINT TInputDXDeviceMouse::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	m_dwButtonPrevious = m_dwButtonCurrent;
	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );

	DWORD              dwItems = 32;
	DIDEVICEOBJECTDATA dod[ 32 ];
	TUtil::MemClear( dod, sizeof( dod ) );

	HRESULT hRes = m_poDXInputDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), dod, &dwItems, 0 );

	if ( FAILED( hRes ) && hRes == ( DIERR_NOTACQUIRED | DIERR_INPUTLOST ) )
	{
		Unacquire();
		Acquire();

		hRes = m_poDXInputDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), dod, &dwItems, 0 );

		if ( hRes != DI_OK )
		{
			return 0;
		}
	}

	for ( DWORD i = 0; i < dwItems; i++ )
	{
		switch ( dod[ i ].dwOfs )
		{
			case DIMOFS_BUTTON0:
			case DIMOFS_BUTTON1:
			case DIMOFS_BUTTON2:
			{
				TINT iButton =
				    ( dod[ i ].dwOfs == DIMOFS_BUTTON0 ) ? ( !m_bReverseButtons ? 0 : 1 ) :
				    ( dod[ i ].dwOfs == DIMOFS_BUTTON1 ) ? ( !m_bReverseButtons ? 1 : 0 ) :
				                                           2;

				TUINT uiButtonFlag = ( 1 << iButton );

				TASSERT( iButton < INPUT_DEVICE_MOUSE_BUTTONS );

				if ( dod[ i ].dwData & 0x80 )
				{
					TASSERT( ( m_dwButtonCurrent & uiButtonFlag ) == 0 );

					if ( ( m_dwButtonCurrent & uiButtonFlag ) == 0 )
					{
						a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_1 + iButton, TInputInterface::EVENT_TYPE_GONE_DOWN ) );
						iNumProcessed += 1;
						m_dwButtonCurrent |= uiButtonFlag;
					}
				}
				else if ( ( m_dwButtonCurrent & uiButtonFlag ) != 0 )
				{
					a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_1 + iButton, TInputInterface::EVENT_TYPE_GONE_UP ) );
					iNumProcessed += 1;
					m_dwButtonCurrent &= ~uiButtonFlag;
				}

				break;
			}
		}
	}

	if ( m_iDeltaPositionX != 0 || m_iDeltaPositionY != 0 )
	{
		a_rEmitter.Throw(
		    TInputInterface::InputEvent(
		        this,
		        AXIS_CURSOR,
		        TInputInterface::EVENT_TYPE_MOVED,
		        m_iDeltaPositionX,
		        m_iDeltaPositionY
		    )
		);

		iNumProcessed += 1;
	}

	if ( m_iDeltaPositionZ != 0 )
	{
		// One wheel notch is 120 (WHEEL_DELTA)
		a_rEmitter.Throw( TInputInterface::InputEvent( this, AXIS_WHEEL, TInputInterface::EVENT_TYPE_MOVED, 0.0f, m_iDeltaPositionZ * ( 1.0f / 120.0f ) ) );
		iNumProcessed += 1;

#ifdef BARNYARD_COMMUNITY_PATCH
		// Retail divides by zero if the device reports no wheel granularity
		TINT iNumSteps = ( m_iWheelGranularity != 0 ) ? m_iDeltaPositionZ / m_iWheelGranularity : 0;
#else
		TINT iNumSteps = m_iDeltaPositionZ / m_iWheelGranularity;
#endif

		// Every step of the wheel is a press of the wheel button (held while the further steps repeat)
		if ( iNumSteps < 0 )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_GONE_DOWN ) );

			for ( TINT i = iNumSteps + 1; i < 0; i++ )
				a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_REPEAT ) );

			a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed += 1 - iNumSteps;
		}
		else if ( iNumSteps > 0 )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_GONE_DOWN ) );

			for ( TINT i = 1; i < iNumSteps; i++ )
				a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_REPEAT ) );

			a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed += iNumSteps + 1;
		}
	}

	return iNumProcessed;
}

// $Barnyard: FUNCTION 006d4000
TINT TInputDXDeviceMouse::GetButtonCount() const
{
	return m_DIDevCaps.dwButtons;
}

// $Barnyard: FUNCTION 006d4010
TINT TInputDXDeviceMouse::GetAxisCount() const
{
	return m_DIDevCaps.dwAxes;
}

// $Barnyard: FUNCTION 006d4020
TBOOL TInputDXDeviceMouse::IsDown( TINT a_iDoodad ) const
{
	if ( ( 0x30000 < a_iDoodad ) && ( a_iDoodad < 0x30009 ) )
	{
#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
		if ( Dev_IsButtonDown( a_iDoodad ) )
			return TTRUE;
#endif

		return ( m_dwButtonCurrent & ( 1 << ( ( a_iDoodad - 1 ) & 0x1F ) ) );
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006d40a0
TINT TInputDXDeviceMouse::GetAxisInt( Doodad a_iDoodad, TINT coord ) const
{
	if ( a_iDoodad == AXIS_CURSOR )
	{
		if ( coord == 0 )
		{
			return m_iDeltaPositionX;
		}

		if ( coord == 1 )
		{
			return m_iDeltaPositionY;
		}
	}

	return 0;
}

// $Barnyard: FUNCTION 006d40d0
TFLOAT TInputDXDeviceMouse::GetAxisFloat( Doodad a_iDoodad, TINT coord ) const
{
	if ( a_iDoodad == AXIS_CURSOR )
	{
		if ( coord == 0 )
		{
			return TFLOAT( m_iDeltaPositionX );
		}

		if ( coord == 1 )
		{
			return TFLOAT( m_iDeltaPositionY );
		}
	}
	else if ( a_iDoodad == AXIS_WHEEL )
	{
		return TFLOAT( m_iDeltaPositionZ );
	}

	return 0.0f;
}

TBOOL TInputDXDeviceMouse::IsEnabled() const
{
	return TTRUE;
}

// $Barnyard: FUNCTION 006d3c50
TBOOL TInputDXDeviceMouse::Initialise()
{
	m_bReverseButtons  = TFALSE;
	m_dwButtonCurrent  = 0;
	m_dwButtonPrevious = 0;

	POINT cursorPos;
	GetCursorPos( &cursorPos );

	m_iPositionX      = cursorPos.x;
	m_iPositionY      = cursorPos.y;
	m_iPositionZ      = 0;
	m_iDeltaPositionX = 0;
	m_iDeltaPositionY = 0;
	m_iDeltaPositionZ = 0;
	m_iUnk1           = 0;
	m_iUnk2           = 0;
	m_iUnk3           = 0;
	m_iUnk4           = 0;
	m_iUnk5           = 0;
	m_iUnk6           = -1;
	m_iUnk7           = -1;

	DIPROPDWORD dwordProperty;
	dwordProperty.diph.dwSize       = sizeof( dwordProperty );
	dwordProperty.diph.dwHeaderSize = sizeof( DIPROPHEADER );
	dwordProperty.diph.dwObj        = DIMOFS_Z;
	dwordProperty.diph.dwHow        = DIPH_BYOFFSET;

	HRESULT hr = m_poDXInputDevice->GetProperty( DIPROP_GRANULARITY, &dwordProperty.diph );

	if ( hr != DIERR_OBJECTNOTFOUND )
	{
		m_iWheelGranularity = ( hr == DI_OK ) ? dwordProperty.dwData : 0;
		return TTRUE;
	}

	m_iWheelGranularity = 0;
	return TTRUE;
}

TBOOL TInputDXDeviceMouse::Deinitialise()
{
	Release();
	return TTRUE;
}

// $Barnyard: FUNCTION 006d3e80
void TInputDXDeviceMouse::RefreshDirect()
{
	if ( IsAcquired() )
	{
		m_dwButtonPrevious = m_dwButtonCurrent;
		m_dwButtonCurrent  = 0;

		if ( SUCCEEDED( m_poDXInputDevice->Poll() ) )
		{
			DIMOUSESTATE mouseState;
			HRESULT      hRes = m_poDXInputDevice->GetDeviceState( sizeof( DIMOUSESTATE ), &mouseState );

			if ( SUCCEEDED( hRes ) )
			{
				if ( mouseState.rgbButtons[ 0 ] & 0x80 ) // LeftClick
					m_dwButtonCurrent |= m_bReverseButtons ? 2 : 1;

				if ( mouseState.rgbButtons[ 1 ] & 0x80 ) // RightClick
					m_dwButtonCurrent |= m_bReverseButtons ? 1 : 2;

				if ( mouseState.rgbButtons[ 2 ] & 0x80 ) // Wheeling yeeet
					m_dwButtonCurrent |= INPUT_DEVICE_MOUSE_WHEEL;
			}
		}
	}
}

// $Barnyard: FUNCTION 006d4060
TBOOL TInputDXDeviceMouse::WasDown( TINT a_iDoodad ) const
{
	if ( ( 0x30000 < a_iDoodad ) && ( a_iDoodad < 0x30009 ) )
	{
		return ( m_dwButtonPrevious & ( ( a_iDoodad - 1 ) & 0x1F ) );
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006d4110
void TInputDXDeviceMouse::SetExclusive( HWND a_hWindow, TBOOL a_bExclusive )
{
	m_poDXInputDevice->SetCooperativeLevel(
	    a_hWindow,
	    a_bExclusive ? ( DISCL_EXCLUSIVE | DISCL_FOREGROUND ) : ( DISCL_NONEXCLUSIVE | DISCL_FOREGROUND )
	);
}

// $Barnyard: FUNCTION 006d4140
void TInputDXDeviceMouse::SetCurrentPosition( TINT a_iX, TINT a_iY )
{
	auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	TMath::Clip( m_iPositionX, 0, TINT( pDisplayParams->uiWidth ) );
	TMath::Clip( m_iPositionY, 0, TINT( pDisplayParams->uiHeight ) );
}

// $Barnyard: FUNCTION 006d3d00
TBOOL const TInputDXDeviceMouse::BindToDIDevice( HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL a_bExclusive )
{
	TASSERT( a_poDeviceInstance != NULL );
	TASSERT( a_poDXInputDevice != NULL );

	Release();
	TUtil::MemCopy( &m_oDeviceInstance, a_poDeviceInstance, sizeof( DIDEVICEINSTANCEA ) );

	m_poDXInputDevice = a_poDXInputDevice;

	m_DIDevCaps = { sizeof( DIDEVCAPS ) };
	m_poDXInputDevice->GetCapabilities( &m_DIDevCaps );

	HRESULT hRes = m_poDXInputDevice->SetDataFormat( &c_dfDIMouse );

	if ( hRes != DI_OK )
		return TFALSE;

	if ( a_hMainWindow )
	{
		m_poDXInputDevice->SetCooperativeLevel( a_hMainWindow, DISCL_FOREGROUND | ( a_bExclusive ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE ) );
	}

	DIPROPDWORD dipdw{};
	dipdw.diph.dwSize       = sizeof( DIPROPDWORD );
	dipdw.diph.dwHeaderSize = sizeof( DIPROPHEADER );
	dipdw.diph.dwObj        = 0;
	dipdw.diph.dwHow        = DIPH_DEVICE;
	dipdw.dwData            = DIPROPAXISMODE_REL;
	hRes                    = m_poDXInputDevice->SetProperty( DIPROP_AXISMODE, &dipdw.diph );

	return hRes == DI_OK;
}

TOSHI_NAMESPACE_END
