#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputDeviceController.h"

namespace Toshi
{
	class TInputDXDeviceController : public TInputDeviceController
	{
	public:
		virtual TBOOL Initialise() { return TTRUE; }

		static TBOOL IsDirectInputController(LPCDIDEVICEINSTANCE a_poDeviceInstance);
		TBOOL const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice);
		
		static int CALLBACK EnumerateObjectCallback(LPCDIDEVICEOBJECTINSTANCE a_poDeviceInstance, LPVOID a_pData)
		{
			return 1;
		}

	private:
		TBOOL m_bIsXInput;
		IDirectInputDevice8* m_pDXInputDevice;
		DIDEVCAPS m_pDXDiDevCaps;
	};
}