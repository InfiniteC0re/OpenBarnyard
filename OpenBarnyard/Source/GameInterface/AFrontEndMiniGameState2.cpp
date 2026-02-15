#include "pch.h"
#include "AFrontEndMiniGameState2.h"
#include "MiniGame/AMiniGameManager.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "GUI/T2GUIFontManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Helpers/APlayerProgress.h"
#include "Player/APlayerManager.h"
#include "Input/AInputHandler.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AFrontEndMiniGameState2 );

// $Barnyard: FUNCTION 00409ce0
AFrontEndMiniGameState2::AFrontEndMiniGameState2( TBOOL a_bHideVariantSelector )
    : m_iSelectedMiniGame( -1 )
    , m_iPrevMiniGame( -1 )
    , m_iNextMiniGame( -1 )
    , m_iHidingMiniGame( -1 )
    , m_pRectangles( TNULL )
    , m_iNumMiniGames( 0 )
    , m_eSliderState( SLIDERSTATE_IDLE )
    , m_bIsRelease( a_bHideVariantSelector )
    , m_bMouseDirty( TFALSE )
    , m_bMiniGameHovered( TFALSE )
    , m_bLeftArrowHovered( TFALSE )
    , m_bRightArrowHovered( TFALSE )
{
}

// $Barnyard: FUNCTION 00409de0
// $Barnyard: FUNCTION 0040a070
AFrontEndMiniGameState2::~AFrontEndMiniGameState2()
{
}

// $Barnyard: FUNCTION 00409f40
TBOOL AFrontEndMiniGameState2::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return BaseClass::ProcessInput( a_pInputEvent );

	if ( a_pInputEvent->m_eEventType == TInputInterface::EVENT_TYPE_MOVED )
	{
		m_bMouseDirty = TTRUE;
		return TTRUE;
	}

	// Filter out all events except LMB and any key down
	if ( a_pInputEvent->m_eEventType != TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->m_iDoodad != TInputDeviceMouse::BUTTON_1 )
		return BaseClass::ProcessInput( a_pInputEvent );

	// Process click on minigame
	if ( m_bMiniGameHovered )
	{
		OnMenuItemActivated( NULL );
		return TTRUE;
	}

	// Process click on the left arrow
	if ( m_bLeftArrowHovered )
	{
		TBOOL bStopEvents;
		return ProcessCommand( AInputCommand_Left, a_pInputEvent, bStopEvents );
	}

	// Process click on the right arrow
	if ( m_bRightArrowHovered )
	{
		TBOOL bStopEvents;
		return ProcessCommand( AInputCommand_Right, a_pInputEvent, bStopEvents );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0040bc40
TBOOL AFrontEndMiniGameState2::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return ABYardMenuState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	// Go to the prev menu
	if ( a_eInputCommand == AInputCommand_Cancel )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::ui::UI_MENUBACK );

		m_eMenuState        = MENUSTATE_MENU_DISAPPEAR;
		m_iSelectedMiniGame = -1;
		return TTRUE;
	}

	if ( m_oMenu.IsFocused() )
	{
		if ( a_eInputCommand == AInputCommand_OK )
		{
			// Skip mouse events
			if ( a_pInputEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) ) ) return TFALSE;
		}

		if ( a_pInputEvent->m_eEventType == TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->m_eEventType == TInputInterface::EVENT_TYPE_REPEAT )
		{
			switch ( a_eInputCommand )
			{
				case AInputCommand_Left:
				{
					ASoundManager::GetSingleton()->PlayCue( soundbank::ui::UI_MENUCLICK );
					m_aArrows[ 0 ].GetTransform().SetScale( 1.25f, 1.25f );

					if ( m_eSliderState == SLIDERSTATE_SWITCHING_TO_NEXT )
					{
						const TINT iWasHiding = m_iHidingMiniGame;

						m_iHidingMiniGame   = m_iNextMiniGame;
						m_iNextMiniGame     = m_iSelectedMiniGame;
						m_iSelectedMiniGame = m_iPrevMiniGame;
						m_iPrevMiniGame     = iWasHiding;

						UpdateText();
						m_eSliderState = SLIDERSTATE_SWITCHING_TO_PREV;
					}
					else
					{
						m_eSliderState = SLIDERSTATE_SWITCH_TO_PREV;
					}

					a_pInputEvent->StartRepeat( 0.5f, 0.2f );
					return TTRUE;
				}
				case AInputCommand_Right:
				{
					ASoundManager::GetSingleton()->PlayCue( soundbank::ui::UI_MENUCLICK );
					m_aArrows[ 1 ].GetTransform().SetScale( 1.25f, 1.25f );

					if ( m_eSliderState == SLIDERSTATE_SWITCHING_TO_PREV )
					{
						const TINT iWasHiding   = m_iHidingMiniGame;
						const TINT iWasSelected = m_iSelectedMiniGame;

						m_iHidingMiniGame   = m_iPrevMiniGame;
						m_iSelectedMiniGame = m_iNextMiniGame;
						m_iPrevMiniGame     = iWasSelected;
						m_iNextMiniGame     = iWasHiding;

						UpdateText();
						m_eSliderState = SLIDERSTATE_SWITCHING_TO_NEXT;
					}
					else
					{
						m_eSliderState = SLIDERSTATE_SWITCH_TO_NEXT;
					}

					a_pInputEvent->StartRepeat( 0.5f, 0.2f );
					return TTRUE;
				}
			}
		}
	}

	return ABYardMenuState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0040a270
void AFrontEndMiniGameState2::OnInsertion()
{
	m_iSelectedMiniGame = 0;

	m_iNumMiniGames = ( m_bIsRelease ) ?
	    g_pMiniGameManager->GetNumVisibleMiniGames() :
	    g_pMiniGameManager->GetTotalNumMiniGames();

	m_pRectangles = new T2GUIRectangle[ m_iNumMiniGames ];

	TFLOAT fMaxIconWidth  = 0.0f;
	TFLOAT fMaxIconHeight = 0.0f;

	for ( TINT i = 0; i < m_iNumMiniGames; i++ )
	{
		// Get correct icon for this minigame
		const TCHAR* szIconName = m_bIsRelease ?
		    g_pMiniGameManager->GetVisibleMiniGame( i )->m_szIconName :
		    g_pMiniGameManager->GetMiniGame( i )->m_szIconName;

		T2GUITextureSection* pIcon = T2GUITextureSectionManager::GetTextureSection( szIconName );
		TVALIDPTR( pIcon );

		TFLOAT fIconWidth  = pIcon->GetWidth();
		TFLOAT fIconHeight = pIcon->GetHeight();

		// Update max size of an icon
		if ( fMaxIconWidth < fIconWidth )
			fMaxIconWidth = fIconWidth;

		if ( fMaxIconHeight < fIconHeight )
			fMaxIconHeight = fIconHeight;

		// Set correct settings for the icon
		m_pRectangles[ i ].SetDimensions( fIconWidth, fIconHeight );
		m_pRectangles[ i ].SetTextureSection( pIcon );

		m_oRoot.AddChildTail( m_pRectangles[ i ] );

		m_iPrevMiniGame = m_iSelectedMiniGame - 1;
		if ( m_iPrevMiniGame < 0 )
			m_iPrevMiniGame = m_iNumMiniGames - 1;

		m_iNextMiniGame = m_iSelectedMiniGame + 1;
		if ( m_iNextMiniGame >= m_iNumMiniGames )
			m_iNextMiniGame = 0;

		if ( i == m_iSelectedMiniGame )
		{
			// Setting up selected minigame element
			m_pRectangles[ i ].GetTransform().Scale( 1.1f, 1.1f );
			m_pRectangles[ i ].SetColour( COLOR_WHITE );

			m_pRectangles[ i ].Show();
		}
		else if ( i == m_iPrevMiniGame )
		{
			// Setting up minigame element which is previous to selected
			m_pRectangles[ i ].SetTransform( -100.0f, 0.0f, 0.0f );

			T2GUITransform scaleTransform;
			scaleTransform.SetScale( 0.5f, 0.5f );

			m_pRectangles[ i ].GetTransform().PreMultiply( scaleTransform );
			m_pRectangles[ i ].Show();
			m_pRectangles[ i ].SetColour( COLOR_BLACK );

			m_pRectangles[ i ].Show();
		}
		else if ( i == m_iNextMiniGame )
		{
			// Setting up minigame element which is next to selected
			m_pRectangles[ i ].SetTransform( 100.0f, 0.0f, 0.0f );

			T2GUITransform scaleTransform;
			scaleTransform.SetScale( 0.5f, 0.5f );

			m_pRectangles[ i ].GetTransform().PreMultiply( scaleTransform );
			m_pRectangles[ i ].Show();
			m_pRectangles[ i ].SetColour( COLOR_BLACK );
		}
		else
		{
			m_pRectangles[ i ].Hide();
		}
	}

	// Setup arrows
	T2GUITextureSection* pArrowLeft = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	m_aArrows[ 0 ].SetDimensions( pArrowLeft->GetWidth(), pArrowLeft->GetHeight() );
	m_aArrows[ 0 ].SetTextureSection( pArrowLeft );
	m_aArrows[ 0 ].SetTransform( -150.0f, 0.0f, 0.0f );

	T2GUITextureSection* pArrowRight = T2GUITextureSectionManager::GetTextureSection( "left_arrow2" );
	m_aArrows[ 1 ].SetDimensions( pArrowRight->GetWidth(), pArrowRight->GetHeight() );
	m_aArrows[ 1 ].SetTextureSection( pArrowRight );
	m_aArrows[ 1 ].SetTransform( 150.0f, 0.0f, 0.0f );

	T2_FOREACH_ARRAY_BACK( m_aArrows, it )
	{
		m_oRoot.AddChildTail( m_aArrows[ it ] );
	}

	m_oMenu.AddMenuItem( m_oRoot );

	// Setup title element
	m_oMiniGameTitle.Create( T2GUIFontManager::FindFont( "Rekord26" ), 400.0f );
	m_oMiniGameTitle.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oMiniGameTitle.SetColour( COLOR_WHITE );
	m_oMiniGameTitle.SetShadowEnabled( TTRUE );
	m_oMiniGameTitle.SetShadowColour( TCOLOR4( 0, 0, 0, 160 ) );
	m_oMiniGameTitle.SetAlpha( 0.0f );
	m_oMiniGameTitle.SetShadowAlpha( 0.0f );
	m_oMiniGameTitle.SetShadowOffset( m_oDialogTitle.GetScale() * 5.0f, m_oDialogTitle.GetScale() * 5.0f );
	m_oRoot.AddChildTail( m_oMiniGameTitle );

	// Setup description element
	m_oMiniGameDescription.Create( T2GUIFontManager::FindFont( "Rekord18" ), 400.0f );
	m_oMiniGameDescription.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oMiniGameDescription.SetColour( AGUI2STYLE_COLOR_PRIMARY );
	m_oMiniGameDescription.SetShadowEnabled( TTRUE );
	m_oMiniGameDescription.SetShadowColour( TCOLOR4( 0, 0, 0, 160 ) );
	m_oMiniGameDescription.SetAlpha( 0.0f );
	m_oMiniGameDescription.SetShadowAlpha( 0.0f );
	m_oMiniGameDescription.SetShadowOffset( m_oDialogTitle.GetScale() * 5.0f, m_oDialogTitle.GetScale() * 5.0f );
	m_oRoot.AddChildTail( m_oMiniGameDescription );

	// Update text about current minigame
	UpdateText();

	m_oRoot.SetDimensions( fMaxIconWidth * 3.75f, m_oMiniGameTitle.GetHeight() + m_oMiniGameDescription.GetHeight() + fMaxIconHeight + 64.0f );
	m_oMenu.SetFocus( TTRUE );
	m_oMenu.SetFocusAt( m_oRoot );

	m_oDialogTitle.SetText( g_pLocaleManager->GetString( 237 ) );
	m_bFocusedElementBloated = TFALSE;

	BaseClass::OnInsertion();
}

// $Barnyard: FUNCTION 00409e80
void AFrontEndMiniGameState2::OnRemoval()
{
	for ( TINT i = 0; i < m_iNumMiniGames; i++ )
		m_pRectangles[ i ].RemoveSelf();

	m_oMiniGameTitle.RemoveSelf();
	m_oMiniGameDescription.RemoveSelf();
	m_oRoot.RemoveSelf();
	m_aArrows[ 0 ].RemoveSelf();
	m_aArrows[ 1 ].RemoveSelf();

	if ( m_pRectangles )
		delete[] m_pRectangles;

#ifdef BARNYARD_COMMUNITY_PATCH
	m_pRectangles = TNULL;
#endif // BARNYARD_COMMUNITY_PATCH

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 0040aa00
TBOOL AFrontEndMiniGameState2::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Update mouse
	if ( m_eMenuState == ABYardMenuState::MENUSTATE_MENU_VISIBLE && m_bMouseDirty )
	{
		T2GUITransform oTransform;
		TVector2       vecCursorPos = g_pGUI2->GetMouseCursor().GetCursorPos();
		TVector2       vecTransformed;

		// Is minigame hovered?
		m_pRectangles[ m_iSelectedMiniGame ].GetInvScreenTransform( oTransform );
		oTransform.Transform( vecTransformed, vecCursorPos );
		m_bMiniGameHovered = m_pRectangles[ m_iSelectedMiniGame ].IsPointInside( vecTransformed.x, vecTransformed.y );

		if ( !m_bMiniGameHovered )
		{
			m_aArrows[ 0 ].GetInvScreenTransform( oTransform );
			oTransform.Transform( vecTransformed, vecCursorPos );
			m_bLeftArrowHovered = m_aArrows[ 0 ].IsPointInside( vecTransformed.x, vecTransformed.y );
		}

		if ( !m_bMiniGameHovered && !m_bLeftArrowHovered )
		{
			m_aArrows[ 1 ].GetInvScreenTransform( oTransform );
			oTransform.Transform( vecTransformed, vecCursorPos );
			m_bRightArrowHovered = m_aArrows[ 1 ].IsPointInside( vecTransformed.x, vecTransformed.y );
		}

		m_bMouseDirty = TFALSE;
	}

	if ( m_eMenuState == ABYardMenuState::MENUSTATE_MENU_APPEAR || m_eMenuState == ABYardMenuState::MENUSTATE_MENU_DISAPPEAR )
	{
		m_oMiniGameTitle.SetAlpha( m_fMenuOpacity );
		m_oMiniGameTitle.SetShadowAlpha( m_fMenuOpacity * 0.6f );
		m_oMiniGameDescription.SetAlpha( m_fMenuOpacity );
		m_oMiniGameDescription.SetShadowAlpha( m_fMenuOpacity * 0.6f );
	}
	else if ( m_eMenuState == ABYardMenuState::MENUSTATE_DIALOG_APPEAR || m_eMenuState == ABYardMenuState::MENUSTATE_DIALOG_DISAPPEAR )
	{
		m_oMiniGameTitle.SetAlpha( 0.0f );
		m_oMiniGameTitle.SetShadowAlpha( 0.0f );
		m_oMiniGameDescription.SetAlpha( 0.0f );
		m_oMiniGameDescription.SetShadowAlpha( 0.0f );
	}

	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return BaseClass::OnUpdate( a_fDeltaTime );

	// Update time
	m_fTime += a_fDeltaTime;

	// Animate arrows on hover
	const TFLOAT fLerpSpeed       = a_fDeltaTime * 10.0f;
	TFLOAT       fRightArrowScale = 1.0f;
	TFLOAT       fLeftArrowScale  = 1.0f;

	if ( m_bRightArrowHovered )
		fRightArrowScale = 1.25f;

	if ( m_bLeftArrowHovered )
		fLeftArrowScale = 1.25f;

	m_aArrows[ 1 ].GetTransform().SetScale(
	    TMath::LERPClamped( m_aArrows[ 1 ].GetTransform().GetScaleX(), fRightArrowScale, fLerpSpeed ),
	    TMath::LERPClamped( m_aArrows[ 1 ].GetTransform().GetScaleX(), fRightArrowScale, fLerpSpeed )
	);

	m_aArrows[ 0 ].GetTransform().SetScale(
	    TMath::LERPClamped( m_aArrows[ 0 ].GetTransform().GetScaleX(), fLeftArrowScale, fLerpSpeed ),
	    TMath::LERPClamped( m_aArrows[ 0 ].GetTransform().GetScaleX(), fLeftArrowScale, fLerpSpeed )
	);

	// Animate minigame
	if ( m_oMenu.IsFocused() && m_eSliderState == 0 )
	{
		TFLOAT fScale = TMath::Sin( TMath::TWO_PI * m_fTotalTime ) * 0.05f + 1.1f;
		m_pRectangles[ m_iSelectedMiniGame ].GetTransform().SetScale( fScale, fScale );
	}

	switch ( m_eSliderState )
	{
		case SLIDERSTATE_SWITCH_TO_NEXT:
		{
			TINT iPrevHiding = m_iHidingMiniGame;

			m_iHidingMiniGame   = m_iPrevMiniGame;
			m_iPrevMiniGame     = m_iSelectedMiniGame;
			m_iSelectedMiniGame = m_iNextMiniGame;

			m_iNextMiniGame += 1;
			if ( m_iNextMiniGame >= m_iNumMiniGames )
				m_iNextMiniGame = 0;

			if ( iPrevHiding != m_iNextMiniGame )
			{
				m_pRectangles[ m_iNextMiniGame ].SetColour( 0 );
				m_pRectangles[ m_iNextMiniGame ].SetTransform( 100.0f * 2.0f, 0.0f );
				m_pRectangles[ m_iNextMiniGame ].GetTransform().Scale( 0.5f, 0.5f );
				m_pRectangles[ m_iNextMiniGame ].Show();

				if ( iPrevHiding != -1 )
				{
					m_pRectangles[ iPrevHiding ].SetColour( 0 );
					m_pRectangles[ iPrevHiding ].SetTransform( 0.0f, 0.0f );
					m_pRectangles[ iPrevHiding ].GetTransform().Scale( 0.5f, 0.5f );
					m_pRectangles[ iPrevHiding ].Hide();
				}
			}

			m_pRectangles[ m_iSelectedMiniGame ].SetAlpha( 1.0f );
			m_eSliderState = SLIDERSTATE_SWITCHING_TO_NEXT;
			UpdateText();

			break;
		}
		case SLIDERSTATE_SWITCH_TO_PREV:
		{
			TINT iPrevHiding = m_iHidingMiniGame;

			m_iHidingMiniGame   = m_iNextMiniGame;
			m_iNextMiniGame     = m_iSelectedMiniGame;
			m_iSelectedMiniGame = m_iPrevMiniGame;

			m_iPrevMiniGame -= 1;
			if ( m_iPrevMiniGame < 0 )
				m_iPrevMiniGame = m_iNumMiniGames - 1;

			if ( iPrevHiding != m_iPrevMiniGame )
			{
				m_pRectangles[ m_iPrevMiniGame ].SetColour( 0 );
				m_pRectangles[ m_iPrevMiniGame ].SetTransform( 100.0f * -2.0f, 0.0f );
				m_pRectangles[ m_iPrevMiniGame ].GetTransform().Scale( 0.5f, 0.5f );
				m_pRectangles[ m_iPrevMiniGame ].Show();

				if ( iPrevHiding != -1 )
				{
					m_pRectangles[ iPrevHiding ].SetColour( 0 );
					m_pRectangles[ iPrevHiding ].SetTransform( 0.0f, 0.0f );
					m_pRectangles[ iPrevHiding ].GetTransform().Scale( 0.5f, 0.5f );
					m_pRectangles[ iPrevHiding ].Hide();
				}
			}

			m_pRectangles[ m_iSelectedMiniGame ].SetAlpha( 1.0f );
			m_eSliderState = SLIDERSTATE_SWITCHING_TO_PREV;
			UpdateText();

			break;
		}
		// [1/19/2026 InfiniteC0re]
		// NOTE: this is depressing, but the original code had two duplicates of the code below for both SLIDERSTATE_SWITCHING_TO_PREV
		// and SLIDERSTATE_SWITCHING_TO_NEXT just like switching and the only different was REALLY A SINGLE LINE OF CODE. Who was up to make it????
		// I combined them, because it makes much more sense...
		case SLIDERSTATE_SWITCHING_TO_PREV:
		case SLIDERSTATE_SWITCHING_TO_NEXT:
		{
			TASSERT( m_iHidingMiniGame != -1 );
			auto& rPrev = m_pRectangles[ m_iPrevMiniGame ];
			auto& rNext = m_pRectangles[ m_iNextMiniGame ];
			auto& rCurr = m_pRectangles[ m_iSelectedMiniGame ];
			auto& rHide = m_pRectangles[ m_iHidingMiniGame ];

			auto& rPrevTransform = rPrev.GetTransform();
			auto& rNextTransform = rNext.GetTransform();
			auto& rCurrTransform = rCurr.GetTransform();
			auto& rHideTransform = rHide.GetTransform();

			const TFLOAT flCurrX = rCurr.GetTransform().GetTranslation().x;
			const TFLOAT flCurrY = rCurr.GetTransform().GetTranslation().y;

			const TBOOL bAnimationOver = TMath::Abs( flCurrX ) <= 0.1f;
			if ( bAnimationOver )
			{
				rPrevTransform.SetIdentity();
				rPrevTransform.SetTranslation( -100.0f, flCurrY );
				rPrevTransform.Scale( 0.5f, 0.5f );
				rPrev.SetColour( COLOR_BLACK );

				rNextTransform.SetIdentity();
				rNextTransform.SetTranslation( 100.0f, flCurrY );
				rNextTransform.Scale( 0.5f, 0.5f );
				rNext.SetColour( COLOR_BLACK );

				rCurrTransform.SetIdentity();
				rCurrTransform.SetTranslation( 0.0f, flCurrY );
				rCurrTransform.Scale( 1.1f, 1.1f );
				rCurr.SetColour( IsCurrentMiniGameAvailable() ? COLOR_WHITE : COLOR_BLACK );

				// Completely hide item that was previously hiding
				if ( m_iHidingMiniGame != -1 )
				{
					m_pRectangles[ m_iHidingMiniGame ].SetColour( TCOLOR4( 0, 0, 0, 0 ) );
					m_pRectangles[ m_iHidingMiniGame ].Hide();
					m_iHidingMiniGame = -1;
				}

				m_fTotalTime   = 0.0f;
				m_eSliderState = SLIDERSTATE_IDLE;
			}
			else
			{
				// Store current scale values
				const TFLOAT flCurrScale = rCurrTransform.GetScaleX();
				const TFLOAT flNextScale = rNextTransform.GetScaleX();
				const TFLOAT flPrevScale = rPrevTransform.GetScaleX();

				// Update previous element
				const TFLOAT flPrevPosX = rPrevTransform.GetTranslation().x;
				const TFLOAT flPrevPosY = rPrevTransform.GetTranslation().y;
				rPrevTransform.SetIdentity();
				rPrevTransform.SetTranslation( TMath::LERPClamped( flPrevPosX, -100.0f, 5.0f * a_fDeltaTime ), flPrevPosY );

				// Update current element
				const TFLOAT flCurrPosX = rCurrTransform.GetTranslation().x;
				const TFLOAT flCurrPosY = rCurrTransform.GetTranslation().y;
				rCurrTransform.SetIdentity();
				rCurrTransform.SetTranslation( TMath::LERPClamped( flCurrPosX, 0.0f, 5.0f * a_fDeltaTime ), flCurrPosY );

				// Update next element
				const TFLOAT flNextPosX = rNextTransform.GetTranslation().x;
				const TFLOAT flNextPosY = rNextTransform.GetTranslation().y;
				rNextTransform.SetIdentity();
				rNextTransform.SetTranslation( TMath::LERPClamped( flNextPosX, 100.0f, 5.0f * a_fDeltaTime ), flNextPosY );

				// Update hiding element
				const TFLOAT flHidePosX = rHideTransform.GetTranslation().x;
				const TFLOAT flHidePosY = rHideTransform.GetTranslation().y;
				rHideTransform.SetIdentity();

				// Below is the line that was the reason someonde decided to copy this whole case for SLIDERSTATE_SWITCHING_TO_PREV
				rHideTransform.SetTranslation( TMath::LERPClamped( flHidePosX, 100.0f * ( m_eSliderState == SLIDERSTATE_SWITCHING_TO_NEXT ? -1.0f : 1.0f ) * 2.0f, 5.0f * a_fDeltaTime ), flHidePosY );

				// Update scales
				const TFLOAT flPrevScaleNew = TMath::LERPClamped( flPrevScale, 0.5f, 5.0f * a_fDeltaTime );
				const TFLOAT flCurrScaleNew = TMath::LERPClamped( flCurrScale, 1.1f, 5.0f * a_fDeltaTime );
				const TFLOAT flNextScaleNew = TMath::LERPClamped( flNextScale, 0.5f, 5.0f * a_fDeltaTime );
				rPrevTransform.Scale( flPrevScaleNew, flPrevScaleNew );
				rCurrTransform.Scale( flCurrScaleNew, flCurrScaleNew );
				rNextTransform.Scale( flNextScaleNew, flNextScaleNew );
				rHideTransform.Scale( 0.5f, 0.5f );

				// Interpolate color for the prev element
				TUINT aColor[ 4 ];
				TColor::GetComponents( rPrev.GetColour(), aColor[ 0 ], aColor[ 1 ], aColor[ 2 ], aColor[ 3 ] );
				for ( TINT i = 1; i < 4; i++ ) // skip alpha
					aColor[ i ] = TUINT( TMath::LERPClamped( TFLOAT( aColor[ i ] ), 0.0f, 5.0f * a_fDeltaTime ) );

				// Update alpha and set the color
				aColor[ 0 ] = TUINT( TMath::LERPClamped( TFLOAT( aColor[ 0 ] ), 255.0f, 5.0f * a_fDeltaTime ) );
				rPrev.SetColour( TCOLOR4( aColor[ 1 ], aColor[ 2 ], aColor[ 3 ], aColor[ 0 ] ) );

				// Interpolate color for the next element
				TColor::GetComponents( rNext.GetColour(), aColor[ 0 ], aColor[ 1 ], aColor[ 2 ], aColor[ 3 ] );
				for ( TINT i = 1; i < 4; i++ ) // skip alpha
					aColor[ i ] = TUINT( TMath::LERPClamped( TFLOAT( aColor[ i ] ), 0.0f, 5.0f * a_fDeltaTime ) );

				// Update alpha and set the color
				aColor[ 0 ] = TUINT( TMath::LERPClamped( TFLOAT( aColor[ 0 ] ), 255.0f, 5.0f * a_fDeltaTime ) );
				rNext.SetColour( TCOLOR4( aColor[ 1 ], aColor[ 2 ], aColor[ 3 ], aColor[ 0 ] ) );

				// Interpolate color for the current element
				if ( IsCurrentMiniGameAvailable() )
				{
					TColor::GetComponents( rCurr.GetColour(), aColor[ 0 ], aColor[ 1 ], aColor[ 2 ], aColor[ 3 ] );
					for ( TINT i = 0; i < 4; i++ ) // don't skip alpha this time
						aColor[ i ] = TUINT( TMath::LERPClamped( TFLOAT( aColor[ i ] ), 255.0f, 5.0f * a_fDeltaTime ) );

					rCurr.SetColour( TCOLOR4( aColor[ 1 ], aColor[ 2 ], aColor[ 3 ], aColor[ 0 ] ) );
				}

				// Update hiding element
				rHide.SetAlpha( TMath::LERPClamped( TFLOAT( TCOLOR_GET_ALPHA( rHide.GetColour() ) / 255.0f ), 0.0f, 5.0f * a_fDeltaTime ) );
			}
			break;
		}
	}

	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0040a0f0
void AFrontEndMiniGameState2::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	if ( m_bIsRelease && !g_pPlayerProgress->IsMiniGameAvailable( m_iSelectedMiniGame ) )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );
		return;
	}

	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );
}

// $Barnyard: FUNCTION 0040be80
void AFrontEndMiniGameState2::OnMenuClose()
{
	// Immediately hide the hiding minigame
	if ( m_iHidingMiniGame != -1 )
	{
		m_pRectangles[ m_iHidingMiniGame ].SetAlpha( 0.0f );
		m_pRectangles[ m_iHidingMiniGame ].Hide();
		m_iHidingMiniGame = -1;
	}

	if ( m_iSelectedMiniGame == -1 )
	{
		BaseClass::Remove();
		return;
	}

	// Fill player set
	APlayerManager* pPlyrMgr = APlayerManager::GetSingleton();
	APlayerSet*     pPlyrSet = pPlyrMgr->GetPlayerSet( 1 );

	pPlyrSet->AddHumanPlayer( AInputHandler::GetSingleton()->GetMainController() );

	const TINT      iMinNumPlayers  = AMiniGameManager::GetSingleton()->GetMiniGame( m_iSelectedMiniGame )->m_iMinNumPlayers;
	const TPString8 strMiniGameName = AMiniGameManager::GetSingleton()->GetMiniGame( m_iSelectedMiniGame )->m_strMiniGameName;

	// If mud jumpers, fill with AI players
	if ( strMiniGameName == TPS8D( "AGateCrashMiniGameState" ) )
	{
		for ( TINT i = iMinNumPlayers - pPlyrSet->GetNumHumanPlayers(); i > 0; i-- )
			pPlyrSet->AddAIPlayer();
	}

	// Balance teams
	pPlyrSet->MakeTeamsFair();
	AMiniGameManager::GetSingleton()->MakeValidMiniGamePlayerSet( m_iSelectedMiniGame );
	pPlyrSet->FinishUpAddingAIs();

	TINT iMiniGameId            = ( !m_bIsRelease ) ? m_iSelectedMiniGame : AMiniGameManager::GetSingleton()->GetHiddenMiniGameIndex( m_iSelectedMiniGame );
	TINT iNumSelectableVariants = ( !m_bIsRelease ) ? AMiniGameManager::GetSingleton()->GetMiniGame( m_iSelectedMiniGame )->m_iNumSelectableVariants : 0;

	if ( TTRUE || iNumSelectableVariants <= 1 )
	{
		// Hide UI elements
		m_oDialogTitleBackground.Hide();
		m_oDialog.Hide();

		TTODO( "Set weather, game time and AGameStateController flags" );

		AMiniGameManager::GetSingleton()->SetMiniGameVariant(
		    m_bIsRelease ? AMiniGameManager::GetSingleton()->GetMiniGame( iMiniGameId )->m_iDefaultVariant : 0
		);

		AGameStateController::StartMiniGame( iMiniGameId, TFALSE );
	}
	else
	{
		TASSERT( TFALSE && "AFrontEndVariantSelectState" );
	}
}

// $Barnyard: FUNCTION 0040a150
void AFrontEndMiniGameState2::UpdateText()
{
	const TWCHAR* wcsTitle;
	const TWCHAR* wcsDescription;

	if ( !m_bIsRelease )
	{
		wcsTitle       = g_pLocaleManager->GetString( g_pMiniGameManager->GetMiniGame( m_iSelectedMiniGame )->m_eLocaleTitle );
		wcsDescription = g_pLocaleManager->GetString( g_pMiniGameManager->GetMiniGame( m_iSelectedMiniGame )->m_eLocaleDescription );
	}
	else if ( !g_pPlayerProgress->IsMiniGameAvailable( m_iSelectedMiniGame ) )
	{
		wcsTitle       = g_pLocaleManager->GetString( 238 );
		wcsDescription = g_pLocaleManager->GetString( 239 );
	}
	else
	{
		wcsTitle       = g_pLocaleManager->GetString( g_pMiniGameManager->GetVisibleMiniGame( m_iSelectedMiniGame )->m_eLocaleTitle );
		wcsDescription = g_pLocaleManager->GetString( g_pMiniGameManager->GetVisibleMiniGame( m_iSelectedMiniGame )->m_eLocaleDescription );
	}

	m_oMiniGameTitle.SetText( wcsTitle );
	m_oMiniGameDescription.SetText( wcsDescription );
}

// $Barnyard: FUNCTION 0040a090
TBOOL AFrontEndMiniGameState2::IsCurrentMiniGameAvailable() const
{
	TASSERT( m_iSelectedMiniGame >= 0 && m_iSelectedMiniGame < 32 );

	return !m_bIsRelease || g_pPlayerProgress->IsMiniGameAvailable( m_iSelectedMiniGame % 32 );
}
