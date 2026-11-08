#include "ToshiPCH.h"
#include "TInputDeviceController_WASM.h"

#include <emscripten.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

static Toshi::TInputWASMDeviceController::PadState s_oPadState;

extern "C" EMSCRIPTEN_KEEPALIVE void* WASM_GetPadState()
{
	return &s_oPadState;
}

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TInputWASMDeviceController );

TInputWASMDeviceController::TInputWASMDeviceController()
{
	TUtil::MemClear( &m_oState, sizeof( m_oState ) );
	TUtil::MemClear( &m_oOldState, sizeof( m_oOldState ) );
}

TInputWASMDeviceController::~TInputWASMDeviceController()
{
}

TBOOL TInputWASMDeviceController::Acquire()
{
	SetAcquired( TTRUE );
	return TTRUE;
}

TBOOL TInputWASMDeviceController::Unacquire()
{
	SetAcquired( TFALSE );
	return TTRUE;
}

void TInputWASMDeviceController::Release()
{
}

TBOOL TInputWASMDeviceController::IsMouseMissing()
{
	return HASANYFLAG( s_oPadState.uiFlags, PADFLAGS_NOMOUSE );
}

void TInputWASMDeviceController::Update( TFLOAT a_fDeltaTime )
{
	m_oOldState = m_oState;
	m_oState    = s_oPadState;

	UpdateVirtualButtons();
}

TINT TInputWASMDeviceController::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );
	iNumProcessed += ProcessVirtualButtons( a_rEmitter, a_fDeltaTime );

	const TUINT32 uiChanged = m_oState.uiButtons ^ m_oOldState.uiButtons;

	for ( TINT i = 0; i < 16; i++ )
	{
		if ( uiChanged & ( 1u << i ) )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_DPAD_UP + i, ( m_oState.uiButtons & ( 1u << i ) ) ? TInputInterface::EVENT_TYPE_GONE_DOWN : TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed++;
		}
	}

	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_LEFT, TInputInterface::EVENT_TYPE_VIRTUAL, m_oState.afSticks[ 0 ], m_oState.afSticks[ 1 ] ) );
	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_RIGHT, TInputInterface::EVENT_TYPE_VIRTUAL, m_oState.afSticks[ 2 ], m_oState.afSticks[ 3 ] ) );

	return iNumProcessed + 2;
}

TBOOL TInputWASMDeviceController::IsDown( Doodad a_iDoodad ) const
{
	const TINT iBit = a_iDoodad - BUTTON_DPAD_UP;

	if ( iBit < 0 || iBit >= 16 )
		return TInputDeviceController::IsDown( a_iDoodad );

	return ( m_oState.uiButtons & ( 1u << iBit ) ) != 0;
}

TBOOL TInputWASMDeviceController::WasDown( Doodad a_iDoodad ) const
{
	const TINT iBit = a_iDoodad - BUTTON_DPAD_UP;

	if ( iBit < 0 || iBit >= 16 )
		return TInputDeviceController::WasDown( a_iDoodad );

	return ( m_oOldState.uiButtons & ( 1u << iBit ) ) != 0;
}

TINT TInputWASMDeviceController::GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const
{
	return TINT( GetAxisFloat( a_iDoodad, a_iAxis ) * STICK_SATURATE );
}

TFLOAT TInputWASMDeviceController::GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const
{
	if ( a_iAxis != 0 && a_iAxis != 1 )
		return 0.0f;

	if ( a_iDoodad == STICK_LEFT )
		return m_oState.afSticks[ a_iAxis ];

	if ( a_iDoodad == STICK_RIGHT )
		return m_oState.afSticks[ 2 + a_iAxis ];

	return 0.0f;
}

TOSHI_NAMESPACE_END
