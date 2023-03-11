#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputDeviceMouse.h"

namespace Toshi
{
	class TInputDXDeviceMouse : public TInputDeviceMouse
	{
	public:
		TInputDXDeviceMouse() : TInputDeviceMouse()
		{

		}

		virtual void Release();

		bool const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice);

	};
}