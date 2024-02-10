#include "ToshiPCH.h"
#include "TInputDeviceKeyboard.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

#define GET_KEY_FROM_DOODAD(a_iDoodad) \
	((a_iDoodad) == VIRTKEY_SHIFT ? STRINGIFY(VIRTKEY_SHIFT) : \
	(a_iDoodad) == VIRTKEY_CONTROL ? STRINGIFY(VIRTKEY_CONTROL) : \
	(a_iDoodad) == VIRTKEY_ALT ? STRINGIFY(VIRTKEY_ALT) : \
	(a_iDoodad) == KEY_ESCAPE ? STRINGIFY(KEY_ESCAPE) : \
	(a_iDoodad) == KEY_1 ? STRINGIFY(KEY_1) : \
	(a_iDoodad) == KEY_2 ? STRINGIFY(KEY_2) : \
	(a_iDoodad) == KEY_3 ? STRINGIFY(KEY_3) : \
	(a_iDoodad) == KEY_4 ? STRINGIFY(KEY_4) : \
	(a_iDoodad) == KEY_5 ? STRINGIFY(KEY_5) : \
	(a_iDoodad) == KEY_6 ? STRINGIFY(KEY_6) : \
	(a_iDoodad) == KEY_7 ? STRINGIFY(KEY_7) : \
	(a_iDoodad) == KEY_8 ? STRINGIFY(KEY_8) : \
	(a_iDoodad) == KEY_9 ? STRINGIFY(KEY_9) : \
	(a_iDoodad) == KEY_0 ? STRINGIFY(KEY_0) : \
	(a_iDoodad) == KEY_MINUS ? STRINGIFY(KEY_MINUS) : \
	(a_iDoodad) == KEY_EQUALS ? STRINGIFY(KEY_EQUALS) : \
	(a_iDoodad) == KEY_BACK ? STRINGIFY(KEY_BACK) : \
	(a_iDoodad) == KEY_TAB ? STRINGIFY(KEY_TAB) : \
	(a_iDoodad) == KEY_Q ? STRINGIFY(KEY_Q) : \
	(a_iDoodad) == KEY_W ? STRINGIFY(KEY_W) : \
	(a_iDoodad) == KEY_E ? STRINGIFY(KEY_E) : \
	(a_iDoodad) == KEY_R ? STRINGIFY(KEY_R) : \
	(a_iDoodad) == KEY_T ? STRINGIFY(KEY_T) : \
	(a_iDoodad) == KEY_Y ? STRINGIFY(KEY_Y) : \
	(a_iDoodad) == KEY_U ? STRINGIFY(KEY_U) : \
	(a_iDoodad) == KEY_I ? STRINGIFY(KEY_I) : \
	(a_iDoodad) == KEY_O ? STRINGIFY(KEY_O) : \
	(a_iDoodad) == KEY_P ? STRINGIFY(KEY_P) : \
	(a_iDoodad) == KEY_LBRACKET ? STRINGIFY(KEY_LBRACKET) : \
	(a_iDoodad) == KEY_RBRACKET ? STRINGIFY(KEY_RBRACKET) : \
	(a_iDoodad) == KEY_RETURN ? STRINGIFY(KEY_RETURN) : \
	(a_iDoodad) == KEY_LCONTROL ? STRINGIFY(KEY_LCONTROL) : \
	(a_iDoodad) == KEY_A ? STRINGIFY(KEY_A) : \
	(a_iDoodad) == KEY_S ? STRINGIFY(KEY_S) : \
	(a_iDoodad) == KEY_D ? STRINGIFY(KEY_D) : \
	(a_iDoodad) == KEY_F ? STRINGIFY(KEY_F) : \
	(a_iDoodad) == KEY_G ? STRINGIFY(KEY_G) : \
	(a_iDoodad) == KEY_H ? STRINGIFY(KEY_H) : \
	(a_iDoodad) == KEY_J ? STRINGIFY(KEY_J) : \
	(a_iDoodad) == KEY_K ? STRINGIFY(KEY_K) : \
	(a_iDoodad) == KEY_L ? STRINGIFY(KEY_L) : \
	(a_iDoodad) == KEY_SEMICOLON ? STRINGIFY(KEY_SEMICOLON) : \
	(a_iDoodad) == KEY_APOSTROPHE ? STRINGIFY(KEY_APOSTROPHE) : \
	(a_iDoodad) == KEY_GRAVE ? STRINGIFY(KEY_GRAVE) : \
	(a_iDoodad) == KEY_LSHIFT ? STRINGIFY(KEY_LSHIFT) : \
	(a_iDoodad) == KEY_BACKSLASH ? STRINGIFY(KEY_BACKSLASH) : \
	(a_iDoodad) == KEY_Z ? STRINGIFY(KEY_Z) : \
	(a_iDoodad) == KEY_X ? STRINGIFY(KEY_X) : \
	(a_iDoodad) == KEY_C ? STRINGIFY(KEY_C) : \
	(a_iDoodad) == KEY_V ? STRINGIFY(KEY_V) : \
	(a_iDoodad) == KEY_B ? STRINGIFY(KEY_B) : \
	(a_iDoodad) == KEY_N ? STRINGIFY(KEY_N) : \
	(a_iDoodad) == KEY_M ? STRINGIFY(KEY_M) : \
	(a_iDoodad) == KEY_COMMA ? STRINGIFY(KEY_COMMA) : \
	(a_iDoodad) == KEY_PERIOD ? STRINGIFY(KEY_PERIOD) : \
	(a_iDoodad) == KEY_SLASH ? STRINGIFY(KEY_SLASH) : \
	(a_iDoodad) == KEY_RSHIFT ? STRINGIFY(KEY_RSHIFT) : \
	(a_iDoodad) == KEY_MULTIPLY ? STRINGIFY(KEY_MULTIPLY) : \
	(a_iDoodad) == KEY_LMENU ? STRINGIFY(KEY_LMENU) : \
	(a_iDoodad) == KEY_SPACE ? STRINGIFY(KEY_SPACE) : \
	(a_iDoodad) == KEY_CAPITAL ? STRINGIFY(KEY_CAPITAL) : \
	(a_iDoodad) == KEY_F1 ? STRINGIFY(KEY_F1) : \
	(a_iDoodad) == KEY_F2 ? STRINGIFY(KEY_F2) : \
	(a_iDoodad) == KEY_F3 ? STRINGIFY(KEY_F3) : \
	(a_iDoodad) == KEY_F4 ? STRINGIFY(KEY_F4) : \
	(a_iDoodad) == KEY_F5 ? STRINGIFY(KEY_F5) : \
	(a_iDoodad) == KEY_F6 ? STRINGIFY(KEY_F6) : \
	(a_iDoodad) == KEY_F7 ? STRINGIFY(KEY_F7) : \
	(a_iDoodad) == KEY_F8 ? STRINGIFY(KEY_F8) : \
	(a_iDoodad) == KEY_F9 ? STRINGIFY(KEY_F9) : \
	(a_iDoodad) == KEY_F10 ? STRINGIFY(KEY_F10) : \
	(a_iDoodad) == KEY_NUMLOCK ? STRINGIFY(KEY_NUMLOCK) : \
	(a_iDoodad) == KEY_SCROLL ? STRINGIFY(KEY_SCROLL) : \
	(a_iDoodad) == KEY_NUMPAD7 ? STRINGIFY(KEY_NUMPAD7) : \
	(a_iDoodad) == KEY_NUMPAD8 ? STRINGIFY(KEY_NUMPAD8) : \
	(a_iDoodad) == KEY_NUMPAD9 ? STRINGIFY(KEY_NUMPAD9) : \
	(a_iDoodad) == KEY_SUBTRACT ? STRINGIFY(KEY_SUBTRACT) : \
	(a_iDoodad) == KEY_NUMPAD4 ? STRINGIFY(KEY_NUMPAD4) : \
	(a_iDoodad) == KEY_NUMPAD5 ? STRINGIFY(KEY_NUMPAD5) : \
	(a_iDoodad) == KEY_NUMPAD6 ? STRINGIFY(KEY_NUMPAD6) : \
	(a_iDoodad) == KEY_ADD ? STRINGIFY(KEY_ADD) : \
	(a_iDoodad) == KEY_NUMPAD1 ? STRINGIFY(KEY_NUMPAD1) : \
	(a_iDoodad) == KEY_NUMPAD2 ? STRINGIFY(KEY_NUMPAD2) : \
	(a_iDoodad) == KEY_NUMPAD3 ? STRINGIFY(KEY_NUMPAD3) : \
	(a_iDoodad) == KEY_NUMPAD0 ? STRINGIFY(KEY_NUMPAD0) : \
	(a_iDoodad) == KEY_DECIMAL ? STRINGIFY(KEY_DECIMAL) : \
	(a_iDoodad) == KEY_F11 ? STRINGIFY(KEY_F11) : \
	(a_iDoodad) == KEY_F12 ? STRINGIFY(KEY_F12) : \
	(a_iDoodad) == KEY_F13 ? STRINGIFY(KEY_F13) : \
	(a_iDoodad) == KEY_F14 ? STRINGIFY(KEY_F14) : \
	(a_iDoodad) == KEY_F15 ? STRINGIFY(KEY_F15) : \
	(a_iDoodad) == KEY_KANA ? STRINGIFY(KEY_KANA) : \
	(a_iDoodad) == KEY_CONVERT ? STRINGIFY(KEY_CONVERT) : \
	(a_iDoodad) == KEY_NOCONVERT ? STRINGIFY(KEY_NOCONVERT) : \
	(a_iDoodad) == KEY_YEN ? STRINGIFY(KEY_YEN) : \
	(a_iDoodad) == KEY_NUMPADEQUALS ? STRINGIFY(KEY_NUMPADEQUALS) : \
	(a_iDoodad) == KEY_CIRCUMFLEX ? STRINGIFY(KEY_CIRCUMFLEX) : \
	(a_iDoodad) == KEY_AT ? STRINGIFY(KEY_AT) : \
	(a_iDoodad) == KEY_COLON ? STRINGIFY(KEY_COLON) : \
	(a_iDoodad) == KEY_UNDERLINE ? STRINGIFY(KEY_UNDERLINE) : \
	(a_iDoodad) == KEY_KANJI ? STRINGIFY(KEY_KANJI) : \
	(a_iDoodad) == KEY_STOP ? STRINGIFY(KEY_STOP) : \
	(a_iDoodad) == KEY_AX ? STRINGIFY(KEY_AX) : \
	(a_iDoodad) == KEY_UNLABELED ? STRINGIFY(KEY_UNLABELED) : \
	(a_iDoodad) == KEY_NUMPADENTER ? STRINGIFY(KEY_NUMPADENTER) : \
	(a_iDoodad) == KEY_RCONTROL ? STRINGIFY(KEY_RCONTROL) : \
	(a_iDoodad) == KEY_NUMPADCOMMA ? STRINGIFY(KEY_NUMPADCOMMA) : \
	(a_iDoodad) == KEY_DIVIDE ? STRINGIFY(KEY_DIVIDE) : \
	(a_iDoodad) == KEY_SYSRQ ? STRINGIFY(KEY_SYSRQ) : \
	(a_iDoodad) == KEY_RMENU ? STRINGIFY(KEY_RMENU) : \
	(a_iDoodad) == KEY_HOME ? STRINGIFY(KEY_HOME) : \
	(a_iDoodad) == KEY_UP ? STRINGIFY(KEY_UP) : \
	(a_iDoodad) == KEY_PRIOR ? STRINGIFY(KEY_PRIOR) : \
	(a_iDoodad) == KEY_LEFT ? STRINGIFY(KEY_LEFT) : \
	(a_iDoodad) == KEY_RIGHT ? STRINGIFY(KEY_RIGHT) : \
	(a_iDoodad) == KEY_END ? STRINGIFY(KEY_END) : \
	(a_iDoodad) == KEY_DOWN ? STRINGIFY(KEY_DOWN) : \
	(a_iDoodad) == KEY_NEXT ? STRINGIFY(KEY_NEXT) : \
	(a_iDoodad) == KEY_INSERT ? STRINGIFY(KEY_INSERT) : \
	(a_iDoodad) == KEY_DELETE ? STRINGIFY(KEY_DELETE) : \
	(a_iDoodad) == KEY_LWIN ? STRINGIFY(KEY_LWIN) : \
	(a_iDoodad) == KEY_RWIN ? STRINGIFY(KEY_RWIN) : \
	(a_iDoodad) == KEY_APPS ? STRINGIFY(KEY_APPS) : \
	"UNKNOWN")

namespace Toshi {

	TDEFINE_CLASS_NORUNTIME(TInputDeviceKeyboard);

	TINT TInputDeviceKeyboard::ProcessVirtualButtons(EventEmitter& emitter, float flt)
	{
		emitter.Throw(TInputInterface::InputEvent(this, 0x20001, TInputInterface::EVENT_TYPE_UNKNOWN, m_mags[0], m_mags[1]));
		TIMPLEMENT();
		return 0;
	}

	const TCHAR* TInputDeviceKeyboard::GetButtonFromDoodad(Doodad a_iDoodad) const
	{
		return GET_KEY_FROM_DOODAD(a_iDoodad);
	}

	void TInputDeviceKeyboard::SetVirtStickDoodads(TINT a_iMapID, TINT x, TINT y, TINT z, TINT w)
	{
		TASSERT(a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS);
		m_VirtStickDoodads[a_iMapID][0] = x;
		m_VirtStickDoodads[a_iMapID][1] = y;
		m_VirtStickDoodads[a_iMapID][2] = z;
		m_VirtStickDoodads[a_iMapID][3] = w;
	}

	void TInputDeviceKeyboard::GetVirtStickDoodads(TINT a_iMapID, TINT& x, TINT& y, TINT& z, TINT& w)
	{
		TASSERT(a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS);
		x = m_VirtStickDoodads[a_iMapID][0];
		y = m_VirtStickDoodads[a_iMapID][1];
		z = m_VirtStickDoodads[a_iMapID][2];
		w = m_VirtStickDoodads[a_iMapID][3];
	}

	void TInputDeviceKeyboard::UpdateVirtualButtons()
	{
		m_mags[2] = m_mags[0];
		m_mags[3] = m_mags[1];
		m_mags[0] = 0;
		m_mags[1] = 0;

		TIMPLEMENT();
	}

	TInputDeviceKeyboard::TInputDeviceKeyboard()
	{
		m_VirtStickDoodads[0] = TNULL;
		m_VirtStickDoodads[1] = TNULL;
		m_VirtStickDoodads[2] = TNULL;
		m_VirtStickDoodads[3] = TNULL;
		m_ipMapArray2[0] = TNULL;
		m_ipMapArray2[1] = TNULL;
		m_ipMapArray2[2] = TNULL;
		m_ipMapArray2[3] = TNULL;
	}

	TBOOL TInputDeviceKeyboard::GetDoodadProperties(Doodad a_iDoodad, DoodadProperties& doodadProps) const
	{
		if ((0x1ffff < a_iDoodad) && (a_iDoodad < 0x20080))
		{
			doodadProps.m_iUnk = 0;
			doodadProps.m_bFlag = TFALSE;
			return TTRUE;
		}

		return TFALSE;
	}

}
