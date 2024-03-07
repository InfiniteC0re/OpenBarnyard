#include "ToshiPCH.h"
#include "TInputDeviceMouse_Win.h"
#include "Platform/DX8/TRenderInterface_DX8.h"
#include "Platform/Windows/Input/TInputInterface_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TDEFINE_CLASS(TInputDXDeviceMouse);

	TBOOL TInputDXDeviceMouse::Acquire()
	{
		HRESULT hr = m_poDXInputDevice->Acquire();

		if (FAILED(hr))
		{
			SetAcquired(TFALSE);
			return TFALSE;
		}
		
		SetAcquired(TTRUE);

		if (hr != S_FALSE)
		{
			RefreshDirect();
		}

		return TTRUE;
	}

	TBOOL TInputDXDeviceMouse::Unacquire()
	{
		SetAcquired(TFALSE); 
		HRESULT hr = m_poDXInputDevice->Unacquire();
		return SUCCEEDED(hr);
	}

	void TInputDXDeviceMouse::Release()
	{
		if (m_poDXInputDevice)
		{
			m_poDXInputDevice->Release();
			m_poDXInputDevice = NULL;
		}
	}

	void TInputDXDeviceMouse::Update(TFLOAT a_fDeltaTime)
	{
		if (IsAcquired())
		{
			m_iDeltaPositionX = 0;
			m_iDeltaPositionY = 0;
			m_iDeltaPositionZ = 0;

			HRESULT hRes = m_poDXInputDevice->Poll();

			if (SUCCEEDED(hRes))
			{
				DIMOUSESTATE state;
				hRes = m_poDXInputDevice->GetDeviceState(sizeof(state), &state);

				if (SUCCEEDED(hRes))
				{
					m_iDeltaPositionX += state.lX;
					m_iPositionX += state.lX;
					m_iDeltaPositionY += state.lY;
					m_iPositionY += state.lY;
					m_iDeltaPositionZ += state.lZ;
					m_iPositionZ += state.lZ;

					if (TRenderInterface::GetSingleton())
					{
						auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

						if (!pDisplayParams->bWindowed)
						{
							TMath::Clip(m_iPositionX, 0, TINT(pDisplayParams->uiWidth));
							TMath::Clip(m_iPositionY, 0, TINT(pDisplayParams->uiHeight));
						}
					}
				}
			}
		}
	}

	TBOOL TInputDXDeviceMouse::Flush()
	{
		DWORD dwItems = INFINITE;

		if (IsAcquired())
		{
			HRESULT hr = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &dwItems, 0);

			if (SUCCEEDED(hr))
			{
				RefreshDirect();
				return TTRUE;
			}
		}

		return TFALSE;
	}

	TINT TInputDXDeviceMouse::ProcessEvents(EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime)
	{
		m_dwButtonPrevious = m_dwButtonCurrent;
		TINT iNumProcessed = ProcessRepeats(a_rEmitter, a_fDeltaTime);

		DWORD dwItems = 32;
		DIDEVICEOBJECTDATA dod[32];
		TUtil::MemClear(dod, sizeof(dod));

		HRESULT hRes = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, &dwItems, 0);

		if (FAILED(hRes) && hRes == (DIERR_NOTACQUIRED | DIERR_INPUTLOST))
		{
			Unacquire();
			Acquire();

			hRes = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, &dwItems, 0);

			if (hRes != DI_OK)
			{
				return 0;
			}
		}

		for (DWORD i = 0; i < dwItems; i++)
		{
			switch (dod[i].dwOfs)
			{
			case DIMOFS_BUTTON0:
			case DIMOFS_BUTTON1:
			case DIMOFS_BUTTON2:
			{
				TINT iButton =
					(dod[i].dwOfs == DIMOFS_BUTTON0) ? (!m_bReverseButtons ? 0 : 1) :
					(dod[i].dwOfs == DIMOFS_BUTTON1) ? (!m_bReverseButtons ? 1 : 0) : 2;

				TUINT uiButtonFlag = (1 << iButton);

				TASSERT(iButton < INPUT_DEVICE_MOUSE_BUTTONS);

				if (dod[i].dwData & 0x80)
				{
					TASSERT((m_dwButtonCurrent & uiButtonFlag) == 0);

					if ((m_dwButtonCurrent & uiButtonFlag) == 0)
					{
						a_rEmitter.Throw(TInputInterface::InputEvent(this, BUTTON_1 + iButton, TInputInterface::EVENT_TYPE_GONE_DOWN));
						m_dwButtonCurrent |= uiButtonFlag;
					}
				}
				else if ((m_dwButtonCurrent & uiButtonFlag) != 0)
				{
					a_rEmitter.Throw(TInputInterface::InputEvent(this, BUTTON_1 + iButton, TInputInterface::EVENT_TYPE_GONE_UP));
					m_dwButtonCurrent &= ~uiButtonFlag;
				}

				iNumProcessed += 1;
				break;
			}
			}
		}

		if (m_iDeltaPositionX != 0 && m_iDeltaPositionY != 0)
		{
			a_rEmitter.Throw(
				TInputInterface::InputEvent(
					this,
					0x3000b,
					TInputInterface::EVENT_TYPE_MOVED,
					m_iDeltaPositionX,
					m_iDeltaPositionY
				)
			);

			iNumProcessed += 1;
		}

		if (m_iDeltaPositionZ != 0)
		{
			TTODO("Throw scroll events");
		}

		return iNumProcessed;
	}

	TINT TInputDXDeviceMouse::GetButtonCount() const
	{
		return m_DIDevCaps.dwButtons;
	}

	TINT TInputDXDeviceMouse::GetAxisCount() const
	{
		return m_DIDevCaps.dwAxes;
	}

	TBOOL TInputDXDeviceMouse::IsDown(TINT a_iDoodad) const
	{
		if ((0x30000 < a_iDoodad) && (a_iDoodad < 0x30009))
		{
			return (m_dwButtonCurrent & (1 << ((a_iDoodad - 1) & 0x1F)));
		}

		return TFALSE;
	}

	TINT TInputDXDeviceMouse::GetAxisInt(Doodad a_iDoodad, TINT coord) const
	{
		if (a_iDoodad == AXIS_CURSOR)
		{
			if (coord == 0)
			{
				return m_iDeltaPositionX;
			}

			if (coord == 1)
			{
				return m_iDeltaPositionY;
			}
		}

		return 0;
	}

	TFLOAT TInputDXDeviceMouse::GetAxisFloat(Doodad a_iDoodad, TINT coord) const
	{
		if (a_iDoodad == AXIS_CURSOR)
		{
			if (coord == 0)
			{
				return TFLOAT(m_iDeltaPositionX);
			}

			if (coord == 1)
			{
				return TFLOAT(m_iDeltaPositionY);
			}
		}
		else if (a_iDoodad == AXIS_WHEEL)
		{
			return TFLOAT(m_iDeltaPositionZ);
		}

		return 0.0f;
	}

	TBOOL TInputDXDeviceMouse::IsEnabled() const
	{
		return TTRUE;
	}

	TBOOL TInputDXDeviceMouse::Initialise()
	{
		m_bReverseButtons = TFALSE;
		m_dwButtonCurrent = 0;
		m_dwButtonPrevious = 0;

		POINT cursorPos;
		GetCursorPos(&cursorPos);

		m_iPositionX = cursorPos.x;
		m_iPositionY = cursorPos.y;
		m_iPositionZ = 0;
		m_iDeltaPositionX = 0;
		m_iDeltaPositionY = 0;
		m_iDeltaPositionZ = 0;
		m_iUnk1 = 0;
		m_iUnk2 = 0;
		m_iUnk3 = 0;
		m_iUnk4 = 0;
		m_iUnk5 = 0;
		m_iUnk6 = -1;
		m_iUnk7 = -1;

		DIPROPDWORD dwordProperty;
		dwordProperty.diph.dwSize = sizeof(dwordProperty);
		dwordProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dwordProperty.diph.dwObj = DIMOFS_Z;
		dwordProperty.diph.dwHow = DIPH_BYOFFSET;

		HRESULT hr = m_poDXInputDevice->GetProperty(DIPROP_GRANULARITY, &dwordProperty.diph);

		if (hr != DIERR_OBJECTNOTFOUND)
		{
			m_bInitiliased = (hr != DI_OK) - 1 & dwordProperty.diph.dwHeaderSize;
			return TTRUE;
		}

		m_bInitiliased = FALSE;
		return TTRUE;
	}

	TBOOL TInputDXDeviceMouse::Deinitialise()
	{
		Release();
		return TTRUE;
	}

	void TInputDXDeviceMouse::RefreshDirect()
	{
		if (IsAcquired())
		{
			m_dwButtonPrevious = m_dwButtonCurrent;
			m_dwButtonCurrent = 0;

			if (SUCCEEDED(m_poDXInputDevice->Poll()))
			{
				DIMOUSESTATE mouseState;
				HRESULT hRes = m_poDXInputDevice->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
				
				if (SUCCEEDED(hRes))
				{
					if (mouseState.rgbButtons[0] & 0x80) // LeftClick
						m_dwButtonCurrent |= m_bReverseButtons ? 2 : 1;
					
					if (mouseState.rgbButtons[1] & 0x80) // RightClick
						m_dwButtonCurrent |= m_bReverseButtons ? 1 : 2;

					if (mouseState.rgbButtons[2] & 0x80) // Wheeling yeeet
						m_dwButtonCurrent |= INPUT_DEVICE_MOUSE_WHEEL;
				}
			}
		}
	}

	TBOOL TInputDXDeviceMouse::WasDown(TINT a_iDoodad) const
	{
		if ((0x30000 < a_iDoodad) && (a_iDoodad < 0x30009))
		{
			return (m_dwButtonPrevious & ((a_iDoodad - 1) & 0x1F));
		}

		return TFALSE;
	}

	void TInputDXDeviceMouse::SetExclusive(HWND a_hWindow, TBOOL a_bExclusive)
	{
		m_poDXInputDevice->SetCooperativeLevel(
			a_hWindow,
			a_bExclusive ? (DISCL_EXCLUSIVE | DISCL_FOREGROUND) : (DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)
		);
	}

	void TInputDXDeviceMouse::SetCurrentPosition(TINT a_iX, TINT a_iY)
	{
		auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

		TMath::Clip(m_iPositionX, 0, TINT(pDisplayParams->uiWidth));
		TMath::Clip(m_iPositionY, 0, TINT(pDisplayParams->uiHeight));
	}

	TBOOL const TInputDXDeviceMouse::BindToDIDevice(HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL a_bExclusive)
	{
		TASSERT(a_poDeviceInstance != NULL);
		TASSERT(a_poDXInputDevice != NULL);

		Release();
		TUtil::MemCopy(&m_oDeviceInstance, a_poDeviceInstance, sizeof(DIDEVICEINSTANCEA));

		m_poDXInputDevice = a_poDXInputDevice;

		m_DIDevCaps = { sizeof(DIDEVCAPS) };
		m_poDXInputDevice->GetCapabilities(&m_DIDevCaps);

		HRESULT hRes = m_poDXInputDevice->SetDataFormat(&c_dfDIMouse);

		if (hRes != DI_OK)
			return TFALSE;

		if (a_hMainWindow)
		{
			m_poDXInputDevice->SetCooperativeLevel(a_hMainWindow, DISCL_FOREGROUND | (a_bExclusive ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE));
		}

		DIPROPDWORD dipdw{};
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = DIPROPAXISMODE_REL;
		hRes = m_poDXInputDevice->SetProperty(DIPROP_AXISMODE, &dipdw.diph);

		return hRes == DI_OK;
	}

}
