#include "ToshiPCH.h"
#include "TInputInterface_Win.h"
#include "TInputDeviceController_Win.h"
#include "TInputDeviceMouse_Win.h"
#include "TInputDeviceKeyboard_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: GetClass 006d7380, CreateObject 006d73b0, CreateObjectInPlace 006d73f0
TDEFINE_CLASS( TInputDXInterface );

// $Barnyard: FUNCTION 006d7770
TBOOL TInputDXInterface::Initialise()
{
	TASSERT( TNULL == m_poDirectInput8 );
	HRESULT hRes = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&m_poDirectInput8, NULL );

	if ( SUCCEEDED( hRes ) )
	{
		if ( m_hMainWindow == NULL )
		{
			m_hMainWindow = FindWindowA( "TRenderD3DInterface", NULL );
		}

		hRes = m_poDirectInput8->EnumDevices( DI8DEVCLASS_ALL, TInputDXInterface::EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY );

		if ( hRes == DI_OK )
		{
			return TTRUE;
		}
		else
		{
			Deinitialise();
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006d7390
TBOOL TInputDXInterface::Deinitialise()
{
	TInputInterface::ReleaseAllDevices();

	if ( m_poDirectInput8 != NULL )
	{
		m_poDirectInput8->Release();
		m_poDirectInput8 = NULL;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006d77e0
void TInputDXInterface::SetExclusiveMode( TBOOL a_bIsExclusive )
{
	auto pMouse = GetDeviceByIndex<TInputDXDeviceMouse>( 0 );
	pMouse->Unacquire();
	pMouse->SetExclusive( m_hMainWindow, a_bIsExclusive );
	pMouse->Acquire();
	TInputInterface::SetExclusiveMode( a_bIsExclusive );
}

TBOOL TInputDXInterface::LostDevice()
{
	auto input = GetDirectInput();

	if ( input != NULL )
	{
		return input->EnumDevices( DI8DEVCLASS_ALL, EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY ) != DI_OK;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006d7470
BOOL TInputDXInterface::EnumerateDeviceCallback( LPCDIDEVICEINSTANCEA a_poDeviceInstance, LPVOID poDXInputInterface )
{
	TInputDXInterface* pInputInterface = TSTATICCAST( TInputDXInterface, poDXInputInterface );

	TASSERT( poDXInputInterface != NULL );
	TASSERT( a_poDeviceInstance != NULL );

	auto pInput8 = pInputInterface->m_poDirectInput8;

	switch ( GET_DIDEVICE_TYPE( a_poDeviceInstance->dwDevType ) )
	{
		case DI8DEVTYPE_MOUSE:
		{
			LPDIRECTINPUTDEVICE8A pInputDevice;
			auto                  pMouse      = pInputInterface->GetDeviceByIndex<TInputDXDeviceMouse>( 0 );
			TBOOL                 bIsNewMouse = TFALSE;

			if ( pInput8->CreateDevice( GUID_SysMouse, &pInputDevice, NULL ) != DI_OK )
				return DIENUM_CONTINUE;

			if ( pMouse == TNULL )
			{
				pMouse = new TInputDXDeviceMouse;

				if ( pMouse == TNULL )
				{
					pInputDevice->Release();
					return DIENUM_CONTINUE;
				}

				bIsNewMouse = TTRUE;
			}
			else if ( pMouse->GetDeviceInstance().dwDevType & DIDEVTYPE_HID )
			{
				pInputDevice->Release();
				return DIENUM_CONTINUE;
			}

			if ( !pMouse->BindToDIDevice( pInputInterface->m_hMainWindow, a_poDeviceInstance, pInputDevice, pInputInterface->m_bExclusive ) )
			{
				if ( !bIsNewMouse )
					pInputInterface->RemoveDevice( pMouse );

				delete pMouse;
				return DIENUM_CONTINUE;
			}

			DIPROPDWORD oProperty;
			oProperty.diph.dwSize       = sizeof( DIPROPDWORD );
			oProperty.diph.dwHeaderSize = sizeof( DIPROPHEADER );
			oProperty.diph.dwObj        = 0;
			oProperty.diph.dwHow        = DIPH_DEVICE;
			oProperty.dwData            = 0x20;

			if ( FAILED( pInputDevice->SetProperty( DIPROP_BUFFERSIZE, &oProperty.diph ) ) )
				return DIENUM_STOP;

			if ( bIsNewMouse == TTRUE )
			{
				pMouse->Initialise();
				pMouse->Acquire();
				pInputInterface->AddDevice( pMouse );
			}

			return DIENUM_CONTINUE;
		}
		case DI8DEVTYPE_KEYBOARD:
		{
			LPDIRECTINPUTDEVICE8A pInputDevice;

			if ( pInput8->CreateDevice( a_poDeviceInstance->guidInstance, &pInputDevice, NULL ) != DI_OK )
				return DIENUM_CONTINUE;

			TInputDXDeviceKeyboard* pKeyboard = new TInputDXDeviceKeyboard();

			if ( pKeyboard == TNULL )
			{
				pInputDevice->Release();
				return DIENUM_CONTINUE;
			}

			if ( !pKeyboard->BindToDIDevice( pInputInterface->GetMainWindow(), a_poDeviceInstance, pInputDevice ) )
			{
				delete pKeyboard;
				return DIENUM_CONTINUE;
			}

			DIPROPDWORD oProperty;
			oProperty.diph.dwSize       = sizeof( DIPROPDWORD );
			oProperty.diph.dwHeaderSize = sizeof( DIPROPHEADER );
			oProperty.diph.dwObj        = 0;
			oProperty.diph.dwHow        = DIPH_DEVICE;
			oProperty.dwData            = 0x20;

			if ( FAILED( pInputDevice->SetProperty( DIPROP_BUFFERSIZE, &oProperty.diph ) ) )
				return DIENUM_STOP;

			HRESULT hRes = pInputDevice->SetCooperativeLevel(
			    pInputInterface->GetMainWindow(),
			    pInputInterface->GetExclusiveMode() == TTRUE ? ( DISCL_NOWINKEY | DISCL_FOREGROUND | DISCL_EXCLUSIVE ) : ( DISCL_NOWINKEY | DISCL_FOREGROUND | DISCL_NONEXCLUSIVE )
			);

			if ( hRes != DI_OK )
			{
				delete pKeyboard;
				return DIENUM_CONTINUE;
			}

			pKeyboard->Initialise();
			pKeyboard->Acquire();
			pInputInterface->AddDevice( pKeyboard );

			return DIENUM_CONTINUE;
		}
		case DI8DEVTYPE_JOYSTICK:
		case DI8DEVTYPE_GAMEPAD:
		{
			LPDIRECTINPUTDEVICE8A pInputDevice;

			if ( pInput8->CreateDevice( a_poDeviceInstance->guidInstance, &pInputDevice, NULL ) != DI_OK )
				return DIENUM_CONTINUE;

			TInputDXDeviceController* pController = new TInputDXDeviceController();

			if ( pController == TNULL )
			{
				pInputDevice->Release();
				return DIENUM_CONTINUE;
			}

			if ( !pController->BindToDIDevice( pInputInterface->m_hMainWindow, a_poDeviceInstance, pInputDevice ) )
			{
				delete pController;
				return DIENUM_CONTINUE;
			}

			pController->Initialise();
			pInputInterface->AddDevice( pController );
			pController->Acquire();

			return DIENUM_CONTINUE;
		}
		default:
			break;
	}

	return DIENUM_CONTINUE;
}

TOSHI_NAMESPACE_END
