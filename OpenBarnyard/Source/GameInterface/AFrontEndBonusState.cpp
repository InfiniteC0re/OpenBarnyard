#include "pch.h"
#include "AFrontEndBonusState.h"
#include "AFrontEndArtGalleryState.h"
#include "ACreditsState.h"
#include "AMovieState.h"
#include "AGameStateController.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "SoundBank/ui.h"
#include "SoundBank/music.h"
#include "GUI/AGUI2Style.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass of the TClass
// $Barnyard: FUNCTION 00407760
TDEFINE_CLASS_NORUNTIME( AFrontEndBonusState );

// $Barnyard: FUNCTION 00407770
AFrontEndBonusState::AFrontEndBonusState( MODE a_eMode )
{
	m_eMode              = a_eMode;
	m_iActivatedButtonID = -1;
}

// $Barnyard: FUNCTION 004077f0
// $Barnyard: FUNCTION 00407a50
AFrontEndBonusState::~AFrontEndBonusState()
{
}

// NOTE: folded with AOptionsState::ProcessCommand (00407aa0) in the original
TBOOL AFrontEndBonusState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_eInputCommand == AInputCommand_Cancel )
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00407880
void AFrontEndBonusState::OnInsertion()
{
	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	const TINT  iNumButtons = GetNumButtons();
	const TINT* pTexts      = ( m_eMode != MODE_BONUSES ) ? ms_aTrailerTexts : ms_aBonusTexts;

	for ( TINT i = 0; i < iNumButtons; i++ )
	{
		const TWCHAR* wszText = g_pLocaleManager->GetString( pTexts[ i ] );

		m_aButtons[ i ].Create( pFont, pFont->GetTextWidth( g_pLocaleManager->GetString( pTexts[ i ] ) ), wszText, 0.0f );
		m_aButtons[ i ].SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		m_aButtons[ i ].SetEnabledColour( AGUI2STYLE_COLOR_SECONDARY );
		m_aButtons[ i ].SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		m_aButtons[ i ].SetFocusedScale( 1.0f );
		m_aButtons[ i ].OnFocusLost();
		m_aButtons[ i ].SetId( i );
		m_aButtons[ i ].SetTextShadow( TTRUE, 0xa0000000, 0.6f );
		m_aButtons[ i ].SetTextShadowOffset( m_aButtons[ i ].GetTextBox().GetScale() * 5.0f );
		m_oMenu.AddMenuItem( m_aButtons[ i ] );
	}

	m_oMenu.SetFocusAt( m_aButtons[ 0 ] );
	m_oMenu.SetFocus( TFALSE );
	m_oMenu.SetItemSpacing( 5.0f );
	m_oDialogTitle.SetText( g_pLocaleManager->GetString( ( m_eMode != MODE_BONUSES ) ? 74 : 236 ) );

	BaseClass::OnInsertion();
}

// $Barnyard: FUNCTION 00407a00
void AFrontEndBonusState::OnRemoval()
{
	const TINT iNumButtons = GetNumButtons();

	for ( TINT i = 0; i < iNumButtons; i++ )
		m_aButtons[ i ].RemoveSelf();

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00407a70
void AFrontEndBonusState::OnActivate()
{
	m_iActivatedButtonID = -1;
	AMusicManager::GetSingleton()->PlayBackgroundMusic( soundbank::MUSIC_FRONTEND );

	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 00407860
void AFrontEndBonusState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	AGUI2Button* pButton = TSTATICCAST( AGUI2Button, a_pMenuItem );
	m_iActivatedButtonID = pButton->GetId();

	ABYardMenuState::OnButtonActivated( pButton );
}

// $Barnyard: FUNCTION 00407ae0
void AFrontEndBonusState::OnMenuClose()
{
	if ( m_eMode == MODE_BONUSES )
	{
		AGameState* pState;

		switch ( m_iActivatedButtonID )
		{
			case BONUSBUTTON_ARTGALLERY:
				pState = new AFrontEndArtGalleryState( AFrontEndArtGalleryState::MODE_ART );
				break;
			case BONUSBUTTON_MOVIEGALLERY:
				pState = new AFrontEndArtGalleryState( AFrontEndArtGalleryState::MODE_MOVIES );
				break;
			case BONUSBUTTON_CREDITS:
				pState = new ACreditsState( TFALSE, TFALSE );
				break;
			default:
				Remove();
				return;
		}

		AGameStateController::GetSingleton()->PushState( pState );
	}
	else if ( m_eMode == MODE_TRAILERS )
	{
		if ( TUINT( m_iActivatedButtonID ) > 1 )
		{
			Remove();
			return;
		}

		AMusicManager::GetSingleton()->StopBackgroundMusic();
		AGameStateController::GetSingleton()->PushState( new AMovieState( ms_aTrailerMovies[ m_iActivatedButtonID ], TFALSE, TNULL, TFALSE ) );
	}
}
