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
		virtual bool Initialise();
		virtual bool Acquire();
		virtual bool Unacquire();
		virtual bool Flush();
		virtual int ProcessEvents(TGenericEmitter& emitter, float flt);

		bool const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, bool exclusive);


		static constexpr int sm_ciMouseBufferSize = 0x20000000;

		IDirectInputDevice8* m_poDXInputDevice;
		DIDEVCAPS m_DIDevCaps;
	};
}