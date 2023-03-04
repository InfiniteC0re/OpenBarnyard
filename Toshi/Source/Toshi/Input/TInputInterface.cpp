#include "ToshiPCH.h"
#include "TInputInterface.h"

namespace Toshi
{
	TInputDevice* TInputInterface::GetDeviceByIndex(TClass* pClass, size_t index)
	{
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

	bool TInputInterface::Deinitialise()
	{
		return false;
	}

	bool TInputInterface::AcquireAll()
	{
		for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
		{
			if (it->IsAquired())
			{
				TIMPLEMENT();
			}
		}
		return false;
	}

	bool TInputInterface::UnacquireAll()
	{
		return false;
	}

	bool TInputInterface::FlushAll()
	{
		return false;
	}

	void TInputInterface::SetExclusiveMode(bool mode)
	{
		m_bIsExclusiveMode = mode;
	}

	bool TInputInterface::GetExclusiveMode() const
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