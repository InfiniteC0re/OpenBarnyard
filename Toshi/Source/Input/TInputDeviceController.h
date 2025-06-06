/**
 * @file TInputDeviceController.h
 * Controller input device interface for the Toshi engine
 * 
 * This file defines the controller input device that provides:
 * - Game controller input handling
 * - Button and axis state management
 * - Controller-specific input mapping
 * - Force feedback support
 */

#pragma once
#include "Input/TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputDeviceController : public TInputDevice
{
public:
	TDECLARE_CLASS( TInputDeviceController, TInputDevice );

public:
	constexpr static TUINT NUM_BUTTONS = 8;

	enum VIBRATION_ACTUATOR
	{

	};

public:
	TInputDeviceController();
	~TInputDeviceController();

	virtual TBOOL        GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& a_rProperties ) const override;
	virtual const TCHAR* GetButtonFromDoodad( Doodad a_iDoodad ) const override;
	virtual TBOOL        IsDown( Doodad a_iDoodad ) const override;

	virtual void  SetVibration( VIBRATION_ACTUATOR a_eActuator, TFLOAT a_fValue );
	virtual TBOOL WasDown( Doodad a_iDoodad ) const;
	virtual void  Unknown1() = 0;
	virtual TBOOL Unknown2();
	virtual TINT  ProcessVirtualButtons( EventEmitter& a_rEmitter, float a_fDeltaTime );

private:
	TBOOL* m_CurrentState; // 0x3C
	TBOOL* m_OldState;     // 0x40
	TFLOAT m_fMag1;        // 0x44
	TFLOAT m_fMag2;        // 0x48
};

TOSHI_NAMESPACE_END
