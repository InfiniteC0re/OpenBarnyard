#pragma once
#include <Toshi/Core/TTask.h>
#include <Toshi/Input/TInputDeviceMouse.h>
#include <Toshi/Input/TInputDeviceKeyboard.h>
#include <Toshi/Input/TInputDeviceController.h>
#include <Toshi2/T2Vector.h>

TOBJECT(AInputHandler, Toshi::TTask, TTRUE),
	public Toshi::TSingleton<AInputHandler>
{
public:
	AInputHandler();
	~AInputHandler();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;

	Toshi::TInputDeviceMouse* GetMouseDevice()
	{
		return m_pMouseDevice;
	}

	Toshi::TInputDeviceKeyboard* GetKeyboardDevice()
	{
		return m_pKeyboardDevice;
	}

private:
	TBOOL ProcessInputEvent(Toshi::TInputInterface* a_pInputInterface, Toshi::TInputInterface::InputEvent* a_pEvent);

private:
	Toshi::TInputDeviceMouse* m_pMouseDevice;
	Toshi::TInputDeviceKeyboard* m_pKeyboardDevice;
	Toshi::T2Vector<Toshi::TInputDevice*, 8> m_Devices;
	Toshi::TInputDeviceController* m_pMainController;
	// ...
	Toshi::TListener<Toshi::TInputInterface, Toshi::TInputInterface::InputEvent, AInputHandler> m_InputListener;
	// ...
	TFLOAT m_fDeltaTime;
	TBOOL m_bIsPaused;
	TBOOL m_bIsLastInputValid;
};
