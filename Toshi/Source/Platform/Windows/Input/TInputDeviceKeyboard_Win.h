#pragma once
#include "Toshi/Input/TInputDeviceKeyboard.h"
#include "Toshi2/T2Array.h"

#include "Includes.h"

namespace Toshi {

	class TInputDXDeviceKeyboard : public TInputDeviceKeyboard
	{
	public:
		TDECLARE_CLASS(TInputDeviceKeyboard);

	public:
		struct KeyInfo
		{
			wchar_t* wszKeyName;
			DWORD dwKeyScanCode;
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
		virtual TBOOL IsEnabled() const override;
		virtual TBOOL IsShiftDown() const override;
		virtual TBOOL IsControlDown() const override;
		virtual TBOOL IsAltDown() const override;
		virtual TBOOL WasDown(int doodad) const override;
		virtual wchar_t* TranslateDoodadToCharacter(int doodad) const override;
		virtual TBOOL Initialise();
		virtual TBOOL Deinitialise();
		virtual void RefreshDirect();

	protected:
		TBOOL HandleKeyChange(TEmitter<TInputInterface, TInputInterface::InputEvent>& a_Emitter, int a_iKeyIndex, uint8_t a_ui8KeyState);
		static int TranslateDXToDoodad(int doodad);
		static int TranslateDoodadToDX(int dxkey);
	
	public:
		const TBOOL BindToDIDevice(HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice);
		static BOOL CALLBACK EnumObjectCallback(LPCDIDEVICEOBJECTINSTANCEA a_poObjectInstance, LPVOID a_pvRef);

	private:
		int m_iSomeNum;
		TUINT8* m_pKeyStates1;
		TUINT8* m_pKeyStates2;
		TUINT8 m_aBuffer[512];
		T2Array<KeyInfo, KEYBOARD_NUM_DOODADS> m_aKeys;
		TBOOL m_bIsUpdating;
		DIDEVICEINSTANCEA m_oDeviceInstance;
		DIDEVCAPS m_DIDevCaps;
		IDirectInputDevice8A* m_poDXInputDevice;
		TBOOL m_bFlag;
	};

}
