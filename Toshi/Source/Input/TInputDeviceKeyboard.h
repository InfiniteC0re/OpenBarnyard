#pragma once
#include "TInputInterface.h"

namespace Toshi
{
	class TInputDeviceKeyboard : public TInputDevice
	{
	public:
		TDECLARE_CLASS(TInputDeviceKeyboard, TInputDevice);

	public:
#pragma region KEYS
		static const TINT VIRTKEY_SHIFT = 0x20002;
		static const TINT VIRTKEY_CONTROL = 0x20003;
		static const TINT VIRTKEY_ALT = 0x20004;

		static const TINT KEY_ESCAPE = 0x20005;

		static const TINT KEY_1 = 0x20006;
		static const TINT KEY_2 = 0x20007;
		static const TINT KEY_3 = 0x20008;
		static const TINT KEY_4 = 0x20009;
		static const TINT KEY_5 = 0x2000A;
		static const TINT KEY_6 = 0x2000B;
		static const TINT KEY_7 = 0x2000C;
		static const TINT KEY_8 = 0x2000D;
		static const TINT KEY_9 = 0x2000E;
		static const TINT KEY_0 = 0x2000F;

		static const TINT KEY_MINUS = 0x20010;
		static const TINT KEY_EQUALS = 0x20011;
		static const TINT KEY_BACK = 0x20012;
		static const TINT KEY_TAB = 0x20013;

		static const TINT KEY_Q = 0x20014;
		static const TINT KEY_W = 0x20015;
		static const TINT KEY_E = 0x20016;
		static const TINT KEY_R = 0x20017;
		static const TINT KEY_T = 0x20018;
		static const TINT KEY_Y = 0x20019;
		static const TINT KEY_U = 0x2001A;
		static const TINT KEY_I = 0x2001B;
		static const TINT KEY_O = 0x2001C;
		static const TINT KEY_P = 0x2001D;

		static const TINT KEY_LBRACKET = 0x2001E;
		static const TINT KEY_RBRACKET = 0x2001F;
		static const TINT KEY_RETURN = 0x20020;
		static const TINT KEY_LCONTROL = 0x20021;

		static const TINT KEY_A = 0x20022;
		static const TINT KEY_S = 0x20023;
		static const TINT KEY_D = 0x20024;
		static const TINT KEY_F = 0x20025;
		static const TINT KEY_G = 0x20026;
		static const TINT KEY_H = 0x20027;
		static const TINT KEY_J = 0x20028;
		static const TINT KEY_K = 0x20029;
		static const TINT KEY_L = 0x2002A;

		static const TINT KEY_SEMICOLON = 0x2002B;
		static const TINT KEY_APOSTROPHE = 0x2002C;
		static const TINT KEY_GRAVE = 0x2002D;
		static const TINT KEY_LSHIFT = 0x2002E;
		static const TINT KEY_BACKSLASH = 0x2002F;

		static const TINT KEY_Z = 0x20030;
		static const TINT KEY_X = 0x20031;
		static const TINT KEY_C = 0x20032;
		static const TINT KEY_V = 0x20033;
		static const TINT KEY_B = 0x20034;
		static const TINT KEY_N = 0x20035;
		static const TINT KEY_M = 0x20036;

		static const TINT KEY_COMMA = 0x20037;
		static const TINT KEY_PERIOD = 0x20038;
		static const TINT KEY_SLASH = 0x20039;
		static const TINT KEY_RSHIFT = 0x2003A;
		static const TINT KEY_MULTIPLY = 0x2003B;
		static const TINT KEY_LMENU = 0x2003c;
		static const TINT KEY_SPACE = 0x2003d;
		static const TINT KEY_CAPITAL = 0x2003e;

		static const TINT KEY_F1 = 0x2003f;
		static const TINT KEY_F2 = 0x20040;
		static const TINT KEY_F3 = 0x20041;
		static const TINT KEY_F4 = 0x20042;
		static const TINT KEY_F5 = 0x20043;
		static const TINT KEY_F6 = 0x20044;
		static const TINT KEY_F7 = 0x20045;
		static const TINT KEY_F8 = 0x20046;
		static const TINT KEY_F9 = 0x20047;
		static const TINT KEY_F10 = 0x20048;

		static const TINT KEY_NUMLOCK = 0x20049;
		static const TINT KEY_SCROLL = 0x2004a;
		static const TINT KEY_NUMPAD7 = 0x2004b;
		static const TINT KEY_NUMPAD8 = 0x2004c;
		static const TINT KEY_NUMPAD9 = 0x2004d;
		static const TINT KEY_SUBTRACT = 0x2004e;
		static const TINT KEY_NUMPAD4 = 0x2004f;
		static const TINT KEY_NUMPAD5 = 0x20050;
		static const TINT KEY_NUMPAD6 = 0x20051;
		static const TINT KEY_ADD = 0x20052;
		static const TINT KEY_NUMPAD1 = 0x20053;
		static const TINT KEY_NUMPAD2 = 0x20054;
		static const TINT KEY_NUMPAD3 = 0x20055;
		static const TINT KEY_NUMPAD0 = 0x20056;
		static const TINT KEY_DECIMAL = 0x20057;
		static const TINT KEY_F11 = 0x20058;
		static const TINT KEY_F12 = 0x20059;
		static const TINT KEY_F13 = 0x2005a;
		static const TINT KEY_F14 = 0x2005b;
		static const TINT KEY_F15 = 0x2005c;
		static const TINT KEY_KANA = 0x2005d;
		static const TINT KEY_CONVERT = 0x2005e;
		static const TINT KEY_NOCONVERT = 0x2005f;
		static const TINT KEY_YEN = 0x20060;
		static const TINT KEY_NUMPADEQUALS = 0x20061;
		static const TINT KEY_CIRCUMFLEX = 0x20062;
		static const TINT KEY_AT = 0x20063;
		static const TINT KEY_COLON = 0x20064;
		static const TINT KEY_UNDERLINE = 0x20065;
		static const TINT KEY_KANJI = 0x20066;
		static const TINT KEY_STOP = 0x20067;
		static const TINT KEY_AX = 0x20068;
		static const TINT KEY_UNLABELED = 0x20069;
		static const TINT KEY_NUMPADENTER = 0x2006a;
		static const TINT KEY_RCONTROL = 0x2006b;
		static const TINT KEY_NUMPADCOMMA = 0x2006c;
		static const TINT KEY_DIVIDE = 0x2006d;
		static const TINT KEY_SYSRQ = 0x2006e;
		static const TINT KEY_RMENU = 0x2006f;
		static const TINT KEY_HOME = 0x20070;
		static const TINT KEY_PAUSE = 0x20071;
		static const TINT KEY_UP = 0x20072;
		static const TINT KEY_PRIOR = 0x20073;
		static const TINT KEY_LEFT = 0x20074;
		static const TINT KEY_RIGHT = 0x20075;
		static const TINT KEY_END = 0x20076;
		static const TINT KEY_DOWN = 0x20077;
		static const TINT KEY_NEXT = 0x20078;
		static const TINT KEY_INSERT = 0x20079;
		static const TINT KEY_DELETE = 0x2007a;
		static const TINT KEY_LWIN = 0x2007b;
		static const TINT KEY_RWIN = 0x2007c;
		static const TINT KEY_APPS = 0x2007d;
		static const TINT KEY_POWER = 0x2007e;
		static const TINT KEY_SLEEP = 0x2007f;
#pragma endregion

		static const size_t KEYBOARD_NUM_DOODADS = 128;
		static const size_t VIRTSTK_DIRECTIONAL_MAPS = 2;

	public:
		TInputDeviceKeyboard();
		~TInputDeviceKeyboard() = default;

		virtual TBOOL GetDoodadProperties(Doodad a_iDoodad, DoodadProperties& doodadProps) const override;

		virtual TINT GetAxisCount() const override { return 0; }
		virtual const TCHAR* GetButtonFromDoodad(Doodad a_iDoodad) const override;
		virtual TINT GetAxisInt(Doodad a_iDoodad, TINT axis) const override { return 0; }
		virtual float GetAxisFloat(Doodad a_iDoodad, TINT axis) const override { return 0; }

		virtual TBOOL IsShiftDown() const = 0;
		virtual TBOOL IsControlDown() const = 0;
		virtual TBOOL IsAltDown() const = 0;
		virtual TBOOL WasDown(Doodad a_iDoodad) const = 0;
		virtual TWCHAR* TranslateDoodadToCharacter(Doodad a_iDoodad) const = 0;
		virtual TINT ProcessVirtualButtons(EventEmitter& emitter, float flt);
		
		void SetVirtStickDoodads(TINT a_iMapID, TINT x, TINT y, TINT z, TINT w);
		void GetVirtStickDoodads(TINT a_iMapID, TINT& x, TINT& y, TINT& z, TINT& w);

	protected:
		void UpdateVirtualButtons();

	protected:
		TINT  m_mags[4];              // 0x40
		TINT* m_VirtStickDoodads[4];  // 0x50
		TINT* m_ipMapArray2[4];       // 0x60
	};
}