#include "ToshiPCH.h"
#include "TInputInterface_Win.h"
#include TOSHI_MULTIPLATFORM(Input/TInputDeviceController)
#include "TInputDeviceController_XInput.h"
#include "TInputDeviceController_Wiin.h"
#include <Platform/Windows/Input/TInputDeviceKeyboard_Win.h>
#include TOSHI_MULTIPLATFORM(Input/TInputDeviceMouse)

namespace Toshi
{
    TBOOL TInputDXInterface::Initialise()
    {
        TASSERT(TNULL == m_poDirectInput8);
        HRESULT hRes = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&m_poDirectInput8, NULL);

        if (SUCCEEDED(hRes))
        {
            if (m_hMainWindow == NULL)
            {
                m_hMainWindow = FindWindowA("TRenderD3D", NULL);
            }
            hRes = m_poDirectInput8->EnumDevices(DI8DEVCLASS_ALL, TInputDXInterface::EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY);
            if (hRes == DI_OK)
            {
                for (size_t i = 0; i < 4; i++)
                {
                    //AddDevice(new TInputDeviceController_XInput());
                    //AddDevice(new TInputWiinDeviceController());
                }

                UINT numJoyDevs = joyGetNumDevs();

                for (size_t i = 0; i < numJoyDevs; i++)
                {
                    JOYCAPSA joyCaps;
                    MMRESULT res = joyGetDevCapsA(i, &joyCaps, sizeof(joyCaps));
                    if (res == 0)
                    {
                        TUtil::Log("Joystick[%d] \'%s\' \'%s\'", i, joyCaps.szPname, joyCaps.szRegKey);
                    }
                }

                return TTRUE;
            }
            Deinitialise();
        }

        return TFALSE;
    }

    TBOOL TInputDXInterface::Deinitialise()
    {
        TInputInterface::Deinitialise();

        if (m_poDirectInput8 != NULL)
        {
            m_poDirectInput8->Release();
            m_poDirectInput8 = NULL;
        }

        return TTRUE;
    }

    TBOOL TInputDXInterface::LostDevice()
    {
        auto input = GetDirectInput();
        if (input != NULL)
        {
            return input->EnumDevices(DI8DEVCLASS_ALL, EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY) != DI_OK;
        }
        return TFALSE;
    }

    BOOL TInputDXInterface::EnumerateDeviceCallback(LPCDIDEVICEINSTANCEA a_poDeviceInstance, LPVOID poDXInputInterface)
    {
        char fmtStr[37];
        TInputDXInterface* inputInterface = (TInputDXInterface*)poDXInputInterface;
        LPDIRECTINPUTDEVICE8A inputDevice;
        TInputDXDeviceMouse* inputMouse;
        TInputDXDeviceController* inputController;
        TInputDXDeviceKeyboard* inputKeyboard;
        HRESULT hr;
        TBOOL addMouse = TFALSE;
        TBOOL addKeyboard = TFALSE;
        TBOOL res;
        
        TASSERT(poDXInputInterface != NULL);
        TASSERT(a_poDeviceInstance != NULL);

        // Check if any slots are left
        if (ms_iNumDevices >= MAXDEVICESNUM)
            return FALSE;

        // Check if this device is not registered yet
        for (size_t i = 0; i < ms_iNumDevices; i++)
        {
            if (ms_RegisteredDevices[i] == a_poDeviceInstance->guidInstance)
            {
                return TRUE;
            }
        }

        // Add the device to the list of registered
        size_t iDeviceIndex = ms_iNumDevices++;
        ms_RegisteredDevices[iDeviceIndex] = a_poDeviceInstance->guidInstance;

        switch (GET_DIDEVICE_TYPE(a_poDeviceInstance->dwDevType))
        {
        case DI8DEVTYPE_MOUSE:
            inputMouse = inputInterface->GetDeviceByIndex<TInputDXDeviceMouse>();

            hr = inputInterface->m_poDirectInput8->CreateDevice(GUID_SysMouse, &inputDevice, NULL);

            if (hr != DI_OK)
            {
                return DIENUM_CONTINUE;
            }

            if (inputMouse == TNULL)
            {
                inputMouse = new TInputDXDeviceMouse();
                addMouse = TTRUE;
                TBOOL res = inputMouse->BindToDIDevice(inputInterface->GetMainWindow(), a_poDeviceInstance, inputDevice, inputInterface->m_bExclusive);
                if (res)
                {
                    DIPROPDWORD dwordProperty{};
                    dwordProperty.diph.dwSize = sizeof(DIPROPDWORD);
                    dwordProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
                    dwordProperty.diph.dwObj = 0;
                    dwordProperty.diph.dwHow = DIPH_DEVICE;
                    dwordProperty.dwData = TInputDXDeviceMouse::sm_ciMouseBufferSize;
                    
                    HRESULT hr = inputDevice->SetProperty(DIPROP_BUFFERSIZE, &dwordProperty.diph);

                    if (FAILED(hr)) return TFALSE;

                    if (addMouse)
                    {
                        inputMouse->Initialise();
                        inputMouse->Acquire();
                        inputInterface->AddDevice(inputMouse);
                    }
                }
                if (!addMouse)
                {
                    inputInterface->RemoveDevice(inputMouse);
                }
            }
            TIMPLEMENT();
            break;

        case DI8DEVTYPE_KEYBOARD:

            hr = inputInterface->m_poDirectInput8->CreateDevice(a_poDeviceInstance->guidInstance, &inputDevice, NULL);

            if (hr != DI_OK)
            {
                return DIENUM_CONTINUE;
            }

            inputKeyboard = new TInputDXDeviceKeyboard();
            addKeyboard = TTRUE;
            res = inputKeyboard->BindToDIDevice(inputInterface->GetMainWindow(), a_poDeviceInstance, inputDevice, inputInterface->m_bExclusive);
            if (res)
            {
            }

            TIMPLEMENT();
            break;
        case DI8DEVTYPE_JOYSTICK:
        case DI8DEVTYPE_GAMEPAD:
        case DI8DEVTYPE_1STPERSON:
            TIMPLEMENT();
            
            Toshi::T2String8::Format(fmtStr, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", 
                a_poDeviceInstance->guidProduct.Data1, a_poDeviceInstance->guidProduct.Data2, 
                a_poDeviceInstance->guidProduct.Data3, a_poDeviceInstance->guidProduct.Data4[0],
                a_poDeviceInstance->guidProduct.Data4[1], a_poDeviceInstance->guidProduct.Data4[2],
                a_poDeviceInstance->guidProduct.Data4[3], a_poDeviceInstance->guidProduct.Data4[4],
                a_poDeviceInstance->guidProduct.Data4[5], a_poDeviceInstance->guidProduct.Data4[6],
                a_poDeviceInstance->guidProduct.Data4[7]);

            if (!TInputDXDeviceController::IsDirectInputController(a_poDeviceInstance))
            {
                TUtil::Log("Added XInput Controller: \'%s\' (%s) - NON-PSX", a_poDeviceInstance->tszProductName, fmtStr);
                return DIENUM_CONTINUE;
            }

            TUtil::Log("Added Direct Input Controller: \'%s\' (%s) - NON-PSX", a_poDeviceInstance->tszProductName, fmtStr);

            inputController = inputInterface->GetDeviceByIndex<TInputDXDeviceController>();
            hr = inputInterface->m_poDirectInput8->CreateDevice(a_poDeviceInstance->guidInstance, &inputDevice, NULL);

            if (hr != DI_OK)
            {
                return DIENUM_CONTINUE;
            }

            if (inputController == TNULL)
            {
                // new DXController
				//inputController = new TInputDXDeviceController();
				//inputController->BindToDIDevice(inputInterface->GetMainWindow(), a_poDeviceInstance, inputDevice);
            }

            break;
        default:
            break;
        }

        return DIENUM_CONTINUE;
    }
}