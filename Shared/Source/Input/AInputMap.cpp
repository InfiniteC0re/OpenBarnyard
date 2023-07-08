#include "pch.h"
#include "AInputMap.h"
#include "Toshi/Input/TInputDeviceKeyboard.h"

using namespace Toshi;

#define BIND_MAP
#define BIND_VECTOR oBinds
#define BIND_ACTION3(MAP, ACTION, KEY1)             { BIND_VECTOR.PushBack(1); BIND_VECTOR.PushBack(KEY1); MAP.Insert(ACTION, BIND_VECTOR); BIND_VECTOR.Clear(); }
#define BIND_ACTION4(MAP, ACTION, KEY1, KEY2)       { BIND_VECTOR.PushBack(2); BIND_VECTOR.PushBack(KEY1); BIND_VECTOR.PushBack(KEY2); MAP.Insert(ACTION, BIND_VECTOR); BIND_VECTOR.Clear(); }
#define BIND_ACTION5(MAP, ACTION, KEY1, KEY2, KEY3) { BIND_VECTOR.PushBack(3); BIND_VECTOR.PushBack(KEY1); BIND_VECTOR.PushBack(KEY2); BIND_VECTOR.PushBack(KEY3); MAP.Insert(ACTION, BIND_VECTOR); BIND_VECTOR.Clear(); }
#define BIND_ACTION(...) CALL_OVERLOAD(BIND_ACTION, __VA_ARGS__)

void AInputMap::InitialiseButtonMap()
{
	BindVector oBinds;
	
	// ActionButtonMap1
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_0, TInputDeviceKeyboard::KEY_ESCAPE, 0x10009);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_1, TInputDeviceKeyboard::KEY_SPACE, 0x1000b);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_2, TInputDeviceKeyboard::KEY_UP, 0x10001, 0x10011);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_3, TInputDeviceKeyboard::KEY_DOWN, 0x10003, 0x10013);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_4, TInputDeviceKeyboard::KEY_LEFT, 0x10004, 0x10014);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_5, TInputDeviceKeyboard::KEY_RIGHT, 0x10002, 0x10012);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_6, TInputDeviceKeyboard::KEY_RETURN, TInputDeviceKeyboard::KEY_RBRACKET, 0x10006);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_8, TInputDeviceKeyboard::KEY_PERIOD, 0x10010);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_9, TInputDeviceKeyboard::KEY_COMMA, 0x1000E);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_10, TInputDeviceKeyboard::KEY_F2);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_20, TInputDeviceKeyboard::KEY_GRAVE);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_21, TInputDeviceKeyboard::KEY_T, 0x10009);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_22, TInputDeviceKeyboard::VIRTKEY_ALT, 0x10019);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_23, TInputDeviceKeyboard::KEY_R);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_26, TInputDeviceKeyboard::KEY_F1);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_27, TInputDeviceKeyboard::KEY_F5);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_18, TInputDeviceKeyboard::KEY_F6);
	BIND_ACTION(m_ActionButtonMap1, ACTIONBUTTON_19, TInputDeviceKeyboard::KEY_F7);

	// ActionButtonMap2
	BIND_ACTION(m_ActionButtonMap2, ACTIONBUTTON_20, TInputDeviceKeyboard::KEY_GRAVE, TInputDeviceKeyboard::KEY_ESCAPE, 0x10009);
	BIND_ACTION(m_ActionButtonMap2, ACTIONBUTTON_24, TInputDeviceKeyboard::KEY_BACK);
	BIND_ACTION(m_ActionButtonMap2, ACTIONBUTTON_2, TInputDeviceKeyboard::KEY_UP, 0x10001, 0x10011);
	BIND_ACTION(m_ActionButtonMap2, ACTIONBUTTON_3, TInputDeviceKeyboard::KEY_DOWN, 0x10003, 0x10013);
	BIND_ACTION(m_ActionButtonMap2, ACTIONBUTTON_25, TInputDeviceKeyboard::KEY_TAB);
	BIND_ACTION(m_ActionButtonMap2, ACTIONBUTTON_28, TInputDeviceKeyboard::KEY_RETURN);
	BIND_ACTION(m_ActionButtonMap2, ACTIONBUTTON_29, TInputDeviceKeyboard::KEY_A);

	// ActionButtonMap5
	BIND_ACTION(m_ActionButtonMap5, ACTIONBUTTON_1, 0x10006, TInputDeviceKeyboard::KEY_RETURN);

	// ActionButtonMap8
	BIND_ACTION(m_ActionButtonMap8, ACTIONBUTTON_42, 0x1000B, 0x10009, TInputDeviceKeyboard::KEY_SPACE);
	BIND_ACTION(m_ActionButtonMap8, ACTIONBUTTON_43, 0x10006, TInputDeviceKeyboard::KEY_RETURN);

	TIMPLEMENT_D("Init other ActionButtonMaps");
}

void AInputMap::NamePadDoodads()
{
	// Controller
	m_NamePadDoodadsMap.Insert(0x10001, 0x75E);
	m_NamePadDoodadsMap.Insert(0x10002, 0x75F);
	m_NamePadDoodadsMap.Insert(0x10003, 0x760);
	m_NamePadDoodadsMap.Insert(0x10004, 0x761);
	m_NamePadDoodadsMap.Insert(0x10005, 0x762);
	m_NamePadDoodadsMap.Insert(0x10006, 0x763);
	m_NamePadDoodadsMap.Insert(0x10007, 0x764);
	m_NamePadDoodadsMap.Insert(0x10008, 0x765);
	m_NamePadDoodadsMap.Insert(0x10009, 0x766);
	m_NamePadDoodadsMap.Insert(0x1000A, 0x767);
	m_NamePadDoodadsMap.Insert(0x1000B, 0x768);
	m_NamePadDoodadsMap.Insert(0x1000C, 0x769);
	m_NamePadDoodadsMap.Insert(0x1000D, 0x76A);
	m_NamePadDoodadsMap.Insert(0x1000E, 0x76B);
	m_NamePadDoodadsMap.Insert(0x1000F, 0x76C);
	m_NamePadDoodadsMap.Insert(0x10010, 0x76D);
	m_NamePadDoodadsMap.Insert(0x10011, 0x76E);
	m_NamePadDoodadsMap.Insert(0x10012, 0x76F);
	m_NamePadDoodadsMap.Insert(0x10013, 0x770);
	m_NamePadDoodadsMap.Insert(0x10014, 0x771);
	m_NamePadDoodadsMap.Insert(0x10015, 0x772);
	m_NamePadDoodadsMap.Insert(0x10016, 0x773);
	m_NamePadDoodadsMap.Insert(0x10017, 0x774);
	m_NamePadDoodadsMap.Insert(0x10018, 0x775);
	m_NamePadDoodadsMap.Insert(0x10019, 0x776);
	m_NamePadDoodadsMap.Insert(0x1001A, 0x777);
	m_NamePadDoodadsMap.Insert(0x1001B, 0x778);

	// Keyboard
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_ESCAPE, 0x779);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_1, 0x77A);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_2, 0x77B);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_3, 0x77C);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_4, 0x77D);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_5, 0x77E);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_6, 0x77F);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_7, 0x780);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_8, 0x781);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_9, 0x782);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_0, 0x783);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_MINUS, 0x784);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_EQUALS, 0x785);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_BACK, 0x786);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_TAB, 0x787);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_Q, 0x788);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_W, 0x789);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_E, 0x78A);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_R, 0x78B);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_T, 0x78C);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_Y, 0x78D);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_U, 0x78E);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_I, 0x78F);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_O, 0x790);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_P, 0x791);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_LBRACKET, 0x792);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_RBRACKET, 0x793);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_RETURN, 0x794);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_LCONTROL, 0x795);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_A, 0x796);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_S, 0x797);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_D, 0x798);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F, 0x799);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_G, 0x79A);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_H, 0x79B);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_J, 0x79C);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_K, 0x79D);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_L, 0x79E);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_SEMICOLON, 0x79F);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_APOSTROPHE, 0x7A0);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_GRAVE, 0x7A1);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_LSHIFT, 0x7A2);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_BACKSLASH, 0x7A3);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_Z, 0x7A4);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_X, 0x7A5);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_C, 0x7A6);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_V, 0x7A7);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_B, 0x7A8);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_N, 0x7A9);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_M, 0x7AA);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_COMMA, 0x7AB);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_PERIOD, 0x7AC);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_SLASH, 0x7AD);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_RSHIFT, 0x7AE);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_MULTIPLY, 0x7AF);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_LMENU, 0x7B0);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_SPACE, 0x7B1);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_CAPITAL, 0x7B2);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F1, 0x7B3);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F2, 0x7B4);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F3, 0x7B5);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F4, 0x7B6);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F5, 0x7B7);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F6, 0x7B8);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F7, 0x7B9);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F8, 0x7BA);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F9, 0x7BB);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F10, 0x7BC);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMLOCK, 0x7BD);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_SCROLL, 0x7BE);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD7, 0x7BF);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD8, 0x7C0);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD9, 0x7C1);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_SUBTRACT, 0x7C2);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD4, 0x7C3);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD5, 0x7C4);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD6, 0x7C5);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_ADD, 0x7C6);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD1, 0x7C7);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD2, 0x7C8);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD3, 0x7C9);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPAD0, 0x7CA);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_DECIMAL, 0x7CB);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F11, 0x7CC);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F12, 0x7CD);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F13, 0x7CE);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F14, 0x7DF);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_F15, 0x7D0);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_KANA, 0x7D1);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_CONVERT, 0x7D2);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NOCONVERT, 0x7D3);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_YEN, 0x7D4);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPADEQUALS, 0x7D5);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_CIRCUMFLEX, 0x7D6);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_AT, 0x7D7);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_COLON, 0x7D8);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_UNDERLINE, 0x7D9);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_KANJI, 0x7DA);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_STOP, 0x7DB);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_AX, 0x7DC);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_UNLABELED, 0x7DD);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPADENTER, 0x7DE);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_RCONTROL, 0x7DF);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_NUMPADCOMMA, 0x7E0);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_DIVIDE, 0x7E1);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_SYSRQ, 0x7E2);
	m_NamePadDoodadsMap.Insert(TInputDeviceKeyboard::KEY_RMENU, 0x7E3);
	TIMPLEMENT();
}

AInputMap::ButtonMap* AInputMap::GetButtonMap(INPUTCONTEXT m_eInputContext)
{
	switch (m_eInputContext) {
	case AInputMap::INPUTCONTEXT_UNK:
		return &m_ActionButtonMap1;
	case AInputMap::INPUTCONTEXT_UNK2:
		return &m_ActionButtonMap2;
	case AInputMap::INPUTCONTEXT_UNK3:
		return &m_ActionButtonMap3;
	case AInputMap::INPUTCONTEXT_UNK4:
		return &m_ActionButtonMap4;
	case AInputMap::INPUTCONTEXT_UNK5:
		return &m_ActionButtonMap5;
	case AInputMap::INPUTCONTEXT_UNK6:
		return &m_ActionButtonMap6;
	case AInputMap::INPUTCONTEXT_UNK7:
		return &m_ActionButtonMap7;
	case AInputMap::INPUTCONTEXT_UNK8:
		return &m_ActionButtonMap8;
	case AInputMap::INPUTCONTEXT_UNK9:
		return &m_ActionButtonMap9;
	case AInputMap::INPUTCONTEXT_UNK10:
		return &m_ActionButtonMap10;
	default:
		return TNULL;
	}
}
