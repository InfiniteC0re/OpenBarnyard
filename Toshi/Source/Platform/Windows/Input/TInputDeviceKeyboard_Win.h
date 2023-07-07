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

		virtual TBOOL Acquire() override;
		virtual TBOOL Unacquire() override;
		virtual void Release() override;
		virtual void Update(float deltaTime) override;
		virtual TBOOL Flush() override;
		virtual int ProcessEvents(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float deltaTime) override;
		virtual int GetButtonCount() const override;
		virtual TBOOL IsDown(int doodad) const override;
		virtual TBOOL Unknown2() const override;
		virtual TBOOL IsShiftDown() const override;
		virtual TBOOL IsControlDown() const override;
		virtual TBOOL IsAltDown() const override;
		virtual TBOOL WasDown(int doodad) const override;
		virtual wchar_t* TranslateDoodadToCharacter(int doodad) const override;
		virtual TBOOL Initialise();
		virtual TBOOL Deinitialise();
		virtual void RefreshDirect();

	protected:
		static int TranslateDXToDoodad(int doodad);
		static int TranslateDoodadToDX(int dxkey);
	
	public:
		const TBOOL BindToDIDevice(HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL a_bExclusive);
		static BOOL CALLBACK EnumObjectCallback(LPCDIDEVICEOBJECTINSTANCEA a_poDeviceInstance, LPVOID a_pvRef);

	private:
		int m_iSomeNum;                                 // 0x070
		uint8_t* m_pKeyStates1;                         // 0x074
		uint8_t* m_pKeyStates2;                         // 0x078
		uint8_t m_aBuffer[512];                         // 0x07C
		T2Array<KeyInfo, KEYBOARD_NUM_DOODADS> m_Array; // 0x27C
		TBOOL m_bIsUpdating;                            // 0x67C
		DIDEVICEINSTANCEA m_oDeviceInstance;            // 0x680
		DIDEVCAPS m_DIDevCaps;                          // 0x8C4
		IDirectInputDevice8A* m_poDXInputDevice;        // 0x8F0
	};

}
