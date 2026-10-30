#include "PrimeDebugPCH.h"
#include "PKeyboard.h"

#ifdef PRIMEDEBUG

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TSTATICASSERT( PKeyboard::KEY_NUMOF == 111 );
TSTATICASSERT( PKeyboard::KEY_HOME == 0x5A && PKeyboard::KEY_UP == 0x5C && PKeyboard::KEY_DOWN == 0x61 && PKeyboard::KEY_ESCAPE == 0x3D && PKeyboard::KEY_RETURN == 0x41 );
TSTATICASSERT( PKeyboard::KEY_LEFT == 0x5E && PKeyboard::KEY_RIGHT == 0x5F && PKeyboard::KEY_PERIOD == 0x47 && PKeyboard::KEY_DECIMAL == 0x51 && PKeyboard::KEY_NUMPAD0 == 0x33 );

TBOOL PKeyboard::ms_abKeys[ KEY_NUMOF ];
TBOOL PKeyboard::ms_abPrevKeys[ KEY_NUMOF ];

static const TCHAR* s_aKeyNames[ PKeyboard::KEY_NUMOF ] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"Numpad0",
	"Numpad1",
	"Numpad2",
	"Numpad3",
	"Numpad4",
	"Numpad5",
	"Numpad6",
	"Numpad7",
	"Numpad8",
	"Numpad9",
	"Escape",
	"Minus",
	"Back",
	"Tab",
	"Return",
	"LeftControl",
	"LeftShift",
	"Comma",
	"Slash",
	"Backslash",
	"Period",
	"RightShift",
	"Multiply",
	"LeftAlt",
	"Space",
	"Capital",
	"Numlock",
	"Scroll",
	"Subtract",
	"Add",
	"Decimal",
	"Kana",
	"Convert",
	"NoConvert",
	"NumpadEquals",
	"Kanji",
	"RightControl",
	"Divide",
	"RightAlt",
	"Home",
	"Pause",
	"Up",
	"Prior",
	"Left",
	"Right",
	"End",
	"Down",
	"Next",
	"Insert",
	"Delete",
	"LeftWindows",
	"RightWindows",
	"AppMenu",
	"Sleep",
	"Tilde",
	"Equal",
	"Semicolon",
	"Quotation",
	"LeftBrace",
	"RightBrace",
};

// The current states become the previous ones
void PKeyboard::EndFrame()
{
	for ( TINT i = 0; i < KEY_NUMOF; i++ )
		ms_abPrevKeys[ i ] = ms_abKeys[ i ];
}

void PKeyboard::Reset()
{
	for ( TINT i = 0; i < KEY_NUMOF; i++ )
	{
		ms_abKeys[ i ]     = TFALSE;
		ms_abPrevKeys[ i ] = TFALSE;
	}
}

// $deBlob2: FUNCTION 0078bfd9
TBOOL PKeyboard::IsPressed( TINT a_iKey )
{
	TBOOL bPressed = TFALSE;

	if ( ms_abKeys[ a_iKey ] && !ms_abPrevKeys[ a_iKey ] )
		bPressed = TTRUE;

	return bPressed;
}

// $deBlob2: FUNCTION 0078bff8
TBOOL PKeyboard::IsReleased( TINT a_iKey )
{
	TBOOL bReleased = TFALSE;

	if ( !ms_abKeys[ a_iKey ] && ms_abPrevKeys[ a_iKey ] )
		bReleased = TTRUE;

	return bReleased;
}

// Not case sensitive
// $deBlob2: FUNCTION 007e2795
TBOOL PKeyboard::GetKeyFromName( TINT* a_pKey, const TCHAR* a_szName )
{
	for ( TINT i = 0; i < KEY_NUMOF; i++ )
	{
		if ( _stricmp( s_aKeyNames[ i ], a_szName ) == 0 )
		{
			*a_pKey = i;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 0080e4b2
const TCHAR* PKeyboard::GetKeyName( TINT a_iKey )
{
	return s_aKeyNames[ a_iKey ];
}

#endif // PRIMEDEBUG
