#include "pch.h"
#include "AFrontEndArtGalleryState.h"
#include "AGalleryState.h"
#include "AMovieState.h"
#include "AGameStateController.h"
#include "Helpers/APlayerProgress.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "SoundBank/ui.h"
#include "SoundBank/music.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Input/TInputDeviceMouse.h>
#include <Toshi/TColor.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AFrontEndArtGalleryState );

// $Barnyard: FUNCTION 00404900
AFrontEndArtGalleryState::AFrontEndArtGalleryState( MODE a_eMode )
{
	m_eMode                = a_eMode;
	m_pItems               = TNULL;
	m_iNumItems            = 0;
	m_iCurrent             = -1;
	m_iPrev                = -1;
	m_iNext                = -1;
	m_iOld                 = -1;
	m_eScrollState         = SCROLLSTATE_READY;
	m_fAnalogTime          = 0.0f;
	m_bMouseMoved          = TFALSE;
	m_bMouseOverCurrent    = TFALSE;
	m_bMouseOverLeftArrow  = TFALSE;
	m_bMouseOverRightArrow = TFALSE;
}

// $Barnyard: FUNCTION 00404a00
// $Barnyard: FUNCTION 00404c30
AFrontEndArtGalleryState::~AFrontEndArtGalleryState()
{
}

// $Barnyard: FUNCTION 00404bb0
TBOOL AFrontEndArtGalleryState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
	{
		// The hovered elements are checked in OnUpdate
		m_bMouseMoved = TTRUE;
		return TTRUE;
	}

	if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->GetDoodad() != TInputDeviceMouse::BUTTON_1 )
		return BaseClass::ProcessInput( a_pInputEvent );

	if ( m_bMouseOverCurrent == TTRUE )
	{
		OnMenuItemActivated( TNULL );
		return TTRUE;
	}

	if ( m_bMouseOverLeftArrow == TTRUE )
	{
		TBOOL bStopEvents;
		return ProcessCommand( AInputCommand_Left, a_pInputEvent, bStopEvents );
	}

	if ( m_bMouseOverRightArrow == TTRUE )
	{
		TBOOL bStopEvents;
		return ProcessCommand( AInputCommand_Right, a_pInputEvent, bStopEvents );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00404c50
TBOOL AFrontEndArtGalleryState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return TFALSE;

	if ( a_eInputCommand == AInputCommand_Cancel )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );
		m_eMenuState = MENUSTATE_MENU_DISAPPEAR;
		m_iCurrent   = -1;
		return TTRUE;
	}

	if ( !m_oMenu.IsFocused() )
		return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( a_eInputCommand == AInputCommand_OK )
	{
		// Clicks are handled by ProcessInput
		if ( a_pInputEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) ) )
			return TFALSE;
	}
	else if ( a_eInputCommand == AInputCommand_Move )
	{
		// Analog stick
		if ( m_fAnalogTime <= 0.2f )
			return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

		TBOOL  bHandled = TFALSE;
		TFLOAT fX       = a_pInputEvent->GetMagnitudeFloat( 0 );
		a_pInputEvent->GetMagnitudeFloat( 1 );

		if ( fX < -0.5f )
		{
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
			m_aArrows[ ARROW_LEFT ].GetTransform().SetScale( 1.25f, 1.25f );
			m_eScrollState = SCROLLSTATE_INIT_PREV;
			bHandled       = TTRUE;
		}
		else if ( fX > 0.5f )
		{
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
			m_aArrows[ ARROW_RIGHT ].GetTransform().SetScale( 1.25f, 1.25f );
			m_eScrollState = SCROLLSTATE_INIT_NEXT;
			bHandled       = TTRUE;
		}

		if ( fX < -0.5f || fX > 0.5f )
			m_fAnalogTime = 0.0f;

		if ( bHandled == TTRUE )
			return TTRUE;

		return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}

	if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT )
	{
		if ( a_eInputCommand == AInputCommand_Left )
		{
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
			m_aArrows[ ARROW_LEFT ].GetTransform().SetScale( 1.25f, 1.25f );
			m_eScrollState = SCROLLSTATE_INIT_PREV;
			a_pInputEvent->StartRepeat( 0.5f, 0.2f );
			return TTRUE;
		}

		if ( a_eInputCommand == AInputCommand_Right )
		{
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
			m_aArrows[ ARROW_RIGHT ].GetTransform().SetScale( 1.25f, 1.25f );
			m_eScrollState = SCROLLSTATE_INIT_NEXT;
			a_pInputEvent->StartRepeat( 0.5f, 0.2f );
			return TTRUE;
		}
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// Lerps the RGB components of a_uiColour towards a_fRGB and the alpha towards a_fAlpha
// NOTE: inlined in the original
static TUINT32 LerpColour( TUINT32 a_uiColour, TFLOAT a_fRGB, TFLOAT a_fAlpha, TFLOAT a_fProgress )
{
	TUINT aARGB[ 4 ];
	TColor::GetComponents( a_uiColour, aARGB[ 0 ], aARGB[ 1 ], aARGB[ 2 ], aARGB[ 3 ] );

	for ( TINT i = 1; i < 4; i++ )
		aARGB[ i ] = TUINT( TMath::LERPClamped( TFLOAT( TINT( aARGB[ i ] ) ), a_fRGB, a_fProgress ) );

	aARGB[ 0 ] = TUINT( TMath::LERPClamped( TFLOAT( TINT( aARGB[ 0 ] ) ), a_fAlpha, a_fProgress ) );

	return TCOLOR4( aARGB[ 1 ], aARGB[ 2 ], aARGB[ 3 ], aARGB[ 0 ] );
}

// $Barnyard: FUNCTION 00405a40
TBOOL AFrontEndArtGalleryState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && m_bMouseMoved == TTRUE )
	{
		// Is the mouse over the current item or the arrows?
		T2GUITransform oFromScreen;
		TVector2       vLocalPos;
		TVector2       vCursorPos = g_pGUI2->GetMouseCursor().GetCursorPos();

		m_pItems[ m_iCurrent ].GetInvScreenTransform( oFromScreen );
		oFromScreen.Transform( vLocalPos, vCursorPos );
		m_bMouseOverCurrent = m_pItems[ m_iCurrent ].IsPointInside( vLocalPos.x, vLocalPos.y );

		if ( !m_bMouseOverCurrent )
		{
			m_aArrows[ ARROW_LEFT ].GetInvScreenTransform( oFromScreen );
			oFromScreen.Transform( vLocalPos, vCursorPos );
			m_bMouseOverLeftArrow = m_aArrows[ ARROW_LEFT ].IsPointInside( vLocalPos.x, vLocalPos.y );

			if ( !m_bMouseOverCurrent && !m_bMouseOverLeftArrow )
			{
				m_aArrows[ ARROW_RIGHT ].GetInvScreenTransform( oFromScreen );
				oFromScreen.Transform( vLocalPos, vCursorPos );
				m_bMouseOverRightArrow = m_aArrows[ ARROW_RIGHT ].IsPointInside( vLocalPos.x, vLocalPos.y );
			}
		}

		m_bMouseMoved = TFALSE;
	}

	if ( m_eMenuState == MENUSTATE_MENU_APPEAR || m_eMenuState == MENUSTATE_MENU_DISAPPEAR )
	{
		m_oName.SetAlpha( m_fMenuOpacity );
		m_oName.SetShadowAlpha( m_fMenuOpacity * s_fShadowAlpha );
		m_oDescription.SetAlpha( m_fMenuOpacity );
		m_oDescription.SetShadowAlpha( m_fMenuOpacity * s_fShadowAlpha );
	}
	else if ( m_eMenuState == MENUSTATE_DIALOG_APPEAR || m_eMenuState == MENUSTATE_DIALOG_DISAPPEAR )
	{
		m_oName.SetAlpha( 0.0f );
		m_oName.SetShadowAlpha( 0.0f );
		m_oDescription.SetAlpha( 0.0f );
		m_oDescription.SetShadowAlpha( 0.0f );
	}

	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return BaseClass::OnUpdate( a_fDeltaTime );

	m_fAnalogTime += a_fDeltaTime;

	// The hovered arrows grow
	TFLOAT fRightScale = ( m_bMouseOverRightArrow == TTRUE ) ? 1.25f : 1.0f;
	TFLOAT fLeftScale  = ( m_bMouseOverLeftArrow == TTRUE ) ? 1.25f : 1.0f;

	T2GUITransform& rRightTransform = m_aArrows[ ARROW_RIGHT ].GetTransform();
	rRightTransform.SetScale(
	    TMath::LERPClamped( rRightTransform.GetScaleX(), fRightScale, a_fDeltaTime * 10.0f ),
	    TMath::LERPClamped( rRightTransform.GetScaleY(), fRightScale, a_fDeltaTime * 10.0f )
	);

	T2GUITransform& rLeftTransform = m_aArrows[ ARROW_LEFT ].GetTransform();
	rLeftTransform.SetScale(
	    TMath::LERPClamped( rLeftTransform.GetScaleX(), fLeftScale, a_fDeltaTime * 10.0f ),
	    TMath::LERPClamped( rLeftTransform.GetScaleY(), fLeftScale, a_fDeltaTime * 10.0f )
	);

	// Pulse the current item
	if ( m_oMenu.IsFocused() && m_eScrollState == SCROLLSTATE_READY )
	{
		TFLOAT fScale = TMath::Sin( m_fTotalTime * TMath::TWO_PI ) * 0.05f + 1.1f;
		m_pItems[ m_iCurrent ].GetTransform().SetScale( fScale, fScale );
	}

	switch ( m_eScrollState )
	{
		case SCROLLSTATE_INIT_NEXT:
		{
			TINT iOld  = m_iOld;
			m_iOld     = m_iPrev;
			m_iPrev    = m_iCurrent;
			m_iCurrent = m_iNext;
			m_iNext    = m_iNext + 1;

			if ( m_iNext >= m_iNumItems )
				m_iNext = 0;

			if ( iOld != m_iNext )
			{
				// The new item comes from the right
				T2GUIRectangle& rNew = m_pItems[ m_iNext ];
				rNew.SetColour( 0 );
				rNew.SetTransform( ms_fXOffset + ms_fXOffset, 0.0f );
				rNew.GetTransform().Scale( 0.5f, 0.5f );
				rNew.Show();

				if ( iOld != -1 )
				{
					T2GUIRectangle& rOld = m_pItems[ iOld ];
					rOld.SetColour( 0 );
					rOld.SetTransform( 0.0f, 0.0f );
					rOld.GetTransform().Scale( 0.5f, 0.5f );
					rOld.Hide();
				}
			}

			m_pItems[ m_iCurrent ].SetAlpha( 1.0f );
			m_eScrollState = SCROLLSTATE_NEXT;
			UpdateTexts();
			break;
		}
		case SCROLLSTATE_NEXT:
			UpdateScrolling( a_fDeltaTime, ms_fNextLerpRate, ms_fXOffset * -2.0f );
			break;
		case SCROLLSTATE_INIT_PREV:
		{
			TINT iOld  = m_iOld;
			m_iOld     = m_iNext;
			m_iNext    = m_iCurrent;
			m_iCurrent = m_iPrev;
			m_iPrev    = m_iPrev - 1;

			if ( m_iPrev < 0 )
				m_iPrev = m_iNumItems - 1;

			if ( iOld != m_iPrev )
			{
				// The new item comes from the left
				T2GUIRectangle& rNew = m_pItems[ m_iPrev ];
				rNew.SetColour( 0 );
				rNew.SetTransform( ms_fXOffset * -2.0f, 0.0f );
				rNew.GetTransform().Scale( 0.5f, 0.5f );
				rNew.Show();

				if ( iOld != -1 )
				{
					T2GUIRectangle& rOld = m_pItems[ iOld ];
					rOld.SetColour( 0 );
					rOld.SetTransform( 0.0f, 0.0f );
					rOld.GetTransform().Scale( 0.5f, 0.5f );
					rOld.Hide();
				}
			}

			m_pItems[ m_iCurrent ].SetAlpha( 1.0f );
			m_eScrollState = SCROLLSTATE_PREV;
			UpdateTexts();
			break;
		}
		case SCROLLSTATE_PREV:
			UpdateScrolling( a_fDeltaTime, ms_fPrevLerpRate, ms_fXOffset + ms_fXOffset );
			break;
		default:
			break;
	}

	return BaseClass::OnUpdate( a_fDeltaTime );
}

void AFrontEndArtGalleryState::UpdateScrolling( TFLOAT a_fDeltaTime, TFLOAT a_fLerpRate, TFLOAT a_fOldItemX )
{
	T2GUIRectangle& rPrev = m_pItems[ m_iPrev ];
	T2GUIRectangle& rCurr = m_pItems[ m_iCurrent ];
	T2GUIRectangle& rNext = m_pItems[ m_iNext ];
	T2GUIRectangle& rOld  = m_pItems[ m_iOld ];

	T2GUITransform& rPrevTransform = rPrev.GetTransform();
	T2GUITransform& rCurrTransform = rCurr.GetTransform();
	T2GUITransform& rNextTransform = rNext.GetTransform();
	T2GUITransform& rOldTransform  = rOld.GetTransform();

	const TFLOAT fCurrY = rCurrTransform.GetTranslation().y;

	if ( TMath::Abs( rCurrTransform.GetTranslation().x ) <= ms_fMinDiff )
	{
		// The current item has arrived, snap everything to its place
		rPrevTransform.SetIdentity();
		rPrevTransform.SetTranslation( -ms_fXOffset, fCurrY );
		rPrevTransform.Scale( 0.5f, 0.5f );
		rPrev.SetColour( TCOLOR4( 0, 0, 0, 255 ) );

		rNextTransform.SetIdentity();
		rNextTransform.SetTranslation( ms_fXOffset, fCurrY );
		rNextTransform.Scale( 0.5f, 0.5f );
		rNext.SetColour( TCOLOR4( 0, 0, 0, 255 ) );

		rCurrTransform.SetIdentity();
		rCurrTransform.SetTranslation( 0.0f, fCurrY );
		rCurrTransform.Scale( ms_fSelectedScale, ms_fSelectedScale );
		rCurr.SetColour( IsCurrentUnlocked() ? TCOLOR4( 255, 255, 255, 255 ) : TCOLOR4( 0, 0, 0, 255 ) );

		if ( m_iOld != -1 )
		{
			m_pItems[ m_iOld ].SetColour( 0 );
			m_pItems[ m_iOld ].Hide();
			m_iOld = -1;
		}

		m_fTotalTime   = 0.0f;
		m_eScrollState = SCROLLSTATE_READY;
		return;
	}

	const TFLOAT fProgress = a_fDeltaTime * a_fLerpRate;

	const TFLOAT fPrevScale = rPrevTransform.GetScaleX();
	const TFLOAT fCurrScale = rCurrTransform.GetScaleX();
	const TFLOAT fNextScale = rNextTransform.GetScaleX();

	// Positions
	TFLOAT fX = rPrevTransform.GetTranslation().x;
	TFLOAT fY = rPrevTransform.GetTranslation().y;
	rPrevTransform.SetIdentity();
	rPrevTransform.SetTranslation( TMath::LERPClamped( fX, -ms_fXOffset, fProgress ), fY );

	fX = rCurrTransform.GetTranslation().x;
	fY = rCurrTransform.GetTranslation().y;
	rCurrTransform.SetIdentity();
	rCurrTransform.SetTranslation( TMath::LERPClamped( fX, 0.0f, fProgress ), fY );

	fX = rNextTransform.GetTranslation().x;
	fY = rNextTransform.GetTranslation().y;
	rNextTransform.SetIdentity();
	rNextTransform.SetTranslation( TMath::LERPClamped( fX, ms_fXOffset, fProgress ), fY );

	fX = rOldTransform.GetTranslation().x;
	fY = rOldTransform.GetTranslation().y;
	rOldTransform.SetIdentity();
	rOldTransform.SetTranslation( TMath::LERPClamped( fX, a_fOldItemX, fProgress ), fY );

	// Scales
	TFLOAT fScale = TMath::LERPClamped( fPrevScale, 0.5f, fProgress );
	rPrevTransform.Scale( fScale, fScale );

	fScale = TMath::LERPClamped( fCurrScale, ms_fSelectedScale, fProgress );
	rCurrTransform.Scale( fScale, fScale );

	fScale = TMath::LERPClamped( fNextScale, 0.5f, fProgress );
	rNextTransform.Scale( fScale, fScale );

	rOldTransform.Scale( 0.5f, 0.5f );

	// Colours: the sides go black, the current item white if it's unlocked and the old one fades out
	rPrev.SetColour( LerpColour( rPrev.GetColour(), 0.0f, 255.0f, fProgress ) );
	rNext.SetColour( LerpColour( rNext.GetColour(), 0.0f, 255.0f, fProgress ) );

	if ( IsCurrentUnlocked() )
		rCurr.SetColour( LerpColour( rCurr.GetColour(), 255.0f, 255.0f, fProgress ) );

	TFLOAT fAlpha = TCOLOR_GET_ALPHA( rOld.GetColour() ) / 255.0f;
	rOld.SetAlpha( TMath::LERPClamped( fAlpha, 0.0f, fProgress ) );
}

// $Barnyard: FUNCTION 004053f0
void AFrontEndArtGalleryState::OnInsertion()
{
	m_iCurrent  = 0;
	m_iNumItems = NUM_ITEMS;
	m_pItems    = new T2GUIRectangle[ NUM_ITEMS ];

	TFLOAT fMaxW = 0.0f;
	TFLOAT fMaxH = 0.0f;

	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		T2GUITextureSection* pTex = T2GUITextureSectionManager::GetTextureSection( ( m_eMode != MODE_ART ) ? ms_aMovieIcons[ i ] : ms_aArtIcons[ i ] );

		TFLOAT fTexW = pTex->GetWidth();
		TFLOAT fTexH = pTex->GetHeight();

		if ( fTexW > fMaxW )
			fMaxW = fTexW;

		if ( fTexH > fMaxH )
			fMaxH = fTexH;

		m_pItems[ i ].SetDimensions( fTexW, fTexH );
		m_pItems[ i ].SetTextureSection( pTex );
		m_oMenuItem.AddChildTail( m_pItems[ i ] );

		// Only the current item and the ones next to it are visible
		m_iPrev = m_iCurrent - 1;
		if ( m_iPrev < 0 )
			m_iPrev = m_iNumItems - 1;

		m_iNext = m_iCurrent + 1;
		if ( m_iNext >= m_iNumItems )
			m_iNext = 0;

		if ( i == m_iCurrent || i == m_iPrev || i == m_iNext )
		{
			if ( i == m_iPrev )
			{
				m_pItems[ i ].SetTransform( -ms_fXOffset, 0.0f );

				T2GUITransform oScale;
				oScale.SetFromScale( 0.5f, 0.5f );
				m_pItems[ i ].GetTransform().PreMultiply( oScale );
				m_pItems[ i ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );
			}
			else if ( i == m_iNext )
			{
				m_pItems[ i ].SetTransform( ms_fXOffset, 0.0f );

				T2GUITransform oScale;
				oScale.SetFromScale( 0.5f, 0.5f );
				m_pItems[ i ].GetTransform().PreMultiply( oScale );
				m_pItems[ i ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );
			}
			else
			{
				m_pItems[ i ].GetTransform().Scale( 1.1f, 1.1f );
				m_pItems[ i ].SetColour( IsCurrentUnlocked() ? TCOLOR4( 255, 255, 255, 255 ) : TCOLOR4( 0, 0, 0, 255 ) );
			}

			m_pItems[ i ].Show();
		}
		else
		{
			m_pItems[ i ].Hide();
		}
	}

	// Arrows
	T2GUITextureSection* pTex  = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	TFLOAT               fTexW = pTex->GetWidth();
	TFLOAT               fTexH = pTex->GetHeight();
	m_aArrows[ ARROW_LEFT ].SetDimensions( fTexW, fTexH );
	m_aArrows[ ARROW_LEFT ].SetTextureSection( pTex );
	m_aArrows[ ARROW_LEFT ].SetTransform( -150.0f, 0.0f );

	// NOTE: the right arrow uses the dimensions of the left one
	pTex = T2GUITextureSectionManager::GetTextureSection( "left_arrow2" );
	m_aArrows[ ARROW_RIGHT ].SetDimensions( fTexW, fTexH );
	m_aArrows[ ARROW_RIGHT ].SetTextureSection( pTex );
	m_aArrows[ ARROW_RIGHT ].SetTransform( 150.0f, 0.0f );

	for ( TINT i = 0; i < ARROW_NUMOF; i++ )
		m_oMenuItem.AddChildTail( m_aArrows[ i ] );

	m_oMenu.AddMenuItem( m_oMenuItem );

	// Name of the current item
	m_oName.Create( T2GUIFontManager::FindFont( "Rekord26" ), 400.0f );
	m_oName.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oName.SetColour( TCOLOR4( 255, 255, 255, 255 ) );
	m_oName.SetShadowEnabled( TTRUE );
	m_oName.SetShadowColour( 0xa0000000 );
	m_oName.SetAlpha( 0.0f );
	m_oName.SetShadowAlpha( 0.0f );
	m_oName.SetShadowOffset( m_oDialogTitle.GetScale() * 5.0f, m_oDialogTitle.GetScale() * 5.0f );
	m_oMenuItem.AddChildTail( m_oName );

	// Description of the current item
	m_oDescription.Create( T2GUIFontManager::FindFont( "Rekord18" ), 400.0f );
	m_oDescription.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oDescription.SetColour( TCOLOR4( 255, 200, 0, 255 ) );
	m_oDescription.SetShadowEnabled( TTRUE );
	m_oDescription.SetShadowColour( 0xa0000000 );
	m_oDescription.SetAlpha( 0.0f );
	m_oDescription.SetShadowAlpha( 0.0f );
	m_oDescription.SetShadowOffset( m_oDialogTitle.GetScale() * 5.0f, m_oDialogTitle.GetScale() * 5.0f );
	m_oMenuItem.AddChildTail( m_oDescription );

	UpdateTexts();

	m_oMenuItem.SetDimensions( fMaxW * 3.75f, m_oName.GetHeight() + m_oDescription.GetHeight() + fMaxH + 64.0f );

	m_oMenu.SetFocus( TTRUE );
	m_oMenu.SetFocusAt( m_oMenuItem );

	m_oDialogTitle.SetText( g_pLocaleManager->GetString( ( m_eMode != MODE_ART ) ? 73 : 72 ) );
	m_bFocusedElementBloated = TFALSE;

	BaseClass::OnInsertion();
}

// $Barnyard: FUNCTION 00404af0
void AFrontEndArtGalleryState::OnRemoval()
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
		m_pItems[ i ].RemoveSelf();

	m_oName.RemoveSelf();
	m_oDescription.RemoveSelf();
	m_oMenuItem.RemoveSelf();
	m_aArrows[ ARROW_LEFT ].RemoveSelf();
	m_aArrows[ ARROW_RIGHT ].RemoveSelf();

	if ( m_pItems )
		delete[] m_pItems;

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 004059c0
void AFrontEndArtGalleryState::OnActivate()
{
	m_eScrollState = SCROLLSTATE_READY;
	m_oMenu.SetFocus( TTRUE );
	ResetItems();
	UpdateTexts();

	if ( m_iOld != -1 )
	{
		m_pItems[ m_iOld ].SetAlpha( 0.0f );
		m_pItems[ m_iOld ].Hide();
	}

	AMusicManager::GetSingleton()->PlayBackgroundMusic( soundbank::MUSIC_FRONTEND );
	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 004052b0
void AFrontEndArtGalleryState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	if ( !IsCurrentUnlocked() )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );
		return;
	}

	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );

	// NOTE: the menu item isn't a button, ABYardMenuState animates whatever is at the place of the text of a button
	// (inside m_aArrows) until the menu is closed, which puts it back. Kept as in the original, with the same layout.
	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );
}

// $Barnyard: FUNCTION 00404f40
void AFrontEndArtGalleryState::OnMenuClose()
{
	if ( m_eMode == MODE_ART && TUINT( m_iCurrent ) <= NUM_ITEMS - 1 )
	{
		AMusicManager::GetSingleton()->StopBackgroundMusic();
		AGameStateController::GetSingleton()->PushState( new AGalleryState( m_iCurrent ) );
	}
	else if ( TUINT( m_iCurrent ) <= NUM_ITEMS - 1 )
	{
		AMusicManager::GetSingleton()->StopBackgroundMusic();
		AGameStateController::GetSingleton()->PushState( new AMovieState( ms_aMovies[ m_iCurrent ], TFALSE, TNULL, TTRUE ) );
	}
	else
	{
		Remove();
	}
}

// $Barnyard: FUNCTION 00405050
TBOOL AFrontEndArtGalleryState::IsCurrentUnlocked() const
{
	TINT iBit = ( m_eMode != MODE_ART ) ? m_iCurrent + 22 : m_iCurrent + 13;
	return ( APlayerProgress::GetSingleton()->GetUnlockedFlags() & ( 1 << iBit ) ) != 0;
}

// $Barnyard: FUNCTION 00405090
void AFrontEndArtGalleryState::ResetItems()
{
	const TFLOAT fCurrY = m_pItems[ m_iCurrent ].GetTransform().GetTranslation().y;

	T2GUITransform& rPrevTransform = m_pItems[ m_iPrev ].GetTransform();
	rPrevTransform.SetIdentity();
	rPrevTransform.SetTranslation( -ms_fXOffset, fCurrY );

	T2GUITransform oScale;
	oScale.SetFromScale( 0.5f, 0.5f );
	rPrevTransform.PreMultiply( oScale );
	m_pItems[ m_iPrev ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );

	T2GUITransform& rNextTransform = m_pItems[ m_iNext ].GetTransform();
	rNextTransform.SetIdentity();
	rNextTransform.SetTranslation( ms_fXOffset, fCurrY );
	rNextTransform.PreMultiply( oScale );
	m_pItems[ m_iNext ].SetColour( TCOLOR4( 0, 0, 0, 255 ) );

	T2GUITransform& rCurrTransform = m_pItems[ m_iCurrent ].GetTransform();
	rCurrTransform.SetIdentity();
	rCurrTransform.SetTranslation( 0.0f, fCurrY );
	rCurrTransform.Scale( ms_fSelectedScale, ms_fSelectedScale );
	m_pItems[ m_iCurrent ].SetColour( IsCurrentUnlocked() ? TCOLOR4( 255, 255, 255, 255 ) : TCOLOR4( 0, 0, 0, 255 ) );

	if ( m_iOld != -1 )
	{
		m_pItems[ m_iOld ].SetColour( 0 );
		m_pItems[ m_iOld ].Hide();
		m_iOld = -1;
	}

	m_fTotalTime   = 0.0f;
	m_eScrollState = SCROLLSTATE_READY;
}

// $Barnyard: FUNCTION 00405330
void AFrontEndArtGalleryState::UpdateTexts()
{
	const TWCHAR* wszName;
	const TWCHAR* wszDescription;

	if ( IsCurrentUnlocked() )
	{
		if ( m_eMode != MODE_ART )
		{
			wszName        = g_pLocaleManager->GetString( ms_aMovieNames[ m_iCurrent ] );
			wszDescription = g_pLocaleManager->GetString( ms_aMovieDescriptions[ m_iCurrent ] );
		}
		else
		{
			wszName        = g_pLocaleManager->GetString( ms_aArtNames[ m_iCurrent ] );
			wszDescription = g_pLocaleManager->GetString( ms_aArtDescriptions[ m_iCurrent ] );
		}
	}
	else
	{
		// Locked
		wszName        = g_pLocaleManager->GetString( 238 );
		wszDescription = g_pLocaleManager->GetString( 240 );
	}

	m_oName.SetText( wszName );
	m_oDescription.SetText( wszDescription );
}
