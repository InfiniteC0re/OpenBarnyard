#pragma once
#include "Input/TInputInterface.h"

namespace Toshi
{
	class TInputDeviceController : public TInputDevice
	{
	public:
		TDECLARE_CLASS(TInputDevice);

	public:
		constexpr static TUINT NUM_BUTTONS = 8;

		enum VIBRATION_ACTUATOR
		{

		};

	public:
		TInputDeviceController()
		{
			m_CurrentState = new TBOOL[NUM_BUTTONS];
			m_OldState = new TBOOL[NUM_BUTTONS];
			TUtil::MemClear(m_CurrentState, sizeof(*m_CurrentState) * NUM_BUTTONS);
			TUtil::MemClear(m_OldState, sizeof(*m_OldState) * NUM_BUTTONS);
		}

		~TInputDeviceController() = default;

		virtual TBOOL GetDoodadProperties(Doodad a_iDoodad, DoodadProperties& a_rProperties) const override;
		virtual const TCHAR* GetButtonFromDoodad(Doodad a_iDoodad) const override;
		virtual TBOOL IsDown(Doodad a_iDoodad) const override;
		
		virtual void SetVibration(VIBRATION_ACTUATOR a_eActuator, TFLOAT a_fValue);
		virtual TBOOL WasDown(Doodad a_iDoodad) const;
		virtual void Unknown1() = 0;
		virtual TBOOL Unknown2();
		virtual TINT ProcessVirtualButtons(EventEmitter& a_rEmitter, float a_fDeltaTime);

	private:
		TBOOL* m_CurrentState; // 0x3C
		TBOOL* m_OldState;     // 0x40
		TFLOAT m_fMag1;        // 0x44
		TFLOAT m_fMag2;        // 0x48
	};
}
