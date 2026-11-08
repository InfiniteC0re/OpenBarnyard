#pragma once
#include "Input/TInputDeviceController.h"

TOSHI_NAMESPACE_START

// The page's gamepads and on-screen touch controls, the page writes their state to the memory of WASM_GetPadState
class TInputWASMDeviceController : public TInputDeviceController
{
public:
	TDECLARE_CLASS( TInputWASMDeviceController, TInputDeviceController );

	static constexpr TINT STICK_SATURATE = 32000;

	struct PadState
	{
		TUINT32 uiButtons; // a bit for each doodad from BUTTON_DPAD_UP on
		TFLOAT  afSticks[ 4 ]; // X/Y of the left stick and X/Y of the right one, Y is down
		TUINT32 uiFlags;
	};

	using PADFLAGS = TUINT32;
	enum PADFLAGS_ : PADFLAGS
	{
		PADFLAGS_NONE = 0,
		PADFLAGS_SONY    = BITFLAG( 0 ), // the last pad used is a PlayStation one
		PADFLAGS_NOMOUSE = BITFLAG( 1 ), // a phone or tablet without a mouse
	};

public:
	TInputWASMDeviceController();
	~TInputWASMDeviceController();

	virtual TBOOL  Acquire() override;
	virtual TBOOL  Unacquire() override;
	virtual void   Release() override;
	virtual void   Update( TFLOAT a_fDeltaTime ) override;
	virtual TBOOL  Flush() override { return TTRUE; }
	virtual TINT   ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) override;
	virtual TINT   GetButtonCount() const override { return 16; }
	virtual TINT   GetAxisCount() const override { return 2; }
	virtual TBOOL  IsDown( Doodad a_iDoodad ) const override;
	virtual TINT   GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TFLOAT GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TBOOL  IsEnabled() const override { return TTRUE; }

	virtual TBOOL WasDown( Doodad a_iDoodad ) const override;
	virtual TINT  GetPortIndex() override { return 0; }

	TBOOL IsSonyController() const { return HASANYFLAG( m_oState.uiFlags, PADFLAGS_SONY ); }

	static TBOOL IsMouseMissing();

private:
	PadState m_oState;
	PadState m_oOldState;
};

TOSHI_NAMESPACE_END
