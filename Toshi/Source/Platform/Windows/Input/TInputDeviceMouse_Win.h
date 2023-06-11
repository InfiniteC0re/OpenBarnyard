#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputDeviceMouse.h"

namespace Toshi
{
	class TInputDXDeviceMouse : public TInputDeviceMouse
	{
	public:
		TInputDXDeviceMouse() : TInputDeviceMouse()
		{
			m_poDXInputDevice = NULL;
		}

		virtual ~TInputDXDeviceMouse()
		{
			Deinitialise();
		}

		virtual void Release();
		virtual void Update();
		virtual bool Initialise();
		virtual bool Deinitialise();
		virtual bool Acquire();
		virtual bool Unacquire();
		virtual bool Flush();
		virtual int ProcessEvents(TGenericEmitter& emitter, float flt);
		virtual void RefreshDirect();

		bool const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, bool exclusive);

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
		bool m_bInitiliased; // 0x80
		DWORD m_dwButtonCurrent;
		DWORD m_dwButtonPrevious;
		int m_field0x40;
		Axis m_aAxis;
		float m_fWheelAxis; // 0x60 very unsure
		int m_field0x80;
		bool m_bUnk; // 0x35
	};
}