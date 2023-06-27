#pragma once
#include <dinput.h>
#include "Toshi/Input/TInputDeviceKeyboard.h"

namespace Toshi
{
	class TInputDXDeviceKeyboard : public TInputDeviceKeyboard
	{
	public:
		virtual TBOOL Initialise();
		virtual void Release();
	protected:
		static int TranslateDXToDoodad(int doodad);
	public:
		TBOOL const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, TBOOL exclusive);
		static BOOL CALLBACK EnumObjectCallback(LPCDIDEVICEOBJECTINSTANCE a_poDeviceInstance, LPVOID a_pvRef);

		GUID m_oGUID; // 0x680
		DIDEVCAPS m_DIDevCaps;                  // 0x8C4
		IDirectInputDevice8* m_poDXInputDevice; // 0x8F0
	};

}

