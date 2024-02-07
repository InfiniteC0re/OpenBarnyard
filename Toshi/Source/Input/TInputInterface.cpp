#include "ToshiPCH.h"
#include "TInputInterface.h"
#include "TInputDeviceKeyboard.h"
#include "TInputDeviceMouse.h"

namespace Toshi {

	TDEFINE_CLASS_NORUNTIME(TInputInterface);

	TInputDevice* TInputInterface::GetDeviceByIndex(TClass* a_pClass, TUINT a_uiIndex)
	{
		if (m_DeviceList.Count() == 0)
		{
			return TNULL;
		}

		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); it++)
		{
			if (it->GetClass()->IsA(a_pClass))
			{
				if (a_uiIndex == 0)
				{
					return it;
				}
				else
				{
					a_uiIndex -= 1;
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

	TBOOL TInputInterface::AcquireAll()
	{
		TBOOL bResult = TTRUE;

		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); it++)
		{
			bResult &= it->Acquire();
		}

		return bResult;
	}

	TBOOL TInputInterface::UnacquireAll()
	{
		TBOOL bResult = TTRUE;

		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); it++)
		{
			bResult &= it->Unacquire();
		}

		return bResult;
	}

	TBOOL TInputInterface::FlushAll()
	{
		TBOOL bResult = TTRUE;

		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); it++)
		{
			bResult &= it->Flush();
		}

		return bResult;
	}

	void TInputInterface::SetExclusiveMode(TBOOL a_bIsExclusive)
	{
		m_bIsExclusiveMode = a_bIsExclusive;
	}

	TBOOL TInputInterface::GetExclusiveMode() const
	{
		return m_bIsExclusiveMode;
	}

	TINT TInputInterface::ProcessEvents(TFLOAT a_fDeltaTime)
	{
		TINT iNumProcessed = 0;

		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); it++)
		{
			if (it->IsAcquired())
			{
				it->Update(a_fDeltaTime);
			}
		}

		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); it++)
		{
			if (it->IsAcquired())
			{
				iNumProcessed += it->ProcessEvents(m_InputEventEmitter, a_fDeltaTime);
			}
		}

		return iNumProcessed;
	}

	void TInputInterface::StopAllRepeats()
	{
		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); it++)
		{
			it->StopAllRepeats();
		}
	}

	void TInputInterface::ReleaseAllDevices()
	{
		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); it++)
		{
			it->Unacquire();
			it->Release();
			it->Remove();
			delete it;
		}
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_bIsMagnitudeFloat = TFALSE;
		m_iAxisCount = 0;
		m_wszString[0] = L'\0';
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, wchar_t* a_wszString)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_bIsMagnitudeFloat = TFALSE;
		m_iAxisCount = 0;
		TStringManager::String16Copy(m_wszString, a_wszString, 3);
		m_Magnitude.Ints[0] = 0;
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_Magnitude.Ints[0] = a_iMagnitude1;
		m_bIsMagnitudeFloat = TFALSE;
		m_iAxisCount = 1;
		m_wszString[0] = L'\0';
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1, TINT a_iMagnitude2)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_Magnitude.Ints[0] = a_iMagnitude1;
		m_Magnitude.Ints[1] = a_iMagnitude2;
		m_bIsMagnitudeFloat = TFALSE;
		m_iAxisCount = 2;
		m_wszString[0] = L'\0';
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_Magnitude.Floats[0] = a_fMagnitude1;
		m_bIsMagnitudeFloat = TTRUE;
		m_iAxisCount = 1;
		m_wszString[0] = L'\0';
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1, TFLOAT a_fMagnitude2)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_Magnitude.Floats[0] = a_fMagnitude1;
		m_Magnitude.Floats[1] = a_fMagnitude2;
		m_bIsMagnitudeFloat = TTRUE;
		m_iAxisCount = 2;
		m_wszString[0] = L'\0';
	}

	TINT TInputInterface::InputEvent::GetMagnitudeInt(TINT a_iAxis)
	{
		TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount());

		if (IsMagnitudeInt())
		{
			return m_Magnitude.Ints[a_iAxis];
		}
		else
		{
			if (m_Magnitude.Floats[a_iAxis] < -0.5f)
			{
				return -1;
			}

			return 0.5f < m_Magnitude.Floats[a_iAxis];
		}
	}

	TFLOAT TInputInterface::InputEvent::GetMagnitudeFloat(TINT a_iAxis)
	{
		TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount());

		if (IsMagnitudeFloat())
		{
			return m_Magnitude.Floats[a_iAxis];
		}

		return (TFLOAT)m_Magnitude.Ints[a_iAxis];
	}

}