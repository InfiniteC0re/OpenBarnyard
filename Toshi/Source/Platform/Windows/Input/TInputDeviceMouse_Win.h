#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputDeviceMouse.h"

namespace Toshi
{
	class TInputDXDeviceMouse : public TInputDeviceMouse
	{
	public:
		union Axis
		{
			int m_iX, m_iY;
			float m_fX, m_fY;
		};

	public:
		TInputDXDeviceMouse()
		{
			m_poDXInputDevice = NULL;
		}

		virtual ~TInputDXDeviceMouse()
		{
			Deinitialise();
		}

		virtual TBOOL Acquire() override;
		virtual TBOOL Unacquire() override;
		virtual void Release() override;
		virtual void Update(float deltaTime) override;
		virtual TBOOL Flush() override;
		virtual int ProcessEvents(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float deltaTime) override;
		virtual int GetButtonCount() const override;
		virtual int GetAxisCount() const override;
		virtual TBOOL IsDown(int doodad) const override;
		virtual int GetAxisInt(int doodad, int axis) const override;
		virtual float GetAxisFloat(int doodad, int axis) const override;
		virtual TBOOL IsEnabled() const override;
		virtual TBOOL Initialise();
		virtual TBOOL Deinitialise();
		virtual void RefreshDirect();
		virtual TBOOL WasDown(int doodad) const;

		TBOOL const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL exclusive);

	public:
		static constexpr size_t sm_ciMouseBufferSize = 0x20000000U;

	private:
		TBOOL m_bReverseButtons;
		DWORD m_dwButtonCurrent;
		DWORD m_dwButtonPrevious;
		POINT m_CursorPos;
		Axis m_aAxis;
		int m_field0x80;
		TBOOL m_bUnk; // 0x35
		float m_fWheelAxis; // 0x60 very unsure
		TBOOL m_bInitiliased;                    // 0x80
		DIDEVICEINSTANCEA m_oDeviceInstance;     // 0x084
		DIDEVCAPS m_DIDevCaps;                   // 0x2C8
		IDirectInputDevice8A* m_poDXInputDevice; // 0x2F4
	};
}