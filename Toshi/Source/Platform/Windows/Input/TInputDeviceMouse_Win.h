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
		virtual int Unknown1() const override;
		virtual TBOOL IsDown(int doodad) const override;
		virtual int GetAxisInt(int doodad, int axis) const override;
		virtual float GetAxisFloat(int doodad, int axis) const override;
		virtual TBOOL Unknown2() const override;
		virtual TBOOL Initialise();
		virtual TBOOL Deinitialise();
		virtual void RefreshDirect();
		virtual TBOOL WasDown(int doodad) const;

		TBOOL const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCE a_poDeviceInstance, IDirectInputDevice8* a_poDXInputDevice, TBOOL exclusive);

	public:
		static constexpr size_t sm_ciMouseBufferSize = 0x20000000U;

	private:
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