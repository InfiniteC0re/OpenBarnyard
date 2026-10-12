#include "pch.h"
#include "ADifficultyOptionState.h"

#ifdef BARNYARD_LATEST

#  include "AOptions.h"
#  include "Sound/ASoundManager.h"
#  include "SoundBank/ui.h"
#  include "GUI/T2GUIFontManager.h"
#  include "Locale/ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 80156d60, CreateObject 80156d6c, CreateObjectInPlace 80156d74 (both return 0)
TDEFINE_CLASS_NORUNTIME( ADifficultyOptionState );

// $BarnyardWiiJP: FUNCTION 80156d84
ADifficultyOptionState::ADifficultyOptionState()
{
}

// $BarnyardWiiJP: FUNCTION 80156de0
ADifficultyOptionState::~ADifficultyOptionState()
{
}

const TWCHAR* ADifficultyOptionState::GetText( TINT a_iLocaleString )
{
	// NOTE: Japanese isn't supported and the PC locale doesn't have the Wii strings, so these are English
	switch ( a_iLocaleString )
	{
		case BUTTON_TEXTS[ DIFFICULTYBUTTON_EASY ]: return L"Easy";
		case BUTTON_TEXTS[ DIFFICULTYBUTTON_NORMAL ]: return L"Normal";
		default: return L"Difficulty";
	}
}

// $BarnyardWiiJP: FUNCTION 80157064
TBOOL ADifficultyOptionState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

TPSTRING8_DECLARE( DialogInputMap );

// $BarnyardWiiJP: FUNCTION 80156e54
void ADifficultyOptionState::OnInsertion()
{
	SetInputMap( TPS8( DialogInputMap ) );
	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	for ( TINT i = 0; i < DIFFICULTYBUTTON_NUMOF; i++ )
	{
		const TWCHAR* wszText = GetText( BUTTON_TEXTS[ i ] );

		m_aButtons[ i ].Create( pFont, pFont->GetTextWidth( wszText ), wszText, 10.0f );
		m_aButtons[ i ].SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		m_aButtons[ i ].SetEnabledColour( AGUI2STYLE_COLOR_SECONDARY );
		m_aButtons[ i ].SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		m_aButtons[ i ].SetFocusedScale( 1.0f );
		m_aButtons[ i ].OnFocusLost();
		m_aButtons[ i ].SetId( i );
		m_aButtons[ i ].SetTextShadow( TTRUE, 0xa0000000, 0.6f );
		m_aButtons[ i ].SetTextShadowOffset( 5.0f );
		m_oMenu.AddMenuItem( m_aButtons[ i ] );
	}

	m_oMenu.SetFocusAt( m_aButtons[ DIFFICULTYBUTTON_EASY ] );
	m_oDialogTitle.SetText( GetText( TITLE_TEXT ) );

	BaseClass::OnInsertion();
}

// $BarnyardWiiJP: FUNCTION 80156fe8
void ADifficultyOptionState::OnRemoval()
{
	for ( TINT i = 0; i < DIFFICULTYBUTTON_NUMOF; i++ )
	{
		m_aButtons[ i ].RemoveSelf();
	}

	BaseClass::OnRemoval();
}

// $BarnyardWiiJP: FUNCTION 80157050
void ADifficultyOptionState::OnActivate()
{
	m_iActivatedButtonID = -1;
	BaseClass::OnActivate();
}

// $BarnyardWiiJP: FUNCTION 801570e4
void ADifficultyOptionState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, a_pMenuItem );
	m_iActivatedButtonID = pButton->GetId();

	ABYardMenuState::OnButtonActivated( pButton );
}

// $BarnyardWiiJP: FUNCTION 801570f0
void ADifficultyOptionState::OnMenuClose()
{
	if ( m_iActivatedButtonID == -1 )
	{
		AGameState::Remove();
		return;
	}

	// NOTE: the Wii clears the main menu antics flag of APlayerProgress::IsEasyDifficulty, sets or clears
	// SLOTFLAGS_EASY of the current save slot and starts the new game (AGameStateController flags 0x20 and 0x2002).
	// Here the choice is a setting of OpenBarnyard.cfg and the menu goes back to the options.
	AOptions* pOptions = AOptions::GetSingleton();
	pOptions->SetEasyDifficulty( m_iActivatedButtonID == DIFFICULTYBUTTON_EASY );
	pOptions->SaveExtraSettings();

	TINFO( "Difficulty: %s\n", pOptions->IsEasyDifficulty() ? "Easy" : "Normal" );

	AGameState::Remove();
}

#endif // BARNYARD_LATEST
