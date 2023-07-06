#pragma once
#include "Toshi/Input/TInputDeviceKeyboard.h"
#include "Toshi2/T2Array.h"

#include <dinput.h>

namespace Toshi {

	class TInputDXDeviceKeyboard : public TInputDeviceKeyboard
	{
	public:
		struct KeyInfo
		{
			int Unk1;
			int Unk2;
		};

	public:
		TInputDXDeviceKeyboard();

		virtual TBOOL Initialise();
		virtual void Release();

	protected:
		static int TranslateDXToDoodad(int doodad);
	
	public:
		TBOOL const BindToDIDevice(HWND a_hMainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, TBOOL a_bExclusive);
		static BOOL CALLBACK EnumObjectCallback(LPCDIDEVICEOBJECTINSTANCE a_poDeviceInstance, LPVOID a_pvRef);

	private:
		T2Array<KeyInfo, KEYBOARD_NUM_DOODADS> m_Keys; // 0x27C (?)
		GUID m_oGUID;                                  // 0x680
		DIDEVCAPS m_DIDevCaps;                         // 0x8C4
		IDirectInputDevice8* m_poDXInputDevice;        // 0x8F0
	};

}
