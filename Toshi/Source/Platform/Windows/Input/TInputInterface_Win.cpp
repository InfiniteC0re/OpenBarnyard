#include "ToshiPCH.h"
#include "TInputInterface_Win.h"
#include "TInputDeviceController_Win.h"
#include "TInputDeviceMouse_Win.h"
#include "TInputDeviceController_XInput.h"
#include "TInputDeviceKeyboard_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{

TDEFINE_CLASS( TInputDXInterface );

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

void TInputDXInterface::SetExclusiveMode( TBOOL a_bIsExclusive )
{
    auto pMouse = GetDeviceByIndex< TInputDXDeviceMouse >( 0 );
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

BOOL TInputDXInterface::EnumerateDeviceCallback( LPCDIDEVICEINSTANCEA a_poDeviceInstance, LPVOID poDXInputInterface )
{
    TInputDXInterface* pInputInterface = TSTATICCAST( TInputDXInterface, poDXInputInterface );

    TASSERT( poDXInputInterface != NULL );
    TASSERT( a_poDeviceInstance != NULL );

    auto pInput8 = pInputInterface->m_poDirectInput8;

    switch ( GET_DIDEVICE_TYPE( a_poDeviceInstance->dwDevType ) )
    {
        case DI8DEVTYPE_KEYBOARD:
        {
            LPDIRECTINPUTDEVICE8A pInputDevice;
            HRESULT               hRes = pInputInterface->m_poDirectInput8->CreateDevice( a_poDeviceInstance->guidInstance, &pInputDevice, NULL );

            if ( hRes == DI_OK )
            {
                TInputDXDeviceKeyboard* pKeyboard = new TInputDXDeviceKeyboard();

                if ( pKeyboard )
                {
                    TBOOL bRes = pKeyboard->BindToDIDevice( pInputInterface->GetMainWindow(), a_poDeviceInstance, pInputDevice );

                    if ( bRes )
                    {
                        DIPROPDWORD oProperty;
                        oProperty.dwData            = 0x20;
                        oProperty.diph.dwSize       = sizeof( DIPROPDWORD );
                        oProperty.diph.dwHeaderSize = sizeof( DIPROPHEADER );
                        oProperty.diph.dwObj        = 0;
                        oProperty.diph.dwHow        = DIPH_DEVICE;

                        HRESULT hRes = pInputDevice->SetProperty( DIPROP_BUFFERSIZE, &oProperty.diph );

                        if ( FAILED( hRes ) )
                            return DIENUM_STOP;

                        hRes = pInputDevice->SetCooperativeLevel(
                            pInputInterface->GetMainWindow(),
                            pInputInterface->GetExclusiveMode() ? ( DISCL_NOWINKEY | DISCL_FOREGROUND | DISCL_EXCLUSIVE ) : ( DISCL_NONEXCLUSIVE | DISCL_BACKGROUND ) );

                        if ( hRes == DI_OK )
                        {
                            pKeyboard->Initialise();
                            pKeyboard->Acquire();
                            pInputInterface->AddDevice( pKeyboard );
                        }
                        else
                        {
                            delete pKeyboard;
                            return DIENUM_CONTINUE;
                        }
                    }
                }
            }
            break;
        }
        case DI8DEVTYPE_MOUSE:
        {
            LPDIRECTINPUTDEVICE8A pInputDevice;
            auto                  pMouse = pInputInterface->GetDeviceByIndex< TInputDXDeviceMouse >( 0 );
            auto                  hRes   = pInput8->CreateDevice( GUID_SysMouse, &pInputDevice, NULL );

            TBOOL bIsNewMouse = TFALSE;

            if ( hRes == DI_OK )
            {
                if ( pMouse == TNULL )
                {
                    pMouse      = new TInputDXDeviceMouse;
                    bIsNewMouse = TTRUE;

                    if ( pMouse == TNULL )
                    {
                        pInputDevice->Release();
                        return DIENUM_CONTINUE;
                    }
                }
                else if ( pMouse->GetDeviceInstance().dwDevType & DIDEVTYPE_HID )
                {
                    pInputDevice->Release();
                    return DIENUM_CONTINUE;
                }

                if ( !pMouse->BindToDIDevice( pInputInterface->m_hMainWindow, a_poDeviceInstance, pInputDevice, pInputInterface->m_bExclusive ) )
                {
                    if ( !bIsNewMouse )
                    {
                        pInputInterface->RemoveDevice( pMouse );
                    }

                    delete pMouse;
                    return DIENUM_CONTINUE;
                }

                DIPROPDWORD oProperty;
                oProperty.diph.dwSize       = sizeof( DIPROPDWORD );
                oProperty.diph.dwHeaderSize = sizeof( DIPROPHEADER );
                oProperty.diph.dwObj        = 0;
                oProperty.diph.dwHow        = DIPH_DEVICE;
                oProperty.dwData            = 0x20;
                hRes                        = pInputDevice->SetProperty( DIPROP_BUFFERSIZE, &oProperty.diph );

                if ( FAILED( hRes ) )
                {
                    return DIENUM_STOP;
                }

                if ( bIsNewMouse )
                {
                    pMouse->Initialise();
                    pMouse->Acquire();
                    pInputInterface->AddDevice( pMouse );
                    return DIENUM_CONTINUE;
                }
            }

            break;
            /*inputMouse = pInputInterface->GetDeviceByIndex<TInputDXDeviceMouse>();

			HRESULT hRes = pInputInterface->m_poDirectInput8->CreateDevice(GUID_SysMouse, &inputDevice, NULL);

			if (hRes != DI_OK)
				return DIENUM_CONTINUE;

			if (inputMouse == TNULL)
			{
				inputMouse = new TInputDXDeviceMouse();
				addMouse = TTRUE;
				TBOOL bRes = inputMouse->BindToDIDevice(pInputInterface->GetMainWindow(), a_poDeviceInstance, inputDevice, pInputInterface->m_bExclusive);

				if (bRes)
				{
					DIPROPDWORD oProperty;
					oProperty.diph.dwSize = sizeof(DIPROPDWORD);
					oProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
					oProperty.diph.dwObj = 0;
					oProperty.diph.dwHow = DIPH_DEVICE;
					oProperty.dwData = TInputDXDeviceMouse::sm_ciMouseBufferSize;

					HRESULT hr = inputDevice->SetProperty(DIPROP_BUFFERSIZE, &oProperty.diph);

					if (FAILED(hr))
						return DIENUM_STOP;

					if (addMouse)
					{
						inputMouse->Initialise();
						inputMouse->Acquire();
						pInputInterface->AddDevice(inputMouse);
					}
				}
				if (!addMouse)
				{
					pInputInterface->RemoveDevice(inputMouse);
				}
			}
			TIMPLEMENT();
			break;*/
        }
        //case DI8DEVTYPE_JOYSTICK:
        //case DI8DEVTYPE_GAMEPAD:
        //case DI8DEVTYPE_1STPERSON:
        //{
        //	TIMPLEMENT();

        //	TCHAR fmtStr[37];
        //	Toshi::T2String8::Format(fmtStr, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
        //		a_poDeviceInstance->guidProduct.Data1, a_poDeviceInstance->guidProduct.Data2,
        //		a_poDeviceInstance->guidProduct.Data3, a_poDeviceInstance->guidProduct.Data4[0],
        //		a_poDeviceInstance->guidProduct.Data4[1], a_poDeviceInstance->guidProduct.Data4[2],
        //		a_poDeviceInstance->guidProduct.Data4[3], a_poDeviceInstance->guidProduct.Data4[4],
        //		a_poDeviceInstance->guidProduct.Data4[5], a_poDeviceInstance->guidProduct.Data4[6],
        //		a_poDeviceInstance->guidProduct.Data4[7]);

        //	if (!TInputDXDeviceController::IsDirectInputController(a_poDeviceInstance))
        //	{
        //		TUtil::Log("Added XInput Controller: \'%s\' (%s) - NON-PSX", a_poDeviceInstance->tszProductName, fmtStr);
        //		return DIENUM_CONTINUE;
        //	}

        //	TUtil::Log("Added Direct Input Controller: \'%s\' (%s) - NON-PSX", a_poDeviceInstance->tszProductName, fmtStr);

        //	inputController = pInputInterface->GetDeviceByIndex<TInputDXDeviceController>();
        //	HRESULT hRes = pInputInterface->m_poDirectInput8->CreateDevice(a_poDeviceInstance->guidInstance, &inputDevice, NULL);

        //	if (hRes != DI_OK)
        //		return DIENUM_CONTINUE;

        //	if (inputController == TNULL)
        //	{
        //		// new DXController
        //		//inputController = new TInputDXDeviceController();
        //		//inputController->BindToDIDevice(pInputInterface->GetMainWindow(), a_poDeviceInstance, inputDevice);
        //	}

        //	break;
        //}
        default:
            break;
    }

    return DIENUM_CONTINUE;
}
} // namespace Toshi