#include "ToshiPCH.h"
#include "TInputInterface.h"
#include "TInputDeviceKeyboard.h"
#include "TInputDeviceMouse.h"

namespace Toshi
{
    TInputDevice* TInputInterface::GetDeviceByIndex(TClass* pClass, size_t index)
    {
        if (m_DeviceList.Count() == 0) return TNULL;
        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            if (it->GetClass()->IsA(pClass))
            {
                if (index == 0)
                {
                    return it;
                }
                else
                {
                    index -= 1;
                }
            }
        }

        return TNULL;
    }

    TInputDeviceKeyboard* TInputInterface::GetKeyboardByIndex(size_t index)
    {
        TInputDeviceKeyboard* pKeyboard = static_cast<TInputDeviceKeyboard*>(GetDeviceByIndex(TGetClass(TInputDeviceKeyboard), index));
        TASSERT(pKeyboard == TNULL || pKeyboard->GetClass()->IsA(TGetClass(TInputDeviceKeyboard)));

        return pKeyboard;
    }

    TInputDeviceMouse* TInputInterface::GetMouseByIndex(size_t index)
    {
        TInputDeviceMouse* pMouse = static_cast<TInputDeviceMouse*>(GetDeviceByIndex(TGetClass(TInputDeviceMouse), index));
        TASSERT(pMouse == TNULL || pMouse->GetClass()->IsA(TGetClass(TInputDeviceMouse)));

        return pMouse;
    }

    Toshi::TClass* TInputInterface::GetSpecificDeviceByIndex(TClass* pClass, size_t index)
    {
        TInputDevice* pDevice = GetDeviceByIndex(pClass, index);
        TASSERT(pDevice == TNULL || pDevice->GetClass()->IsA(pClass));
        if (pDevice == TNULL) return TNULL;
        return pDevice->GetClass();
    }

    void TInputInterface::AddDevice(TInputDevice* device)
    {
        if (device != TNULL)
        {
            device->SetInputInterface(this);
            m_DeviceList.InsertTail(*device);
        }
    }

    void TInputInterface::RemoveDevice(TInputDevice* device)
    {
        if (device != TNULL)
        {
            device->SetInputInterface(this);
            m_DeviceList.Remove(*device);
        }
    }

    TBOOL TInputInterface::Deinitialise()
    {
        return TFALSE;
    }

    TBOOL TInputInterface::AcquireAll()
    {
        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            if (it->IsAquired())
            {
                TIMPLEMENT();
            }
        }
        return TFALSE;
    }

    TBOOL TInputInterface::UnacquireAll()
    {
        return TFALSE;
    }

    TBOOL TInputInterface::FlushAll()
    {
        return TFALSE;
    }

    void TInputInterface::SetExclusiveMode(TBOOL mode)
    {
        m_bIsExclusiveMode = mode;
    }

    TBOOL TInputInterface::GetExclusiveMode() const
    {
        return m_bIsExclusiveMode;
    }

    int TInputInterface::ProcessEvents(float fUnk)
    {
        int events = 0;

        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            if (it->IsAquired())
            {
                TIMPLEMENT();
            }
        }

        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            if (it->IsAquired())
            {
                TIMPLEMENT();
            }
        }

        return events;
    }

    void TInputInterface::StopAllRepeats()
    {
        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            it->StopAllRepeats();
        }
    }
}