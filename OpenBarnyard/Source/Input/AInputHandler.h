#pragma once
#include <Toshi/TTask.h>
#include <Input/TInputDeviceMouse.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Toshi/T2Vector.h>

class AInputHandler : public Toshi::TTask, public Toshi::TSingleton< AInputHandler >
{
public:
    TDECLARE_CLASS( AInputHandler, Toshi::TTask );

public:
    AInputHandler();
    ~AInputHandler();

    virtual TBOOL OnCreate() override;
    virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

    Toshi::TInputDeviceMouse* GetMouseDevice()
    {
        return m_pMouseDevice;
    }

    Toshi::TInputDeviceKeyboard* GetKeyboardDevice()
    {
        return m_pKeyboardDevice;
    }

private:
    TBOOL ProcessInputEvent( Toshi::TInputInterface* a_pInputInterface, Toshi::TInputInterface::InputEvent* a_pEvent );

private:
    Toshi::TInputDeviceMouse*                  m_pMouseDevice;
    Toshi::TInputDeviceKeyboard*               m_pKeyboardDevice;
    Toshi::T2Vector< Toshi::TInputDevice*, 8 > m_Devices;
    Toshi::TInputDeviceController*             m_pMainController;
    // ...
    Toshi::TListener< Toshi::TInputInterface, Toshi::TInputInterface::InputEvent, AInputHandler > m_InputListener;
    // ...
    TFLOAT m_fDeltaTime;
    TBOOL  m_bIsPaused;
    TBOOL  m_bIsLastInputValid;
};
