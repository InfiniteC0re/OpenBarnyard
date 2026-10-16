//-----------------------------------------------------------------------------
// A state that allows selection of the number of players that can join into a multiplayer game
//-----------------------------------------------------------------------------
#include "pch.h"
#include "AFrontEndPlayersState.h"
#include "AFrontEndMiniGameState2.h"
#include "AGameStateController.h"
#include "Input/AInputHandler.h"
#include "Player/APlayerManager.h"
#include "MiniGame/AMiniGameManager.h"
#include "Helpers/AGameLoader.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2Style.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AFrontEndPlayersState );

TPSTRING8_DECLARE( DialogInputMap );

// MARK is this required ???
const TINT AFrontEndPlayersState::sa_iMenuItemLocaleIDs[ Item_NUMOF ] = {
	250, // ALocale2::Misc_2Players, "2 Players"
	251, // ALocale2::Misc_3Players, "3 Players"
	252, // ALocale2::Misc_4Players, "4 Players"
};

//! Default constructor
AFrontEndPlayersState::AFrontEndPlayersState()
    : m_iSelectedItem( 0 )
{
	SetInputMap( TPS8( DialogInputMap ) );

	// NOTE: the original calls this m_afRotations, it's deleted by ABYardMenuState
	m_pButtonRotations = new TFLOAT[ Item_NUMOF ];
	TVALIDPTR( m_pButtonRotations );

	m_pButtonRotations[ 0 ] = TMath::DegToRad( 2.0f );
	m_pButtonRotations[ 1 ] = TMath::DegToRad( -3.0f );
	m_pButtonRotations[ 2 ] = TMath::DegToRad( 2.0f );
}

//! Destructor.
// $BarnyardPS2: FUNCTION 00113740
AFrontEndPlayersState::~AFrontEndPlayersState()
{
}

// $BarnyardPS2: FUNCTION 001137d8
void AFrontEndPlayersState::OnInsertion()
{
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	TVALIDPTR( pFont );

	/*
	TINT iNumControllers = AInputHandler::GetSingleton()->GetNumControllers();
	*/
	//TINT iNumControllers = 4;

	for ( TINT i = 0; i < Item_NUMOF; ++i )
	{
		const TWCHAR* szString = g_pLocaleManager->GetString( sa_iMenuItemLocaleIDs[ i ] );
		m_aMenuItems[ i ].Create( pFont, pFont->GetTextWidth( szString ), szString, 10.0f );
		m_aMenuItems[ i ].SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );   // AGUI2ColoursBY::s_iWhite
		m_aMenuItems[ i ].SetEnabledColour( AGUI2STYLE_COLOR_PRIMARY );   // AGUI2ColoursBY::s_iYellow
		m_aMenuItems[ i ].SetDisabledColour( AGUI2STYLE_COLOR_DISABLED ); // AGUI2ColoursBY::s_iGrey
		m_aMenuItems[ i ].SetFocusedScale( 1.0f );
		m_aMenuItems[ i ].OnFocusLost();
		m_aMenuItems[ i ].SetId( i );
		m_aMenuItems[ i ].SetScaleOnFocus( TFALSE );

		m_aMenuItems[ i ].GetTransform().Rotate( m_pButtonRotations[ i ] );

		m_aMenuItems[ i ].SetImage( "Panel_01", 1.25f, 1.5f );

		AGUI2TextBox& rTextBox = m_aMenuItems[ i ].GetTextBox();
		rTextBox.SetShadowEnabled( TTRUE );
		rTextBox.SetShadowColour( 0xa0000000 );
		rTextBox.SetShadowAlpha( ABYardMenuState::s_fShadowAlpha );
		// NOTE: the offset is stored as it is (PS2), SetShadowOffset would multiply it by the scale again
		rTextBox.SetShadowOffsetUnscaled( rTextBox.GetScale() * 5.0f, rTextBox.GetScale() * 5.0f );

		m_oMenu.AddMenuItem( m_aMenuItems[ i ] );
	}

	m_oMenu.SetFocusAt( m_aMenuItems[ 0 ] );

	// Setup menu
	m_oDialogTitle.SetText( L"CHOOSE PLAYERS!" );

	// DISABLE BACKGROUND
	m_oDialog.Hide();

	// Disable selected item pulse
	m_bFocusedElementBloated = TFALSE;

	pFont = T2GUIFontManager::FindFont( "Rekord18" );
	TVALIDPTR( pFont );

	const TWCHAR* sMsg = L"Insufficient controllers detected for Barnyard Antics!";
	m_oControllersPrompt.Create( pFont, 0.0f );
	m_oControllersPrompt.SetTextAndFitWidth( sMsg );
	m_oControllersPrompt.Hide();
	m_oControllersPrompt.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oControllersPrompt.SetTranslation( 0.0f, -30.0f );

	AGUI2::GetRootElement()->AddChildTail( m_oControllersPrompt );

	BaseClass::OnInsertion();
}

// $BarnyardPS2: FUNCTION 00113a90
void AFrontEndPlayersState::OnRemoval()
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// The original never unlinks the prompt from the GUI root before the state is destroyed
	m_oControllersPrompt.RemoveSelf();

	for ( TINT i = 0; i < Item_NUMOF; ++i )
		m_aMenuItems[ i ].RemoveSelf();
#endif // BARNYARD_COMMUNITY_PATCH

	BaseClass::OnRemoval();
}

// $BarnyardPS2: FUNCTION 00113ab0
TBOOL AFrontEndPlayersState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
	{
		m_iSelectedItem = -1;
		m_eMenuState    = MENUSTATE_MENU_DISAPPEAR;
		return TTRUE;
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $BarnyardPS2: FUNCTION 00113af8
TBOOL AFrontEndPlayersState::OnUpdate( TFLOAT a_fDeltaTime )
{
	UpdateMenuButtons( a_fDeltaTime );

	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && m_bIgnoreInputs )
	{
		UpdateFocusedButton( a_fDeltaTime );
	}

	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE )
	{
		//TINT iNumControllers = AInputHandler::GetSingleton()->GetNumControllers();
		TINT iNumControllers = 4;

		TINT iDisabledSelectedItem = -1;
		for ( TINT i = 0; i < Item_NUMOF; ++i )
		{
			TINT iUserData = m_aMenuItems[ i ].GetId(); // Num players data for this item

			if ( iUserData <= iNumControllers - 2 )
			{
				// Item was previously disabled, it shouldn't be selected!
				if ( !m_aMenuItems[ i ].IsEnabled() )
				{
					m_aMenuItems[ i ].SetEnabled( TTRUE );
				}
			}
			else
			{
				if ( m_oMenu.GetFocusedMenuItem() == &m_aMenuItems[ i ] )
				{
					// Need to special case this one, where the currently selected item needs to be disabled
					iDisabledSelectedItem = i;
				}
				else if ( m_aMenuItems[ i ].IsEnabled() )
				{
					m_aMenuItems[ i ].SetEnabled( TFALSE );
				}
			}
		}

		// Selected item was disabled, find an enabled sibling that we can select
		if ( iDisabledSelectedItem != -1 )
		{
			AGUI2MenuItem* pItem = m_oMenu.GetFocusedMenuItem()->GetNextMenuItem();
			while ( !pItem->IsEnabled() &&
			        pItem != m_oMenu.GetFocusedMenuItem() )
			{
				pItem = pItem->GetNextMenuItem();
			}

			if ( pItem == m_oMenu.GetFocusedMenuItem() )
			{
				// Not enough controllers available for multiplayer
				m_iSelectedItem = -1;
				m_eMenuState    = MENUSTATE_MENU_DISAPPEAR;
				return TTRUE;
			}
			else
			{
				TINT iData = TSTATICCAST( AGUI2Button, pItem )->GetId();
				TINFO( "Update, Selecting item with data [%d]\n", iData );

				m_oMenu.SetFocusAt( *pItem );

				// *now* we can disable the offending item
				m_aMenuItems[ iDisabledSelectedItem ].SetEnabled( TFALSE );
			}
		}
	}

	// Update which
	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $BarnyardPS2: FUNCTION 00113cd8
void AFrontEndPlayersState::OnActivate()
{
	m_iSelectedItem = 0;

	TINT iNumControllers = AInputHandler::GetSingleton()->GetNumControllers();

	if ( iNumControllers < m_iSelectedItem + 2 )
	{
		m_oControllersPrompt.Show();
	}
	else
	{
		m_oControllersPrompt.Hide();
	}
	/*
	// Add the menu to the gui
	T2GUIElement *pScreenElem = T2GUI::GetInstance()->GetContext()->GetRootElement();
	TVALIDPTR(pScreenElem);
	pScreenElem->AddChildTail(&m_oDialog);
	*/

#ifndef TOSHI_SKU_WINDOWS
	// Unlock the inputhandler so that new controllers can be recognised
	AInputHandler::GetSingleton()->Lock( TFALSE );
#endif // !TOSHI_SKU_WINDOWS

	APlayerManager::GetSingleton()->GetMiniGamePlayerSet()->Reset();

	BaseClass::OnActivate();

	// Override initial state in main menu
	m_eMenuState = MENUSTATE_MENU_APPEAR;
}

// $BarnyardPS2: FUNCTION 00113d70
void AFrontEndPlayersState::OnDeactivate()
{
	/*
	m_oDialog.Remove();
	*/
	BaseClass::OnDeactivate();
}

// $BarnyardPS2: FUNCTION 00113d90
void AFrontEndPlayersState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	TVALIDPTR( a_pMenuItem );
	m_iSelectedItem = TSTATICCAST( AGUI2Button, a_pMenuItem )->GetId();

	TINT iNumControllers = AInputHandler::GetSingleton()->GetNumControllers();

	if ( m_iSelectedItem + 2 > iNumControllers )
	{
		// Disallow selection as insufficient controllers plugged in
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );
		return;
	}

	// Lock the inputhandler to the current control set
	// so that new controllers won't be recognised
#ifndef TOSHI_SKU_WINDOWS
	AInputHandler::GetSingleton()->Lock( TTRUE );
#endif // !TOSHI_SKU_WINDOWS

	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );

	// Override state here so I can animate a button press before leaving this state
	m_eMenuState = MENUSTATE_MENU_VISIBLE;
}

// $BarnyardPS2: FUNCTION 00113e10
void AFrontEndPlayersState::OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus )
{
	m_iSelectedItem = TSTATICCAST( AGUI2Button, &a_rNewFocus )->GetId();

	TINT iNumControllers = AInputHandler::GetSingleton()->GetNumControllers();

	if ( iNumControllers < m_iSelectedItem + 2 )
	{
		m_oControllersPrompt.Show();
	}
	else
	{
		m_oControllersPrompt.Hide();
	}
}

// $BarnyardPS2: FUNCTION 00113e80
void AFrontEndPlayersState::OnMenuClose()
{
	if ( m_iSelectedItem >= 0 )
	{
		// Add players to playerset from each controller
		APlayerSet* pMiniGameSet = APlayerManager::GetSingleton()->GetMiniGamePlayerSet();
		//for (TINT i = 0; i < AInputHandler::GetSingleton()->GetNumControllers(); ++i)
		for ( TINT i = 0; i < m_iSelectedItem + 2; ++i )
		{
			// Add first n controllers as human players
			TInputDeviceController* pController = AInputHandler::GetSingleton()->GetController( i );
			TVALIDPTR( pController );

			pMiniGameSet->AddHumanPlayer( pController, -1 );
		}

		pMiniGameSet->MakeTeamsFair();

		// Tell the gameloader we're in an antics session
		AGameLoader::s_bAntics = TTRUE;

		// Clear session results as we're starting a new antics session
		AMiniGameManager::GetSingleton()->ClearSessionResults();

		// No AI or variant control in multiplayer
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: the PS2 build passes FALSE (the default argument of the original source), which is the debug
		// mode of the minigame list: it resets the minigame player set on activation, dropping the players added above
		AFrontEndMiniGameState2* pState = new AFrontEndMiniGameState2( TTRUE );
#else
		AFrontEndMiniGameState2* pState = new AFrontEndMiniGameState2( TFALSE );
#endif
		TVALIDPTR( pState );

		AGameStateController::GetSingleton()->PushState( pState );
	}
	else
	{
		BaseClass::Remove();
	}
}
