#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputDeviceController.h"

namespace Toshi
{
	class TInputDXDeviceController : public TInputDeviceController
	{
	public:

		virtual bool Initialise() { return true; }

		static bool IsXInputController(LPCDIDEVICEINSTANCE a_poDeviceInstance);
		bool const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice);

		bool m_isXInput;
	};
}