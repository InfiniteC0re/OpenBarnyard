#include "pch.h"
#include "AInputMapManager.h"
#include "AInputHandler.h"
#include "Memory/AMemory.h"
#include "Locale/ALocaleManager.h"
#include "HUD/AMenuNavHudElement.h"

#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceMouse.h>
#include <Input/TInputDeviceController.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00596de0, CreateObject 0059c930, CreateObjectInPlace 0059c990
TDEFINE_CLASS( AInputMapManager );

// $Barnyard: FUNCTION 0059c840
AInputMapManager::AInputMapManager()
    : m_pActiveInputMap( TNULL ), m_iNumPushedInputMaps( 0 ), m_oDoodadToNameMap( AMemory::GetAllocator( AMemory::POOL_Misc ) ), m_oKeyMap( AMemory::GetAllocator( AMemory::POOL_Misc ) ), m_oCommandMap( AMemory::GetAllocator( AMemory::POOL_Misc ) ), m_oDoodadToCustomNameMap( AMemory::GetAllocator( AMemory::POOL_Misc ) )
{
	InitMouseDoodads();
	InitKeyboardDoodads();
	InitGamepadDoodads();
	InitCommandMap();
}

// $Barnyard: FUNCTION 0059ae20
// $Barnyard: FUNCTION 0059c910
AInputMapManager::~AInputMapManager()
{
	while ( m_InputMaps.Size() != 0 )
	{
		AInputMap* pInputMap = m_InputMaps.Back().Value();
		m_InputMaps.PopBack();

		if ( pInputMap )
			delete pInputMap;
	}
}

// $Barnyard: FUNCTION 00597140
TBOOL AInputMapManager::ReadControlsData()
{
	TString8 controlsFilePath;
	controlsFilePath.Format( "Data/Options/%s.trb", "Controls_Win" );

	TTRB trb;
	auto eLoadResult = trb.Load( controlsFilePath );

	if ( eLoadResult != TTRB::ERROR_OK )
	{
		return TFALSE;
	}

	auto properties = PBProperties::LoadFromTRB( trb );

	while ( m_InputMaps.Size() != 0 )
	{
		auto pInputMap = m_InputMaps.Back();
		m_InputMaps.PopBack();

		if ( pInputMap )
		{
			delete pInputMap.Get();
		}
	}

	for ( auto it = properties->Begin(); it != properties->End(); it++ )
	{
		auto pInputMap = new AInputMap( TPString8( it->GetName().GetString() ) );

		pInputMap->LoadFromProperties( *it->GetValue()->GetProperties() );
		m_InputMaps.PushBack( pInputMap );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00596e00
void AInputMapManager::PushInputMap( AInputMap* a_pInputMap )
{
	TASSERT( m_iNumPushedInputMaps < MAX_NUM_INPUT_MAPS );
	m_iNumPushedInputMaps++;

	if ( m_iNumPushedInputMaps < 32 )
	{
		m_apPushedInputMaps[ m_iNumPushedInputMaps - 1 ] = a_pInputMap;
	}

	m_pActiveInputMap = a_pInputMap;
}

// $Barnyard: FUNCTION 00596e40
AInputMap* AInputMapManager::PopInputMap()
{
	TASSERT( m_iNumPushedInputMaps > 0 );
	m_iNumPushedInputMaps = Toshi::TMath::Max( m_iNumPushedInputMaps - 1, 0 );

	m_pActiveInputMap = ( m_iNumPushedInputMaps != 0 ) ? m_apPushedInputMaps[ m_iNumPushedInputMaps - 1 ] : TNULL;
	return m_apPushedInputMaps[ m_iNumPushedInputMaps - 1 ];
}

// $Barnyard: FUNCTION 00596ee0
AInputMap* AInputMapManager::FindMap( const Toshi::TPString8& a_rMapName )
{
	for ( auto it = m_InputMaps.Begin(); it != m_InputMaps.End(); it++ )
	{
		if ( ( *it )->GetName() == a_rMapName )
		{
			return *it;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00596f50
TBOOL AInputMapManager::SetMap( const Toshi::TPString8& a_rMapName )
{
	auto pMap = FindMap( a_rMapName );
	if ( !pMap ) return TFALSE;

	m_pActiveInputMap = pMap;
	return TTRUE;
}

struct GlyphEntry
{
	TINT          iDoodad;
	const TWCHAR* wszGlyph;
};

static const GlyphEntry s_aGlyphsPS2[] = {
	{ TInputDeviceController::BUTTON_CONTEXT_1, L"\x0F1A" },        // Triangle
	{ TInputDeviceController::BUTTON_CONTEXT_2, L"\x0F0D" },        // Circle
	{ TInputDeviceController::BUTTON_CONTEXT_3, L"\x0F0E" },        // Cross
	{ TInputDeviceController::BUTTON_CONTEXT_4, L"\x0F0F" },        // Square
	{ TInputDeviceController::BUTTON_SHOULDER_LEFT_1, L"\x0F08" },  // L1
	{ TInputDeviceController::BUTTON_SHOULDER_LEFT_2, L"\x0F11" },  // L2
	{ TInputDeviceController::BUTTON_SHOULDER_RIGHT_1, L"\x0F09" }, // R1
	{ TInputDeviceController::BUTTON_SHOULDER_RIGHT_2, L"\x0F10" }, // R2
	{ TInputDeviceController::VIRTSTK_DPAD, L"\x0F16" },            // D-pad
};

static const GlyphEntry s_aGlyphsGC[] = {
	{ TInputDeviceController::BUTTON_CONTEXT_1, L"\x0F0A" },        // Y
	{ TInputDeviceController::BUTTON_CONTEXT_2, L"\x0F0C" },        // B
	{ TInputDeviceController::BUTTON_CONTEXT_3, L"\x0F0B" },        // A
	{ TInputDeviceController::BUTTON_CONTEXT_4, L"\x0F1B" },        // X
	{ TInputDeviceController::BUTTON_SHOULDER_LEFT_1, L"\x0F19" },  // L
	{ TInputDeviceController::BUTTON_SHOULDER_RIGHT_1, L"\x0F20" }, // R
	{ TInputDeviceController::BUTTON_SHOULDER_RIGHT_2, L"\x0F1C" }, // Z
	{ TInputDeviceController::VIRTSTK_DPAD, L"\x0F01" },            // D-pad
};

// $Barnyard: FUNCTION 00597790
void AInputMapManager::GetEventCommands( const Toshi::TInputInterface::InputEvent* a_pEvent, AInputCommandArray& a_rCommandArray )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	
	TBOOL bIsCurrentlyUsingController = AInputHandler::GetSingleton()->IsControllerUsed();

	if ( m_bIsUsingController != bIsCurrentlyUsingController )
	{
		if ( bIsCurrentlyUsingController )
		{
			// Unbind keyboard and mouse
			DeinitMouseDoodads();
			DeinitKeyboardDoodads();

			// Apply correct remaps to draw controller glyphs
			TSIZE             iNumRemaps = ( AInputHandler::GetSingleton()->IsControllerXInput() ) ? TARRAYSIZE( s_aGlyphsGC ) : TARRAYSIZE( s_aGlyphsPS2 );
			const GlyphEntry* pRemaps    = ( AInputHandler::GetSingleton()->IsControllerXInput() ) ? s_aGlyphsGC : s_aGlyphsPS2;

			for ( TSIZE i = 0; i < iNumRemaps; i++ )
			{
				m_oDoodadToCustomNameMap.Insert( pRemaps[ i ].iDoodad, pRemaps[ i ].wszGlyph );
			}
		}
		else
		{
			InitMouseDoodads();
			InitKeyboardDoodads();
			m_oDoodadToCustomNameMap.Clear();
		}

		AMenuNavHudElement::UpdateText();

		m_bIsUsingController = bIsCurrentlyUsingController;
	}
#endif // BARNYARD_COMMUNITY_PATCH

	a_rCommandArray.Clear();

	if ( m_pActiveInputMap )
	{
		auto pCommandMap      = &m_pActiveInputMap->GetCommandMap();
		auto pCommandIndexRes = pCommandMap->FindLast( a_pEvent->GetDoodad() );

		while ( pCommandMap->IsValid( pCommandIndexRes ) )
		{
			auto  eCommand    = AInputMap::GetBindingCommand( pCommandIndexRes->GetSecond() );
			auto  pCommandRes = m_oCommandMap.Find( eCommand );
			TBOOL bCommandValid;

			if ( m_oCommandMap.IsValid( pCommandRes ) )
			{
				bCommandValid = TTRUE;
				auto command  = pCommandRes->GetSecond();

				if ( !command.IsEventTypeAllowed( a_pEvent->GetEventType() ) )
				{
					bCommandValid = TFALSE;
				}
			}
			else
			{
				bCommandValid = TFALSE;
			}

			if ( bCommandValid )
			{
				a_rCommandArray.AddCommand( eCommand );
			}

			// Look if there are any other commands binded to this doodad
			pCommandIndexRes = pCommandMap->FindNext( pCommandIndexRes, a_pEvent->GetDoodad() );
		}
	}
}

// $Barnyard: FUNCTION 005975d0
TBOOL AInputMapManager::GetCommandLocalisedName( AInputCommand a_eCommand, const TWCHAR*& a_rwszOutName )
{
	auto pCommandRes = m_oCommandMap.Find( a_eCommand );

	if ( !m_oCommandMap.IsValid( pCommandRes ) )
	{
		a_rwszOutName = TNULL;
		return TFALSE;
	}

	Command oCommand = pCommandRes->GetSecond();
	a_rwszOutName    = ( oCommand.uiCode != -1 ) ? g_pLocaleManager->GetString( oCommand.uiCode ) : TNULL;

	return TTRUE;
}

// $Barnyard: FUNCTION 005973b0
TBOOL AInputMapManager::GetDoodadCustomName( Toshi::TInputDevice::Doodad a_iDoodad, const TWCHAR*& a_rwszOutName )
{
	auto pNameRes = m_oDoodadToCustomNameMap.Find( a_iDoodad );

	if ( !m_oDoodadToCustomNameMap.IsValid( pNameRes ) )
	{
		a_rwszOutName = TNULL;
		return TFALSE;
	}

	a_rwszOutName = pNameRes->GetSecond();
	return TTRUE;
}

// $Barnyard: FUNCTION 00597350
TBOOL AInputMapManager::GetDoodadLocalisedName( Toshi::TInputDevice::Doodad a_iDoodad, const TWCHAR*& a_rwszOutName )
{
	auto pKeyRes = m_oKeyMap.Find( a_iDoodad );

	if ( !m_oKeyMap.IsValid( pKeyRes ) )
	{
		a_rwszOutName = TNULL;
		return TFALSE;
	}

	a_rwszOutName = g_pLocaleManager->GetString( pKeyRes->GetSecond() );
	return TTRUE;
}

// $Barnyard: FUNCTION 00597670
AInputCommand AInputMapManager::GetCommandCode( const Toshi::TPString8& a_rCommandName )
{
	for ( auto it = m_oCommandMap.Begin(); it != m_oCommandMap.End(); it++ )
	{
		if ( it->GetSecond().Name == a_rCommandName )
		{
			return it->GetFirst();
		}
	}

	return AInputCommand_Unknown;
}

// $Barnyard: FUNCTION 00597700
Toshi::TInputDevice::Doodad AInputMapManager::GetDoodadFromKey( const Toshi::TPString8& a_rKey )
{
	for ( auto it = m_oDoodadToNameMap.Begin(); it != m_oDoodadToNameMap.End(); it++ )
	{
		if ( it->GetSecond() == a_rKey )
		{
			return it->GetFirst();
		}
	}

	return -1;
}

TBOOL AInputMapManager::HasBindForDoodad( Toshi::TInputDevice::Doodad a_iDoodad )
{
	return m_oDoodadToNameMap.Find( a_iDoodad ) != m_oDoodadToNameMap.End();
}

// $Barnyard: FUNCTION 0059c220
void AInputMapManager::InitMouseDoodads()
{
	BindDoodad( TInputDeviceMouse::BUTTON_1, TPString8( "Button1" ), 0x582 );
	BindDoodad( TInputDeviceMouse::BUTTON_2, TPString8( "Button2" ), 0x583 );
	BindDoodad( TInputDeviceMouse::BUTTON_3, TPString8( "Button3" ), 0x584 );
	BindDoodad( TInputDeviceMouse::BUTTON_4, TPString8( "Button4" ), 0x585 );
	BindDoodad( TInputDeviceMouse::BUTTON_5, TPString8( "Button5" ), 0x586 );
	BindDoodad( TInputDeviceMouse::BUTTON_6, TPString8( "Button6" ), 0x587 );
	BindDoodad( TInputDeviceMouse::BUTTON_7, TPString8( "Button7" ), 0x588 );
	BindDoodad( TInputDeviceMouse::BUTTON_8, TPString8( "Button8" ), 0x589 );
	BindDoodad( TInputDeviceMouse::BUTTON_WHEEL_FORWARD, TPString8( "Wheel Fwd" ), 0x58A );
	BindDoodad( TInputDeviceMouse::BUTTON_WHEEL_BACKWARD, TPString8( "Wheel Back" ), 0x58B );
	BindDoodad( TInputDeviceMouse::AXIS_CURSOR, TPString8( "Cursor Axis" ), 0x58C );
	BindDoodad( TInputDeviceMouse::AXIS_WHEEL, TPString8( "Wheel Axis" ), 0x58D );
}

// $Barnyard: FUNCTION 0059af20
void AInputMapManager::InitKeyboardDoodads()
{
	BindDoodad( TInputDeviceKeyboard::VIRTKEY, TPString8( "Virtual Stick" ), 0x506 );
	BindDoodad( TInputDeviceKeyboard::KEY_LSHIFT, TPString8( "Left Shift" ), 0x530 );
	BindDoodad( TInputDeviceKeyboard::KEY_RSHIFT, TPString8( "Right Shift" ), 0x53c );
	BindDoodad( TInputDeviceKeyboard::KEY_LCONTROL, TPString8( "Left Control" ), 0x523 );
	BindDoodad( TInputDeviceKeyboard::KEY_RCONTROL, TPString8( "Right Control" ), 0x56d );
	BindDoodad( TInputDeviceKeyboard::KEY_LMENU, TPString8( "Left Alt" ), 0x53e );
	BindDoodad( TInputDeviceKeyboard::KEY_RMENU, TPString8( "Right Alt" ), 0x571 );
	BindDoodad( TInputDeviceKeyboard::KEY_LWIN, TPString8( "Left Windows" ), 0x57d );
	BindDoodad( TInputDeviceKeyboard::KEY_RWIN, TPString8( "Right Windows" ), 0x57e );
	BindDoodad( TInputDeviceKeyboard::KEY_ESCAPE, TPString8( "Escape" ), 0x507 );
	BindDoodad( TInputDeviceKeyboard::KEY_1, TPString8( "1" ), 0x508 );
	BindDoodad( TInputDeviceKeyboard::KEY_2, TPString8( "2" ), 0x509 );
	BindDoodad( TInputDeviceKeyboard::KEY_3, TPString8( "3" ), 0x50a );
	BindDoodad( TInputDeviceKeyboard::KEY_4, TPString8( "4" ), 0x50b );
	BindDoodad( TInputDeviceKeyboard::KEY_5, TPString8( "5" ), 0x50c );
	BindDoodad( TInputDeviceKeyboard::KEY_6, TPString8( "6" ), 0x50d );
	BindDoodad( TInputDeviceKeyboard::KEY_7, TPString8( "7" ), 0x50e );
	BindDoodad( TInputDeviceKeyboard::KEY_8, TPString8( "8" ), 0x50f );
	BindDoodad( TInputDeviceKeyboard::KEY_9, TPString8( "9" ), 0x510 );
	BindDoodad( TInputDeviceKeyboard::KEY_0, TPString8( "0" ), 0x511 );
	BindDoodad( TInputDeviceKeyboard::KEY_MINUS, TPString8( "Minus" ), 0x512 );
	BindDoodad( TInputDeviceKeyboard::KEY_EQUALS, TPString8( "Equals" ), 0x513 );
	BindDoodad( TInputDeviceKeyboard::KEY_BACK, TPString8( "Backspace" ), 0x514 );
	BindDoodad( TInputDeviceKeyboard::KEY_TAB, TPString8( "Tab" ), 0x515 );
	BindDoodad( TInputDeviceKeyboard::KEY_Q, TPString8( "Q" ), 0x516 );
	BindDoodad( TInputDeviceKeyboard::KEY_W, TPString8( "W" ), 0x517 );
	BindDoodad( TInputDeviceKeyboard::KEY_E, TPString8( "E" ), 0x518 );
	BindDoodad( TInputDeviceKeyboard::KEY_R, TPString8( "R" ), 0x519 );
	BindDoodad( TInputDeviceKeyboard::KEY_T, TPString8( "T" ), 0x51a );
	BindDoodad( TInputDeviceKeyboard::KEY_Y, TPString8( "Y" ), 0x51b );
	BindDoodad( TInputDeviceKeyboard::KEY_U, TPString8( "U" ), 0x51c );
	BindDoodad( TInputDeviceKeyboard::KEY_I, TPString8( "I" ), 0x51d );
	BindDoodad( TInputDeviceKeyboard::KEY_O, TPString8( "O" ), 0x51e );
	BindDoodad( TInputDeviceKeyboard::KEY_P, TPString8( "P" ), 0x51f );
	BindDoodad( TInputDeviceKeyboard::KEY_LBRACKET, TPString8( "Left Bracket" ), 0x520 );
	BindDoodad( TInputDeviceKeyboard::KEY_RBRACKET, TPString8( "Right Bracket" ), 0x521 );
	BindDoodad( TInputDeviceKeyboard::KEY_RETURN, TPString8( "Return" ), 0x522 );
	BindDoodad( TInputDeviceKeyboard::KEY_A, TPString8( "A" ), 0x524 );
	BindDoodad( TInputDeviceKeyboard::KEY_S, TPString8( "S" ), 0x525 );
	BindDoodad( TInputDeviceKeyboard::KEY_D, TPString8( "D" ), 0x526 );
	BindDoodad( TInputDeviceKeyboard::KEY_F, TPString8( "F" ), 0x527 );
	BindDoodad( TInputDeviceKeyboard::KEY_G, TPString8( "G" ), 0x528 );
	BindDoodad( TInputDeviceKeyboard::KEY_H, TPString8( "H" ), 0x529 );
	BindDoodad( TInputDeviceKeyboard::KEY_J, TPString8( "J" ), 0x52a );
	BindDoodad( TInputDeviceKeyboard::KEY_K, TPString8( "K" ), 0x52b );
	BindDoodad( TInputDeviceKeyboard::KEY_L, TPString8( "L" ), 0x52c );
	BindDoodad( TInputDeviceKeyboard::KEY_SEMICOLON, TPString8( "Semicolon" ), 0x52d );
	BindDoodad( TInputDeviceKeyboard::KEY_APOSTROPHE, TPString8( "Apostrophe" ), 0x52e );
	BindDoodad( TInputDeviceKeyboard::KEY_GRAVE, TPString8( "Grave" ), 0x52f );
	BindDoodad( TInputDeviceKeyboard::KEY_BACKSLASH, TPString8( "Backslash" ), 0x531 );
	BindDoodad( TInputDeviceKeyboard::KEY_Z, TPString8( "Z" ), 0x532 );
	BindDoodad( TInputDeviceKeyboard::KEY_X, TPString8( "X" ), 0x533 );
	BindDoodad( TInputDeviceKeyboard::KEY_C, TPString8( "C" ), 0x534 );
	BindDoodad( TInputDeviceKeyboard::KEY_V, TPString8( "V" ), 0x535 );
	BindDoodad( TInputDeviceKeyboard::KEY_B, TPString8( "B" ), 0x536 );
	BindDoodad( TInputDeviceKeyboard::KEY_N, TPString8( "N" ), 0x537 );
	BindDoodad( TInputDeviceKeyboard::KEY_M, TPString8( "M" ), 0x538 );
	BindDoodad( TInputDeviceKeyboard::KEY_COMMA, TPString8( "Comma" ), 0x539 );
	BindDoodad( TInputDeviceKeyboard::KEY_PERIOD, TPString8( "Period" ), 0x53a );
	BindDoodad( TInputDeviceKeyboard::KEY_SLASH, TPString8( "Slash" ), 0x53b );
	BindDoodad( TInputDeviceKeyboard::KEY_MULTIPLY, TPString8( "Multiply" ), 0x53d );
	BindDoodad( TInputDeviceKeyboard::KEY_SPACE, TPString8( "Space" ), 0x53f );
	BindDoodad( TInputDeviceKeyboard::KEY_CAPITAL, TPString8( "Capslock" ), 0x540 );
	BindDoodad( TInputDeviceKeyboard::KEY_F1, TPString8( "F1" ), 0x541 );
	BindDoodad( TInputDeviceKeyboard::KEY_F2, TPString8( "F2" ), 0x542 );
	BindDoodad( TInputDeviceKeyboard::KEY_F3, TPString8( "F3" ), 0x543 );
	BindDoodad( TInputDeviceKeyboard::KEY_F4, TPString8( "F4" ), 0x544 );
	BindDoodad( TInputDeviceKeyboard::KEY_F5, TPString8( "F5" ), 0x545 );
	BindDoodad( TInputDeviceKeyboard::KEY_F6, TPString8( "F6" ), 0x546 );
	BindDoodad( TInputDeviceKeyboard::KEY_F7, TPString8( "F7" ), 0x547 );
	BindDoodad( TInputDeviceKeyboard::KEY_F8, TPString8( "F8" ), 0x548 );
	BindDoodad( TInputDeviceKeyboard::KEY_F9, TPString8( "F9" ), 0x549 );
	BindDoodad( TInputDeviceKeyboard::KEY_F10, TPString8( "F10" ), 0x54a );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMLOCK, TPString8( "NumLock" ), 0x54b );
	BindDoodad( TInputDeviceKeyboard::KEY_SCROLL, TPString8( "ScrollLock" ), 0x54c );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD7, TPString8( "Numpad 7" ), 0x54d );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD8, TPString8( "Numpad 8" ), 0x54e );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD9, TPString8( "Numpad 9" ), 0x54f );
	BindDoodad( TInputDeviceKeyboard::KEY_SUBTRACT, TPString8( "Subtract" ), 0x550 );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD4, TPString8( "Numpad 4" ), 0x551 );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD5, TPString8( "Numpad 5" ), 0x552 );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD6, TPString8( "Numpad 6" ), 0x553 );
	BindDoodad( TInputDeviceKeyboard::KEY_ADD, TPString8( "Add" ), 0x554 );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD1, TPString8( "Numpad 1" ), 0x555 );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD2, TPString8( "Numpad 2" ), 0x556 );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD3, TPString8( "Numpad 3" ), 0x557 );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPAD0, TPString8( "Numpad 0" ), 0x558 );
	BindDoodad( TInputDeviceKeyboard::KEY_DECIMAL, TPString8( "Decimal" ), 0x559 );
	BindDoodad( TInputDeviceKeyboard::KEY_F11, TPString8( "F11" ), 0x55a );
	BindDoodad( TInputDeviceKeyboard::KEY_F12, TPString8( "F12" ), 0x55b );
	BindDoodad( TInputDeviceKeyboard::KEY_F13, TPString8( "F13" ), 0x55c );
	BindDoodad( TInputDeviceKeyboard::KEY_F14, TPString8( "F14" ), 0x55d );
	BindDoodad( TInputDeviceKeyboard::KEY_F15, TPString8( "F15" ), 0x55e );
	BindDoodad( TInputDeviceKeyboard::KEY_KANA, TPString8( "Kana" ), 0x55f );
	BindDoodad( TInputDeviceKeyboard::KEY_CONVERT, TPString8( "Convert" ), 0x560 );
	BindDoodad( TInputDeviceKeyboard::KEY_NOCONVERT, TPString8( "No Convert" ), 0x561 );
	BindDoodad( TInputDeviceKeyboard::KEY_YEN, TPString8( "Yen" ), 0x562 );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPADEQUALS, TPString8( "Numpad Equals" ), 0x563 );
	BindDoodad( TInputDeviceKeyboard::KEY_CIRCUMFLEX, TPString8( "Circumflex" ), 0x564 );
	BindDoodad( TInputDeviceKeyboard::KEY_AT, TPString8( "At" ), 0x565 );
	BindDoodad( TInputDeviceKeyboard::KEY_COLON, TPString8( "Colon" ), 0x566 );
	BindDoodad( TInputDeviceKeyboard::KEY_UNDERLINE, TPString8( "Underline" ), 0x567 );
	BindDoodad( TInputDeviceKeyboard::KEY_KANJI, TPString8( "Kanji" ), 0x568 );
	BindDoodad( TInputDeviceKeyboard::KEY_STOP, TPString8( "Stop" ), 0x569 );
	BindDoodad( TInputDeviceKeyboard::KEY_AX, TPString8( "Ax" ), 0x56a );
	BindDoodad( TInputDeviceKeyboard::KEY_UNLABELED, TPString8( "Unlabeled" ), 0x56b );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPADENTER, TPString8( "Numpad Enter" ), 0x56c );
	BindDoodad( TInputDeviceKeyboard::KEY_NUMPADCOMMA, TPString8( "Numpad Comma" ), 0x56e );
	BindDoodad( TInputDeviceKeyboard::KEY_DIVIDE, TPString8( "Divide" ), 0x56f );
	BindDoodad( TInputDeviceKeyboard::KEY_SYSRQ, TPString8( "SysRq" ), 0x570 );
	BindDoodad( TInputDeviceKeyboard::KEY_PAUSE, TPString8( "Pause" ), 0x572 );
	BindDoodad( TInputDeviceKeyboard::KEY_HOME, TPString8( "Home" ), 0x573 );
	BindDoodad( TInputDeviceKeyboard::KEY_UP, TPString8( "Up" ), 0x574 );
	BindDoodad( TInputDeviceKeyboard::KEY_PRIOR, TPString8( "Prior" ), 0x575 );
	BindDoodad( TInputDeviceKeyboard::KEY_LEFT, TPString8( "Left" ), 0x576 );
	BindDoodad( TInputDeviceKeyboard::KEY_RIGHT, TPString8( "Right" ), 0x577 );
	BindDoodad( TInputDeviceKeyboard::KEY_END, TPString8( "End" ), 0x578 );
	BindDoodad( TInputDeviceKeyboard::KEY_DOWN, TPString8( "Down" ), 0x579 );
	BindDoodad( TInputDeviceKeyboard::KEY_NEXT, TPString8( "Next" ), 0x57a );
	BindDoodad( TInputDeviceKeyboard::KEY_INSERT, TPString8( "Insert" ), 0x57b );
	BindDoodad( TInputDeviceKeyboard::KEY_DELETE, TPString8( "Delete" ), 0x57c );
	BindDoodad( TInputDeviceKeyboard::KEY_APPS, TPString8( "Apps" ), 0x57f );
	BindDoodad( TInputDeviceKeyboard::KEY_POWER, TPString8( "Power" ), 0x580 );
	BindDoodad( TInputDeviceKeyboard::KEY_SLEEP, TPString8( "Sleep" ), 0x581 );
}

void AInputMapManager::DeinitMouseDoodads()
{
	UnbindDoodad( TInputDeviceMouse::BUTTON_1 );
	UnbindDoodad( TInputDeviceMouse::BUTTON_2 );
	UnbindDoodad( TInputDeviceMouse::BUTTON_3 );
	UnbindDoodad( TInputDeviceMouse::BUTTON_4 );
	UnbindDoodad( TInputDeviceMouse::BUTTON_5 );
	UnbindDoodad( TInputDeviceMouse::BUTTON_6 );
	UnbindDoodad( TInputDeviceMouse::BUTTON_7 );
	UnbindDoodad( TInputDeviceMouse::BUTTON_8 );
	UnbindDoodad( TInputDeviceMouse::BUTTON_WHEEL_FORWARD );
	UnbindDoodad( TInputDeviceMouse::BUTTON_WHEEL_BACKWARD );
	UnbindDoodad( TInputDeviceMouse::AXIS_CURSOR );
	UnbindDoodad( TInputDeviceMouse::AXIS_WHEEL );
}

void AInputMapManager::DeinitKeyboardDoodads()
{
	UnbindDoodad( TInputDeviceKeyboard::VIRTKEY );
	UnbindDoodad( TInputDeviceKeyboard::KEY_LSHIFT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_RSHIFT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_LCONTROL );
	UnbindDoodad( TInputDeviceKeyboard::KEY_RCONTROL );
	UnbindDoodad( TInputDeviceKeyboard::KEY_LMENU );
	UnbindDoodad( TInputDeviceKeyboard::KEY_RMENU );
	UnbindDoodad( TInputDeviceKeyboard::KEY_LWIN );
	UnbindDoodad( TInputDeviceKeyboard::KEY_RWIN );
	UnbindDoodad( TInputDeviceKeyboard::KEY_ESCAPE );
	UnbindDoodad( TInputDeviceKeyboard::KEY_1 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_2 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_3 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_4 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_5 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_6 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_7 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_8 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_9 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_0 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_MINUS );
	UnbindDoodad( TInputDeviceKeyboard::KEY_EQUALS );
	UnbindDoodad( TInputDeviceKeyboard::KEY_BACK );
	UnbindDoodad( TInputDeviceKeyboard::KEY_TAB );
	UnbindDoodad( TInputDeviceKeyboard::KEY_Q );
	UnbindDoodad( TInputDeviceKeyboard::KEY_W );
	UnbindDoodad( TInputDeviceKeyboard::KEY_E );
	UnbindDoodad( TInputDeviceKeyboard::KEY_R );
	UnbindDoodad( TInputDeviceKeyboard::KEY_T );
	UnbindDoodad( TInputDeviceKeyboard::KEY_Y );
	UnbindDoodad( TInputDeviceKeyboard::KEY_U );
	UnbindDoodad( TInputDeviceKeyboard::KEY_I );
	UnbindDoodad( TInputDeviceKeyboard::KEY_O );
	UnbindDoodad( TInputDeviceKeyboard::KEY_P );
	UnbindDoodad( TInputDeviceKeyboard::KEY_LBRACKET );
	UnbindDoodad( TInputDeviceKeyboard::KEY_RBRACKET );
	UnbindDoodad( TInputDeviceKeyboard::KEY_RETURN );
	UnbindDoodad( TInputDeviceKeyboard::KEY_A );
	UnbindDoodad( TInputDeviceKeyboard::KEY_S );
	UnbindDoodad( TInputDeviceKeyboard::KEY_D );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F );
	UnbindDoodad( TInputDeviceKeyboard::KEY_G );
	UnbindDoodad( TInputDeviceKeyboard::KEY_H );
	UnbindDoodad( TInputDeviceKeyboard::KEY_J );
	UnbindDoodad( TInputDeviceKeyboard::KEY_K );
	UnbindDoodad( TInputDeviceKeyboard::KEY_L );
	UnbindDoodad( TInputDeviceKeyboard::KEY_SEMICOLON );
	UnbindDoodad( TInputDeviceKeyboard::KEY_APOSTROPHE );
	UnbindDoodad( TInputDeviceKeyboard::KEY_GRAVE );
	UnbindDoodad( TInputDeviceKeyboard::KEY_BACKSLASH );
	UnbindDoodad( TInputDeviceKeyboard::KEY_Z );
	UnbindDoodad( TInputDeviceKeyboard::KEY_X );
	UnbindDoodad( TInputDeviceKeyboard::KEY_C );
	UnbindDoodad( TInputDeviceKeyboard::KEY_V );
	UnbindDoodad( TInputDeviceKeyboard::KEY_B );
	UnbindDoodad( TInputDeviceKeyboard::KEY_N );
	UnbindDoodad( TInputDeviceKeyboard::KEY_M );
	UnbindDoodad( TInputDeviceKeyboard::KEY_COMMA );
	UnbindDoodad( TInputDeviceKeyboard::KEY_PERIOD );
	UnbindDoodad( TInputDeviceKeyboard::KEY_SLASH );
	UnbindDoodad( TInputDeviceKeyboard::KEY_MULTIPLY );
	UnbindDoodad( TInputDeviceKeyboard::KEY_SPACE );
	UnbindDoodad( TInputDeviceKeyboard::KEY_CAPITAL );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F1 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F2 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F3 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F4 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F5 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F6 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F7 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F8 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F9 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F10 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMLOCK );
	UnbindDoodad( TInputDeviceKeyboard::KEY_SCROLL );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD7 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD8 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD9 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_SUBTRACT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD4 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD5 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD6 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_ADD );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD1 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD2 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD3 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPAD0 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_DECIMAL );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F11 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F12 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F13 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F14 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_F15 );
	UnbindDoodad( TInputDeviceKeyboard::KEY_KANA );
	UnbindDoodad( TInputDeviceKeyboard::KEY_CONVERT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NOCONVERT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_YEN );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPADEQUALS );
	UnbindDoodad( TInputDeviceKeyboard::KEY_CIRCUMFLEX );
	UnbindDoodad( TInputDeviceKeyboard::KEY_AT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_COLON );
	UnbindDoodad( TInputDeviceKeyboard::KEY_UNDERLINE );
	UnbindDoodad( TInputDeviceKeyboard::KEY_KANJI );
	UnbindDoodad( TInputDeviceKeyboard::KEY_STOP );
	UnbindDoodad( TInputDeviceKeyboard::KEY_AX );
	UnbindDoodad( TInputDeviceKeyboard::KEY_UNLABELED );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPADENTER );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NUMPADCOMMA );
	UnbindDoodad( TInputDeviceKeyboard::KEY_DIVIDE );
	UnbindDoodad( TInputDeviceKeyboard::KEY_SYSRQ );
	UnbindDoodad( TInputDeviceKeyboard::KEY_PAUSE );
	UnbindDoodad( TInputDeviceKeyboard::KEY_HOME );
	UnbindDoodad( TInputDeviceKeyboard::KEY_UP );
	UnbindDoodad( TInputDeviceKeyboard::KEY_PRIOR );
	UnbindDoodad( TInputDeviceKeyboard::KEY_LEFT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_RIGHT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_END );
	UnbindDoodad( TInputDeviceKeyboard::KEY_DOWN );
	UnbindDoodad( TInputDeviceKeyboard::KEY_NEXT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_INSERT );
	UnbindDoodad( TInputDeviceKeyboard::KEY_DELETE );
	UnbindDoodad( TInputDeviceKeyboard::KEY_APPS );
	UnbindDoodad( TInputDeviceKeyboard::KEY_POWER );
	UnbindDoodad( TInputDeviceKeyboard::KEY_SLEEP );
}

// $Barnyard: FUNCTION 0059c410
void AInputMapManager::InitGamepadDoodads()
{
	BindDoodad( TInputDeviceController::BUTTON_DPAD_UP, TPString8( "DPadUp" ), 0x58e );
	BindDoodad( TInputDeviceController::BUTTON_DPAD_RIGHT, TPString8( "DPadRight" ), 0x58f );
	BindDoodad( TInputDeviceController::BUTTON_DPAD_DOWN, TPString8( "DPadDown" ), 0x590 );
	BindDoodad( TInputDeviceController::BUTTON_DPAD_LEFT, TPString8( "DPadLeft" ), 0x591 );
	BindDoodad( TInputDeviceController::BUTTON_SELECT, TPString8( "Select" ), 0x592 );
	BindDoodad( TInputDeviceController::BUTTON_START, TPString8( "Start" ), 0x593 );
	BindDoodad( TInputDeviceController::BUTTON_STICK_LEFT, TPString8( "Left Stick Click" ), 0x594 );
	BindDoodad( TInputDeviceController::BUTTON_STICK_RIGHT, TPString8( "Right Stick Click" ), 0x595 );
	BindDoodad( TInputDeviceController::BUTTON_CONTEXT_1, TPString8( "Context1" ), 0x596 );
	BindDoodad( TInputDeviceController::BUTTON_CONTEXT_2, TPString8( "Context2" ), 0x597 );
	BindDoodad( TInputDeviceController::BUTTON_CONTEXT_3, TPString8( "Context3" ), 0x598 );
	BindDoodad( TInputDeviceController::BUTTON_CONTEXT_4, TPString8( "Context4" ), 0x599 );
	BindDoodad( TInputDeviceController::BUTTON_SHOULDER_LEFT_1, TPString8( "L1" ), 0x59a );
	BindDoodad( TInputDeviceController::BUTTON_SHOULDER_LEFT_2, TPString8( "L2" ), 0x59b );
	BindDoodad( TInputDeviceController::BUTTON_SHOULDER_RIGHT_1, TPString8( "R1" ), 0x59c );
	BindDoodad( TInputDeviceController::BUTTON_SHOULDER_RIGHT_2, TPString8( "R2" ), 0x59d );
	BindDoodad( TInputDeviceController::VIRTBTN_STICK_LEFT_UP, TPString8( "Left Stick Up" ), 0x59e );
	BindDoodad( TInputDeviceController::VIRTBTN_STICK_LEFT_RIGHT, TPString8( "Left Stick Right" ), 0x59f );
	BindDoodad( TInputDeviceController::VIRTBTN_STICK_LEFT_DOWN, TPString8( "Left Stick Down" ), 0x5a0 );
	BindDoodad( TInputDeviceController::VIRTBTN_STICK_LEFT_LEFT, TPString8( "Left Stick Left" ), 0x5a1 );
	BindDoodad( TInputDeviceController::VIRTBTN_STICK_RIGHT_UP, TPString8( "Right Stick Up" ), 0x5a2 );
	BindDoodad( TInputDeviceController::VIRTBTN_STICK_RIGHT_RIGHT, TPString8( "Right Stick Right" ), 0x5a3 );
	BindDoodad( TInputDeviceController::VIRTBTN_STICK_RIGHT_DOWN, TPString8( "Right Stick Down" ), 0x5a4 );
	BindDoodad( TInputDeviceController::VIRTBTN_STICK_RIGHT_LEFT, TPString8( "Right Stick Left" ), 0x5a5 );
	BindDoodad( TInputDeviceController::VIRTSTK_DPAD, TPString8( "Virtual DPad Stick" ), 0x5a8 );
	BindDoodad( TInputDeviceController::STICK_LEFT, TPString8( "Left Stick" ), 0x5a6 );
	BindDoodad( TInputDeviceController::STICK_RIGHT, TPString8( "Right Stick" ), 0x5a7 );
}

// $Barnyard: FUNCTION 00597d40
void AInputMapManager::InitCommandMap()
{
#define ADD_COMMAND( NAME, UNK, EVENT_TYPE )             m_oCommandMap.Insert( AInputCommand_##NAME, { TPString8( #NAME ), UNK, EVENT_TYPE } )
#define ADD_COMMAND_EX( COMMAND, NAME, UNK, EVENT_TYPE ) m_oCommandMap.Insert( COMMAND, { TPString8( NAME ), UNK, EVENT_TYPE } )

	ADD_COMMAND( Unknown, -1, 0 );
	ADD_COMMAND_EX( AInputCommand_Empty, Toshi::TPString8(), -1, 0 );
	ADD_COMMAND( OK, 0x4b9, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Cancel, 0x4ba, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Retry, 0x4bb, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Back, 0x4bc, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Close, 0x4bd, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Quit, 0x4be, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Start, 0x4bf, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Select, 0x4c0, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Yes, 0x4c2, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( No, 0x4c3, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Restart, 0x4c4, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Leave, 0x4c5, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Save, 0x4c6, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Move, 0x4c9, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Up, 0x4c7, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Down, 0x4c8, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LeftWithUp, 0x4eb, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( RightWithUp, 0x4ea, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Left, 0x4ca, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Right, 0x4cb, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Forward, 0x4cc, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( ForwardWithUp, 0x4ec, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SuperSpeed_Forward_Debug, 0x4f6, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SuperSpeed, 0x4f7, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( PanLeft, 0x4fe, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( PanRight, 0x4ff, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Reload, 0x4f4, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Dodge, 0x4f5, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Backward, 0x4cd, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( BackwardWithUp, 0x4ed, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( ZoomIn, 0x4d3, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( ZoomOut, 0x4d4, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Look, 0x4d5, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LookUp, 0x4d6, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LookDown, 0x4d7, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LookLeft, 0x4d8, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LookRight, 0x4d9, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LookUpWithUp, 0x4b7, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LookDownWithUp, 0x4b8, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LookLeftWithUp, 0x4b5, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( LookRightWithUp, 0x4b6, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Context1, 0x4da, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Context2, 0x4db, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Context3, 0x4dc, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Context4, 0x4dd, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Menu, 0x4e4, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( DebugMenu, 0x4e5, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Interact, 0x4e6, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Use, 0x4f2, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Jump, 0x4f0, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Shoot, 0x4e7, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( ZLock, 0x500, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( ShootAlt, 0x4e8, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Switch, 0x4f9, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SideStep, 0x4f8, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Speed, 0x4ef, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Brake, 0x4ee, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Accelerator, 0x4e9, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Bell, 0x4f3, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Trick1, 0x4fa, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Trick2, 0x4fb, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Trick3, 0x4fc, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Trick4, 0x4fd, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SelectUp, 0x4de, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SelectDown, 0x4df, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SelectLeft, 0x4e0, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SelectRight, 0x4e1, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SelectPrevious, 0x4e2, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( SelectNext, 0x4e3, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( ElevateCameraCW, 0x4ce, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( ElevateCameraCCW, 0x4cf, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( RotateCameraCW, 0x4d0, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( RotateCamera, 0x4d1, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( RotateCameraCCW, 0x4d2, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Inventory, 0x4f1, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Icons, 0x501, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( ZLock, 0x500, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Quad, -1, INPUTEVENT_GONE_DOWN | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND( Horn, 0x4b4, INPUTEVENT_GONE_DOWN | INPUTEVENT_GONE_UP | INPUTEVENT_REPEAT | INPUTEVENT_VIRTUAL | INPUTEVENT_MOVED );
	ADD_COMMAND_EX( AInputCommand_CUSTOM_COMMAND, "CUSTOM COMMAND", -1, 0 );
}

// $Barnyard: FUNCTION 0059ad40
void AInputMapManager::BindDoodad( Toshi::TInputDevice::Doodad a_iDoodad, const Toshi::TPString8& a_ButtonName, ActionId a_uiAction )
{
	m_oDoodadToNameMap.Insert( a_iDoodad, a_ButtonName );
	m_oKeyMap.Insert( a_iDoodad, a_uiAction );
}

void AInputMapManager::UnbindDoodad( Toshi::TInputDevice::Doodad a_iDoodad )
{
	m_oDoodadToNameMap.FindAndRemove( a_iDoodad );
	m_oKeyMap.FindAndRemove( a_iDoodad );
}

// T2Map/T2RedBlackTree methods (template instances for m_oKeyMap, m_oDoodadToNameMap and m_oCommandMap):

// Clear

// $Barnyard: FUNCTION 00597920

// node allocation (doodad/name pair, command pair)

// $Barnyard: FUNCTION 00597980

// $Barnyard: FUNCTION 00597a00

// Insert

// $Barnyard: FUNCTION 00597a80

// $Barnyard: FUNCTION 00597b20

// $Barnyard: FUNCTION 00597bd0

// $Barnyard: FUNCTION 00597c60

// Delete (range)

// $Barnyard: FUNCTION 00597cf0
// T2Pair::second returned by value (folded): <Doodad, TPString8> 00597400, <AInputCommand, Command> 00597430
// $Barnyard: FUNCTION 00597400
// $Barnyard: FUNCTION 00597430
