#include "pch.h"
#include "AFrontEndLanguageState.h"
#include "AFrontEndMainMenuState2.h"
#include "AGameStateController.h"
#include "AOptionsState.h"
#include "AOptions.h"
#include "ALoadScreen.h"
#include "HUD/AHudManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "SoundBank/ui.h"
#include "SoundBank/music.h"
#include "GUI/AGUI2Style.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#ifdef BARNYARD_COMMUNITY_PATCH
TDEFINE_CLASS( AFrontEndLanguageState );
#else
// NOTE: CreateObject returns TNULL on PC (the class is stripped)
TDEFINE_CLASS_NORUNTIME( AFrontEndLanguageState );
#endif

// PC 0x007167b8 (unreferenced, 5 languages), PS2 SLES_543.78 0x0050ab60 (the same without German)
const AFrontEndLanguageState::LanguageInfo AFrontEndLanguageState::ms_aLanguages[ LANGUAGE_NUMOF ] = {
#ifdef BARNYARD_COMMUNITY_PATCH
	{ ALocaleManager::Lang_EnglishUK, L"English (UK)", L"Language", L"%c_DialogInputMap[OK] Confirm", L"%c_DialogInputMap[Cancel] Cancel" },
	{ ALocaleManager::Lang_English, L"English (US)", L"Language", L"%c_DialogInputMap[OK] Confirm", L"%c_DialogInputMap[Cancel] Cancel" },
#endif
	{ ALocaleManager::Lang_Netherlandic, L"Nederlands", L"Taal", L"%c_DialogInputMap[OK] Bevestigen", L"%c_DialogInputMap[Cancel] Annuleren" },
	{ ALocaleManager::Lang_German, L"Deutsch", L"Sprache", L"%c_DialogInputMap[OK] Best\u00e4tigen", L"%c_DialogInputMap[Cancel] Abbrechen" },
	{ ALocaleManager::Lang_Italian, L"Italiano", L"Lingua", L"%c_DialogInputMap[OK] Conferma", L"%c_DialogInputMap[Cancel] Annulla" },
	{ ALocaleManager::Lang_Spanish, L"Espa\u00f1ol", L"Idioma", L"%c_DialogInputMap[OK] Confirmar", L"%c_DialogInputMap[Cancel] Cancelar" },
	{ ALocaleManager::Lang_French, L"Fran\u00e7ais", L"Langue", L"%c_DialogInputMap[OK] Confirmer", L"%c_DialogInputMap[Cancel] Annuler" },
};

// $BarnyardPS2EU: FUNCTION 0010e5b8
AFrontEndLanguageState::AFrontEndLanguageState( AGameState* a_pNextState, TBOOL a_bFirstBoot )
    : m_iSelectedLanguage( -1 )
    , m_pNextState( a_pNextState )
    , m_eLanguage( ALocaleManager::Lang_EnglishUK )
    , m_bFirstBoot( a_bFirstBoot )
{
}

// $BarnyardPS2EU: FUNCTION 0010e658
AFrontEndLanguageState::~AFrontEndLanguageState()
{
}

// $BarnyardPS2EU: FUNCTION 0010ead8
TBOOL AFrontEndLanguageState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
	{
		// A language has to be picked at boot
		if ( m_bFirstBoot )
			return TTRUE;

		m_iSelectedLanguage = -1;
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $BarnyardPS2EU: FUNCTION 0010ea78
TBOOL AFrontEndLanguageState::OnUpdate( TFLOAT a_fDeltaTime )
{
	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $BarnyardPS2EU: FUNCTION 0010e788
void AFrontEndLanguageState::OnInsertion()
{
	// Hide what's behind the screen at boot
	if ( m_bFirstBoot )
		SetHasRectangle2( TTRUE );

	m_eLanguage = g_pLocaleManager->GetLanguage();

	// Focus the current language (the first one if it isn't in the list)
	TINT iCurrent = 0;

	if ( m_eLanguage != ms_aLanguages[ 0 ].eLanguage )
	{
		for ( TINT i = 1; i < LANGUAGE_NUMOF; i++ )
		{
			if ( ms_aLanguages[ i ].eLanguage == m_eLanguage )
			{
				iCurrent = i;
				break;
			}
		}
	}

	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	for ( TINT i = 0; i < LANGUAGE_NUMOF; i++ )
	{
		AGUI2Button& rButton = m_aButtons[ i ];

		rButton.Create( pFont, pFont->GetTextWidth( ms_aLanguages[ i ].wszName ), ms_aLanguages[ i ].wszName, 0.0f );
		rButton.SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		rButton.SetEnabledColour( AGUI2STYLE_COLOR_SECONDARY );
		rButton.SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		rButton.SetFocusedScale( 1.0f );
		rButton.OnFocusLost();
		rButton.SetId( i );
		rButton.SetTextShadow( TTRUE, 0xa0000000, s_fShadowAlpha );
		rButton.SetTextShadowOffset( 5.0f );

		m_oMenu.AddMenuItem( rButton );
	}

	m_oMenu.T2GUIElement::SetFocus( TFALSE );
	m_oMenu.SetItemSpacing( 5.0f );
	m_oDialogTitle.SetText( ms_aLanguages[ iCurrent ].wszTitle );
	m_iSelectedLanguage = iCurrent;

	BaseClass::OnInsertion();

	UpdateTexts();
	m_oMenu.SetFocusAt( m_aButtons[ iCurrent ] );
	m_oRectangle2.SetColour( 0xff000000 );
}

// $BarnyardPS2EU: FUNCTION 0010e9f0
void AFrontEndLanguageState::OnRemoval()
{
	for ( TINT i = 0; i < LANGUAGE_NUMOF; i++ )
		m_aButtons[ i ].RemoveSelf();

	if ( !m_bFirstBoot ) AMenuNavHudElement::UpdateText();

	BaseClass::OnRemoval();
}

// $BarnyardPS2EU: FUNCTION 0010ea98
void AFrontEndLanguageState::OnActivate()
{
	BaseClass::OnActivate();
}

// $BarnyardPS2EU: FUNCTION 0010eab8
void AFrontEndLanguageState::OnDeactivate()
{
	BaseClass::OnDeactivate();
}

// $BarnyardPS2EU: FUNCTION 0010e6e8
void AFrontEndLanguageState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	m_iSelectedLanguage = TSTATICCAST( AGUI2Button, a_pMenuItem )->GetId();
	UpdateTexts();

	OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );
}

// $BarnyardPS2EU: FUNCTION 0010e730
void AFrontEndLanguageState::OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus )
{
	m_iSelectedLanguage = TSTATICCAST( AGUI2Button, &a_rNewFocus )->GetId();
	UpdateTexts();

	BaseClass::OnMenuItemFocused( a_rOldFocus, a_rNewFocus );
}

// $BarnyardPS2EU: FUNCTION 0010eb68
void AFrontEndLanguageState::OnMenuClose()
{
	if ( m_iSelectedLanguage != -1 )
	{
		const T2Locale::Lang eLanguage = ms_aLanguages[ m_iSelectedLanguage ].eLanguage;

		// Closing the options saves the game
		if ( g_pLocaleManager->GetLanguage() != eLanguage )
			AOptionsState::ms_bSettingsChanged = TTRUE;

		if ( !m_bFirstBoot )
		{
			AMusicManager::GetSingleton()->StopBackgroundMusic();
			ASoundManager::GetSingleton()->OnUpdate( 0.0f );
		}

		// NOTE: PS2 also sets a global (0x005461f4) that nothing reads, ARootTask::CreateStartupGameStates sets
		// it too when the console language is used
		AOptions::GetSingleton()->SetLanguage( eLanguage );
		g_pLocaleManager->SetLanguage( eLanguage );

		if ( !m_bFirstBoot )
			AMusicManager::GetSingleton()->PlayBackgroundMusic( soundbank::MUSIC_FRONTEND );
	}

	if ( !m_bFirstBoot )
	{
		// The old locale strings are gone, show the new ones
		g_oLoadScreen.UpdateLoadingText();

		AOptionsState* pOptionsState = TSTATICCAST( AOptionsState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "AOptionsState" ) ) );

		if ( pOptionsState )
			pOptionsState->UpdateTexts();

		AFrontEndMainMenuState2* pMainMenuState = TSTATICCAST( AFrontEndMainMenuState2, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "AFrontEndMainMenuState2" ) ) );

		if ( pMainMenuState )
			pMainMenuState->UpdateTexts();
	}

	if ( m_pNextState )
		AGameStateController::GetSingleton()->ReplaceState( m_pNextState );
	else
		Remove();
}

// $BarnyardPS2EU: FUNCTION 0010ed50
void AFrontEndLanguageState::UpdateTexts()
{
	const LanguageInfo& rLanguage = ms_aLanguages[ m_iSelectedLanguage ];

	if ( !m_bFirstBoot )
	{
		AMenuNavHudElement* pMenuNav = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();

		if ( pMenuNav )
			pMenuNav->SetTextStrings( rLanguage.wszOK, rLanguage.wszCancel, TNULL );
	}

	m_oDialogTitle.SetTextAndFitWidth( rLanguage.wszTitle );
}
