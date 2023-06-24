#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputDeviceMouse.h"

namespace Toshi
{
	class TInputDXDeviceMouse : public TInputDeviceMouse
	{
	public:
		TInputDXDeviceMouse()
		{
			m_poDXInputDevice = NULL;
		}

		virtual ~TInputDXDeviceMouse()
		{
			Deinitialise();
		}

		virtual void Release();
		virtual void Update(float deltaTime);
		virtual TBOOL Initialise();
		virtual TBOOL Deinitialise();
		virtual TBOOL Acquire();
		virtual TBOOL Unacquire();
		virtual TBOOL Flush();
		virtual int ProcessEvents(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float deltaTime);
		virtual void RefreshDirect();
		virtual TBOOL IsDown(int doodad) const
		{
			return doodad >= BUTTON_1 && doodad <= BUTTON_8 ? ((1 << doodad) & m_dwButtonCurrent) != 0 : TFALSE;
		}
		virtual TBOOL WasDown(int doodad) const
		{
			return doodad >= BUTTON_1 && doodad <= BUTTON_8 ? ((1 << doodad) & m_dwButtonPrevious) != 0 : TFALSE;
		}

		TBOOL const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, TBOOL exclusive);

		DWORD GetAxisCount() const
		{
			return m_DIDevCaps.dwAxes;
		}

		DWORD GetButtonCount() const
		{
			return m_DIDevCaps.dwButtons;
		}

		enum Coord
		{
			X,
			Y
		};

		int GetAxisInt(int doodad, Coord coord) const;
		float GetAxisFloat(int doodad, Coord coord) const;

		union Axis
		{
			int m_iX, m_iY;
			float m_fX, m_fY;
		};

		static constexpr int sm_ciMouseBufferSize = 0x20000000;

		IDirectInputDevice8* m_poDXInputDevice;
		DIDEVCAPS m_DIDevCaps;
		POINT m_CursorPos;
		DWORD m_dwButtonCurrent;
		DWORD m_dwButtonPrevious;
		int m_iLeftButton;
		Axis m_aAxis;
		int m_field0x80;
		TBOOL m_bUnk; // 0x35
		float m_fWheelAxis; // 0x60 very unsure
		TBOOL m_bInitiliased; // 0x80
		GUID m_oGUID; // 0x84
	};
}