#pragma once
#include <Toshi/Input/TInputDeviceMouse.h>
#include <Toshi/Input/TInputDeviceKeyboard.h>

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
	Toshi::TInputDeviceMouse* m_pMouseDevice;
	Toshi::TInputDeviceKeyboard* m_pKeyboardDevice;
};
