#include "ToshiPCH.h"
#include "TInputInterface_Win.h"
#include <joystickapi.h>

namespace Toshi
{
    bool TInputDXInterface::Initialise()
    {
        TASSERT(TNULL == m_poDirectInput8);
        HRESULT hRes = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)m_poDirectInput8, NULL);

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

    BOOL TInputDXInterface::EnumerateDeviceCallback(LPCDIDEVICEINSTANCE a_poDeviceInstance, LPVOID poDXInputInterface)
    {
        TASSERT(poDXInputInterface != NULL);
        TASSERT(a_poDeviceInstance != NULL);

        switch (GET_DIDEVICE_TYPE(a_poDeviceInstance->dwDevType))
        {
        case DI8DEVTYPE_MOUSE:

            TIMPLEMENT();
            break;
        case DI8DEVTYPE_KEYBOARD:
            TIMPLEMENT();
            break;
        case DI8DEVTYPE_JOYSTICK:
        case DI8DEVTYPE_GAMEPAD:
        case DI8DEVTYPE_1STPERSON:
            TIMPLEMENT();
            char* fmtStr;
            Toshi::T2String8::Format(fmtStr, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX");
            break;
        default:
            break;
        }

        return DIENUM_CONTINUE;
    }
}