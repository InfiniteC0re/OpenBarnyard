#include "pch.h"
#include "AOptionsState.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"
#include "AGameStateController.h"
#include "AControlOptionsState.h"
#include "AAudioOptionsState.h"
#include "ACameraOptionsState.h"
#include "ADifficultyOptionState.h"
#include "SaveLoadSKU.h"
#include "Helpers/APlayerProgress.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AOptionsState );

// $Barnyard: FUNCTION 00444ef0
AOptionsState::AOptionsState()
{
}

// $Barnyard: FUNCTION 00444f50
// $Barnyard: FUNCTION 00445170
AOptionsState::~AOptionsState()
{
}

// $Barnyard: FUNCTION 00407aa0
TBOOL AOptionsState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

TBOOL AOptionsState::OnUpdate( TFLOAT a_fDeltaTime )
{
	return BaseClass::OnUpdate( a_fDeltaTime );
}

TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 00444fc0
// $BarnyardWiiJP: FUNCTION 8015736c
void AOptionsState::OnInsertion()
{
	SetInputMap( TPS8( DialogInputMap ) );
	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	for ( TINT i = 0; i < OPTIONSBUTTON_NUMOF; i++ )
	{
#ifdef BARNYARD_LATEST
		// NOTE: the Wii skips OPTIONSBUTTON_CONTROLS here (no mouse), the PC keeps it
		const TWCHAR* wszText;

		if ( i == OPTIONSBUTTON_CAMERA )
			wszText = ACameraOptionsState::GetCameraText();
		else if ( i == OPTIONSBUTTON_DIFFICULTY )
			wszText = ADifficultyOptionState::GetText( OPTIONSBUTTON_TEXTS[ i ] );
		else
			wszText = g_pLocaleManager->GetString( OPTIONSBUTTON_TEXTS[ i ] );
#else
		const TWCHAR* wszText = g_pLocaleManager->GetString( OPTIONSBUTTON_TEXTS[ i ] );
#endif

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

	m_oMenu.SetFocusAt( m_aButtons[ 0 ] );
	m_oDialogTitle.SetText( g_pLocaleManager->GetString( 284 ) );

	BaseClass::OnInsertion();
}

// $Barnyard: FUNCTION 004450f0
void AOptionsState::OnRemoval()
{
	for ( TINT i = 0; i < OPTIONSBUTTON_NUMOF; i++ )
	{
		m_aButtons[ i ].RemoveSelf();
	}

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00445130
void AOptionsState::OnActivate()
{
	m_iActivatedButtonID = -1;
	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 00445150
void AOptionsState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, a_pMenuItem );
	m_iActivatedButtonID = pButton->GetId();

	ABYardMenuState::OnButtonActivated( pButton );
}

// $Barnyard: FUNCTION 00445190
// $BarnyardWiiJP: FUNCTION 80157614
void AOptionsState::OnMenuClose()
{
	if ( m_iActivatedButtonID != -1 )
	{
		AGameState* pState = TNULL;

		if ( m_iActivatedButtonID == OPTIONSBUTTON_AUDIO )
			pState = new AAudioOptionsState();
		else if ( m_iActivatedButtonID == OPTIONSBUTTON_CONTROLS )
			pState = new AControlOptionsState();
#ifdef BARNYARD_LATEST
		else if ( m_iActivatedButtonID == OPTIONSBUTTON_CAMERA )
			pState = new ACameraOptionsState();
		else if ( m_iActivatedButtonID == OPTIONSBUTTON_DIFFICULTY )
			pState = new ADifficultyOptionState();
#endif

		AGameStateController::GetSingleton()->PushState( pState );
		return;
	}

	AGameState::Remove();

	if ( ms_bSettingsChanged )
	{
		// Save the changed settings
		AGameStateController::GetSingleton()->SaveCurrentGameState( TFALSE, TFALSE, TTRUE );

		ms_bSettingsChanged = TFALSE;
	}
}
