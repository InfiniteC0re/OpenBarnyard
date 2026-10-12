#include "pch.h"
#include "AFrontEndMiniGameState2.h"
#include "Helpers/AHighScoreManager.h"
#include "AFrontEndVariantSelectState.h"
#include "FX/AGameTimeFXManager.h"
#include "Helpers/AWeatherManager.h"
#include "Helpers/AGameTimeManager.h"
#include "Player/APlayerProfileManager.h"
#include "MiniGame/AMiniGameManager.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "GUI/T2GUIFontManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/AGameLoader.h"
#include "Player/APlayerManager.h"
#include "Input/AInputHandler.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00409cd0, CreateObject 00409fd0, CreateObjectInPlace 0040a030
TDEFINE_CLASS( AFrontEndMiniGameState2 );

static TFLOAT s_fXOffset       = 100.0f; // 0x00772648
static TFLOAT s_fSelectedScale = 1.1f;   // 0x0077264c
static TFLOAT s_fMinDiff       = 0.1f;   // 0x00772650

// $Barnyard: FUNCTION 00409ce0
AFrontEndMiniGameState2::AFrontEndMiniGameState2( TBOOL a_bAntics )
    : m_aIconRects( TNULL )
    , m_iNumRects( 0 )
    , m_iCurrSelection( -1 )
    , m_iLeftSelection( -1 )
    , m_iRightSelection( -1 )
    , m_iNewSelection( -1 )
    , m_eLocalState( LocalState_Ready )
    , m_bAntics( a_bAntics )
    , m_fAnalogTime( 0.0f )
    , m_bMouseMoved( TFALSE )
    , m_bMouseOverSelected( TFALSE )
    , m_bMouseOverLeftArrow( TFALSE )
    , m_bMouseOverRightArrow( TFALSE )
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
		// Mouse movement (gets handled in the Update) as we can get the mouse position from the AGUI2MouseCursor
		m_bMouseMoved = TTRUE;
		return TTRUE;
	}
	else if ( a_pInputEvent->m_eEventType == TInputInterface::EVENT_TYPE_GONE_DOWN && a_pInputEvent->m_iDoodad == TInputDeviceMouse::BUTTON_1 )
	{
		if ( m_bMouseOverSelected )
		{
			// Hack a menu selection
			OnMenuItemActivated( TNULL );
			return TTRUE;
		}
		else if ( m_bMouseOverLeftArrow )
		{
			// Hack a left processcommand
			TBOOL bExitProc;
			return ProcessCommand( AInputCommand_Left, a_pInputEvent, bExitProc );
		}
		else if ( m_bMouseOverRightArrow )
		{
			// Hack a right processcommand
			TBOOL bExitProc;
			return ProcessCommand( AInputCommand_Right, a_pInputEvent, bExitProc );
		}

		return TTRUE;
	}

	return BaseClass::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 0040bc40
TBOOL AFrontEndMiniGameState2::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( a_eInputCommand == AInputCommand_Cancel )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::ui::UI_MENUBACK );

		m_eMenuState     = MENUSTATE_MENU_DISAPPEAR;
		m_iCurrSelection = -1;
		return TTRUE;
	}

	if ( m_oMenu.IsFocused() )
	{
		if ( a_eInputCommand == AInputCommand_OK && a_pInputEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) ) )
		{
			// Swallow the OK so it gets handled in the ProcessInput
			return TFALSE;
		}

		if ( a_pInputEvent->m_eEventType == TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->m_eEventType == TInputInterface::EVENT_TYPE_REPEAT )
		{
			static const TFLOAT s_fRepeatDelay = 0.5f;

			if ( a_eInputCommand == AInputCommand_Left )
			{
				ASoundManager::GetSingleton()->PlayCue( soundbank::ui::UI_MENUCLICK );
				m_aArrows[ Arrow_Left ].GetTransform().SetScale( 1.25f, 1.25f );

				if ( m_eLocalState == LocalState_RotateLeft )
				{
					// Transition immediately from left rotation to right rotation
					TINT iTemp        = m_iNewSelection;
					m_iNewSelection   = m_iRightSelection; // The selection we fade out
					m_iRightSelection = m_iCurrSelection;
					m_iCurrSelection  = m_iLeftSelection;
					m_iLeftSelection  = iTemp;

					SetTexts();
					m_eLocalState = LocalState_RotateRight;
				}
				else
				{
					m_eLocalState = LocalState_InitRotateRight;
				}

				a_pInputEvent->StartRepeat( s_fRepeatDelay, ms_fAnalogTimeDelay );
				return TTRUE;
			}
			else if ( a_eInputCommand == AInputCommand_Right )
			{
				ASoundManager::GetSingleton()->PlayCue( soundbank::ui::UI_MENUCLICK );
				m_aArrows[ Arrow_Right ].GetTransform().SetScale( 1.25f, 1.25f );

				if ( m_eLocalState == LocalState_RotateRight )
				{
					// Transition immediately from right rotation to left rotation
					TINT iTemp        = m_iNewSelection;
					m_iNewSelection   = m_iLeftSelection; // The selection we fade out
					m_iLeftSelection  = m_iCurrSelection;
					m_iCurrSelection  = m_iRightSelection;
					m_iRightSelection = iTemp;

					SetTexts();
					m_eLocalState = LocalState_RotateLeft;
				}
				else
				{
					m_eLocalState = LocalState_InitRotateLeft;
				}

				a_pInputEvent->StartRepeat( s_fRepeatDelay, ms_fAnalogTimeDelay );
				return TTRUE;
			}
		}
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0040a270
void AFrontEndMiniGameState2::OnInsertion()
{
	m_iCurrSelection = 0;

	m_iNumRects = ( m_bAntics ) ?
	    AMiniGameManager::GetSingleton()->GetNumVisibleMiniGames() :
	    AMiniGameManager::GetSingleton()->GetTotalNumMiniGames();

	m_aIconRects = new T2GUIRectangle[ m_iNumRects ];

	TFLOAT fMaxW = 0.0f;
	TFLOAT fMaxH = 0.0f;

	for ( TINT i = 0; i < m_iNumRects; i++ )
	{
		const TCHAR* szIconName = m_bAntics ?
		    AMiniGameManager::GetSingleton()->GetVisibleMiniGame( i )->m_szIconName :
		    AMiniGameManager::GetSingleton()->GetMiniGame( i )->m_szIconName;

		T2GUITextureSection* pTex = T2GUITextureSectionManager::GetTextureSection( szIconName );
		TVALIDPTR( pTex );

		TFLOAT fTexW = pTex->GetWidth();
		TFLOAT fTexH = pTex->GetHeight();

		if ( fTexW > fMaxW )
			fMaxW = fTexW;

		if ( fTexH > fMaxH )
			fMaxH = fTexH;

		m_aIconRects[ i ].SetDimensions( fTexW, fTexH );
		m_aIconRects[ i ].SetTextureSection( pTex );

		m_oGamesMenuItem.AddChildTail( m_aIconRects[ i ] );

		// Only the current item and the two either side are visible
		m_iLeftSelection = m_iCurrSelection - 1;
		if ( m_iLeftSelection < 0 )
			m_iLeftSelection = m_iNumRects - 1;

		m_iRightSelection = m_iCurrSelection + 1;
		if ( m_iRightSelection >= m_iNumRects )
			m_iRightSelection = 0;

		if ( i == m_iCurrSelection || i == m_iLeftSelection || i == m_iRightSelection )
		{
			if ( i == m_iLeftSelection )
			{
				m_aIconRects[ i ].SetTransform( -s_fXOffset, 0.0f );

				T2GUITransform oScale;
				oScale.SetFromScale( 0.5f, 0.5f );
				m_aIconRects[ i ].GetTransform().PreMultiply( oScale );

				m_aIconRects[ i ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );
			}
			else if ( i == m_iRightSelection )
			{
				m_aIconRects[ i ].SetTransform( s_fXOffset, 0.0f );

				T2GUITransform oScale;
				oScale.SetFromScale( 0.5f, 0.5f );
				m_aIconRects[ i ].GetTransform().PreMultiply( oScale );

				m_aIconRects[ i ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );
			}
			else if ( i == m_iCurrSelection )
			{
				m_aIconRects[ i ].GetTransform().Scale( 1.1f, 1.1f );

				if ( CurIsAvailableToPlay() )
					m_aIconRects[ i ].SetColour( TCOLOR4( 255, 255, 255, 255 ) );
				else
					m_aIconRects[ i ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );
			}

			m_aIconRects[ i ].Show();
		}
		else
		{
			m_aIconRects[ i ].Hide();
		}
	}

	// Navigation arrows
	T2GUITextureSection* pTex  = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	TFLOAT               fTexW = pTex->GetWidth();
	TFLOAT               fTexH = pTex->GetHeight();
	m_aArrows[ Arrow_Left ].SetDimensions( fTexW, fTexH );
	m_aArrows[ Arrow_Left ].SetTextureSection( pTex );
	m_aArrows[ Arrow_Left ].SetTransform( -150.0f, 0.0f );

	// NOTE: the right arrow uses the dimensions of the left one
	pTex = T2GUITextureSectionManager::GetTextureSection( "left_arrow2" );
	m_aArrows[ Arrow_Right ].SetDimensions( fTexW, fTexH );
	m_aArrows[ Arrow_Right ].SetTextureSection( pTex );
	m_aArrows[ Arrow_Right ].SetTransform( 150.0f, 0.0f );

	for ( TINT i = 0; i < Arrow_NUMOF; i++ )
		m_oGamesMenuItem.AddChildTail( m_aArrows[ i ] );

	m_oMenu.AddMenuItem( m_oGamesMenuItem );

	// Setup minigame label
	m_oGameLabel.Create( T2GUIFontManager::FindFont( "Rekord26" ), 400.0f );
	m_oGameLabel.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oGameLabel.SetColour( TCOLOR4( 255, 255, 255, 255 ) );
	m_oGameLabel.SetShadowEnabled( TTRUE );
	m_oGameLabel.SetShadowColour( 0xa0000000 );
	m_oGameLabel.SetAlpha( 0.0f );
	m_oGameLabel.SetShadowAlpha( 0.0f );
	m_oGameLabel.SetShadowOffset( m_oDialogTitle.GetScale() * 5.0f, m_oDialogTitle.GetScale() * 5.0f );
	m_oGamesMenuItem.AddChildTail( m_oGameLabel );

	// Setup minigame desc
	m_oGameDesc.Create( T2GUIFontManager::FindFont( "Rekord18" ), 400.0f );
	m_oGameDesc.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oGameDesc.SetColour( TCOLOR4( 255, 200, 0, 255 ) );
	m_oGameDesc.SetShadowEnabled( TTRUE );
	m_oGameDesc.SetShadowColour( 0xa0000000 );
	m_oGameDesc.SetAlpha( 0.0f );
	m_oGameDesc.SetShadowAlpha( 0.0f );
	m_oGameDesc.SetShadowOffset( m_oDialogTitle.GetScale() * 5.0f, m_oDialogTitle.GetScale() * 5.0f );
	m_oGamesMenuItem.AddChildTail( m_oGameDesc );

	SetTexts();

	m_oGamesMenuItem.SetDimensions( fMaxW * 3.75f, m_oGameLabel.GetHeight() + m_oGameDesc.GetHeight() + fMaxH + 64.0f );

	// NOTE: the original (later multi-SKU) source has the debug menu with the AI count and the variant items here,
	// commented out

	m_oMenu.SetFocus( TTRUE );
	m_oMenu.SetFocusAt( m_oGamesMenuItem );

	m_oDialogTitle.SetText( g_pLocaleManager->GetString( 237 ) );
	m_bFocusedElementBloated = TFALSE;

	BaseClass::OnInsertion();
}

// $Barnyard: FUNCTION 00409e80
void AFrontEndMiniGameState2::OnRemoval()
{
	for ( TINT i = 0; i < m_iNumRects; i++ )
		m_aIconRects[ i ].RemoveSelf();

	m_oGameLabel.RemoveSelf();
	m_oGameDesc.RemoveSelf();

	m_oGamesMenuItem.RemoveSelf();

	m_aArrows[ Arrow_Left ].RemoveSelf();
	m_aArrows[ Arrow_Right ].RemoveSelf();

	delete[] m_aIconRects;

#ifdef BARNYARD_COMMUNITY_PATCH
	m_aIconRects = TNULL;
#endif // BARNYARD_COMMUNITY_PATCH

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 0040a850
void AFrontEndMiniGameState2::OnActivate()
{
	if ( !m_bAntics )
	{
		// Reset here if we're coming back from cancelling a variation selection
		// Debug Minigames
		APlayerManager::GetSingleton()->GetMiniGamePlayerSet()->Reset();
	}

	m_eLocalState = LocalState_Ready;
	m_oMenu.SetFocus( TTRUE );

	// New middle has arrived, snap to final pos
	if ( m_iLeftSelection != -1 )
	{
		T2GUITransform& rLeftTransform = m_aIconRects[ m_iLeftSelection ].GetTransform();
		const TFLOAT    fY             = rLeftTransform.GetTranslation().y;
		rLeftTransform.SetIdentity();
		rLeftTransform.SetTranslation( -s_fXOffset, fY );
		rLeftTransform.Scale( 0.5f, 0.5f );
		m_aIconRects[ m_iLeftSelection ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );
	}

	if ( m_iRightSelection != -1 )
	{
		T2GUITransform& rRightTransform = m_aIconRects[ m_iRightSelection ].GetTransform();
		const TFLOAT    fY              = rRightTransform.GetTranslation().y;
		rRightTransform.SetIdentity();
		rRightTransform.SetTranslation( s_fXOffset, fY );
		rRightTransform.Scale( 0.5f, 0.5f );
		m_aIconRects[ m_iRightSelection ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );
	}

	if ( m_iCurrSelection != -1 )
	{
		T2GUITransform& rCurrTransform = m_aIconRects[ m_iCurrSelection ].GetTransform();
		const TFLOAT    fY             = rCurrTransform.GetTranslation().y;
		rCurrTransform.SetIdentity();
		rCurrTransform.SetTranslation( 0.0f, fY );
		rCurrTransform.Scale( s_fSelectedScale, s_fSelectedScale );

		if ( CurIsAvailableToPlay() )
			m_aIconRects[ m_iCurrSelection ].SetColour( TCOLOR4( 255, 255, 255, 255 ) );
		else
			m_aIconRects[ m_iCurrSelection ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );
	}

	SetTexts();

	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 0040aa00
TBOOL AFrontEndMiniGameState2::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && m_bMouseMoved )
	{
		// is mouse over the central item or nav arrows?
		T2GUITransform oFromScreen;
		TVector2       vLocalPos;
		TVector2       vCursorPos = g_pGUI2->GetMouseCursor().GetCursorPos();

		// Mouseover CurrItem test
		m_aIconRects[ m_iCurrSelection ].GetInvScreenTransform( oFromScreen ); // Screen->Local transform
		oFromScreen.Transform( vLocalPos, vCursorPos );                        // Transform cursor screen pos into local space
		m_bMouseOverSelected = m_aIconRects[ m_iCurrSelection ].IsPointInside( vLocalPos.x, vLocalPos.y );

		if ( !m_bMouseOverSelected )
		{
			// Mouseover left arrow test
			m_aArrows[ Arrow_Left ].GetInvScreenTransform( oFromScreen );
			oFromScreen.Transform( vLocalPos, vCursorPos );
			m_bMouseOverLeftArrow = m_aArrows[ Arrow_Left ].IsPointInside( vLocalPos.x, vLocalPos.y );
		}

		if ( !m_bMouseOverSelected && !m_bMouseOverLeftArrow )
		{
			// Mouseover right arrow test
			m_aArrows[ Arrow_Right ].GetInvScreenTransform( oFromScreen );
			oFromScreen.Transform( vLocalPos, vCursorPos );
			m_bMouseOverRightArrow = m_aArrows[ Arrow_Right ].IsPointInside( vLocalPos.x, vLocalPos.y );
		}

		m_bMouseMoved = TFALSE;
	}

	if ( m_eMenuState == MENUSTATE_MENU_APPEAR || m_eMenuState == MENUSTATE_MENU_DISAPPEAR )
	{
		m_oGameLabel.SetAlpha( m_fMenuOpacity );
		m_oGameLabel.SetShadowAlpha( m_fMenuOpacity * ABYardMenuState::s_fShadowAlpha );
		m_oGameDesc.SetAlpha( m_fMenuOpacity );
		m_oGameDesc.SetShadowAlpha( m_fMenuOpacity * ABYardMenuState::s_fShadowAlpha );
	}
	else if ( m_eMenuState == MENUSTATE_DIALOG_APPEAR || m_eMenuState == MENUSTATE_DIALOG_DISAPPEAR )
	{
		m_oGameLabel.SetAlpha( 0.0f );
		m_oGameLabel.SetShadowAlpha( 0.0f );
		m_oGameDesc.SetAlpha( 0.0f );
		m_oGameDesc.SetShadowAlpha( 0.0f );
	}

	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return BaseClass::OnUpdate( a_fDeltaTime );

	m_fAnalogTime += a_fDeltaTime;

	TFLOAT fRightLerpTarget = 1.0f;
	TFLOAT fLeftLerpTarget  = 1.0f;

	if ( m_bMouseOverRightArrow )
		fRightLerpTarget = 1.25f; // Scale up when moused over

	if ( m_bMouseOverLeftArrow )
		fLeftLerpTarget = 1.25f; // Scale up when moused over

	T2GUITransform& rRTransform = m_aArrows[ Arrow_Right ].GetTransform();
	rRTransform.SetScale(
	    TMath::LERPClamped( rRTransform.GetScaleX(), fRightLerpTarget, a_fDeltaTime * 10.0f ),
	    TMath::LERPClamped( rRTransform.GetScaleY(), fRightLerpTarget, a_fDeltaTime * 10.0f )
	);

	T2GUITransform& rLTransform = m_aArrows[ Arrow_Left ].GetTransform();
	rLTransform.SetScale(
	    TMath::LERPClamped( rLTransform.GetScaleX(), fLeftLerpTarget, a_fDeltaTime * 10.0f ),
	    TMath::LERPClamped( rLTransform.GetScaleY(), fLeftLerpTarget, a_fDeltaTime * 10.0f )
	);

	// Pulse the current selection
	if ( m_oMenu.IsFocused() && m_eLocalState == LocalState_Ready )
	{
		TFLOAT fScaleFactor = 1.1f + 0.05f * TMath::Sin( m_fTotalTime * TMath::TWO_PI );
		m_aIconRects[ m_iCurrSelection ].GetTransform().SetScale( fScaleFactor, fScaleFactor );
	}

	switch ( m_eLocalState )
	{
		case LocalState_InitRotateLeft:
		{
			TINT iOldNew      = m_iNewSelection;
			m_iNewSelection   = m_iLeftSelection; // The selection we fade out
			m_iLeftSelection  = m_iCurrSelection;
			m_iCurrSelection  = m_iRightSelection;
			m_iRightSelection = m_iRightSelection + 1;

			if ( m_iRightSelection >= m_iNumRects )
				m_iRightSelection = 0;

			if ( iOldNew != m_iRightSelection )
			{
				// Set init props
				T2GUIRectangle& rRect = m_aIconRects[ m_iRightSelection ];
				rRect.SetColour( 0x00000000 ); // black and fully translucent
				rRect.SetTransform( s_fXOffset * 2.0f, 0.0f );
				rRect.GetTransform().Scale( 0.5f, 0.5f );
				rRect.Show();

				// Hide previously fading new selection if rqd
				if ( iOldNew != -1 )
				{
					T2GUIRectangle& rOldRect = m_aIconRects[ iOldNew ];
					rOldRect.SetColour( 0x00000000 ); // black and fully translucent
					rOldRect.SetTransform( 0.0f, 0.0f );
					rOldRect.GetTransform().Scale( 0.5f, 0.5f );
					rOldRect.Hide();
				}
			}

			m_aIconRects[ m_iCurrSelection ].SetAlpha( 1.0f );
			m_eLocalState = LocalState_RotateLeft;
			SetTexts();

			break;
		}
		case LocalState_InitRotateRight:
		{
			TINT iOldNew      = m_iNewSelection;
			m_iNewSelection   = m_iRightSelection; // The selection we fade out
			m_iRightSelection = m_iCurrSelection;
			m_iCurrSelection  = m_iLeftSelection;
			m_iLeftSelection  = m_iLeftSelection - 1;

			if ( m_iLeftSelection < 0 )
				m_iLeftSelection = m_iNumRects - 1;

			if ( iOldNew != m_iLeftSelection )
			{
				// Set init props
				T2GUIRectangle& rRect = m_aIconRects[ m_iLeftSelection ];
				rRect.SetColour( 0x00000000 ); // black and fully translucent
				rRect.SetTransform( -s_fXOffset * 2.0f, 0.0f );
				rRect.GetTransform().Scale( 0.5f, 0.5f );
				rRect.Show();

				// Hide previously fading new selection if rqd
				if ( iOldNew != -1 )
				{
					T2GUIRectangle& rOldRect = m_aIconRects[ iOldNew ];
					rOldRect.SetColour( 0x00000000 ); // black and fully translucent
					rOldRect.SetTransform( 0.0f, 0.0f );
					rOldRect.GetTransform().Scale( 0.5f, 0.5f );
					rOldRect.Hide();
				}
			}

			m_aIconRects[ m_iCurrSelection ].SetAlpha( 1.0f );
			m_eLocalState = LocalState_RotateRight;
			SetTexts();

			break;
		}
		// [1/19/2026 InfiniteC0re]
		// NOTE: the original has two copies of the code below for LocalState_RotateLeft and LocalState_RotateRight,
		// each with its own static s_fLerpRate (both 5.0f, 0x00772658 and 0x00772654). The only differences are the
		// target position of the fading out icon and the order of resetting m_fTotalTime and hiding m_iNewSelection,
		// so they're combined here.
		case LocalState_RotateLeft:
		case LocalState_RotateRight:
		{
			static TFLOAT s_fLerpRate = 5.0f;

			T2GUIRectangle& rLeft  = m_aIconRects[ m_iLeftSelection ];
			T2GUIRectangle& rRight = m_aIconRects[ m_iRightSelection ];
			T2GUIRectangle& rCurr  = m_aIconRects[ m_iCurrSelection ];
			T2GUIRectangle& rNew   = m_aIconRects[ m_iNewSelection ];

			T2GUITransform& rLeftTransform  = rLeft.GetTransform();
			T2GUITransform& rRightTransform = rRight.GetTransform();
			T2GUITransform& rCurrTransform  = rCurr.GetTransform();
			T2GUITransform& rNewTransform   = rNew.GetTransform();

			// If the new middle item has arrived, we're done
			const TFLOAT fCurrX = rCurrTransform.GetTranslation().x;
			const TFLOAT fCurrY = rCurrTransform.GetTranslation().y;

			if ( TMath::Abs( fCurrX ) <= s_fMinDiff )
			{
				// New middle has arrived, snap to final pos
				rLeftTransform.SetIdentity();
				rLeftTransform.SetTranslation( -s_fXOffset, fCurrY );
				rLeftTransform.Scale( 0.5f, 0.5f );
				rLeft.SetColour( TCOLOR4( 0, 0, 0, 255 ) );

				rRightTransform.SetIdentity();
				rRightTransform.SetTranslation( s_fXOffset, fCurrY );
				rRightTransform.Scale( 0.5f, 0.5f );
				rRight.SetColour( TCOLOR4( 0, 0, 0, 255 ) );

				rCurrTransform.SetIdentity();
				rCurrTransform.SetTranslation( 0.0f, fCurrY );
				rCurrTransform.Scale( s_fSelectedScale, s_fSelectedScale );

				if ( CurIsAvailableToPlay() )
					rCurr.SetColour( TCOLOR4( 255, 255, 255, 255 ) );
				else
					rCurr.SetColour( TCOLOR4( 0, 0, 0, 255 ) );

				if ( m_iNewSelection != -1 )
				{
					m_aIconRects[ m_iNewSelection ].SetColour( TCOLOR4( 0, 0, 0, 0 ) );
					m_aIconRects[ m_iNewSelection ].Hide();
					m_iNewSelection = -1;
				}

				// Reset timer so sine coeff is 0
				m_fTotalTime  = 0.0f;
				m_eLocalState = LocalState_Ready;
			}
			else
			{
				// Update transforms
				// =================================
				const TFLOAT fLerp = a_fDeltaTime * s_fLerpRate;

				const TFLOAT fCurrS  = rCurrTransform.GetScaleX();
				const TFLOAT fLeftS  = rLeftTransform.GetScaleX();
				const TFLOAT fRightS = rRightTransform.GetScaleX();

				// Left icon
				TFLOAT fX = rLeftTransform.GetTranslation().x;
				TFLOAT fY = rLeftTransform.GetTranslation().y;
				rLeftTransform.SetIdentity();
				rLeftTransform.SetTranslation( TMath::LERPClamped( fX, -s_fXOffset, fLerp ), fY );

				// Current icon
				fX = rCurrTransform.GetTranslation().x;
				fY = rCurrTransform.GetTranslation().y;
				rCurrTransform.SetIdentity();
				rCurrTransform.SetTranslation( TMath::LERPClamped( fX, 0.0f, fLerp ), fY );

				// Right icon
				fX = rRightTransform.GetTranslation().x;
				fY = rRightTransform.GetTranslation().y;
				rRightTransform.SetIdentity();
				rRightTransform.SetTranslation( TMath::LERPClamped( fX, s_fXOffset, fLerp ), fY );

				// New icon
				fX = rNewTransform.GetTranslation().x;
				fY = rNewTransform.GetTranslation().y;
				rNewTransform.SetIdentity();
				rNewTransform.SetTranslation( TMath::LERPClamped( fX, ( m_eLocalState == LocalState_RotateLeft ) ? -s_fXOffset * 2.0f : s_fXOffset * 2.0f, fLerp ), fY );

				// Update scales
				// =================================

				// Left icon
				TFLOAT fScale = TMath::LERPClamped( fLeftS, 0.5f, fLerp );
				rLeftTransform.Scale( fScale, fScale );

				// Selected icon
				fScale = TMath::LERPClamped( fCurrS, s_fSelectedScale, fLerp );
				rCurrTransform.Scale( fScale, fScale );

				// Right icon
				fScale = TMath::LERPClamped( fRightS, 0.5f, fLerp );
				rRightTransform.Scale( fScale, fScale );

				// New icon
				rNewTransform.Scale( 0.5f, 0.5f );

				// Update colours
				// =================================

				// Left and right lerp to black
				TUINT aARGB[ 4 ];
				TColor::GetComponents( rLeft.GetColour(), aARGB[ 0 ], aARGB[ 1 ], aARGB[ 2 ], aARGB[ 3 ] );
				for ( TINT i = 1; i < 4; i++ )
					aARGB[ i ] = TUINT( TMath::LERPClamped( TFLOAT( TINT( aARGB[ i ] ) ), 0.0f, fLerp ) );

				aARGB[ 0 ] = TUINT( TMath::LERPClamped( TFLOAT( TINT( aARGB[ 0 ] ) ), 255.0f, fLerp ) );
				rLeft.SetColour( TCOLOR4( aARGB[ 1 ], aARGB[ 2 ], aARGB[ 3 ], aARGB[ 0 ] ) );

				TColor::GetComponents( rRight.GetColour(), aARGB[ 0 ], aARGB[ 1 ], aARGB[ 2 ], aARGB[ 3 ] );
				for ( TINT i = 1; i < 4; i++ )
					aARGB[ i ] = TUINT( TMath::LERPClamped( TFLOAT( TINT( aARGB[ i ] ) ), 0.0f, fLerp ) );

				aARGB[ 0 ] = TUINT( TMath::LERPClamped( TFLOAT( TINT( aARGB[ 0 ] ) ), 255.0f, fLerp ) );
				rRight.SetColour( TCOLOR4( aARGB[ 1 ], aARGB[ 2 ], aARGB[ 3 ], aARGB[ 0 ] ) );

				// Current lerps to white if available
				if ( CurIsAvailableToPlay() )
				{
					TColor::GetComponents( rCurr.GetColour(), aARGB[ 0 ], aARGB[ 1 ], aARGB[ 2 ], aARGB[ 3 ] );
					for ( TINT i = 0; i < 4; i++ )
						aARGB[ i ] = TUINT( TMath::LERPClamped( TFLOAT( TINT( aARGB[ i ] ) ), 255.0f, fLerp ) );

					rCurr.SetColour( TCOLOR4( aARGB[ 1 ], aARGB[ 2 ], aARGB[ 3 ], aARGB[ 0 ] ) );
				}

				// New lerps out
				TFLOAT fAlpha = TCOLOR_GET_ALPHA( rNew.GetColour() ) / 255.0f;
				fAlpha        = TMath::LERPClamped( fAlpha, 0.0f, fLerp );
				rNew.SetAlpha( fAlpha );
			}

			break;
		}
		default:
			break;
	}

	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0040a0f0
void AFrontEndMiniGameState2::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	if ( !CurIsAvailableToPlay() )
	{
		// Disallow selection
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );
		return;
	}

	// Use the current item
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );
}

// $Barnyard: FUNCTION 0040be80
void AFrontEndMiniGameState2::OnMenuClose()
{
	// NOTE: the original source calls Inherited::DoSelection() here, ABYardMenuState::OnMenuClose does nothing

	if ( m_iNewSelection != -1 )
	{
		m_aIconRects[ m_iNewSelection ].SetAlpha( 0.0f );
		m_aIconRects[ m_iNewSelection ].Hide();
		m_iNewSelection = -1;
	}

	if ( m_iCurrSelection == -1 )
	{
		BaseClass::Remove();
		return;
	}

	// Start the selected minigame

#ifdef BARNYARD_COMMUNITY_PATCH
	// Don't start loading a minigame whose game state isn't reconstructed yet, it's only found by
	// name (TClass::Find) and loading it would unload the frontend and then crash
	{
		TINT iCheckedMiniGame = ( !m_bAntics ) ? m_iCurrSelection : AMiniGameManager::GetSingleton()->GetIndexForAntic( m_iCurrSelection );

		if ( !AMiniGameManager::GetSingleton()->GetMiniGame( iCheckedMiniGame )->GetGameStateClass() )
		{
			TWARN( "Minigame '%s' can't be played yet, its game state class isn't implemented\n", AMiniGameManager::GetSingleton()->GetMiniGame( iCheckedMiniGame )->m_strMiniGameName.GetString() );
			BaseClass::Remove();
			return;
		}
	}

	// The main profile's gender is only set by loading a save or starting a new game (AFrontEndNewGameSelectState
	// sets it to male), neither of which is reconstructed yet. With GENDER_NONE no player character gets loaded and
	// spawning the player unit of a minigame crashes, so default to male like a new game does
	if ( APlayerProfileManager::GetSingleton()->GetMainGender() == APlayerProfile::GENDER_NONE )
		APlayerProfileManager::GetSingleton()->SetMainGender( APlayerProfile::GENDER_MALE );
#endif

	APlayerSet* pMiniGameSet = APlayerManager::GetSingleton()->GetMiniGamePlayerSet();

	// Debugging a minigame, we allow 1 human player from the master controller
	// and n AI players, otherwise the playerset will have already been setup in the FrontEndPlayersState
	pMiniGameSet->AddHumanPlayer( AInputHandler::GetSingleton()->GetMainController(), -1 ); // This is ok if it's NULL

	// MinPlayers is now used as the AI count when debugging minigames
	// NOTE: despite the comment above the original reads the max number of players
	const TINT      iMaxPlayers = AMiniGameManager::GetSingleton()->GetMiniGame( m_iCurrSelection )->m_iMaxNumPlayers;
	const TPString8 sName       = AMiniGameManager::GetSingleton()->GetMiniGame( m_iCurrSelection )->m_strMiniGameName;

	if ( sName == TPS8D( "AGateCrashMiniGameState" ) )
	{
		// Fill out AI in mudjumpers
		for ( TINT i = iMaxPlayers - pMiniGameSet->GetHumanPlayerCount(); i > 0; i-- )
			pMiniGameSet->AddAIPlayer();
	}

	pMiniGameSet->MakeTeamsFair();
	AMiniGameManager::GetSingleton()->MakeValidMiniGamePlayerSet( m_iCurrSelection );
	pMiniGameSet->FinishUpAddingAIs();

	TINT iGameIndex;
	TINT iNumVariants;

	if ( m_bAntics )
	{
		iGameIndex   = AMiniGameManager::GetSingleton()->GetIndexForAntic( m_iCurrSelection );
		iNumVariants = 0;
	}
	else
	{
		iGameIndex   = m_iCurrSelection;
		iNumVariants = AMiniGameManager::GetSingleton()->GetMiniGame( m_iCurrSelection )->m_iNumSelectableVariants;
	}

	// Reset the highscores to the trb data before we actually start the game
	AHighScoreManager::ScoreReset( TFALSE );

	if ( iNumVariants > 1 )
	{
		// Transition to the minigame variant selection state
		AGameStateController::GetSingleton()->PushState( new AFrontEndVariantSelectState( iGameIndex ) );
	}
	else
	{
		// Just start the game

		// A bit hacky, but setting the loading background here doesn't work because the gui's background object allocates
		// need to preallocate this somehow so there's no delay before we display the loading screen...hmmm
		m_oDialogTitleBackground.Hide();
		m_oDialog.Hide();

		// This is to set the weather. This is not done in game loaded so that the mini-game,
		// when launched from the sim, can get the current Sim time of day.
		AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE_NORMAL );
		AWeatherManager::GetSingleton()->LockToCurrentWeatherType( TFALSE );
		AGameTimeManager::GetSingleton()->ChangePhaseOfDay( AGameTimeManager::DAYPHASE_MORNING, TTRUE, TTRUE );
		AGameTimeFXManager::GetSingleton()->SetDayPhase( AGameTimeManager::DAYPHASE_MORNING, TFALSE, 0.0f );

		AMiniGameManager::GetSingleton()->SetMiniGameVariant(
		    m_bAntics ? AMiniGameManager::GetSingleton()->GetMiniGame( iGameIndex )->m_iDefaultVariant : 0
		);

		AGameLoader::SetTransitionToMiniGame( iGameIndex, TFALSE );
		AGameStateController::GetSingleton()->SetFadeSound( TTRUE );
	}
}

// $Barnyard: FUNCTION 0040a150
void AFrontEndMiniGameState2::SetTexts()
{
	const TWCHAR* wszLocName;
	const TWCHAR* wszLocDesc;

	if ( m_bAntics )
	{
		if ( CurIsAvailableToPlay() )
		{
			wszLocName = g_pLocaleManager->GetString( AMiniGameManager::GetSingleton()->GetVisibleMiniGame( m_iCurrSelection )->m_eLocaleTitle );
			wszLocDesc = g_pLocaleManager->GetString( AMiniGameManager::GetSingleton()->GetVisibleMiniGame( m_iCurrSelection )->m_eLocaleDescription );
		}
		else
		{
			wszLocName = g_pLocaleManager->GetString( 238 ); // Not available
			wszLocDesc = g_pLocaleManager->GetString( 239 ); // Play to unlock
		}
	}
	else
	{
		wszLocName = g_pLocaleManager->GetString( AMiniGameManager::GetSingleton()->GetMiniGame( m_iCurrSelection )->m_eLocaleTitle );
		wszLocDesc = g_pLocaleManager->GetString( AMiniGameManager::GetSingleton()->GetMiniGame( m_iCurrSelection )->m_eLocaleDescription );
	}

	m_oGameLabel.SetText( wszLocName );
	m_oGameDesc.SetText( wszLocDesc );
}

// $Barnyard: FUNCTION 0040a090
TBOOL AFrontEndMiniGameState2::CurIsAvailableToPlay() const
{
	return !m_bAntics || g_pPlayerProgress->IsMiniGameAvailable( m_iCurrSelection );
}
