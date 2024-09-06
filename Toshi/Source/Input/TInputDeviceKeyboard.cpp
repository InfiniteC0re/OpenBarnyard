#include "ToshiPCH.h"
#include "TInputDeviceKeyboard.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TInputDeviceKeyboard );

struct VirtualButtonDefinition
{
	TInputDevice::Doodad iThrowDoodad;
	TInputDevice::Doodad iAlias1;
	TInputDevice::Doodad iAlias2;
};

static VirtualButtonDefinition s_aVirtualButtons[] = {
	{ TInputDeviceKeyboard::VIRTKEY_SHIFT,
	  TInputDeviceKeyboard::KEY_LSHIFT,
	  TInputDeviceKeyboard::KEY_RSHIFT },

	{ TInputDeviceKeyboard::VIRTKEY_CONTROL,
	  TInputDeviceKeyboard::KEY_LCONTROL,
	  TInputDeviceKeyboard::KEY_RCONTROL },

	{ TInputDeviceKeyboard::VIRTKEY_ALT,
	  TInputDeviceKeyboard::KEY_LMENU,
	  TInputDeviceKeyboard::KEY_RMENU }
};

TINT TInputDeviceKeyboard::ProcessVirtualButtons( EventEmitter& a_rEmitter, TFLOAT flt )
{
	a_rEmitter.Throw( TInputInterface::InputEvent( this, 0x20001, TInputInterface::EVENT_TYPE_VIRTUAL, m_aMags[ 0 ], m_aMags[ 1 ] ) );
	TINT iNumThrownEvents = 1;

	for ( TUINT i = 0; i < TARRAYSIZE( s_aVirtualButtons ); i++ )
	{
		// Check if this virtual button was pressed
		if ( !WasDown( s_aVirtualButtons[ i ].iAlias1 ) &&
		     !WasDown( s_aVirtualButtons[ i ].iAlias2 ) )
		{
			if ( IsDown( s_aVirtualButtons[ i ].iAlias1 ) ||
			     IsDown( s_aVirtualButtons[ i ].iAlias2 ) )
			{
				a_rEmitter.Throw( TInputInterface::InputEvent( this, s_aVirtualButtons[ i ].iThrowDoodad, TInputInterface::EVENT_TYPE_GONE_DOWN ) );
				iNumThrownEvents++;
			}
		}

		// Check if this virtual button was released
		if ( WasDown( s_aVirtualButtons[ i ].iAlias1 ) &&
		     WasDown( s_aVirtualButtons[ i ].iAlias2 ) )
		{
			if ( !IsDown( s_aVirtualButtons[ i ].iAlias1 ) ||
			     !IsDown( s_aVirtualButtons[ i ].iAlias2 ) )
			{
				a_rEmitter.Throw( TInputInterface::InputEvent( this, s_aVirtualButtons[ i ].iThrowDoodad, TInputInterface::EVENT_TYPE_GONE_UP ) );
				iNumThrownEvents++;
			}
		}
	}

	return iNumThrownEvents;
}

const TCHAR* TInputDeviceKeyboard::GetButtonFromDoodad( Doodad a_iDoodad ) const
{
#define CASE( KEY ) \
	case KEY: return STRINGIFY( KEY )

	switch ( a_iDoodad )
	{
		CASE( VIRTKEY_SHIFT );
		CASE( VIRTKEY_CONTROL );
		CASE( VIRTKEY_ALT );
		CASE( KEY_ESCAPE );
		CASE( KEY_1 );
		CASE( KEY_2 );
		CASE( KEY_3 );
		CASE( KEY_4 );
		CASE( KEY_5 );
		CASE( KEY_6 );
		CASE( KEY_7 );
		CASE( KEY_8 );
		CASE( KEY_9 );
		CASE( KEY_0 );
		CASE( KEY_MINUS );
		CASE( KEY_EQUALS );
		CASE( KEY_BACK );
		CASE( KEY_TAB );
		CASE( KEY_Q );
		CASE( KEY_W );
		CASE( KEY_E );
		CASE( KEY_R );
		CASE( KEY_T );
		CASE( KEY_Y );
		CASE( KEY_U );
		CASE( KEY_I );
		CASE( KEY_O );
		CASE( KEY_P );
		CASE( KEY_LBRACKET );
		CASE( KEY_RBRACKET );
		CASE( KEY_RETURN );
		CASE( KEY_LCONTROL );
		CASE( KEY_A );
		CASE( KEY_S );
		CASE( KEY_D );
		CASE( KEY_F );
		CASE( KEY_G );
		CASE( KEY_H );
		CASE( KEY_J );
		CASE( KEY_K );
		CASE( KEY_L );
		CASE( KEY_SEMICOLON );
		CASE( KEY_APOSTROPHE );
		CASE( KEY_GRAVE );
		CASE( KEY_LSHIFT );
		CASE( KEY_BACKSLASH );
		CASE( KEY_Z );
		CASE( KEY_X );
		CASE( KEY_C );
		CASE( KEY_V );
		CASE( KEY_B );
		CASE( KEY_N );
		CASE( KEY_M );
		CASE( KEY_COMMA );
		CASE( KEY_PERIOD );
		CASE( KEY_SLASH );
		CASE( KEY_RSHIFT );
		CASE( KEY_MULTIPLY );
		CASE( KEY_LMENU );
		CASE( KEY_SPACE );
		CASE( KEY_CAPITAL );
		CASE( KEY_F1 );
		CASE( KEY_F2 );
		CASE( KEY_F3 );
		CASE( KEY_F4 );
		CASE( KEY_F5 );
		CASE( KEY_F6 );
		CASE( KEY_F7 );
		CASE( KEY_F8 );
		CASE( KEY_F9 );
		CASE( KEY_F10 );
		CASE( KEY_NUMLOCK );
		CASE( KEY_SCROLL );
		CASE( KEY_NUMPAD7 );
		CASE( KEY_NUMPAD8 );
		CASE( KEY_NUMPAD9 );
		CASE( KEY_SUBTRACT );
		CASE( KEY_NUMPAD4 );
		CASE( KEY_NUMPAD5 );
		CASE( KEY_NUMPAD6 );
		CASE( KEY_ADD );
		CASE( KEY_NUMPAD1 );
		CASE( KEY_NUMPAD2 );
		CASE( KEY_NUMPAD3 );
		CASE( KEY_NUMPAD0 );
		CASE( KEY_DECIMAL );
		CASE( KEY_F11 );
		CASE( KEY_F12 );
		CASE( KEY_F13 );
		CASE( KEY_F14 );
		CASE( KEY_F15 );
		CASE( KEY_KANA );
		CASE( KEY_CONVERT );
		CASE( KEY_NOCONVERT );
		CASE( KEY_YEN );
		CASE( KEY_NUMPADEQUALS );
		CASE( KEY_CIRCUMFLEX );
		CASE( KEY_AT );
		CASE( KEY_COLON );
		CASE( KEY_UNDERLINE );
		CASE( KEY_KANJI );
		CASE( KEY_STOP );
		CASE( KEY_AX );
		CASE( KEY_UNLABELED );
		CASE( KEY_NUMPADENTER );
		CASE( KEY_RCONTROL );
		CASE( KEY_NUMPADCOMMA );
		CASE( KEY_DIVIDE );
		CASE( KEY_SYSRQ );
		CASE( KEY_RMENU );
		CASE( KEY_HOME );
		CASE( KEY_UP );
		CASE( KEY_PRIOR );
		CASE( KEY_LEFT );
		CASE( KEY_RIGHT );
		CASE( KEY_END );
		CASE( KEY_DOWN );
		CASE( KEY_NEXT );
		CASE( KEY_INSERT );
		CASE( KEY_DELETE );
		CASE( KEY_LWIN );
		CASE( KEY_APPS );
		CASE( KEY_RWIN );
		default: return "UNKNOWN";
	}

#undef CASE
}

void TInputDeviceKeyboard::SetVirtStickDoodads( TINT a_iMapID, Doodad a_iDoodad1, Doodad a_iDoodad2, Doodad a_iDoodad3, Doodad a_iDoodad4 )
{
	TASSERT( a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS );
	m_aVirtStickDoodads[ a_iMapID ].iDoodad1 = a_iDoodad1;
	m_aVirtStickDoodads[ a_iMapID ].iDoodad2 = a_iDoodad2;
	m_aVirtStickDoodads[ a_iMapID ].iDoodad3 = a_iDoodad3;
	m_aVirtStickDoodads[ a_iMapID ].iDoodad4 = a_iDoodad4;
}

void TInputDeviceKeyboard::GetVirtStickDoodads( TINT a_iMapID, Doodad& a_rDoodad1, Doodad& a_rDoodad2, Doodad& a_rDoodad3, Doodad& a_rDoodad4 )
{
	TASSERT( a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS );
	a_rDoodad1 = m_aVirtStickDoodads[ a_iMapID ].iDoodad1;
	a_rDoodad2 = m_aVirtStickDoodads[ a_iMapID ].iDoodad2;
	a_rDoodad3 = m_aVirtStickDoodads[ a_iMapID ].iDoodad3;
	a_rDoodad4 = m_aVirtStickDoodads[ a_iMapID ].iDoodad4;
}

void TInputDeviceKeyboard::UpdateVirtualButtons()
{
	m_aMags[ 2 ] = m_aMags[ 0 ];
	m_aMags[ 3 ] = m_aMags[ 1 ];
	m_aMags[ 1 ] = 0;
	m_aMags[ 0 ] = 0;

	for ( TINT i = 0; i < VIRTSTK_DIRECTIONAL_MAPS; i++ )
	{
		if ( m_aVirtStickDoodads[ i ].iDoodad2 > 0 && IsDown( m_aVirtStickDoodads[ i ].iDoodad2 ) )
			m_aMags[ 1 ] = -1.0f;

		if ( m_aVirtStickDoodads[ i ].iDoodad1 > 0 && IsDown( m_aVirtStickDoodads[ i ].iDoodad1 ) )
			m_aMags[ 1 ] = 1.0f;

		if ( m_aVirtStickDoodads[ i ].iDoodad3 > 0 && IsDown( m_aVirtStickDoodads[ i ].iDoodad3 ) )
			m_aMags[ 0 ] = -1.0f;

		if ( m_aVirtStickDoodads[ i ].iDoodad4 > 0 && IsDown( m_aVirtStickDoodads[ i ].iDoodad4 ) )
			m_aMags[ 0 ] = 1.0f;
	}
}

TInputDeviceKeyboard::TInputDeviceKeyboard()
{
}

TBOOL TInputDeviceKeyboard::GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& doodadProps ) const
{
	if ( ( a_iDoodad >= DOODAD_START ) && ( a_iDoodad < DOODAD_END ) )
	{
		doodadProps.m_iUnk  = 0;
		doodadProps.m_bFlag = TFALSE;
		return TTRUE;
	}

	return TFALSE;
}

TOSHI_NAMESPACE_END
