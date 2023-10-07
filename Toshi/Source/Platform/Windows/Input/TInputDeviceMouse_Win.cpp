#include "ToshiPCH.h"
#include "TInputDeviceMouse_Win.h"
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/Windows/Input/TInputInterface_Win.h>

namespace Toshi {

	void TInputDXDeviceMouse::Release()
	{
		if (m_poDXInputDevice)
		{
			m_poDXInputDevice->Release();
			m_poDXInputDevice = NULL;
		}
	}

	void TInputDXDeviceMouse::Update(float deltaTime)
	{
		if (IsAcquired())
		{
			/*m_aAxis.m_iX = 0;
			m_aAxis.m_iY = 0;
			m_fWheelAxis = 0;
			auto renderer = TRenderDX11::Interface();
			if (renderer != TNULL)
			{
				auto params = renderer->GetCurrentDisplayParams();
				if (params->Height == 0)
				{
					int x = TSTATICCAST(int, m_CursorPos.x);
					int y = TSTATICCAST(int, m_CursorPos.y);
					TMath::Clip(x, 0, (int)params->Width);
					TMath::Clip(y, 0, (int)params->Height);
				}

				TMSWindow* window = renderer->GetMSWindow();
				m_CursorPos.x = window->m_xPos;
				m_CursorPos.y = window->m_yPos;
			}*/
		}
	}

	TBOOL TInputDXDeviceMouse::Initialise()
	{
		TIMPLEMENT();

		m_bReverseButtons = TFALSE;
		m_dwButtonCurrent = 0;
		m_dwButtonPrevious = 0;

		GetCursorPos(&m_CursorPos);

		DIPROPDWORD dwordProperty;
		dwordProperty.diph.dwSize = sizeof(dwordProperty);
		dwordProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dwordProperty.diph.dwObj = DIMOFS_Z;
		dwordProperty.diph.dwHow = DIPH_BYOFFSET;
	
		HRESULT hr = m_poDXInputDevice->GetProperty(DIPROP_GRANULARITY, &dwordProperty.diph);
		if (hr != DIERR_OBJECTNOTFOUND)
		{
			m_bInitiliased = hr == DI_OK;
			return TTRUE;
		}
		m_bInitiliased = TFALSE;
		return TTRUE;
	}

	TBOOL TInputDXDeviceMouse::Deinitialise()
	{
		Release();
		return TTRUE;
	}

	TBOOL TInputDXDeviceMouse::Acquire()
	{
		HRESULT hr = m_poDXInputDevice->Acquire();

		if (FAILED(hr))
		{
			m_bIsAcquired = TFALSE;
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

	int TInputDXDeviceMouse::ProcessEvents(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float deltaTime)
	{
		DIDEVICEOBJECTDATA dod[32];
		DWORD dwItems = 32;

		m_dwButtonPrevious = m_dwButtonCurrent;
		ProcessRepeats(emitter, deltaTime);
		TUtil::MemClear(dod, sizeof(dod));

		HRESULT hr = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, &dwItems, 0);

		int eventCount = 0;

		if (FAILED(hr) && hr == (DIERR_NOTACQUIRED | DIERR_INPUTLOST))
		{
			Unacquire();
			Acquire();

			hr = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, &dwItems, 0);

			if (hr != DI_OK)
			{
				return eventCount;
			}
		}

		//auto renderer = TRenderDX11::Interface();
		//auto input = TInputDXInterface::GetInterface();
		//TRenderDX11::DisplayParams* params;
		//int unk;

		//for (size_t i = 0; i < dwItems; i++)
		//{
		//	switch (dod[i].dwOfs)
		//	{
		//	case DIMOFS_X: // MouseUp
		//		m_aAxis.m_iX += dod[i].dwData;

		//		if (renderer != TNULL)
		//		{
		//			m_CursorPos.x += dod[i].dwData;
		//		}
		//		else
		//		{
		//			params = renderer->GetCurrentDisplayParams();
		//			if (!params->Unk5)
		//			{
		//				m_CursorPos.x += dod[i].dwData;
		//			}
		//		}
		//		emitter.Throw(TInputInterface::InputEvent(this, AXIS_CURSOR, TInputInterface::EVENT_TYPE_MOVED, m_aAxis.m_iX, m_aAxis.m_iY));
		//		eventCount++;
		//		break;
		//	case DIMOFS_Y: // MouseDown
		//		m_aAxis.m_iY += dod[i].dwData;

		//		if (renderer != TNULL)
		//		{
		//			m_CursorPos.y += dod[i].dwData;
		//		}
		//		else
		//		{
		//			params = renderer->GetCurrentDisplayParams();
		//			if (!params->Unk5)
		//			{
		//				m_CursorPos.y += dod[i].dwData;
		//			}
		//		}
		//		emitter.Throw(TInputInterface::InputEvent(this, AXIS_CURSOR, TInputInterface::EVENT_TYPE_MOVED, m_aAxis.m_iX, m_aAxis.m_iY));
		//		eventCount++;
		//		break;
		//	case DIMOFS_Z: // Wheel
		//		m_fWheelAxis += dod[i].dwData;
		//		emitter.Throw(TInputInterface::InputEvent(this, AXIS_WHEEL, TInputInterface::EVENT_TYPE_MOVED, 0.0f, m_fWheelAxis / WHEEL_DELTA));
		//		unk = m_fWheelAxis / m_field0x80;
		//		// WHEEL_UP
		//		if (unk < 0)
		//		{
		//			emitter.Throw(TInputInterface::InputEvent(this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_GONE_DOWN));
		//			unk++;
		//			if (unk < 0)
		//			{
		//				int i = -unk;
		//				do
		//				{
		//					emitter.Throw(TInputInterface::InputEvent(this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_REPEAT));
		//					i--;
		//				} while (i != 0);
		//			}

		//			emitter.Throw(TInputInterface::InputEvent(this, BUTTON_WHEEL_BACKWARD, TInputInterface::EVENT_TYPE_GONE_UP));
		//		}
		//		// WHEEL_DOWN
		//		else
		//		{
		//			emitter.Throw(TInputInterface::InputEvent(this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_GONE_DOWN));
		//			while (unk = unk - 1, 0 < unk)
		//			{
		//				emitter.Throw(TInputInterface::InputEvent(this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_REPEAT));
		//			}
		//			emitter.Throw(TInputInterface::InputEvent(this, BUTTON_WHEEL_FORWARD, TInputInterface::EVENT_TYPE_GONE_UP));
		//		}
		//		eventCount++;
		//		break;
		//	case DIMOFS_BUTTON0:
		//	case DIMOFS_BUTTON1:
		//	case DIMOFS_BUTTON2:
		//	{
		//		int iButton =
		//			(dod[i].dwOfs == DIMOFS_BUTTON0) ? (!m_bReverseButtons ? 0 : 1) :
		//			(dod[i].dwOfs == DIMOFS_BUTTON1) ? (!m_bReverseButtons ? 1 : 0) : 2;

		//		TASSERT(iButton < INPUT_DEVICE_MOUSE_BUTTONS);

		//		if (dod[i].dwData & 0x80)
		//		{
		//			if ((m_dwButtonCurrent & (1 << iButton)) != 0)
		//			{
		//				// wtf is this assert??????????????
		//				TASSERT((m_dwButtonCurrent & (1 << iButton)) != 0);
		//			}
		//			else
		//			{
		//				emitter.Throw(TInputInterface::InputEvent(this, BUTTON_1 + iButton, TInputInterface::EVENT_TYPE_GONE_DOWN));
		//				m_dwButtonCurrent |= 1 << iButton;
		//			}
		//		}
		//		else
		//		{
		//			emitter.Throw(TInputInterface::InputEvent(this, BUTTON_1 + iButton, TInputInterface::EVENT_TYPE_GONE_UP));
		//			m_dwButtonCurrent &= ~(1 << iButton);
		//		}

		//		eventCount++;
		//		break;
		//	}
		//	default:
		//		break;
		//	}
		//}

		return eventCount;
	}

	int TInputDXDeviceMouse::GetButtonCount() const
	{
		return m_DIDevCaps.dwButtons;
	}

	int TInputDXDeviceMouse::GetAxisCount() const
	{
		return m_DIDevCaps.dwAxes;
	}

	Platform TInputDXDeviceMouse::GetPlatform() const
{
		return Platform::PC;
	}

	void TInputDXDeviceMouse::RefreshDirect()
	{
		if (IsAcquired())
		{
			m_dwButtonPrevious = m_dwButtonCurrent;
			m_dwButtonCurrent = 0;
			HRESULT hr = m_poDXInputDevice->Poll();

			if (SUCCEEDED(hr))
			{
				DIMOUSESTATE mouseState;
				hr = m_poDXInputDevice->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
				
				if (SUCCEEDED(hr))
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

	TBOOL TInputDXDeviceMouse::WasDown(int a_iDoodad) const
	{
		if (a_iDoodad - 0x30001 < 8)
			return (m_dwButtonPrevious & ((a_iDoodad - 1) & 0x1F));

		return TFALSE;
	}

	TBOOL const TInputDXDeviceMouse::BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL exclusive)
	{
		TASSERT(a_poDeviceInstance != NULL);
		TASSERT(a_poDXInputDevice != NULL);

		Release();
		TUtil::MemCopy(&m_oDeviceInstance, a_poDeviceInstance, sizeof(DIDEVICEINSTANCEA));

		m_poDXInputDevice = a_poDXInputDevice;
		m_DIDevCaps.dwSize = sizeof(DIDEVCAPS);

		m_poDXInputDevice->GetCapabilities(&m_DIDevCaps);

		HRESULT hr = m_poDXInputDevice->SetDataFormat(&c_dfDIMouse);

		if (hr != DI_OK)
			return TFALSE;

		if (a_mainWindow)
			m_poDXInputDevice->SetCooperativeLevel(a_mainWindow, DISCL_FOREGROUND | (exclusive ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE));

		DIPROPDWORD dipdw{};
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = DIPROPAXISMODE_REL;

		hr = m_poDXInputDevice->SetProperty(DIPROP_AXISMODE, &dipdw.diph);

		return hr == DI_OK;
	}

	TBOOL TInputDXDeviceMouse::IsDown(int a_iDoodad) const
	{
		if (a_iDoodad - 0x30001 < 8)
			return (m_dwButtonCurrent & ((a_iDoodad - 1) & 0x1F));

		return TFALSE;
	}

	int TInputDXDeviceMouse::GetAxisInt(int doodad, int coord) const
	{
		if (doodad == AXIS_CURSOR)
		{
			if (coord == 0)
				return TSTATICCAST(int, m_aAxis.m_fX);

			if (coord == 1)
				return TSTATICCAST(int, m_aAxis.m_fY);
		}
		else if (doodad == AXIS_WHEEL)
			return TSTATICCAST(int, m_fWheelAxis);

		return 0;
	}

	float TInputDXDeviceMouse::GetAxisFloat(int doodad, int coord) const
	{
		if (doodad == AXIS_CURSOR)
		{
			if (coord == 0)
				return m_aAxis.m_fX;

			if (coord == 1)
				return m_aAxis.m_fY;
		}
		else if (doodad == AXIS_WHEEL)
			return m_fWheelAxis;

		return 0;
	}

	TBOOL TInputDXDeviceMouse::IsEnabled() const
	{
		return TTRUE;
	}

}
