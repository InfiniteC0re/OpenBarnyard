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
		int iNumProcessed = 0;

		for (auto it = m_DeviceList.Tail(); it != m_DeviceList.End(); --it)
		{
			if (it->IsAcquired())
			{
				it->Update(deltaTime);
			}
		}

		for (auto it = m_DeviceList.Tail(); it != m_DeviceList.End(); --it)
		{
			if (it->IsAcquired())
			{
				iNumProcessed += it->ProcessEvents(m_Emitter1, deltaTime);
			}
		}

		return iNumProcessed;
	}

	void TInputInterface::StopAllRepeats()
	{
		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
		{
			it->StopAllRepeats();
		}
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_bIsMagnitudeFloat = TFALSE;
		m_iAxisCount = 0;
		m_wszString[0] = L'\0';
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, wchar_t* a_wszString)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_bIsMagnitudeFloat = TFALSE;
		m_iAxisCount = 0;
		TStringManager::String16Copy(m_wszString, a_wszString, 3);
		m_Magnitude.Ints[0] = 0;
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, int a_iMagnitude1)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_Magnitude.Ints[0] = a_iMagnitude1;
		m_bIsMagnitudeFloat = TFALSE;
		m_iAxisCount = 1;
		m_wszString[0] = L'\0';
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, int a_iMagnitude1, int a_iMagnitude2)
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

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, float a_fMagnitude1)
	{
		m_pSource = a_pDevice;
		m_iDoodad = a_iDoodad;
		m_eEventType = a_eEventType;
		m_Magnitude.Floats[0] = a_fMagnitude1;
		m_bIsMagnitudeFloat = TTRUE;
		m_iAxisCount = 1;
		m_wszString[0] = L'\0';
	}

	TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, float a_fMagnitude1, float a_fMagnitude2)
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

	int TInputInterface::InputEvent::GetMagnitudeInt(int a_iAxis)
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

	float TInputInterface::InputEvent::GetMagnitudeFloat(int a_iAxis)
	{
		TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount());

		if (IsMagnitudeFloat())
		{
			return m_Magnitude.Floats[a_iAxis];
		}

		return (float)m_Magnitude.Ints[a_iAxis];
	}

}