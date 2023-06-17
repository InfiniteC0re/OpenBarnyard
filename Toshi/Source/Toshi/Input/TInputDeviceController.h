#pragma once
#include "Toshi/Input/TInputInterface.h"

namespace Toshi
{
    class TInputDeviceController : public TInputDevice
    {
    public:
        TInputDeviceController()
        {
            m_things = (uint8_t*)TMalloc(8);
            m_things2 = (uint8_t*)TMalloc(8);
            m_things = { 0 };
            m_things2 = { 0 };
        }

    private:
        uint8_t* m_things; // 0x3C
        uint8_t* m_things2; // 0x40
    };
}
