#include "ToshiPCH.h"
#include "TInputInterface_Win.h"
#include TOSHI_MULTIINPUT(TInputDeviceController);
#include TOSHI_MULTIINPUT(TInputDeviceMouse);

namespace Toshi
{
    bool TInputDXInterface::Initialise()
    {
        TASSERT(TNULL == m_poDirectInput8);
        HRESULT hRes = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&m_poDirectInput8, NULL);

        if (hRes == DI_OK)
        {
            if (m_hMainWindow == NULL)
            {
                m_hMainWindow = FindWindowA("TRenderD3D", NULL);
            }
            hRes = m_poDirectInput8->EnumDevices(DI8DEVCLASS_ALL, TInputDXInterface::EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY);
            if (hRes == DI_OK)
            {
                TIMPLEMENT();

                UINT numJoyDevs = joyGetNumDevs();

                for (size_t i = 0; i < numJoyDevs; i++)
                {
                    JOYCAPSA joyCaps;
                    MMRESULT res = joyGetDevCapsA(i, &joyCaps, sizeof(joyCaps));
                    if (res == 0)
                    {
                        TUtil::Log("Joystick[%d] \'%s\' \'%s\'", i, joyCaps.szPname, joyCaps.szOEMVxD);
                    }
                }

                return true;
            }
            Deinitialise();
        }

        return false;
    }

    bool TInputDXInterface::Deinitialise()
    {
        TInputInterface::Deinitialise();

        if (m_poDirectInput8 != NULL)
        {
            m_poDirectInput8->Release();
            m_poDirectInput8 = NULL;
        }

        return true; // ?
    }

    bool TInputDXInterface::LostDevice()
    {
        auto input = GetDirectInput();
        if (input != NULL)
        {
            return input->EnumDevices(DI8DEVCLASS_ALL, EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY) != DI_OK;
        }
        return false;
    }

    BOOL TInputDXInterface::EnumerateDeviceCallback(LPCDIDEVICEINSTANCE a_poDeviceInstance, LPVOID poDXInputInterface)
    {
        TASSERT(poDXInputInterface != NULL);
        TASSERT(a_poDeviceInstance != NULL);
        char fmtStr[37];
        char productName[260];
        TInputDXInterface* inputInterface = (TInputDXInterface*)poDXInputInterface;
        LPDIRECTINPUTDEVICE8 inputDevice;
        TInputDeviceMouse* inputMouse;
        HRESULT hr;


        switch (GET_DIDEVICE_TYPE(a_poDeviceInstance->dwDevType))
        {
        case DI8DEVTYPE_MOUSE:
            inputMouse = inputInterface->GetMouseByIndex(0);
            hr = inputInterface->m_poDirectInput8->CreateDevice(GUID_SysMouse, &inputDevice, NULL);

            // No mouse
            if (hr != DI_OK)
            {
                return DIENUM_CONTINUE;
            }

            if (inputMouse == TNULL)
            {
                // new DXMouse
            }

            TIMPLEMENT();
            break;
        case DI8DEVTYPE_KEYBOARD:
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

            Toshi::TStringManager::StringUnicodeToChar(productName, a_poDeviceInstance->tszProductName, -1);

            if (!TInputDXDeviceController::IsXInputController(a_poDeviceInstance))
            {
                TUtil::Log("Added XInput Controller: \'%s\' (%s) - NON-PSX", productName, fmtStr);
                TUtil::LogConsole("Added XInput Controller: \'%s\' (%s) - NON-PSX", productName, fmtStr);
                return DIENUM_CONTINUE;
            }

            TUtil::Log("Added Direct Input Controller: \'%s\' (%s) - NON-PSX", productName, fmtStr);
            TUtil::LogConsole("Added Direct Input Controller: \'%s\' (%s) - NON-PSX", productName, fmtStr);

            //auto inputDeviceController = new TInputDXDeviceController();
            //inputDeviceController->BindToDIDevice(inputInterface->GetMainWindow(), a_poDeviceInstance, )

            break;
        default:
            break;
        }

        return DIENUM_CONTINUE;
    }
}