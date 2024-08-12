#pragma once
#include "Input/TInputDeviceKeyboard.h"
#include "Toshi/T2Array.h"

#include "Includes.h"

namespace Toshi
{

class TInputDXDeviceKeyboard : public TInputDeviceKeyboard
{
public:
    TDECLARE_CLASS( TInputDXDeviceKeyboard, TInputDeviceKeyboard );

public:
    struct KeyInfo
    {
        TWCHAR* wszKeyName;
        DWORD   dwKeyScanCode;
    };

public:
    TInputDXDeviceKeyboard();

    virtual TBOOL   Acquire() override;
    virtual TBOOL   Unacquire() override;
    virtual void    Release() override;
    virtual void    Update( float deltaTime ) override;
    virtual TBOOL   Flush() override;
    virtual TINT    ProcessEvents( TEmitter< TInputInterface, TInputInterface::InputEvent >& emitter, float deltaTime ) override;
    virtual TINT    GetButtonCount() const override;
    virtual TBOOL   IsDown( TINT doodad ) const override;
    virtual TBOOL   IsEnabled() const override;
    virtual TBOOL   IsShiftDown() const override;
    virtual TBOOL   IsControlDown() const override;
    virtual TBOOL   IsAltDown() const override;
    virtual TBOOL   WasDown( TINT doodad ) const override;
    virtual TWCHAR* TranslateDoodadToCharacter( TINT doodad ) const override;
    virtual TBOOL   Initialise();
    virtual TBOOL   Deinitialise();
    virtual void    RefreshDirect();

protected:
    TBOOL       HandleKeyChange( TEmitter< TInputInterface, TInputInterface::InputEvent >& a_Emitter, TINT a_iKeyIndex, uint8_t a_ui8KeyState );
    static TINT TranslateDXToDoodad( TINT doodad );
    static TINT TranslateDoodadToDX( TINT dxkey );

public:
    const TBOOL          BindToDIDevice( HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice );
    static BOOL CALLBACK EnumObjectCallback( LPCDIDEVICEOBJECTINSTANCEA a_poObjectInstance, LPVOID a_pvRef );

private:
    TINT                                     m_iSomeNum;
    TUINT8*                                  m_pKeyStates1;
    TUINT8*                                  m_pKeyStates2;
    TUINT8                                   m_aBuffer[ 512 ];
    T2Array< KeyInfo, KEYBOARD_NUM_DOODADS > m_aKeys;
    TBOOL                                    m_bIsUpdating;
    DIDEVICEINSTANCEA                        m_oDeviceInstance;
    DIDEVCAPS                                m_DIDevCaps;
    IDirectInputDevice8A*                    m_poDXInputDevice;
    TBOOL                                    m_bFlag;
};

} // namespace Toshi
