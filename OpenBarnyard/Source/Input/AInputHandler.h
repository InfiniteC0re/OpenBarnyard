#pragma once
#include <Toshi/TTask.h>
#include <Input/TInputDeviceMouse.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Toshi/T2Vector.h>

class AInputHandler : public Toshi::TTask
    , public Toshi::TSingleton<AInputHandler>
{
public:
	TDECLARE_CLASS( AInputHandler, Toshi::TTask );

public:
	AInputHandler();
	~AInputHandler();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	Toshi::TInputDeviceMouse*      GetMouseDevice() const { return m_pMouseDevice; }
	Toshi::TInputDeviceKeyboard*   GetKeyboardDevice() const { return m_pKeyboardDevice; }
	Toshi::TInputDeviceController* GetMainController() const { return m_pMainController; }

	TBOOL HadInputLastFrame() const { return m_bHadInputLastFrame; }
	TBOOL IsValidDevice( Toshi::TInputDevice* a_pDevice ) const;

private:
	TBOOL ProcessInputEvent( Toshi::TInputInterface* a_pInputInterface, Toshi::TInputInterface::InputEvent* a_pEvent );

private:
	Toshi::TInputDeviceMouse*                m_pMouseDevice;
	Toshi::TInputDeviceKeyboard*             m_pKeyboardDevice;
	Toshi::T2Vector<Toshi::TInputDevice*, 8> m_Devices;
	Toshi::TInputDeviceController*           m_pMainController;
	// ...
	Toshi::TListener<Toshi::TInputInterface, Toshi::TInputInterface::InputEvent, AInputHandler> m_InputListener;
	// ...
	TFLOAT m_fDeltaTime;
	TBOOL  m_bIsPaused;
	TBOOL  m_bHadInputLastFrame;
};
