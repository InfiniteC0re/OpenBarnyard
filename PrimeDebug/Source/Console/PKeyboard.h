#pragma once
#include "PrimeDebug.h"

#ifdef PRIMEDEBUG

//-----------------------------------------------------------------------------
// Purpose: the key states the debug tools read (Prime's crInput keyboard
// part), in Prime's key order and with its key names (the names "bind.add"
// and "keys.list" use). The game sets the states of the keys once a frame
// (SetKeyDown) and calls EndFrame after the tools have read them.
// NOTE: Prime polls the keyboard (GetAsyncKeyState) itself; here the game
// feeds the states from its own input devices. One keyboard.
//-----------------------------------------------------------------------------
class PKeyboard
{
public:
	enum KEY
	{
		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_F13,
		KEY_F14,
		KEY_F15,
		KEY_NUMPAD0,
		KEY_NUMPAD1,
		KEY_NUMPAD2,
		KEY_NUMPAD3,
		KEY_NUMPAD4,
		KEY_NUMPAD5,
		KEY_NUMPAD6,
		KEY_NUMPAD7,
		KEY_NUMPAD8,
		KEY_NUMPAD9,
		KEY_ESCAPE,
		KEY_MINUS,
		KEY_BACK,
		KEY_TAB,
		KEY_RETURN,
		KEY_LEFTCONTROL,
		KEY_LEFTSHIFT,
		KEY_COMMA,
		KEY_SLASH,
		KEY_BACKSLASH,
		KEY_PERIOD,
		KEY_RIGHTSHIFT,
		KEY_MULTIPLY,
		KEY_LEFTALT,
		KEY_SPACE,
		KEY_CAPITAL,
		KEY_NUMLOCK,
		KEY_SCROLL,
		KEY_SUBTRACT,
		KEY_ADD,
		KEY_DECIMAL,
		KEY_KANA,
		KEY_CONVERT,
		KEY_NOCONVERT,
		KEY_NUMPADEQUALS,
		KEY_KANJI,
		KEY_RIGHTCONTROL,
		KEY_DIVIDE,
		KEY_RIGHTALT,
		KEY_HOME,
		KEY_PAUSE,
		KEY_UP,
		KEY_PRIOR,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_END,
		KEY_DOWN,
		KEY_NEXT,
		KEY_INSERT,
		KEY_DELETE,
		KEY_LEFTWINDOWS,
		KEY_RIGHTWINDOWS,
		KEY_APPMENU,
		KEY_SLEEP,
		KEY_TILDE,
		KEY_EQUAL,
		KEY_SEMICOLON,
		KEY_QUOTATION,
		KEY_LEFTBRACE,
		KEY_RIGHTBRACE,
		KEY_NUMOF
	};

	static constexpr TINT KEY_NAME_SIZE = 32;

public:
	static void  SetKeyDown( KEY a_eKey, TBOOL a_bDown ) { ms_abKeys[ a_eKey ] = a_bDown; }
	static void  EndFrame();
	static void  Reset();
	static TBOOL IsDown( TINT a_iKey ) { return ms_abKeys[ a_iKey ]; }
	static TBOOL IsPressed( TINT a_iKey );
	static TBOOL IsReleased( TINT a_iKey );

	static TBOOL        GetKeyFromName( TINT* a_pKey, const TCHAR* a_szName );
	static const TCHAR* GetKeyName( TINT a_iKey );

private:
	static TBOOL ms_abKeys[ KEY_NUMOF ];
	static TBOOL ms_abPrevKeys[ KEY_NUMOF ];
};

#endif // PRIMEDEBUG
