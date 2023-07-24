#include "ToshiPCH.h"
#include "TInputDeviceMouse.h"

namespace Toshi {

    TBOOL TInputDeviceMouse::GetDoodadProperties(int doodad, TInputDevice::DoodadProperties& props) const
    {
        if (12 < doodad - 0x30000)
        {
            return TFALSE;
        }

        switch (doodad)
        {
        case AXIS_CURSOR:
            props.m_iUnk = 2;
            props.m_bFlag = TFALSE;
            break;
        case AXIS_WHEEL:
            props.m_iUnk = 1;
            props.m_bFlag = TFALSE;
            break;
        default:
            props.m_iUnk = 0;
            props.m_bFlag = TFALSE;
        }

        return TTRUE;
    }

    const char* TInputDeviceMouse::GetButtonFromDoodad(int doodad) const
    {
        switch (doodad)
        {
        case 0x30001:
            return "BUTTON_1";
        case 0x30002:
            return "BUTTON_2";
        case 0x30003:
            return "BUTTON_3";
        case 0x30004:
            return "BUTTON_4";
        case 0x30005:
            return "BUTTON_5";
        case 0x30006:
            return "BUTTON_6";
        case 0x30007:
            return "BUTTON_7";
        case 0x30008:
            return "BUTTON_8";
        case 0x30009:
            return "BUTTON_WHEEL_FORWARD";
        case 0x3000A:
            return "BUTTON_WHEEL_BACKWARD";
        case 0x3000B:
            return "AXIS_CURSOR";
        case 0x3000C:
            return "AXIS_WHEEL";
        default:
            return "UNKNOWN";
        }
    }

}
