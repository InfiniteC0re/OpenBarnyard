#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputDeviceMouse.h"

namespace Toshi
{
	class TInputDXDeviceMouse : public TInputDeviceMouse
	{
	public:
		TDECLARE_CLASS(TInputDeviceMouse);

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
		virtual void Update(TFLOAT a_fDeltaTime) override;
		virtual TBOOL Flush() override;
		virtual TINT ProcessEvents(EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime) override;
		virtual TINT GetButtonCount() const override;
		virtual TINT GetAxisCount() const override;
		virtual TBOOL IsDown(Doodad a_iDoodad) const override;
		virtual TINT GetAxisInt(Doodad a_iDoodad, TINT axis) const override;
		virtual TFLOAT GetAxisFloat(Doodad a_iDoodad, TINT axis) const override;
		virtual TBOOL IsEnabled() const override;

		virtual TBOOL Initialise();
		virtual TBOOL Deinitialise();
		virtual void RefreshDirect();
		virtual TBOOL WasDown(Doodad a_iDoodad) const;

		TBOOL const BindToDIDevice(HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL a_bExclusive);
		
		void SetExclusive(HWND a_hWindow, TBOOL a_bExclusive);
		void SetCurrentPosition(TINT a_iX, TINT a_iY);

		const DIDEVICEINSTANCEA& GetDeviceInstance() const
		{
			return m_oDeviceInstance;
		}

	private:
		TBOOL m_bReverseButtons;
		DWORD m_dwButtonCurrent;
		DWORD m_dwButtonPrevious;
		TINT m_iPositionX;
		TINT m_iPositionY;
		TINT m_iPositionZ;
		TINT m_iDeltaPositionX;
		TINT m_iDeltaPositionY;
		TINT m_iDeltaPositionZ;
		TINT m_iUnk1;
		TINT m_iUnk2;
		TINT m_iUnk3;
		TINT m_iUnk4;
		TINT m_iUnk5;
		TINT m_iUnk6;
		TINT m_iUnk7;
		BOOL m_bInitiliased;
		DIDEVICEINSTANCEA m_oDeviceInstance;
		DIDEVCAPS m_DIDevCaps;
		IDirectInputDevice8A* m_poDXInputDevice;
	};
}