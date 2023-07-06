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

    void TInputInterface::AddDevice(TInputDevice* device)
    {
        if (device != TNULL)
        {
            m_DeviceList.InsertTail(*device);
            device->SetInputInterface(this);
        }
    }

    void TInputInterface::RemoveDevice(TInputDevice* device)
    {
        if (device != TNULL)
        {
            m_DeviceList.Remove(*device);
            device->SetInputInterface(TNULL);
        }
    }

    TBOOL TInputInterface::Deinitialise()
    {
        m_DeviceList.DeleteAll();
        return TTRUE;
    }

    TBOOL TInputInterface::AcquireAll()
    {
        TBOOL bRet = TTRUE;
        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            bRet = it->Acquire();
        }
        return bRet;
    }

    TBOOL TInputInterface::UnacquireAll()
    {
        TBOOL bRet = TTRUE;
        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            bRet = it->Unacquire();
        }
        return bRet;
    }

    TBOOL TInputInterface::FlushAll()
    {
        TBOOL bRet = TTRUE;
        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            bRet = it->Flush();
        }
        return bRet;
    }

    void TInputInterface::SetExclusiveMode(TBOOL mode)
    {
        m_bIsExclusiveMode = mode;
    }

    TBOOL TInputInterface::GetExclusiveMode() const
    {
        return m_bIsExclusiveMode;
    }

    int TInputInterface::ProcessEvents(float deltaTime)
    {
        int events = 0;

        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            if (it->IsAcquired())
            {
                it->Update(deltaTime);
            }
        }

        for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
        {
            if (it->IsAcquired())
            {
                events += it->ProcessEvents(m_Emitter1, deltaTime);
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