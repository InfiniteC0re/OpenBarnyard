#include "pch.h"
#include "AReplayVariantSelectState.h"
#include "MiniGame/AMiniGameManager.h"
#include "GUI/T2GUIFontManager.h"
#include "GUI/AGUI2Style.h"
#include "Locale/ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AReplayVariantSelectState );

TPSTRING8_DECLARE( DialogInputMap );

// $Barnyard: FUNCTION 0043c8d0
AReplayVariantSelectState::AReplayVariantSelectState( AReplayableGame* a_pReplay )
    : m_pReplay( a_pReplay )
{
	SetInputMap( TPS8( DialogInputMap ) );
}

// $Barnyard: FUNCTION 0043c930
// $Barnyard: FUNCTION 0043c9f0
AReplayVariantSelectState::~AReplayVariantSelectState()
{
}

// $Barnyard: FUNCTION 0043c980
TBOOL AReplayVariantSelectState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
	{
		m_iSelectedButton = -1;
		m_eMenuState      = MENUSTATE_MENU_DISAPPEAR;
		return TTRUE;
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0043ca10
void AReplayVariantSelectState::OnInsertion()
{
	T2GUIFont* pFont     = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_SECONDARY );
	AMiniGame* pMiniGame = AMiniGameManager::GetSingleton()->GetMiniGame( m_pReplay->m_iMiniGame );

	m_aButtons = new AGUI2Button[ m_pReplay->GetNumVariants() ];

	TINT iNumButtons = 0;

	for ( TINT i = 0; i < pMiniGame->GetNumSelectableVariants(); i++ )
	{
		if ( m_pReplay->m_abVariants[ i ] != TTRUE )
			continue;

		const TINT    eLocaleTitle = pMiniGame->GetVariant( i ).eLocaleTitle;
		const TWCHAR* wszText      = ( eLocaleTitle == -1 ) ? TNULL : g_pLocaleManager->GetString( eLocaleTitle );

		AGUI2Button& rButton = m_aButtons[ iNumButtons ];
		rButton.Create( pFont, pFont->GetTextWidth( wszText ), wszText, 3.0f );
		rButton.SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		rButton.SetEnabledColour( AGUI2STYLE_COLOR_PRIMARY );
		rButton.SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		rButton.SetFocusedScale( 1.0f );
		rButton.OnFocusLost();
		rButton.SetId( iNumButtons );

		AGUI2TextBox& rTextBox = rButton.GetTextBox();
		rTextBox.SetShadowEnabled( TTRUE );
		rTextBox.SetShadowColour( 0xa0000000 );
		rTextBox.SetShadowAlpha( ABYardMenuState::s_fShadowAlpha );
		rTextBox.SetShadowOffset( rTextBox.GetScale() * 5.0f, rTextBox.GetScale() * 5.0f );

		m_oMenu.AddMenuItem( rButton );

		// NOTE: retail has room for 10 variants here, the replays have up to 12
		m_aiVariants[ iNumButtons ] = i;
		iNumButtons++;
	}

	m_oMenu.SetFocusAt( m_aButtons[ 0 ] );
	m_oDialogTitle.SetText( g_pLocaleManager->GetString( 350 ) );

	BaseClass::OnInsertion();
}

// $Barnyard: FUNCTION 0043c940
void AReplayVariantSelectState::OnRemoval()
{
	delete[] m_aButtons;

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 0043c9c0
void AReplayVariantSelectState::OnActivate()
{
	m_iSelectedButton = 0;
	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 0043c9d0
void AReplayVariantSelectState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, a_pMenuItem );
	m_iSelectedButton    = pButton->GetId();

	ABYardMenuState::OnButtonActivated( pButton );
}

// $Barnyard: FUNCTION 0043cc70
void AReplayVariantSelectState::OnMenuClose()
{
	if ( m_iSelectedButton == -1 )
	{
		Remove();
		return;
	}

	AMiniGameManager::GetSingleton()->SetNextMinigameVariant( m_pReplay->m_strMiniGame, m_aiVariants[ m_iSelectedButton ] + 1 );
	AMiniGameManager::ms_bHasFade = TFALSE;
}
