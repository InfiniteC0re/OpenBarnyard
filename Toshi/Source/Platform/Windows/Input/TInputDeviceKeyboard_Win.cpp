#include "ToshiPCH.h"
#include "TInputDeviceKeyboard_Win.h"

namespace Toshi {

	TInputDXDeviceKeyboard::TInputDXDeviceKeyboard()
	{

	}

	TBOOL TInputDXDeviceKeyboard::Initialise()
	{
		return TFALSE;
	}

	void TInputDXDeviceKeyboard::Release()
	{
	}

	int TInputDXDeviceKeyboard::TranslateDXToDoodad(int doodad)
	{
		switch (doodad)
		{
		case DIK_ESCAPE:
			return TInputDeviceKeyboard::KEY_ESCAPE;
		case DIK_1:
			return TInputDeviceKeyboard::KEY_1;
		case DIK_2:
			return TInputDeviceKeyboard::KEY_2;
		case DIK_3:
			return TInputDeviceKeyboard::KEY_3;
		case DIK_4:
			return TInputDeviceKeyboard::KEY_4;
		case DIK_5:
			return TInputDeviceKeyboard::KEY_5;
		case DIK_6:
			return TInputDeviceKeyboard::KEY_6;
		case DIK_7:
			return TInputDeviceKeyboard::KEY_7;
		case DIK_8:
			return TInputDeviceKeyboard::KEY_8;
		case DIK_9:
			return TInputDeviceKeyboard::KEY_9;
		case DIK_0:
			return TInputDeviceKeyboard::KEY_0;
		case DIK_MINUS:
			return TInputDeviceKeyboard::KEY_MINUS;
		case DIK_EQUALS:
			return TInputDeviceKeyboard::KEY_EQUALS;
		case DIK_BACK:
			return TInputDeviceKeyboard::KEY_BACK;
		case DIK_TAB:
			return TInputDeviceKeyboard::KEY_TAB;
		case DIK_Q:
			return TInputDeviceKeyboard::KEY_Q;
		case DIK_W:
			return TInputDeviceKeyboard::KEY_W;
		case DIK_E:
			return TInputDeviceKeyboard::KEY_E;
		case DIK_R:
			return TInputDeviceKeyboard::KEY_R;
		case DIK_T:
			return TInputDeviceKeyboard::KEY_T;
		case DIK_Y:
			return TInputDeviceKeyboard::KEY_Y;
		case DIK_U:
			return TInputDeviceKeyboard::KEY_U;
		case DIK_I:
			return TInputDeviceKeyboard::KEY_I;
		case DIK_O:
			return TInputDeviceKeyboard::KEY_O;
		case DIK_P:
			return TInputDeviceKeyboard::KEY_P;
		case DIK_LBRACKET:
			return TInputDeviceKeyboard::KEY_LBRACKET;
		case DIK_RBRACKET:
			return TInputDeviceKeyboard::KEY_RBRACKET;
		case DIK_RETURN:
			return TInputDeviceKeyboard::KEY_RETURN;
		case DIK_LCONTROL:
			return TInputDeviceKeyboard::KEY_LCONTROL;
		case DIK_A:
			return TInputDeviceKeyboard::KEY_A;
		case DIK_S:
			return TInputDeviceKeyboard::KEY_S;
		case DIK_D:
			return TInputDeviceKeyboard::KEY_D;
		case DIK_F:
			return TInputDeviceKeyboard::KEY_F;
		case DIK_G:
			return TInputDeviceKeyboard::KEY_G;
		case DIK_H:
			return TInputDeviceKeyboard::KEY_H;
		case DIK_J:
			return TInputDeviceKeyboard::KEY_J;
		case DIK_K:
			return TInputDeviceKeyboard::KEY_K;
		case DIK_L:
			return TInputDeviceKeyboard::KEY_L;
		case DIK_SEMICOLON:
			return TInputDeviceKeyboard::KEY_SEMICOLON;
		case DIK_APOSTROPHE:
			return TInputDeviceKeyboard::KEY_APOSTROPHE;
		case DIK_GRAVE:
			return TInputDeviceKeyboard::KEY_GRAVE;
		case DIK_LSHIFT:
			return TInputDeviceKeyboard::KEY_LSHIFT;
		case DIK_BACKSLASH:
			return TInputDeviceKeyboard::KEY_BACKSLASH;
		case DIK_Z:
			return TInputDeviceKeyboard::KEY_Z;
		case DIK_X:
			return TInputDeviceKeyboard::KEY_X;
		case DIK_C:
			return TInputDeviceKeyboard::KEY_C;
		case DIK_V:
			return TInputDeviceKeyboard::KEY_V;
		case DIK_B:
			return TInputDeviceKeyboard::KEY_B;
		case DIK_N:
			return TInputDeviceKeyboard::KEY_N;
		case DIK_M:
			return TInputDeviceKeyboard::KEY_M;
		case DIK_COMMA:
			return TInputDeviceKeyboard::KEY_COMMA;
		case DIK_PERIOD:
			return TInputDeviceKeyboard::KEY_PERIOD;
		case DIK_SLASH:
			return TInputDeviceKeyboard::KEY_SLASH;
		case DIK_RSHIFT:
			return TInputDeviceKeyboard::KEY_RSHIFT;
		case DIK_MULTIPLY:
			return TInputDeviceKeyboard::KEY_MULTIPLY;
		case DIK_LMENU:
			return TInputDeviceKeyboard::KEY_LMENU;
		case DIK_SPACE:
			return TInputDeviceKeyboard::KEY_SPACE;
		case DIK_CAPITAL:
			return TInputDeviceKeyboard::KEY_CAPITAL;
		case DIK_F1:
			return TInputDeviceKeyboard::KEY_F1;
		case DIK_F2:
			return TInputDeviceKeyboard::KEY_F2;
		case DIK_F3:
			return TInputDeviceKeyboard::KEY_F3;
		case DIK_F4:
			return TInputDeviceKeyboard::KEY_F4;
		case DIK_F5:
			return TInputDeviceKeyboard::KEY_F5;
		case DIK_F6:
			return TInputDeviceKeyboard::KEY_F6;
		case DIK_F7:
			return TInputDeviceKeyboard::KEY_F7;
		case DIK_F8:
			return TInputDeviceKeyboard::KEY_F8;
		case DIK_F9:
			return TInputDeviceKeyboard::KEY_F9;
		case DIK_F10:
			return TInputDeviceKeyboard::KEY_F10;
		case DIK_NUMLOCK:
			return TInputDeviceKeyboard::KEY_NUMLOCK;
		case DIK_SCROLL:
			return TInputDeviceKeyboard::KEY_SCROLL;
		case DIK_NUMPAD7:
			return TInputDeviceKeyboard::KEY_NUMPAD7;
		case DIK_NUMPAD8:
			return TInputDeviceKeyboard::KEY_NUMPAD8;
		case DIK_NUMPAD9:
			return TInputDeviceKeyboard::KEY_NUMPAD9;
		case DIK_SUBTRACT:
			return TInputDeviceKeyboard::KEY_SUBTRACT;
		case DIK_NUMPAD4:
			return TInputDeviceKeyboard::KEY_NUMPAD4;
		case DIK_NUMPAD5:
			return TInputDeviceKeyboard::KEY_NUMPAD5;
		case DIK_NUMPAD6:
			return TInputDeviceKeyboard::KEY_NUMPAD6;
		case DIK_ADD:
			return TInputDeviceKeyboard::KEY_ADD;
		case DIK_NUMPAD1:
			return TInputDeviceKeyboard::KEY_NUMPAD1;
		case DIK_NUMPAD2:
			return TInputDeviceKeyboard::KEY_NUMPAD2;
		case DIK_NUMPAD3:
			return TInputDeviceKeyboard::KEY_NUMPAD3;
		case DIK_NUMPAD0:
			return TInputDeviceKeyboard::KEY_NUMPAD0;
		case DIK_DECIMAL:
			return TInputDeviceKeyboard::KEY_DECIMAL;
		case DIK_F11:
			return TInputDeviceKeyboard::KEY_F11;
		case DIK_F12:
			return TInputDeviceKeyboard::KEY_F12;
		case DIK_F13:
			return TInputDeviceKeyboard::KEY_F13;
		case DIK_F14:
			return TInputDeviceKeyboard::KEY_F14;
		case DIK_F15:
			return TInputDeviceKeyboard::KEY_F15;
		case DIK_KANA:
			return TInputDeviceKeyboard::KEY_KANA;
		case DIK_CONVERT:
			return TInputDeviceKeyboard::KEY_CONVERT;
		case DIK_NOCONVERT:
			return TInputDeviceKeyboard::KEY_NOCONVERT;
		case DIK_YEN:
			return TInputDeviceKeyboard::KEY_YEN;
		case DIK_NUMPADEQUALS:
			return TInputDeviceKeyboard::KEY_NUMPADEQUALS;
		case DIK_CIRCUMFLEX:
			return TInputDeviceKeyboard::KEY_CIRCUMFLEX;
		case DIK_AT:
			return TInputDeviceKeyboard::KEY_AT;
		case DIK_COLON:
			return TInputDeviceKeyboard::KEY_COLON;
		case DIK_UNDERLINE:
			return TInputDeviceKeyboard::KEY_UNDERLINE;
		case DIK_KANJI:
			return TInputDeviceKeyboard::KEY_KANJI;
		case DIK_STOP:
			return TInputDeviceKeyboard::KEY_STOP;
		case DIK_AX:
			return TInputDeviceKeyboard::KEY_AX;
		case DIK_UNLABELED:
			return TInputDeviceKeyboard::KEY_UNLABELED;
		case DIK_NUMPADENTER:
			return TInputDeviceKeyboard::KEY_NUMPADENTER;
		case DIK_RCONTROL:
			return TInputDeviceKeyboard::KEY_RCONTROL;
		case DIK_NUMPADCOMMA:
			return TInputDeviceKeyboard::KEY_NUMPADCOMMA;
		case DIK_DIVIDE:
			return TInputDeviceKeyboard::KEY_DIVIDE;
		case DIK_SYSRQ:
			return TInputDeviceKeyboard::KEY_SYSRQ;
		case DIK_RMENU:
			return TInputDeviceKeyboard::KEY_RMENU;
		case DIK_PAUSE:
			return TInputDeviceKeyboard::KEY_PAUSE;
		case DIK_HOME:
			return TInputDeviceKeyboard::KEY_HOME;
		case DIK_UP:
			return TInputDeviceKeyboard::KEY_UP;
		case DIK_PRIOR:
			return TInputDeviceKeyboard::KEY_PRIOR;
		case DIK_LEFT:
			return TInputDeviceKeyboard::KEY_LEFT;
		case DIK_RIGHT:
			return TInputDeviceKeyboard::KEY_RIGHT;
		case DIK_END:
			return TInputDeviceKeyboard::KEY_END;
		case DIK_DOWN:
			return TInputDeviceKeyboard::KEY_DOWN;
		case DIK_NEXT:
			return TInputDeviceKeyboard::KEY_NEXT;
		case DIK_INSERT:
			return TInputDeviceKeyboard::KEY_INSERT;
		case DIK_DELETE:
			return TInputDeviceKeyboard::KEY_DELETE;
		case DIK_LWIN:
			return TInputDeviceKeyboard::KEY_LWIN;
		case DIK_RWIN:
			return TInputDeviceKeyboard::KEY_RWIN;
		case DIK_APPS:
			return TInputDeviceKeyboard::KEY_APPS;
		case DIK_POWER:
			return TInputDeviceKeyboard::KEY_POWER;
		case DIK_SLEEP:
			return TInputDeviceKeyboard::KEY_SLEEP;
		default:
			return 0;
		}
	}

	TBOOL const TInputDXDeviceKeyboard::BindToDIDevice(HWND a_hMainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, TBOOL a_bExclusive)
	{
		TASSERT(a_poDeviceInstance != NULL);
		TASSERT(a_poDXInputDevice != NULL);

		Initialise();
		Release();

		TUtil::MemCopy(&m_oGUID, &a_poDeviceInstance->guidInstance, sizeof(GUID));
		TIMPLEMENT_D("Weird for loop, could be memcopy although unsure");

		m_poDXInputDevice = a_poDXInputDevice;
		m_DIDevCaps.dwSize = sizeof(DIDEVCAPS);

		m_poDXInputDevice->GetCapabilities(&m_DIDevCaps);
		HRESULT hr = m_poDXInputDevice->SetDataFormat(&c_dfDIKeyboard);

		if (hr != DI_OK)
		{
			return TFALSE;
		}

		if (a_hMainWindow)
		{
			if (m_poDXInputDevice->SetCooperativeLevel(a_hMainWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE) != DI_OK)
			{
				return TFALSE;
			}
		}

		m_poDXInputDevice->EnumObjects(EnumObjectCallback, this, DIDFT_BUTTON);
		return TTRUE;
	}

	BOOL CALLBACK TInputDXDeviceKeyboard::EnumObjectCallback(LPCDIDEVICEOBJECTINSTANCE a_poDeviceInstance, LPVOID a_pvRef)
	{
		return 0;
	}

}
