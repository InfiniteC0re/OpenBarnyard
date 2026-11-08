#include "ToshiPCH.h"
#include "TInputDeviceKeyboard_WASM.h"

#include <emscripten/html5.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TInputWASMDeviceKeyboard );

struct WASMKeyName
{
	const TCHAR* szCode;
	TINT         iDoodad;
};

// The physical keys of the page's key events
static const WASMKeyName s_aKeyNames[] = {
	{ "Escape", TInputDeviceKeyboard::KEY_ESCAPE }, { "Digit1", TInputDeviceKeyboard::KEY_1 }, { "Digit2", TInputDeviceKeyboard::KEY_2 }, { "Digit3", TInputDeviceKeyboard::KEY_3 },
	{ "Digit4", TInputDeviceKeyboard::KEY_4 }, { "Digit5", TInputDeviceKeyboard::KEY_5 }, { "Digit6", TInputDeviceKeyboard::KEY_6 }, { "Digit7", TInputDeviceKeyboard::KEY_7 },
	{ "Digit8", TInputDeviceKeyboard::KEY_8 }, { "Digit9", TInputDeviceKeyboard::KEY_9 }, { "Digit0", TInputDeviceKeyboard::KEY_0 }, { "Minus", TInputDeviceKeyboard::KEY_MINUS },
	{ "Equal", TInputDeviceKeyboard::KEY_EQUALS }, { "Backspace", TInputDeviceKeyboard::KEY_BACK }, { "Tab", TInputDeviceKeyboard::KEY_TAB }, { "KeyQ", TInputDeviceKeyboard::KEY_Q },
	{ "KeyW", TInputDeviceKeyboard::KEY_W }, { "KeyE", TInputDeviceKeyboard::KEY_E }, { "KeyR", TInputDeviceKeyboard::KEY_R }, { "KeyT", TInputDeviceKeyboard::KEY_T },
	{ "KeyY", TInputDeviceKeyboard::KEY_Y }, { "KeyU", TInputDeviceKeyboard::KEY_U }, { "KeyI", TInputDeviceKeyboard::KEY_I }, { "KeyO", TInputDeviceKeyboard::KEY_O },
	{ "KeyP", TInputDeviceKeyboard::KEY_P }, { "BracketLeft", TInputDeviceKeyboard::KEY_LBRACKET }, { "BracketRight", TInputDeviceKeyboard::KEY_RBRACKET }, { "Enter", TInputDeviceKeyboard::KEY_RETURN },
	{ "ControlLeft", TInputDeviceKeyboard::KEY_LCONTROL }, { "KeyA", TInputDeviceKeyboard::KEY_A }, { "KeyS", TInputDeviceKeyboard::KEY_S }, { "KeyD", TInputDeviceKeyboard::KEY_D },
	{ "KeyF", TInputDeviceKeyboard::KEY_F }, { "KeyG", TInputDeviceKeyboard::KEY_G }, { "KeyH", TInputDeviceKeyboard::KEY_H }, { "KeyJ", TInputDeviceKeyboard::KEY_J },
	{ "KeyK", TInputDeviceKeyboard::KEY_K }, { "KeyL", TInputDeviceKeyboard::KEY_L }, { "Semicolon", TInputDeviceKeyboard::KEY_SEMICOLON }, { "Quote", TInputDeviceKeyboard::KEY_APOSTROPHE },
	{ "Backquote", TInputDeviceKeyboard::KEY_GRAVE }, { "ShiftLeft", TInputDeviceKeyboard::KEY_LSHIFT }, { "Backslash", TInputDeviceKeyboard::KEY_BACKSLASH }, { "KeyZ", TInputDeviceKeyboard::KEY_Z },
	{ "KeyX", TInputDeviceKeyboard::KEY_X }, { "KeyC", TInputDeviceKeyboard::KEY_C }, { "KeyV", TInputDeviceKeyboard::KEY_V }, { "KeyB", TInputDeviceKeyboard::KEY_B },
	{ "KeyN", TInputDeviceKeyboard::KEY_N }, { "KeyM", TInputDeviceKeyboard::KEY_M }, { "Comma", TInputDeviceKeyboard::KEY_COMMA }, { "Period", TInputDeviceKeyboard::KEY_PERIOD },
	{ "Slash", TInputDeviceKeyboard::KEY_SLASH }, { "ShiftRight", TInputDeviceKeyboard::KEY_RSHIFT }, { "NumpadMultiply", TInputDeviceKeyboard::KEY_MULTIPLY }, { "AltLeft", TInputDeviceKeyboard::KEY_LMENU },
	{ "Space", TInputDeviceKeyboard::KEY_SPACE }, { "CapsLock", TInputDeviceKeyboard::KEY_CAPITAL }, { "F1", TInputDeviceKeyboard::KEY_F1 }, { "F2", TInputDeviceKeyboard::KEY_F2 },
	{ "F3", TInputDeviceKeyboard::KEY_F3 }, { "F4", TInputDeviceKeyboard::KEY_F4 }, { "F5", TInputDeviceKeyboard::KEY_F5 }, { "F6", TInputDeviceKeyboard::KEY_F6 },
	{ "F7", TInputDeviceKeyboard::KEY_F7 }, { "F8", TInputDeviceKeyboard::KEY_F8 }, { "F9", TInputDeviceKeyboard::KEY_F9 }, { "F10", TInputDeviceKeyboard::KEY_F10 },
	{ "NumLock", TInputDeviceKeyboard::KEY_NUMLOCK }, { "ScrollLock", TInputDeviceKeyboard::KEY_SCROLL }, { "Numpad7", TInputDeviceKeyboard::KEY_NUMPAD7 }, { "Numpad8", TInputDeviceKeyboard::KEY_NUMPAD8 },
	{ "Numpad9", TInputDeviceKeyboard::KEY_NUMPAD9 }, { "NumpadSubtract", TInputDeviceKeyboard::KEY_SUBTRACT }, { "Numpad4", TInputDeviceKeyboard::KEY_NUMPAD4 }, { "Numpad5", TInputDeviceKeyboard::KEY_NUMPAD5 },
	{ "Numpad6", TInputDeviceKeyboard::KEY_NUMPAD6 }, { "NumpadAdd", TInputDeviceKeyboard::KEY_ADD }, { "Numpad1", TInputDeviceKeyboard::KEY_NUMPAD1 }, { "Numpad2", TInputDeviceKeyboard::KEY_NUMPAD2 },
	{ "Numpad3", TInputDeviceKeyboard::KEY_NUMPAD3 }, { "Numpad0", TInputDeviceKeyboard::KEY_NUMPAD0 }, { "NumpadDecimal", TInputDeviceKeyboard::KEY_DECIMAL }, { "F11", TInputDeviceKeyboard::KEY_F11 },
	{ "F12", TInputDeviceKeyboard::KEY_F12 }, { "NumpadEnter", TInputDeviceKeyboard::KEY_NUMPADENTER }, { "ControlRight", TInputDeviceKeyboard::KEY_RCONTROL }, { "NumpadDivide", TInputDeviceKeyboard::KEY_DIVIDE },
	{ "AltRight", TInputDeviceKeyboard::KEY_RMENU }, { "Home", TInputDeviceKeyboard::KEY_HOME }, { "Pause", TInputDeviceKeyboard::KEY_PAUSE }, { "ArrowUp", TInputDeviceKeyboard::KEY_UP },
	{ "PageUp", TInputDeviceKeyboard::KEY_PRIOR }, { "ArrowLeft", TInputDeviceKeyboard::KEY_LEFT }, { "ArrowRight", TInputDeviceKeyboard::KEY_RIGHT }, { "End", TInputDeviceKeyboard::KEY_END },
	{ "ArrowDown", TInputDeviceKeyboard::KEY_DOWN }, { "PageDown", TInputDeviceKeyboard::KEY_NEXT }, { "Insert", TInputDeviceKeyboard::KEY_INSERT }, { "Delete", TInputDeviceKeyboard::KEY_DELETE },
};

static EM_BOOL OnKeyEvent( int a_iEventType, const EmscriptenKeyboardEvent* a_pEvent, void* a_pUserData )
{
	if ( a_pEvent->repeat )
		return EM_TRUE;

	for ( const WASMKeyName& rKey : s_aKeyNames )
	{
		if ( strcmp( rKey.szCode, a_pEvent->code ) == 0 )
		{
			TSTATICCAST( TInputWASMDeviceKeyboard, a_pUserData )->QueueKey( rKey.iDoodad, a_iEventType == EMSCRIPTEN_EVENT_KEYDOWN );
			break;
		}
	}

	return EM_TRUE;
}

static TINT GetKeyIndex( TINT a_iDoodad )
{
	const TINT iIndex = a_iDoodad - 0x20000;
	return ( iIndex >= 0 && iIndex < TInputWASMDeviceKeyboard::MAX_KEY_INDEX ) ? iIndex : -1;
}

TInputWASMDeviceKeyboard::TInputWASMDeviceKeyboard()
{
	TUtil::MemClear( m_abDown, sizeof( m_abDown ) );
	TUtil::MemClear( m_abWasDown, sizeof( m_abWasDown ) );
	m_iNumEvents = 0;

	emscripten_set_keydown_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, OnKeyEvent );
	emscripten_set_keyup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, OnKeyEvent );
}

TInputWASMDeviceKeyboard::~TInputWASMDeviceKeyboard()
{
	emscripten_set_keydown_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, TNULL, EM_TRUE, TNULL );
	emscripten_set_keyup_callback( EMSCRIPTEN_EVENT_TARGET_WINDOW, TNULL, EM_TRUE, TNULL );
}

void TInputWASMDeviceKeyboard::QueueKey( Doodad a_iDoodad, TBOOL a_bDown )
{
	if ( m_iNumEvents < MAX_KEY_EVENTS )
	{
		m_aiEventDoodads[ m_iNumEvents ] = a_iDoodad;
		m_abEventDown[ m_iNumEvents ]    = a_bDown;
		m_iNumEvents++;
	}
}

TBOOL TInputWASMDeviceKeyboard::Acquire()
{
	SetAcquired( TTRUE );
	return TTRUE;
}

TBOOL TInputWASMDeviceKeyboard::Unacquire()
{
	SetAcquired( TFALSE );
	return TTRUE;
}

void TInputWASMDeviceKeyboard::Release()
{
}

void TInputWASMDeviceKeyboard::Update( TFLOAT a_fDeltaTime )
{
}

TBOOL TInputWASMDeviceKeyboard::Flush()
{
	m_iNumEvents = 0;
	return TTRUE;
}

TINT TInputWASMDeviceKeyboard::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );
	iNumProcessed += ProcessVirtualButtons( a_rEmitter, a_fDeltaTime );

	TUtil::MemCopy( m_abWasDown, m_abDown, sizeof( m_abDown ) );

	for ( TINT i = 0; i < m_iNumEvents; i++ )
	{
		const TINT iKey = GetKeyIndex( m_aiEventDoodads[ i ] );

		if ( iKey < 0 || m_abDown[ iKey ] == m_abEventDown[ i ] )
			continue;

		m_abDown[ iKey ] = m_abEventDown[ i ];

		if ( m_abEventDown[ i ] )
			a_rEmitter.Throw( TInputInterface::InputEvent( this, m_aiEventDoodads[ i ], TInputInterface::EVENT_TYPE_GONE_DOWN, TranslateDoodadToCharacter( m_aiEventDoodads[ i ] ) ) );
		else
			a_rEmitter.Throw( TInputInterface::InputEvent( this, m_aiEventDoodads[ i ], TInputInterface::EVENT_TYPE_GONE_UP ) );

		iNumProcessed++;
	}

	m_iNumEvents = 0;
	UpdateVirtualButtons();

	return iNumProcessed;
}

TBOOL TInputWASMDeviceKeyboard::IsDown( Doodad a_iDoodad ) const
{
	if ( a_iDoodad == VIRTKEY_SHIFT ) return IsShiftDown();
	if ( a_iDoodad == VIRTKEY_CONTROL ) return IsControlDown();
	if ( a_iDoodad == VIRTKEY_ALT ) return IsAltDown();

	const TINT iKey = GetKeyIndex( a_iDoodad );
	return iKey >= 0 && m_abDown[ iKey ];
}

TBOOL TInputWASMDeviceKeyboard::WasDown( Doodad a_iDoodad ) const
{
	const TINT iKey = GetKeyIndex( a_iDoodad );
	return iKey >= 0 && m_abWasDown[ iKey ];
}

TBOOL TInputWASMDeviceKeyboard::IsShiftDown() const
{
	return IsDown( KEY_LSHIFT ) || IsDown( KEY_RSHIFT );
}

TBOOL TInputWASMDeviceKeyboard::IsControlDown() const
{
	return IsDown( KEY_LCONTROL ) || IsDown( KEY_RCONTROL );
}

TBOOL TInputWASMDeviceKeyboard::IsAltDown() const
{
	return IsDown( KEY_LMENU ) || IsDown( KEY_RMENU );
}

TWCHAR* TInputWASMDeviceKeyboard::TranslateDoodadToCharacter( Doodad a_iDoodad ) const
{
	static TWCHAR s_awszCharacter[ 2 ];
	s_awszCharacter[ 0 ] = L'\0';
	s_awszCharacter[ 1 ] = L'\0';

	for ( const WASMKeyName& rKey : s_aKeyNames )
	{
		if ( rKey.iDoodad != a_iDoodad )
			continue;

		if ( strncmp( rKey.szCode, "Key", 3 ) == 0 )
			s_awszCharacter[ 0 ] = TWCHAR( IsShiftDown() ? rKey.szCode[ 3 ] : rKey.szCode[ 3 ] - 'A' + 'a' );
		else if ( strncmp( rKey.szCode, "Digit", 5 ) == 0 )
			s_awszCharacter[ 0 ] = TWCHAR( rKey.szCode[ 5 ] );
		else if ( a_iDoodad == KEY_SPACE )
			s_awszCharacter[ 0 ] = L' ';

		break;
	}

	return s_awszCharacter;
}

TOSHI_NAMESPACE_END
