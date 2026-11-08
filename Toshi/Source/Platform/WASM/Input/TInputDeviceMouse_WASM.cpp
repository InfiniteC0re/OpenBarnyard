#include "ToshiPCH.h"
#include "TInputDeviceMouse_WASM.h"

#include <emscripten/html5.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TInputWASMDeviceMouse );

static EM_BOOL OnMouseEvent( int a_iEventType, const EmscriptenMouseEvent* a_pEvent, void* a_pUserData )
{
	auto pMouse = TSTATICCAST( TInputWASMDeviceMouse, a_pUserData );

	if ( a_iEventType == EMSCRIPTEN_EVENT_MOUSEMOVE )
	{
		pMouse->AddMovement( a_pEvent->movementX, a_pEvent->movementY );
		return EM_TRUE;
	}

	// The page numbers the buttons left, middle, right
	const TINT iButton = a_pEvent->button == 0 ? 0 : a_pEvent->button == 2 ? 1 : 2;
	pMouse->SetButton( iButton, a_iEventType == EMSCRIPTEN_EVENT_MOUSEDOWN );

	return EM_TRUE;
}

static EM_BOOL OnWheelEvent( int a_iEventType, const EmscriptenWheelEvent* a_pEvent, void* a_pUserData )
{
	TSTATICCAST( TInputWASMDeviceMouse, a_pUserData )->AddWheel( a_pEvent->deltaY < 0.0 ? 1 : -1 );
	return EM_TRUE;
}

TInputWASMDeviceMouse::TInputWASMDeviceMouse()
{
	m_uiPendingButtons = 0;
	m_uiButtons        = 0;
	m_iPendingX        = 0;
	m_iPendingY        = 0;
	m_iPendingSteps    = 0;
	m_iDeltaX          = 0;
	m_iDeltaY          = 0;
	m_iDeltaSteps      = 0;

	emscripten_set_mousemove_callback( EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, OnMouseEvent );
	emscripten_set_mousedown_callback( EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, OnMouseEvent );
	emscripten_set_mouseup_callback( EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, OnMouseEvent );
	emscripten_set_wheel_callback( EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, OnWheelEvent );
}

TInputWASMDeviceMouse::~TInputWASMDeviceMouse()
{
	emscripten_set_mousemove_callback( EMSCRIPTEN_EVENT_TARGET_DOCUMENT, TNULL, EM_TRUE, TNULL );
	emscripten_set_mousedown_callback( EMSCRIPTEN_EVENT_TARGET_DOCUMENT, TNULL, EM_TRUE, TNULL );
	emscripten_set_mouseup_callback( EMSCRIPTEN_EVENT_TARGET_DOCUMENT, TNULL, EM_TRUE, TNULL );
	emscripten_set_wheel_callback( EMSCRIPTEN_EVENT_TARGET_DOCUMENT, TNULL, EM_TRUE, TNULL );
}

void TInputWASMDeviceMouse::SetButton( TINT a_iButton, TBOOL a_bDown )
{
	if ( a_bDown )
		m_uiPendingButtons |= 1u << a_iButton;
	else
		m_uiPendingButtons &= ~( 1u << a_iButton );
}

TBOOL TInputWASMDeviceMouse::Acquire()
{
	SetAcquired( TTRUE );
	return TTRUE;
}

TBOOL TInputWASMDeviceMouse::Unacquire()
{
	SetAcquired( TFALSE );
	return TTRUE;
}

void TInputWASMDeviceMouse::Release()
{
}

void TInputWASMDeviceMouse::Update( TFLOAT a_fDeltaTime )
{
	m_iDeltaX     = m_iPendingX;
	m_iDeltaY     = m_iPendingY;
	m_iDeltaSteps = m_iPendingSteps;

	m_iPendingX     = 0;
	m_iPendingY     = 0;
	m_iPendingSteps = 0;
}

TBOOL TInputWASMDeviceMouse::Flush()
{
	m_iPendingX     = 0;
	m_iPendingY     = 0;
	m_iPendingSteps = 0;
	return TTRUE;
}

TINT TInputWASMDeviceMouse::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );

	const TUINT32 uiChanged = m_uiPendingButtons ^ m_uiButtons;

	for ( TINT i = 0; i < 3; i++ )
	{
		if ( uiChanged & ( 1u << i ) )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, BUTTON_1 + i, ( m_uiPendingButtons & ( 1u << i ) ) ? TInputInterface::EVENT_TYPE_GONE_DOWN : TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed++;
		}
	}

	m_uiButtons = m_uiPendingButtons;

	if ( m_iDeltaX != 0 || m_iDeltaY != 0 )
	{
		a_rEmitter.Throw( TInputInterface::InputEvent( this, AXIS_CURSOR, TInputInterface::EVENT_TYPE_MOVED, m_iDeltaX, m_iDeltaY ) );
		iNumProcessed++;
	}

	if ( m_iDeltaSteps != 0 )
	{
		a_rEmitter.Throw( TInputInterface::InputEvent( this, AXIS_WHEEL, TInputInterface::EVENT_TYPE_MOVED, 0.0f, TFLOAT( m_iDeltaSteps * 120 ) ) );

		const Doodad iButton = m_iDeltaSteps > 0 ? BUTTON_WHEEL_FORWARD : BUTTON_WHEEL_BACKWARD;
		a_rEmitter.Throw( TInputInterface::InputEvent( this, iButton, TInputInterface::EVENT_TYPE_GONE_DOWN ) );
		a_rEmitter.Throw( TInputInterface::InputEvent( this, iButton, TInputInterface::EVENT_TYPE_GONE_UP ) );
		iNumProcessed += 3;
	}

	return iNumProcessed;
}

TBOOL TInputWASMDeviceMouse::IsDown( Doodad a_iDoodad ) const
{
	if ( a_iDoodad >= BUTTON_1 && a_iDoodad <= BUTTON_3 )
		return ( m_uiButtons & ( 1u << ( a_iDoodad - BUTTON_1 ) ) ) != 0;

	return TFALSE;
}

TINT TInputWASMDeviceMouse::GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const
{
	if ( a_iDoodad == AXIS_CURSOR )
		return a_iAxis == 0 ? m_iDeltaX : a_iAxis == 1 ? m_iDeltaY : 0;

	return 0;
}

TFLOAT TInputWASMDeviceMouse::GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const
{
	if ( a_iDoodad == AXIS_WHEEL )
		return TFLOAT( m_iDeltaSteps * 120 );

	return TFLOAT( GetAxisInt( a_iDoodad, a_iAxis ) );
}

TOSHI_NAMESPACE_END
