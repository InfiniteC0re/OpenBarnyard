#include "ToshiPCH.h"
#include "TInputDeviceKeyboard_Win.h"

namespace Toshi {

	TInputDXDeviceKeyboard::TInputDXDeviceKeyboard()
	{
		m_iSomeNum = 0;
		m_pKeyStates1 = TNULL;
		m_pKeyStates2 = TNULL;
		m_poDXInputDevice = TNULL;
		m_bFlag = TFALSE;
		m_aKeys.InitialiseAll({ TNULL });
	}

	void TInputDXDeviceKeyboard::Release()
	{
		if (m_poDXInputDevice)
		{
			m_poDXInputDevice->Release();
			m_poDXInputDevice = TNULL;
		}
	}

	TBOOL TInputDXDeviceKeyboard::Acquire()
	{
		m_bIsUpdating = TFALSE;
		HRESULT hRes = m_poDXInputDevice->Acquire();

		if (SUCCEEDED(hRes))
		{
			SetAcquired(TTRUE);

			if (hRes != S_FALSE)
				RefreshDirect();
		}
		else
		{
			SetAcquired(TFALSE);
		}

		return TTRUE;
	}

	TBOOL TInputDXDeviceKeyboard::Unacquire()
	{
		m_bIsUpdating = TFALSE;
		SetAcquired(TFALSE);
		
		return SUCCEEDED(m_poDXInputDevice->Unacquire());
	}

	void TInputDXDeviceKeyboard::Update(float deltaTime)
	{
		if (IsAcquired())
			m_bIsUpdating = TTRUE;
	}

	TBOOL TInputDXDeviceKeyboard::Flush()
	{
		if (IsAcquired())
		{
			DWORD dwNumElements = -1;
			HRESULT hRes = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &dwNumElements, 0);

			if (SUCCEEDED(hRes))
			{
				RefreshDirect();
				return TTRUE;
			}
			else
			{
				m_bIsUpdating = TFALSE;
			}
		}

		return TFALSE;
	}

	int TInputDXDeviceKeyboard::ProcessEvents(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float deltaTime)
	{
		if (m_bIsUpdating)
		{
			if (m_bFlag)
			{
				for (size_t i = 0; i < 256; i++)
				{
					if (m_pKeyStates1[i] != m_pKeyStates2[i])
						HandleKeyChange(emitter, i, m_pKeyStates1[i]);
				}

				m_bFlag = TFALSE;
			}

			DWORD dwNumElements = 32;
			DIDEVICEOBJECTDATA aDeviceData[32];
			TUtil::MemClear(aDeviceData, sizeof(aDeviceData));
			HRESULT hRes = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), aDeviceData, &dwNumElements, 0);
			
			if (SUCCEEDED(hRes))
			{
				int iNumProcessed = 0;
				iNumProcessed += ProcessRepeats(emitter, deltaTime);
				iNumProcessed += ProcessVirtualButtons(emitter, deltaTime);

				for (DWORD i = 0; i < dwNumElements; i++)
				{
					HandleKeyChange(emitter, aDeviceData[i].dwOfs, TSTATICCAST(uint8_t, aDeviceData[i].dwData));

					if (aDeviceData[i].dwData != 0)
						iNumProcessed++;
				}

				UpdateVirtualButtons();
				return iNumProcessed;
			}
			else
			{
				m_bIsUpdating = TFALSE;
				Unacquire();
				Acquire();

				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	int TInputDXDeviceKeyboard::GetButtonCount() const
	{
		return 0;
	}

	TBOOL TInputDXDeviceKeyboard::IsDown(int doodad) const
	{
		if (!m_bIsUpdating)
			return TFALSE;

		if (doodad == VIRTKEY_SHIFT)
			return IsShiftDown();
		else if (doodad == VIRTKEY_CONTROL)
			return IsControlDown();
		else if (doodad == VIRTKEY_ALT)
			return IsAltDown();
		else
			return m_pKeyStates1[TranslateDXToDoodad(doodad)] & 0x80;
	}

	TBOOL TInputDXDeviceKeyboard::IsEnabled() const
	{
		return TTRUE;
	}

	TBOOL TInputDXDeviceKeyboard::IsShiftDown() const
	{
		return (m_pKeyStates1[DIK_LSHIFT] & 0x80) || (m_pKeyStates1[DIK_RSHIFT] & 0x80);
	}

	TBOOL TInputDXDeviceKeyboard::IsControlDown() const
	{
		return (m_pKeyStates1[DIK_LCONTROL] & 0x80) || (m_pKeyStates1[DIK_RCONTROL] & 0x80);
	}

	TBOOL TInputDXDeviceKeyboard::IsAltDown() const
	{
		return (m_pKeyStates1[DIK_LALT] & 0x80) || (m_pKeyStates1[DIK_RALT] & 0x80);
	}

	TBOOL TInputDXDeviceKeyboard::WasDown(int doodad) const
	{
		if (!m_bIsUpdating)
			return TFALSE;

		if (doodad == VIRTKEY_SHIFT)
			return (m_pKeyStates2[DIK_LSHIFT] & 0x80) || (m_pKeyStates2[DIK_RSHIFT] & 0x80);
		else if (doodad == VIRTKEY_CONTROL)
			return (m_pKeyStates2[DIK_LCONTROL] & 0x80) || (m_pKeyStates2[DIK_RCONTROL] & 0x80);
		else if (doodad == VIRTKEY_ALT)
			return (m_pKeyStates2[DIK_LALT] & 0x80) || (m_pKeyStates2[DIK_RALT] & 0x80);
		else
			return m_pKeyStates2[TranslateDXToDoodad(doodad)] & 0x80;
	}

	wchar_t* TInputDXDeviceKeyboard::TranslateDoodadToCharacter(int doodad) const
	{
#pragma warning( disable : 4309 )
		static wchar_t s_Buffer[2];
		static constexpr char s_TranslateArrayLowerCase[] = { 0x00, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2d, 0x3d, 0x00, 0x00, 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6f, 0x70, 0x5b, 0x5d, 0x00, 0x00, 0x61, 0x73, 0x64, 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x3b, 0x27, 0x60, 0x00, 0x5c, 0x7a, 0x78, 0x63, 0x76, 0x62, 0x6e, 0x6d, 0x2c, 0x2e, 0x2f, 0x00, 0x2a, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		static constexpr char s_TranslateArrayUpperCase[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26, 0x2a, 0x28, 0x29, 0x5f, 0x2b, 0x00, 0x00, 0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7b, 0x7d, 0x00, 0x00, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x3b, 0x00, 0x7e, 0x00, 0x7c, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x3c, 0x3e, 0x3f, 0x00, 0x2a, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#pragma warning( default : 4309 )

		if (!IsShiftDown())
			s_Buffer[0] = TSTATICCAST(wchar_t, s_TranslateArrayLowerCase[doodad - 0x20000]);
		else
			s_Buffer[0] = TSTATICCAST(wchar_t, s_TranslateArrayUpperCase[doodad - 0x20000]);

		s_Buffer[1] = L'\0';
		return s_Buffer;
	}

	TBOOL TInputDXDeviceKeyboard::Initialise()
	{
		m_iSomeNum = 0;

		TUtil::MemClear(m_aBuffer, sizeof(m_aBuffer));
		m_pKeyStates1 = m_aBuffer + m_iSomeNum * 256;
		m_pKeyStates2 = m_aBuffer + m_iSomeNum * 256 + 256;

		return TTRUE;
	}

	TBOOL TInputDXDeviceKeyboard::Deinitialise()
	{
		Release();
		return TTRUE;
	}

	void TInputDXDeviceKeyboard::RefreshDirect()
	{
		if (!IsAcquired())
			return;

		std::swap(m_pKeyStates1, m_pKeyStates2);
		HRESULT hPoolRes = m_poDXInputDevice->Poll();

		if (SUCCEEDED(hPoolRes) && 
			m_poDXInputDevice->GetDeviceState(256, m_pKeyStates1) == S_OK)
		{
			DWORD dwNumElements = 32;
			m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &dwNumElements, 0);
			m_bFlag = TTRUE;
			m_bIsUpdating = TTRUE;
		}
		else
		{
			m_bIsUpdating = TFALSE;
		}
	}

	TBOOL TInputDXDeviceKeyboard::HandleKeyChange(TEmitter<TInputInterface, TInputInterface::InputEvent>& a_Emitter, int a_iKeyIndex, uint8_t a_ui8KeyState)
	{
		int iDoodad = TranslateDXToDoodad(a_iKeyIndex);

		if (iDoodad == KEY_TAB)
			if (IsAltDown())
				return TFALSE;
		
		if ((a_ui8KeyState & 0x80) == 0)
		{
			// Key released
			m_pKeyStates1[a_iKeyIndex] = 0;
			m_pKeyStates2[a_iKeyIndex] = 0x80;

			a_Emitter.Throw(TInputInterface::InputEvent(this, iDoodad, TInputInterface::EVENT_TYPE_GONE_UP));
		}
		else
		{
			// Key pressed
			m_pKeyStates1[a_iKeyIndex] = 0x80;
			m_pKeyStates2[a_iKeyIndex] = 0;

			wchar_t* wcsCharacter = TranslateDoodadToCharacter(iDoodad);
			a_Emitter.Throw(TInputInterface::InputEvent(this, iDoodad, TInputInterface::EVENT_TYPE_GONE_DOWN, wcsCharacter));
		}

		return TTRUE;
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

	int TInputDXDeviceKeyboard::TranslateDoodadToDX(int dxkey)
	{
		switch (dxkey)
		{
		case TInputDeviceKeyboard::KEY_ESCAPE:
			return DIK_ESCAPE;
		case TInputDeviceKeyboard::KEY_1:
			return DIK_1;
		case TInputDeviceKeyboard::KEY_2:
			return DIK_2;
		case TInputDeviceKeyboard::KEY_3:
			return DIK_3;
		case TInputDeviceKeyboard::KEY_4:
			return DIK_4;
		case TInputDeviceKeyboard::KEY_5:
			return DIK_5;
		case TInputDeviceKeyboard::KEY_6:
			return DIK_6;
		case TInputDeviceKeyboard::KEY_7:
			return DIK_7;
		case TInputDeviceKeyboard::KEY_8:
			return DIK_8;
		case TInputDeviceKeyboard::KEY_9:
			return DIK_9;
		case TInputDeviceKeyboard::KEY_0:
			return DIK_0;
		case TInputDeviceKeyboard::KEY_MINUS:
			return DIK_MINUS;
		case TInputDeviceKeyboard::KEY_EQUALS:
			return DIK_EQUALS;
		case TInputDeviceKeyboard::KEY_BACK:
			return DIK_BACK;
		case TInputDeviceKeyboard::KEY_TAB:
			return DIK_TAB;
		case TInputDeviceKeyboard::KEY_Q:
			return DIK_Q;
		case TInputDeviceKeyboard::KEY_W:
			return DIK_W;
		case TInputDeviceKeyboard::KEY_E:
			return DIK_E;
		case TInputDeviceKeyboard::KEY_R:
			return DIK_R;
		case TInputDeviceKeyboard::KEY_T:
			return DIK_T;
		case TInputDeviceKeyboard::KEY_Y:
			return DIK_Y;
		case TInputDeviceKeyboard::KEY_U:
			return DIK_U;
		case TInputDeviceKeyboard::KEY_I:
			return DIK_I;
		case TInputDeviceKeyboard::KEY_O:
			return DIK_O;
		case TInputDeviceKeyboard::KEY_P:
			return DIK_P;
		case TInputDeviceKeyboard::KEY_LBRACKET:
			return DIK_LBRACKET;
		case TInputDeviceKeyboard::KEY_RBRACKET:
			return DIK_RBRACKET;
		case TInputDeviceKeyboard::KEY_RETURN:
			return DIK_RETURN;
		case TInputDeviceKeyboard::KEY_LCONTROL:
			return DIK_LCONTROL;
		case TInputDeviceKeyboard::KEY_A:
			return DIK_A;
		case TInputDeviceKeyboard::KEY_S:
			return DIK_S;
		case TInputDeviceKeyboard::KEY_D:
			return DIK_D;
		case TInputDeviceKeyboard::KEY_F:
			return DIK_F;
		case TInputDeviceKeyboard::KEY_G:
			return DIK_G;
		case TInputDeviceKeyboard::KEY_H:
			return DIK_H;
		case TInputDeviceKeyboard::KEY_J:
			return DIK_J;
		case TInputDeviceKeyboard::KEY_K:
			return DIK_K;
		case TInputDeviceKeyboard::KEY_L:
			return DIK_L;
		case TInputDeviceKeyboard::KEY_SEMICOLON:
			return DIK_SEMICOLON;
		case TInputDeviceKeyboard::KEY_APOSTROPHE:
			return DIK_APOSTROPHE;
		case TInputDeviceKeyboard::KEY_GRAVE:
			return DIK_GRAVE;
		case TInputDeviceKeyboard::KEY_LSHIFT:
			return DIK_LSHIFT;
		case TInputDeviceKeyboard::KEY_BACKSLASH:
			return DIK_BACKSLASH;
		case TInputDeviceKeyboard::KEY_Z:
			return DIK_Z;
		case TInputDeviceKeyboard::KEY_X:
			return DIK_X;
		case TInputDeviceKeyboard::KEY_C:
			return DIK_C;
		case TInputDeviceKeyboard::KEY_V:
			return DIK_V;
		case TInputDeviceKeyboard::KEY_B:
			return DIK_B;
		case TInputDeviceKeyboard::KEY_N:
			return DIK_N;
		case TInputDeviceKeyboard::KEY_M:
			return DIK_M;
		case TInputDeviceKeyboard::KEY_COMMA:
			return DIK_COMMA;
		case TInputDeviceKeyboard::KEY_PERIOD:
			return DIK_PERIOD;
		case TInputDeviceKeyboard::KEY_SLASH:
			return DIK_SLASH;
		case TInputDeviceKeyboard::KEY_RSHIFT:
			return DIK_RSHIFT;
		case TInputDeviceKeyboard::KEY_MULTIPLY:
			return DIK_MULTIPLY;
		case TInputDeviceKeyboard::KEY_LMENU:
			return DIK_LMENU;
		case TInputDeviceKeyboard::KEY_SPACE:
			return DIK_SPACE;
		case TInputDeviceKeyboard::KEY_CAPITAL:
			return DIK_CAPITAL;
		case TInputDeviceKeyboard::KEY_F1:
			return DIK_F1;
		case TInputDeviceKeyboard::KEY_F2:
			return DIK_F2;
		case TInputDeviceKeyboard::KEY_F3:
			return DIK_F3;
		case TInputDeviceKeyboard::KEY_F4:
			return DIK_F4;
		case TInputDeviceKeyboard::KEY_F5:
			return DIK_F5;
		case TInputDeviceKeyboard::KEY_F6:
			return DIK_F6;
		case TInputDeviceKeyboard::KEY_F7:
			return DIK_F7;
		case TInputDeviceKeyboard::KEY_F8:
			return DIK_F8;
		case TInputDeviceKeyboard::KEY_F9:
			return DIK_F9;
		case TInputDeviceKeyboard::KEY_F10:
			return DIK_F10;
		case TInputDeviceKeyboard::KEY_NUMLOCK:
			return DIK_NUMLOCK;
		case TInputDeviceKeyboard::KEY_SCROLL:
			return DIK_SCROLL;
		case TInputDeviceKeyboard::KEY_NUMPAD7:
			return DIK_NUMPAD7;
		case TInputDeviceKeyboard::KEY_NUMPAD8:
			return DIK_NUMPAD8;
		case TInputDeviceKeyboard::KEY_NUMPAD9:
			return DIK_NUMPAD9;
		case TInputDeviceKeyboard::KEY_SUBTRACT:
			return DIK_SUBTRACT;
		case TInputDeviceKeyboard::KEY_NUMPAD4:
			return DIK_NUMPAD4;
		case TInputDeviceKeyboard::KEY_NUMPAD5:
			return DIK_NUMPAD5;
		case TInputDeviceKeyboard::KEY_NUMPAD6:
			return DIK_NUMPAD6;
		case TInputDeviceKeyboard::KEY_ADD:
			return DIK_ADD;
		case TInputDeviceKeyboard::KEY_NUMPAD1:
			return DIK_NUMPAD1;
		case TInputDeviceKeyboard::KEY_NUMPAD2:
			return DIK_NUMPAD2;
		case TInputDeviceKeyboard::KEY_NUMPAD3:
			return DIK_NUMPAD3;
		case TInputDeviceKeyboard::KEY_NUMPAD0:
			return DIK_NUMPAD0;
		case TInputDeviceKeyboard::KEY_DECIMAL:
			return DIK_DECIMAL;
		case TInputDeviceKeyboard::KEY_F11:
			return DIK_F11;
		case TInputDeviceKeyboard::KEY_F12:
			return DIK_F12;
		case TInputDeviceKeyboard::KEY_F13:
			return DIK_F13;
		case TInputDeviceKeyboard::KEY_F14:
			return DIK_F14;
		case TInputDeviceKeyboard::KEY_F15:
			return DIK_F15;
		case TInputDeviceKeyboard::KEY_KANA:
			return DIK_KANA;
		case TInputDeviceKeyboard::KEY_CONVERT:
			return DIK_CONVERT;
		case TInputDeviceKeyboard::KEY_NOCONVERT:
			return DIK_NOCONVERT;
		case TInputDeviceKeyboard::KEY_YEN:
			return DIK_YEN;
		case TInputDeviceKeyboard::KEY_NUMPADEQUALS:
			return DIK_NUMPADEQUALS;
		case TInputDeviceKeyboard::KEY_CIRCUMFLEX:
			return DIK_CIRCUMFLEX;
		case TInputDeviceKeyboard::KEY_AT:
			return DIK_AT;
		case TInputDeviceKeyboard::KEY_COLON:
			return DIK_COLON;
		case TInputDeviceKeyboard::KEY_UNDERLINE:
			return DIK_UNDERLINE;
		case TInputDeviceKeyboard::KEY_KANJI:
			return DIK_KANJI;
		case TInputDeviceKeyboard::KEY_STOP:
			return DIK_STOP;
		case TInputDeviceKeyboard::KEY_AX:
			return DIK_AX;
		case TInputDeviceKeyboard::KEY_UNLABELED:
			return DIK_UNLABELED;
		case TInputDeviceKeyboard::KEY_NUMPADENTER:
			return DIK_NUMPADENTER;
		case TInputDeviceKeyboard::KEY_RCONTROL:
			return DIK_RCONTROL;
		case TInputDeviceKeyboard::KEY_NUMPADCOMMA:
			return DIK_NUMPADCOMMA;
		case TInputDeviceKeyboard::KEY_DIVIDE:
			return DIK_DIVIDE;
		case TInputDeviceKeyboard::KEY_SYSRQ:
			return DIK_SYSRQ;
		case TInputDeviceKeyboard::KEY_RMENU:
			return DIK_RMENU;
		case TInputDeviceKeyboard::KEY_PAUSE:
			return DIK_PAUSE;
		case TInputDeviceKeyboard::KEY_HOME:
			return DIK_HOME;
		case TInputDeviceKeyboard::KEY_UP:
			return DIK_UP;
		case TInputDeviceKeyboard::KEY_PRIOR:
			return DIK_PRIOR;
		case TInputDeviceKeyboard::KEY_LEFT:
			return DIK_LEFT;
		case TInputDeviceKeyboard::KEY_RIGHT:
			return DIK_RIGHT;
		case TInputDeviceKeyboard::KEY_END:
			return DIK_END;
		case TInputDeviceKeyboard::KEY_DOWN:
			return DIK_DOWN;
		case TInputDeviceKeyboard::KEY_NEXT:
			return DIK_NEXT;
		case TInputDeviceKeyboard::KEY_INSERT:
			return DIK_INSERT;
		case TInputDeviceKeyboard::KEY_DELETE:
			return DIK_DELETE;
		case TInputDeviceKeyboard::KEY_LWIN:
			return DIK_LWIN;
		case TInputDeviceKeyboard::KEY_RWIN:
			return DIK_RWIN;
		case TInputDeviceKeyboard::KEY_APPS:
			return DIK_APPS;
		case TInputDeviceKeyboard::KEY_POWER:
			return DIK_POWER;
		case TInputDeviceKeyboard::KEY_SLEEP:
			return DIK_SLEEP;
		default:
			return 0;
		}
	}

	const TBOOL TInputDXDeviceKeyboard::BindToDIDevice(HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL a_bExclusive)
	{
		TASSERT(a_poDeviceInstance != NULL);
		TASSERT(a_poDXInputDevice != NULL);

		Initialise();
		Release();

		TUtil::MemCopy(&m_oDeviceInstance, a_poDeviceInstance, sizeof(DIDEVICEINSTANCEA));

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

	BOOL CALLBACK TInputDXDeviceKeyboard::EnumObjectCallback(LPCDIDEVICEOBJECTINSTANCEA a_poObjectInstance, LPVOID a_pvRef)
	{
		TInputDXDeviceKeyboard* pKeyboard = TREINTERPRETCAST(TInputDXDeviceKeyboard*, a_pvRef);
		int iDoodad = TranslateDXToDoodad(a_poObjectInstance->dwOfs);

		if (iDoodad - 0x20000U < 0x80)
		{
			iDoodad -= 0x20000U;

			// Save name of key
			auto iKeyNameLength = TStringManager::String8Length(a_poObjectInstance->tszName);
			delete[] pKeyboard->m_aKeys[iDoodad].wszKeyName;
			wchar_t* wcsKeyName = new wchar_t[iKeyNameLength + 1];
			TStringManager::StringCharToUnicode(wcsKeyName, a_poObjectInstance->tszName, -1);
			pKeyboard->m_aKeys[iDoodad].wszKeyName = wcsKeyName;

			DIPROPSTRING oKeyName;
			oKeyName.diph.dwSize = sizeof(DIPROPSTRING);
			oKeyName.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			oKeyName.diph.dwHow = 1;
			oKeyName.diph.dwObj = a_poObjectInstance->dwType >> 8 & 0xffff;
			
			pKeyboard->m_poDXInputDevice->GetProperty(DIPROP_KEYNAME, &oKeyName.diph);

			DIPROPDWORD oKeyScanCode;
			oKeyScanCode.diph.dwSize = sizeof(oKeyScanCode);
			oKeyScanCode.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			oKeyScanCode.diph.dwHow = 1;
			oKeyScanCode.diph.dwObj = a_poObjectInstance->dwType >> 8 & 0xffff;
			
			HRESULT hRes = pKeyboard->m_poDXInputDevice->GetProperty(DIPROP_SCANCODE, &oKeyScanCode.diph);
			
			if (hRes == S_OK)
				pKeyboard->m_aKeys[iDoodad].dwKeyScanCode = oKeyScanCode.dwData;
			else
				pKeyboard->m_aKeys[iDoodad].dwKeyScanCode = -1;
		}

		return TRUE;
	}

}
