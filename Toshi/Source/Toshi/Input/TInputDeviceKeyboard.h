#pragma once
#include "TInputInterface.h"



namespace Toshi
{
    class TInputDeviceKeyboard : public TInputDevice
    {
    public:
        #pragma region KEYS
        static const int VIRTKEY_SHIFT = 0x20002;
        static const int VIRTKEY_CONTROL = 0x20003;
        static const int VIRTKEY_ALT = 0x20004;

        static const int KEY_ESCAPE = 0x20005;

        static const int KEY_1 = 0x20006;
        static const int KEY_2 = 0x20007;
        static const int KEY_3 = 0x20008;
        static const int KEY_4 = 0x20009;
        static const int KEY_5 = 0x2000A;
        static const int KEY_6 = 0x2000B;
        static const int KEY_7 = 0x2000C;
        static const int KEY_8 = 0x2000D;
        static const int KEY_9 = 0x2000E;
        static const int KEY_0 = 0x2000F;

        static const int KEY_MINUS = 0x20010;
        static const int KEY_EQUALS = 0x20011;
        static const int KEY_BACK = 0x20012;
        static const int KEY_TAB = 0x20013;

        static const int KEY_Q = 0x20014;
        static const int KEY_W = 0x20015;
        static const int KEY_E = 0x20016;
        static const int KEY_R = 0x20017;
        static const int KEY_T = 0x20018;
        static const int KEY_Y = 0x20019;
        static const int KEY_U = 0x2001A;
        static const int KEY_I = 0x2001B;
        static const int KEY_O = 0x2001C;
        static const int KEY_P = 0x2001D;

        static const int KEY_LBRACKET = 0x2001E;
        static const int KEY_RBRACKET = 0x2001F;
        static const int KEY_RETURN = 0x20020;
        static const int KEY_LCONTROL = 0x20021;

        static const int KEY_A = 0x20022;
        static const int KEY_S = 0x20023;
        static const int KEY_D = 0x20024;
        static const int KEY_F = 0x20025;
        static const int KEY_G = 0x20026;
        static const int KEY_H = 0x20027;
        static const int KEY_J = 0x20028;
        static const int KEY_K = 0x20029;
        static const int KEY_L = 0x2002A;

        static const int KEY_SEMICOLON = 0x2002B;
        static const int KEY_APOSTROPHE = 0x2002C;
        static const int KEY_GRAVE = 0x2002D;
        static const int KEY_LSHIFT = 0x2002E;
        static const int KEY_BACKSLASH = 0x2002F;

        static const int KEY_Z = 0x20030;
        static const int KEY_X = 0x20031;
        static const int KEY_C = 0x20032;
        static const int KEY_V = 0x20033;
        static const int KEY_B = 0x20034;
        static const int KEY_N = 0x20035;
        static const int KEY_M = 0x20036;

        static const int KEY_COMMA = 0x20037;
        static const int KEY_PERIOD = 0x20038;
        static const int KEY_SLASH = 0x20039;
        static const int KEY_RSHIFT = 0x2003A;
        static const int KEY_MULTIPLY = 0x2003B;
        static const int KEY_LMENU = 0x2003c;
        static const int KEY_SPACE = 0x2003d;
        static const int KEY_CAPITAL = 0x2003e;

        static const int KEY_F1 = 0x2003f;
        static const int KEY_F2 = 0x20040;
        static const int KEY_F3 = 0x20041;
        static const int KEY_F4 = 0x20042;
        static const int KEY_F5 = 0x20043;
        static const int KEY_F6 = 0x20044;
        static const int KEY_F7 = 0x20045;
        static const int KEY_F8 = 0x20046;
        static const int KEY_F9 = 0x20047;
        static const int KEY_F10 = 0x20048;

        static const int KEY_NUMLOCK = 0x20049;
        static const int KEY_SCROLL = 0x2004a;
        static const int KEY_NUMPAD7 = 0x2004b;
        static const int KEY_NUMPAD8 = 0x2004c;
        static const int KEY_NUMPAD9 = 0x2004d;
        static const int KEY_SUBTRACT = 0x2004e;
        static const int KEY_NUMPAD4 = 0x2004f;
        static const int KEY_NUMPAD5 = 0x20050;
        static const int KEY_NUMPAD6 = 0x20051;
        static const int KEY_ADD = 0x20052;
        static const int KEY_NUMPAD1 = 0x20053;
        static const int KEY_NUMPAD2 = 0x20054;
        static const int KEY_NUMPAD3 = 0x20055;
        static const int KEY_NUMPAD0 = 0x20056;
        static const int KEY_DECIMAL = 0x20057;
        static const int KEY_F11 = 0x20058;
        static const int KEY_F12 = 0x20059;
        static const int KEY_F13 = 0x2005a;
        static const int KEY_F14 = 0x2005b;
        static const int KEY_F15 = 0x2005c;
        static const int KEY_KANA = 0x2005d;
        static const int KEY_CONVERT = 0x2005e;
        static const int KEY_NOCONVERT = 0x2005f;
        static const int KEY_YEN = 0x20060;
        static const int KEY_NUMPADEQUALS = 0x20061;
        static const int KEY_CIRCUMFLEX = 0x20062;
        static const int KEY_AT = 0x20063;
        static const int KEY_COLON = 0x20064;
        static const int KEY_UNDERLINE = 0x20065;
        static const int KEY_KANJI = 0x20066;
        static const int KEY_STOP = 0x20067;
        static const int KEY_AX = 0x20068;
        static const int KEY_UNLABELED = 0x20069;
        static const int KEY_NUMPADENTER = 0x2006a;
        static const int KEY_RCONTROL = 0x2006b;
        static const int KEY_NUMPADCOMMA = 0x2006c;
        static const int KEY_DIVIDE = 0x2006d;
        static const int KEY_SYSRQ = 0x2006e;
        static const int KEY_RMENU = 0x2006f;
        static const int KEY_HOME = 0x20070;
        static const int KEY_UP = 0x20072;
        static const int KEY_PRIOR = 0x20073;
        static const int KEY_LEFT = 0x20074;
        static const int KEY_RIGHT = 0x20075;
        static const int KEY_END = 0x20076;
        static const int KEY_DOWN = 0x20077;
        static const int KEY_NEXT = 0x20078;
        static const int KEY_INSERT = 0x20079;
        static const int KEY_DELETE = 0x2007a;
        static const int KEY_LWIN = 0x2007b;
        static const int KEY_RWIN = 0x2007c;
        static const int KEY_APPS = 0x2007d;

#define GET_KEY_FROM_DOODAD(doodad) \
    ((doodad) == VIRTKEY_SHIFT ? STRINGIFY(VIRTKEY_SHIFT) : \
    (doodad) == VIRTKEY_CONTROL ? STRINGIFY(VIRTKEY_CONTROL) : \
    (doodad) == VIRTKEY_ALT ? STRINGIFY(VIRTKEY_ALT) : \
    (doodad) == KEY_ESCAPE ? STRINGIFY(KEY_ESCAPE) : \
    (doodad) == KEY_1 ? STRINGIFY(KEY_1) : \
    (doodad) == KEY_2 ? STRINGIFY(KEY_2) : \
    (doodad) == KEY_3 ? STRINGIFY(KEY_3) : \
    (doodad) == KEY_4 ? STRINGIFY(KEY_4) : \
    (doodad) == KEY_5 ? STRINGIFY(KEY_5) : \
    (doodad) == KEY_6 ? STRINGIFY(KEY_6) : \
    (doodad) == KEY_7 ? STRINGIFY(KEY_7) : \
    (doodad) == KEY_8 ? STRINGIFY(KEY_8) : \
    (doodad) == KEY_9 ? STRINGIFY(KEY_9) : \
    (doodad) == KEY_0 ? STRINGIFY(KEY_0) : \
    (doodad) == KEY_MINUS ? STRINGIFY(KEY_MINUS) : \
    (doodad) == KEY_EQUALS ? STRINGIFY(KEY_EQUALS) : \
    (doodad) == KEY_BACK ? STRINGIFY(KEY_BACK) : \
    (doodad) == KEY_TAB ? STRINGIFY(KEY_TAB) : \
    (doodad) == KEY_Q ? STRINGIFY(KEY_Q) : \
    (doodad) == KEY_W ? STRINGIFY(KEY_W) : \
    (doodad) == KEY_E ? STRINGIFY(KEY_E) : \
    (doodad) == KEY_R ? STRINGIFY(KEY_R) : \
    (doodad) == KEY_T ? STRINGIFY(KEY_T) : \
    (doodad) == KEY_Y ? STRINGIFY(KEY_Y) : \
    (doodad) == KEY_U ? STRINGIFY(KEY_U) : \
    (doodad) == KEY_I ? STRINGIFY(KEY_I) : \
    (doodad) == KEY_O ? STRINGIFY(KEY_O) : \
    (doodad) == KEY_P ? STRINGIFY(KEY_P) : \
    (doodad) == KEY_LBRACKET ? STRINGIFY(KEY_LBRACKET) : \
    (doodad) == KEY_RBRACKET ? STRINGIFY(KEY_RBRACKET) : \
    (doodad) == KEY_RETURN ? STRINGIFY(KEY_RETURN) : \
    (doodad) == KEY_LCONTROL ? STRINGIFY(KEY_LCONTROL) : \
    (doodad) == KEY_A ? STRINGIFY(KEY_A) : \
    (doodad) == KEY_S ? STRINGIFY(KEY_S) : \
    (doodad) == KEY_D ? STRINGIFY(KEY_D) : \
    (doodad) == KEY_F ? STRINGIFY(KEY_F) : \
    (doodad) == KEY_G ? STRINGIFY(KEY_G) : \
    (doodad) == KEY_H ? STRINGIFY(KEY_H) : \
    (doodad) == KEY_J ? STRINGIFY(KEY_J) : \
    (doodad) == KEY_K ? STRINGIFY(KEY_K) : \
    (doodad) == KEY_L ? STRINGIFY(KEY_L) : \
    (doodad) == KEY_SEMICOLON ? STRINGIFY(KEY_SEMICOLON) : \
    (doodad) == KEY_APOSTROPHE ? STRINGIFY(KEY_APOSTROPHE) : \
    (doodad) == KEY_GRAVE ? STRINGIFY(KEY_GRAVE) : \
    (doodad) == KEY_LSHIFT ? STRINGIFY(KEY_LSHIFT) : \
    (doodad) == KEY_BACKSLASH ? STRINGIFY(KEY_BACKSLASH) : \
    (doodad) == KEY_Z ? STRINGIFY(KEY_Z) : \
    (doodad) == KEY_X ? STRINGIFY(KEY_X) : \
    (doodad) == KEY_C ? STRINGIFY(KEY_C) : \
    (doodad) == KEY_V ? STRINGIFY(KEY_V) : \
    (doodad) == KEY_B ? STRINGIFY(KEY_B) : \
    (doodad) == KEY_N ? STRINGIFY(KEY_N) : \
    (doodad) == KEY_M ? STRINGIFY(KEY_M) : \
    (doodad) == KEY_COMMA ? STRINGIFY(KEY_COMMA) : \
    (doodad) == KEY_PERIOD ? STRINGIFY(KEY_PERIOD) : \
    (doodad) == KEY_SLASH ? STRINGIFY(KEY_SLASH) : \
    (doodad) == KEY_RSHIFT ? STRINGIFY(KEY_RSHIFT) : \
    (doodad) == KEY_MULTIPLY ? STRINGIFY(KEY_MULTIPLY) : \
    (doodad) == KEY_LMENU ? STRINGIFY(KEY_LMENU) : \
    (doodad) == KEY_SPACE ? STRINGIFY(KEY_SPACE) : \
    (doodad) == KEY_CAPITAL ? STRINGIFY(KEY_CAPITAL) : \
    (doodad) == KEY_F1 ? STRINGIFY(KEY_F1) : \
    (doodad) == KEY_F2 ? STRINGIFY(KEY_F2) : \
    (doodad) == KEY_F3 ? STRINGIFY(KEY_F3) : \
    (doodad) == KEY_F4 ? STRINGIFY(KEY_F4) : \
    (doodad) == KEY_F5 ? STRINGIFY(KEY_F5) : \
    (doodad) == KEY_F6 ? STRINGIFY(KEY_F6) : \
    (doodad) == KEY_F7 ? STRINGIFY(KEY_F7) : \
    (doodad) == KEY_F8 ? STRINGIFY(KEY_F8) : \
    (doodad) == KEY_F9 ? STRINGIFY(KEY_F9) : \
    (doodad) == KEY_F10 ? STRINGIFY(KEY_F10) : \
    (doodad) == KEY_NUMLOCK ? STRINGIFY(KEY_NUMLOCK) : \
    (doodad) == KEY_SCROLL ? STRINGIFY(KEY_SCROLL) : \
    (doodad) == KEY_NUMPAD7 ? STRINGIFY(KEY_NUMPAD7) : \
    (doodad) == KEY_NUMPAD8 ? STRINGIFY(KEY_NUMPAD8) : \
    (doodad) == KEY_NUMPAD9 ? STRINGIFY(KEY_NUMPAD9) : \
    (doodad) == KEY_SUBTRACT ? STRINGIFY(KEY_SUBTRACT) : \
    (doodad) == KEY_NUMPAD4 ? STRINGIFY(KEY_NUMPAD4) : \
    (doodad) == KEY_NUMPAD5 ? STRINGIFY(KEY_NUMPAD5) : \
    (doodad) == KEY_NUMPAD6 ? STRINGIFY(KEY_NUMPAD6) : \
    (doodad) == KEY_ADD ? STRINGIFY(KEY_ADD) : \
    (doodad) == KEY_NUMPAD1 ? STRINGIFY(KEY_NUMPAD1) : \
    (doodad) == KEY_NUMPAD2 ? STRINGIFY(KEY_NUMPAD2) : \
    (doodad) == KEY_NUMPAD3 ? STRINGIFY(KEY_NUMPAD3) : \
    (doodad) == KEY_NUMPAD0 ? STRINGIFY(KEY_NUMPAD0) : \
    (doodad) == KEY_DECIMAL ? STRINGIFY(KEY_DECIMAL) : \
    (doodad) == KEY_F11 ? STRINGIFY(KEY_F11) : \
    (doodad) == KEY_F12 ? STRINGIFY(KEY_F12) : \
    (doodad) == KEY_F13 ? STRINGIFY(KEY_F13) : \
    (doodad) == KEY_F14 ? STRINGIFY(KEY_F14) : \
    (doodad) == KEY_F15 ? STRINGIFY(KEY_F15) : \
    (doodad) == KEY_KANA ? STRINGIFY(KEY_KANA) : \
    (doodad) == KEY_CONVERT ? STRINGIFY(KEY_CONVERT) : \
    (doodad) == KEY_NOCONVERT ? STRINGIFY(KEY_NOCONVERT) : \
    (doodad) == KEY_YEN ? STRINGIFY(KEY_YEN) : \
    (doodad) == KEY_NUMPADEQUALS ? STRINGIFY(KEY_NUMPADEQUALS) : \
    (doodad) == KEY_CIRCUMFLEX ? STRINGIFY(KEY_CIRCUMFLEX) : \
    (doodad) == KEY_AT ? STRINGIFY(KEY_AT) : \
    (doodad) == KEY_COLON ? STRINGIFY(KEY_COLON) : \
    (doodad) == KEY_UNDERLINE ? STRINGIFY(KEY_UNDERLINE) : \
    (doodad) == KEY_KANJI ? STRINGIFY(KEY_KANJI) : \
    (doodad) == KEY_STOP ? STRINGIFY(KEY_STOP) : \
    (doodad) == KEY_AX ? STRINGIFY(KEY_AX) : \
    (doodad) == KEY_UNLABELED ? STRINGIFY(KEY_UNLABELED) : \
    (doodad) == KEY_NUMPADENTER ? STRINGIFY(KEY_NUMPADENTER) : \
    (doodad) == KEY_RCONTROL ? STRINGIFY(KEY_RCONTROL) : \
    (doodad) == KEY_NUMPADCOMMA ? STRINGIFY(KEY_NUMPADCOMMA) : \
    (doodad) == KEY_DIVIDE ? STRINGIFY(KEY_DIVIDE) : \
    (doodad) == KEY_SYSRQ ? STRINGIFY(KEY_SYSRQ) : \
    (doodad) == KEY_RMENU ? STRINGIFY(KEY_RMENU) : \
    (doodad) == KEY_HOME ? STRINGIFY(KEY_HOME) : \
    (doodad) == KEY_UP ? STRINGIFY(KEY_UP) : \
    (doodad) == KEY_PRIOR ? STRINGIFY(KEY_PRIOR) : \
    (doodad) == KEY_LEFT ? STRINGIFY(KEY_LEFT) : \
    (doodad) == KEY_RIGHT ? STRINGIFY(KEY_RIGHT) : \
    (doodad) == KEY_END ? STRINGIFY(KEY_END) : \
    (doodad) == KEY_DOWN ? STRINGIFY(KEY_DOWN) : \
    (doodad) == KEY_NEXT ? STRINGIFY(KEY_NEXT) : \
    (doodad) == KEY_INSERT ? STRINGIFY(KEY_INSERT) : \
    (doodad) == KEY_DELETE ? STRINGIFY(KEY_DELETE) : \
    (doodad) == KEY_LWIN ? STRINGIFY(KEY_LWIN) : \
    (doodad) == KEY_RWIN ? STRINGIFY(KEY_RWIN) : \
    (doodad) == KEY_APPS ? STRINGIFY(KEY_APPS) : \
    "UNKNOWN")
#pragma endregion

        static const int VIRTSTK_DIRECTIONAL_MAPS = 2;
    public:
        virtual TBOOL GetDoodadProperties(int doodad, DoodadProperties& doodadProps) const
        {
            if (doodad >= 0x20000 && doodad < 0x20080)
            {
                doodadProps.m_iUnk = 0;
                doodadProps.m_iUnk2 = 0;
                return TTRUE;
            }
            return TFALSE;
        }

        virtual int ProcessVirtualButtons(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float flt);

        virtual const char* GetButtonFromDoodad(int doodad) const { return GET_KEY_FROM_DOODAD(doodad); }

        virtual int GetAxisCount() { return 0; }
        virtual int GetAxisInt() const { return 0; }
        virtual int GetAxisFloat(int, int) const { return 0; }

        void SetVirtStickDoodads(int a_iMapID, int x, int y, int z, int w);
        void GetVirtStickDoodads(int a_iMapID, int& x, int& y, int& z, int& w);

    protected:
        void UpdateVirtualButtons();

        int  m_mags[4];              // 0x40
        int* m_VirtStickDoodads[4];  // 0x50
        int* m_ipMapArray2[4];       // 0x60
    };
}