#pragma once
#include "Input/TInputDeviceKeyboard.h"

TOSHI_NAMESPACE_START

class TInputWASMDeviceKeyboard : public TInputDeviceKeyboard
{
public:
	TDECLARE_CLASS( TInputWASMDeviceKeyboard, TInputDeviceKeyboard );

	static constexpr TINT MAX_KEY_INDEX  = 256;
	static constexpr TINT MAX_KEY_EVENTS = 64;

public:
	TInputWASMDeviceKeyboard();
	~TInputWASMDeviceKeyboard();

	virtual TBOOL   Acquire() override;
	virtual TBOOL   Unacquire() override;
	virtual void    Release() override;
	virtual void    Update( TFLOAT a_fDeltaTime ) override;
	virtual TBOOL   Flush() override;
	virtual TINT    ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) override;
	virtual TINT    GetButtonCount() const override { return MAX_KEY_INDEX; }
	virtual TBOOL   IsDown( Doodad a_iDoodad ) const override;
	virtual TBOOL   IsEnabled() const override { return TTRUE; }
	virtual TBOOL   IsShiftDown() const override;
	virtual TBOOL   IsControlDown() const override;
	virtual TBOOL   IsAltDown() const override;
	virtual TBOOL   WasDown( Doodad a_iDoodad ) const override;
	virtual TWCHAR* TranslateDoodadToCharacter( Doodad a_iDoodad ) const override;

	void QueueKey( Doodad a_iDoodad, TBOOL a_bDown );

private:
	TBOOL  m_abDown[ MAX_KEY_INDEX ];
	TBOOL  m_abWasDown[ MAX_KEY_INDEX ];
	Doodad m_aiEventDoodads[ MAX_KEY_EVENTS ];
	TBOOL  m_abEventDown[ MAX_KEY_EVENTS ];
	TINT   m_iNumEvents;
};

TOSHI_NAMESPACE_END
