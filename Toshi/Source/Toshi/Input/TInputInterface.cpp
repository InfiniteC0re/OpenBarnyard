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
}