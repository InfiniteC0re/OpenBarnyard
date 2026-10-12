#include "pch.h"
#include "AFrontEndVariantSelectState.h"
#include "MiniGame/AMiniGameManager.h"
#include "Helpers/AGameLoader.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "GUI/T2GUIFontManager.h"
#include "GUI/AGUI2Style.h"
#include "Locale/ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AFrontEndVariantSelectState );

TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 0040cbe0
AFrontEndVariantSelectState::AFrontEndVariantSelectState( TINT a_iMiniGame )
    : m_iSelectedVariant( 0 )
    , m_iMiniGame( a_iMiniGame )
{
	SetInputMap( TPS8( DialogInputMap ) );
}

// $Barnyard: FUNCTION 0040cc40
// $Barnyard: FUNCTION 0040ccc0
AFrontEndVariantSelectState::~AFrontEndVariantSelectState()
{
}

// $Barnyard: FUNCTION 0040cf00
TBOOL AFrontEndVariantSelectState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::ui::UI_MENUBACK );

		m_iSelectedVariant = -1;
		m_eMenuState       = MENUSTATE_MENU_DISAPPEAR;
		return TTRUE;
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0040cce0
void AFrontEndVariantSelectState::OnInsertion()
{
	T2GUIFont* pFont     = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_SECONDARY );
	AMiniGame* pMiniGame = AMiniGameManager::GetSingleton()->GetMiniGame( m_iMiniGame );

	m_aButtons = new AGUI2Button[ pMiniGame->m_iNumSelectableVariants ];

	for ( TINT i = 0; i < pMiniGame->m_iNumSelectableVariants; i++ )
	{
		const TINT    eLocaleTitle = pMiniGame->m_aVariants[ i ].eLocaleTitle;
		const TWCHAR* wszText      = ( eLocaleTitle == -1 ) ? TNULL : g_pLocaleManager->GetString( eLocaleTitle );

		m_aButtons[ i ].Create( pFont, pFont->GetTextWidth( wszText ), wszText, 3.0f );
		m_aButtons[ i ].SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		m_aButtons[ i ].SetEnabledColour( AGUI2STYLE_COLOR_PRIMARY );
		m_aButtons[ i ].SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		m_aButtons[ i ].SetFocusedScale( 1.0f );
		m_aButtons[ i ].OnFocusLost();
		m_aButtons[ i ].SetId( i );

		AGUI2TextBox& rTextBox = m_aButtons[ i ].GetTextBox();
		rTextBox.SetShadowEnabled( TTRUE );
		rTextBox.SetShadowColour( 0xa0000000 );
		rTextBox.SetShadowAlpha( ABYardMenuState::s_fShadowAlpha );
		rTextBox.SetShadowOffset( rTextBox.GetScale() * 5.0f, rTextBox.GetScale() * 5.0f );

		m_oMenu.AddMenuItem( m_aButtons[ i ] );
	}

	m_oMenu.SetFocusAt( m_aButtons[ 0 ] );
	m_oDialogTitle.SetText( g_pLocaleManager->GetString( 350 ) );

	BaseClass::OnInsertion();
}

// $Barnyard: FUNCTION 0040cc50
void AFrontEndVariantSelectState::OnRemoval()
{
	delete[] m_aButtons;

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 0040cc90
void AFrontEndVariantSelectState::OnActivate()
{
	m_iSelectedVariant = 0;
	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 0040cca0
void AFrontEndVariantSelectState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, a_pMenuItem );
	m_iSelectedVariant   = pButton->GetId();

	ABYardMenuState::OnButtonActivated( pButton );
}

// $Barnyard: FUNCTION 0040cf60
void AFrontEndVariantSelectState::OnMenuClose()
{
	if ( m_iSelectedVariant == -1 )
	{
		BaseClass::Remove();
		return;
	}

	m_oDialogTitleBackground.Hide();
	m_oDialog.Hide();
	m_HUDParams.SetFlags( 0 );

	AMiniGameManager::GetSingleton()->SetMiniGameVariant( m_iSelectedVariant );
	AGameLoader::SetTransitionToMiniGame( m_iMiniGame, TFALSE );
}
