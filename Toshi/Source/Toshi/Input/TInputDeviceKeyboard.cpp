#include "ToshiPCH.h"
#include "TInputDeviceKeyboard.h"

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

namespace Toshi {

	int TInputDeviceKeyboard::ProcessVirtualButtons(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float flt)
	{
		emitter.Throw(TInputInterface::InputEvent(this, 0x20001, TInputInterface::InputEvent::EventType_Unk3, m_mags[0], m_mags[1]));
		TIMPLEMENT();
		return 0;
	}

	const char* TInputDeviceKeyboard::GetButtonFromDoodad(int doodad) const
	{
		return GET_KEY_FROM_DOODAD(doodad);
	}

	void TInputDeviceKeyboard::SetVirtStickDoodads(int a_iMapID, int x, int y, int z, int w)
	{
		TASSERT(a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS);
		m_VirtStickDoodads[a_iMapID][0] = x;
		m_VirtStickDoodads[a_iMapID][1] = y;
		m_VirtStickDoodads[a_iMapID][2] = z;
		m_VirtStickDoodads[a_iMapID][3] = w;
	}

	void TInputDeviceKeyboard::GetVirtStickDoodads(int a_iMapID, int& x, int& y, int& z, int& w)
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

	TBOOL TInputDeviceKeyboard::GetDoodadProperties(int doodad, DoodadProperties& doodadProps) const
	{
		if (doodad >= 0x20000 && doodad < 0x20080)
		{
			doodadProps.m_iUnk = 0;
			doodadProps.m_bFlag = TFALSE;
			return TTRUE;
		}

		return TFALSE;
	}

}
