#pragma once
#include "Includes.h"
#include "Input/TInputDeviceController.h"

namespace Toshi
{
	class TInputDXDeviceController : public TInputDeviceController
	{
	public:
		virtual TBOOL Initialise() { return TTRUE; }

		static TBOOL IsDirectInputController(LPCDIDEVICEINSTANCEA a_poDeviceInstance);
		TBOOL const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice);
		
		static int CALLBACK EnumerateObjectCallback(LPCDIDEVICEOBJECTINSTANCEA a_poDeviceInstance, LPVOID a_pData)
		{
			return 1;
		}

	private:
		TBOOL m_bIsXInput;
		IDirectInputDevice8A* m_pDXInputDevice;
		DIDEVCAPS m_pDXDiDevCaps;
	};
}