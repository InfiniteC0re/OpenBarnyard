#pragma once
#include "TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputDeviceKeyboard : public TInputDevice
{
public:
	TDECLARE_CLASS( TInputDeviceKeyboard, TInputDevice );

public:
#pragma region KEYS
	enum : Doodad
	{
		DOODAD_START     = 0x20000,
		VIRTKEY          = 0x20001,
		VIRTKEY_SHIFT    = 0x20002,
		VIRTKEY_CONTROL  = 0x20003,
		VIRTKEY_ALT      = 0x20004,
		KEY_ESCAPE       = 0x20005,
		KEY_1            = 0x20006,
		KEY_2            = 0x20007,
		KEY_3            = 0x20008,
		KEY_4            = 0x20009,
		KEY_5            = 0x2000A,
		KEY_6            = 0x2000B,
		KEY_7            = 0x2000C,
		KEY_8            = 0x2000D,
		KEY_9            = 0x2000E,
		KEY_0            = 0x2000F,
		KEY_MINUS        = 0x20010,
		KEY_EQUALS       = 0x20011,
		KEY_BACK         = 0x20012,
		KEY_TAB          = 0x20013,
		KEY_Q            = 0x20014,
		KEY_W            = 0x20015,
		KEY_E            = 0x20016,
		KEY_R            = 0x20017,
		KEY_T            = 0x20018,
		KEY_Y            = 0x20019,
		KEY_U            = 0x2001A,
		KEY_I            = 0x2001B,
		KEY_O            = 0x2001C,
		KEY_P            = 0x2001D,
		KEY_LBRACKET     = 0x2001E,
		KEY_RBRACKET     = 0x2001F,
		KEY_RETURN       = 0x20020,
		KEY_LCONTROL     = 0x20021,
		KEY_A            = 0x20022,
		KEY_S            = 0x20023,
		KEY_D            = 0x20024,
		KEY_F            = 0x20025,
		KEY_G            = 0x20026,
		KEY_H            = 0x20027,
		KEY_J            = 0x20028,
		KEY_K            = 0x20029,
		KEY_L            = 0x2002A,
		KEY_SEMICOLON    = 0x2002B,
		KEY_APOSTROPHE   = 0x2002C,
		KEY_GRAVE        = 0x2002D,
		KEY_LSHIFT       = 0x2002E,
		KEY_BACKSLASH    = 0x2002F,
		KEY_Z            = 0x20030,
		KEY_X            = 0x20031,
		KEY_C            = 0x20032,
		KEY_V            = 0x20033,
		KEY_B            = 0x20034,
		KEY_N            = 0x20035,
		KEY_M            = 0x20036,
		KEY_COMMA        = 0x20037,
		KEY_PERIOD       = 0x20038,
		KEY_SLASH        = 0x20039,
		KEY_RSHIFT       = 0x2003A,
		KEY_MULTIPLY     = 0x2003B,
		KEY_LMENU        = 0x2003c,
		KEY_SPACE        = 0x2003d,
		KEY_CAPITAL      = 0x2003e,
		KEY_F1           = 0x2003f,
		KEY_F2           = 0x20040,
		KEY_F3           = 0x20041,
		KEY_F4           = 0x20042,
		KEY_F5           = 0x20043,
		KEY_F6           = 0x20044,
		KEY_F7           = 0x20045,
		KEY_F8           = 0x20046,
		KEY_F9           = 0x20047,
		KEY_F10          = 0x20048,
		KEY_NUMLOCK      = 0x20049,
		KEY_SCROLL       = 0x2004a,
		KEY_NUMPAD7      = 0x2004b,
		KEY_NUMPAD8      = 0x2004c,
		KEY_NUMPAD9      = 0x2004d,
		KEY_SUBTRACT     = 0x2004e,
		KEY_NUMPAD4      = 0x2004f,
		KEY_NUMPAD5      = 0x20050,
		KEY_NUMPAD6      = 0x20051,
		KEY_ADD          = 0x20052,
		KEY_NUMPAD1      = 0x20053,
		KEY_NUMPAD2      = 0x20054,
		KEY_NUMPAD3      = 0x20055,
		KEY_NUMPAD0      = 0x20056,
		KEY_DECIMAL      = 0x20057,
		KEY_F11          = 0x20058,
		KEY_F12          = 0x20059,
		KEY_F13          = 0x2005a,
		KEY_F14          = 0x2005b,
		KEY_F15          = 0x2005c,
		KEY_KANA         = 0x2005d,
		KEY_CONVERT      = 0x2005e,
		KEY_NOCONVERT    = 0x2005f,
		KEY_YEN          = 0x20060,
		KEY_NUMPADEQUALS = 0x20061,
		KEY_CIRCUMFLEX   = 0x20062,
		KEY_AT           = 0x20063,
		KEY_COLON        = 0x20064,
		KEY_UNDERLINE    = 0x20065,
		KEY_KANJI        = 0x20066,
		KEY_STOP         = 0x20067,
		KEY_AX           = 0x20068,
		KEY_UNLABELED    = 0x20069,
		KEY_NUMPADENTER  = 0x2006a,
		KEY_RCONTROL     = 0x2006b,
		KEY_NUMPADCOMMA  = 0x2006c,
		KEY_DIVIDE       = 0x2006d,
		KEY_SYSRQ        = 0x2006e,
		KEY_RMENU        = 0x2006f,
		KEY_HOME         = 0x20070,
		KEY_PAUSE        = 0x20071,
		KEY_UP           = 0x20072,
		KEY_PRIOR        = 0x20073,
		KEY_LEFT         = 0x20074,
		KEY_RIGHT        = 0x20075,
		KEY_END          = 0x20076,
		KEY_DOWN         = 0x20077,
		KEY_NEXT         = 0x20078,
		KEY_INSERT       = 0x20079,
		KEY_DELETE       = 0x2007a,
		KEY_LWIN         = 0x2007b,
		KEY_RWIN         = 0x2007c,
		KEY_APPS         = 0x2007d,
		KEY_POWER        = 0x2007e,
		KEY_SLEEP        = 0x2007f,
		DOODAD_END       = 0x20080
	};
#pragma endregion

	inline static constexpr TUINT KEYBOARD_NUM_DOODADS     = 128;
	inline static constexpr TUINT VIRTSTK_DIRECTIONAL_MAPS = 2;

public:
	TInputDeviceKeyboard();
	~TInputDeviceKeyboard() = default;

	virtual TBOOL GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& doodadProps ) const override;

	virtual TINT         GetAxisCount() const override { return 0; }
	virtual const TCHAR* GetButtonFromDoodad( Doodad a_iDoodad ) const override;
	virtual TINT         GetAxisInt( Doodad a_iDoodad, TINT axis ) const override { return 0; }
	virtual float        GetAxisFloat( Doodad a_iDoodad, TINT axis ) const override { return 0; }

	virtual TBOOL   IsShiftDown() const                                  = 0;
	virtual TBOOL   IsControlDown() const                                = 0;
	virtual TBOOL   IsAltDown() const                                    = 0;
	virtual TBOOL   WasDown( Doodad a_iDoodad ) const                    = 0;
	virtual TWCHAR* TranslateDoodadToCharacter( Doodad a_iDoodad ) const = 0;
	virtual TINT    ProcessVirtualButtons( EventEmitter& a_rEmitter, TFLOAT flt );

	void SetVirtStickDoodads( TINT a_iMapID, Doodad a_iDoodad1, Doodad a_iDoodad2, Doodad a_iDoodad3, Doodad a_iDoodad4 );
	void GetVirtStickDoodads( TINT a_iMapID, Doodad& a_rDoodad1, Doodad& a_rDoodad2, Doodad& a_rDoodad3, Doodad& a_rDoodad4 );

protected:
	void UpdateVirtualButtons();

protected:
	struct VirtStickDoodad
	{
		Doodad iDoodad1 = 0;
		Doodad iDoodad2 = 0;
		Doodad iDoodad3 = 0;
		Doodad iDoodad4 = 0;
	};

	TFLOAT          m_aMags[ 4 ];                                    // 0x40
	VirtStickDoodad m_aVirtStickDoodads[ VIRTSTK_DIRECTIONAL_MAPS ]; // 0x50
};

TOSHI_NAMESPACE_END
