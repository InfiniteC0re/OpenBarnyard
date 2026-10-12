#include "pch.h"
#include "AAudioOptionsState.h"
#include "AOptionsState.h"
#include "AFrontEndMainMenuState2.h"
#include "AOptions.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "SoundBank/ui.h"
#include "SoundBank/music.h"
#include "GUI/T2GUIFontManager.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "Locale/ALocaleManager.h"
#include "GUI/AGUI2.h"
#include "HUD/AHudManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "Input/AInputHandler.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AAudioOptionsState );

TPSTRING8_DECLARE( music );

// $Barnyard: FUNCTION 00442a70
AAudioOptionsState::AAudioOptionsState()
{
	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
	{
		m_aVolumes[ i ]         = 0;
		m_aOriginalVolumes[ i ] = 0;
	}

	m_iFocusedItemId              = 0;
	m_bSettingsChangedOnInsertion = TFALSE;
	m_bLoadedMusicSamples         = TFALSE;
	m_iMusicCue                   = -1;
}

// $Barnyard: FUNCTION 00442b50
// $Barnyard: FUNCTION 00443220
AAudioOptionsState::~AAudioOptionsState()
{
}

// $Barnyard: FUNCTION 004435d0
TBOOL AAudioOptionsState::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE )
	{
		if ( m_oGridMenu.ProcessInputEvent( a_pInputEvent ) )
			return TTRUE;

		for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
		{
			if ( m_aVolumeSliders[ i ].ProcessInputEvent( a_pInputEvent ) )
				return TTRUE;
		}
	}

	return BaseClass::ProcessInput( a_pInputEvent );
}

void AAudioOptionsState::ChangeFocusedVolume( TINT a_iDelta )
{
	// NOTE: inlined twice (Left and Right) in AAudioOptionsState::ProcessCommand
	ASoundManager::GetSingleton()->PlayCue( ( m_iFocusedItemId == AUDIOOPTIONS_SPEECH ) ? soundbank::UI_VO_CLICK : soundbank::UI_MENUCLICK );

	AOptions* pOptions = AOptions::GetSingleton();

	if ( m_iFocusedItemId == AUDIOOPTIONS_EFFECTS )
	{
		m_aVolumes[ AUDIOOPTIONS_EFFECTS ] += a_iDelta;
		TMath::Clip( m_aVolumes[ AUDIOOPTIONS_EFFECTS ], 0, 255 );
		pOptions->SetSfxVolume( m_aVolumes[ AUDIOOPTIONS_EFFECTS ] );
	}
	else if ( m_iFocusedItemId == AUDIOOPTIONS_MUSIC )
	{
		m_aVolumes[ AUDIOOPTIONS_MUSIC ] += a_iDelta;
		TMath::Clip( m_aVolumes[ AUDIOOPTIONS_MUSIC ], 0, 255 );
		pOptions->SetMusicVolume( m_aVolumes[ AUDIOOPTIONS_MUSIC ] );
	}
	else if ( m_iFocusedItemId == AUDIOOPTIONS_SPEECH )
	{
		m_aVolumes[ AUDIOOPTIONS_SPEECH ] += a_iDelta;
		TMath::Clip( m_aVolumes[ AUDIOOPTIONS_SPEECH ], 0, 255 );
		pOptions->SetVOVolume( m_aVolumes[ AUDIOOPTIONS_SPEECH ] );
	}

	pOptions->ApplySettings();
	AOptionsState::ms_bSettingsChanged = TTRUE;
}

// $Barnyard: FUNCTION 00443370
TBOOL AAudioOptionsState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE )
	{
		if ( a_eInputCommand == AInputCommand_Cancel )
		{
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );
			m_eMenuState     = MENUSTATE_MENU_DISAPPEAR;
			m_iFocusedItemId = -1;
			return TTRUE;
		}
		else if ( a_eInputCommand == AInputCommand_Left )
		{
			// One bar is 51 (255 / 5)
			ChangeFocusedVolume( -51 );
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		}
		else if ( a_eInputCommand == AInputCommand_Right )
		{
			ChangeFocusedVolume( 51 );
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		}

		if ( m_oGridMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) )
			return TTRUE;
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

void AAudioOptionsState::UpdateBarColours( TINT a_iSlider, TFLOAT a_fDeltaTime )
{
	// NOTE: inlined three times in AAudioOptionsState::OnUpdate
	TINT iNumLitBars = TMath::CeilToInt( m_aVolumes[ a_iSlider ] * ( 1.0f / 51.0f ) );

	for ( TINT i = 0; i < AVolumeSlider::GetNumBars(); i++ )
	{
		T2GUIRectangle& rBar = m_aVolumeSliders[ a_iSlider ].GetBar( i );

		// Fade the RGB components to white (lit) or black, alpha is kept
		TUINT32 uiColour  = rBar.GetColour();
		TINT    aRGB[ 3 ] = {
            TINT( ( uiColour >> 16 ) & 0xFF ),
            TINT( ( uiColour >> 8 ) & 0xFF ),
            TINT( uiColour & 0xFF )
		};

		TFLOAT fTarget = ( i < iNumLitBars ) ? 255.0f : 0.0f;

		for ( TINT k = 0; k < 3; k++ )
			aRGB[ k ] = TINT( TMath::LERPClamped( TFLOAT( aRGB[ k ] ), fTarget, 5.0f * a_fDeltaTime ) );

		rBar.SetColour( ( uiColour & 0xFF000000 ) | ( aRGB[ 0 ] << 16 ) | ( aRGB[ 1 ] << 8 ) | aRGB[ 2 ] );
	}
}

// $Barnyard: FUNCTION 00442bf0
TBOOL AAudioOptionsState::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Grow the focused label and shrink the others back
	AGUI2MenuGridItem* pFocused = m_oGridMenu.GetFocusedMenuItem();

	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
	{
		T2GUITransform& rTransform = m_aTextItems[ i ].GetTransform();

		TFLOAT   fScaleX      = rTransform.GetScaleX();
		TFLOAT   fScaleY      = rTransform.GetScaleY();
		TVector2 vTranslation = rTransform.GetTranslation();
		rTransform.SetIdentity();

		if ( &m_aTextItems[ i ] == pFocused )
		{
			fScaleX = TMath::LERPClamped( fScaleX, 1.1f, a_fDeltaTime * 10.0f );
			fScaleY = TMath::LERPClamped( fScaleY, 1.1f, a_fDeltaTime * 10.0f );
		}
		else
		{
			fScaleX = TMath::LERPClamped( fScaleX, 1.0f, a_fDeltaTime * 20.0f );
			fScaleY = TMath::LERPClamped( fScaleY, 1.0f, a_fDeltaTime * 20.0f );
		}

		T2GUITransform oScale;
		oScale.Set( TVector2( fScaleX, 0.0f ), TVector2( 0.0f, fScaleY ), TVector2::VEC_ZERO );
		rTransform.PreMultiply( oScale );
		rTransform.SetTranslation( vTranslation );
	}

	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
		UpdateBarColours( i, a_fDeltaTime );

	m_oGridMenu.Update( a_fDeltaTime );
	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00443760
void AAudioOptionsState::OnInsertion()
{
	m_bSettingsChangedOnInsertion = AOptionsState::ms_bSettingsChanged;

	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	m_oGridMenu.CreateGrid( AUDIOOPTIONS_NUMOF, 2, 40.0f, 20.0f );
	m_oGridMenu.SetCallbackUserData( this );
	m_oGridMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	m_oGridMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oGridMenu.SetSelectSound( soundbank::UI_MENUOK );
	m_oGridMenu.BlockActivation( TTRUE );

	// $Barnyard: FUNCTION 004448c0
	m_oGridMenu.SetItemActivationCallback( []( void* a_pUserData, AGUI2MenuGridItem* a_pItem ) {
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
		TSTATICCAST( AAudioOptionsState, a_pUserData )->m_eMenuState = MENUSTATE_MENU_DISAPPEAR;
	} );

	// $Barnyard: FUNCTION 00443070
	m_oGridMenu.SetItemFocusCallback( []( void* a_pUserData, AGUI2MenuGridItem* a_pOldFocus, AGUI2MenuGridItem* a_pNewFocus ) {
		TSTATICCAST( AAudioOptionsState, a_pUserData )->OnMenuGridItemFocused( *a_pOldFocus, *a_pNewFocus );
	} );

	// Create sliders
	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
	{
		// Initialise label
		const TWCHAR* wszText = g_pLocaleManager->GetString( AUDIOOPTIONS_TEXTS[ i ] );

		m_aTextItems[ i ].Create( pFont, pFont->GetTextWidth( wszText ), wszText, 10.0f );
		m_aTextItems[ i ].SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		m_aTextItems[ i ].SetEnabledColour( AGUI2STYLE_COLOR_SECONDARY );
		m_aTextItems[ i ].SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		m_aTextItems[ i ].SetFocusedScale( 1.0f );
		m_aTextItems[ i ].OnFocusLost();
		m_aTextItems[ i ].SetId( i );
		m_aTextItems[ i ].SetTextShadow( TTRUE, 0xa0000000, 0.6f );
		m_aTextItems[ i ].SetTextShadowOffset( 5.0f );
		m_oGridMenu.AddMenuGridItem( m_aTextItems[ i ] );

		// Initialise slider
		m_aVolumeSliders[ i ].Create( "left_arrow", "left_arrow2", "Statue_GoldStar" );
		m_aVolumeSliders[ i ].SetEnabled( TFALSE );
		m_aVolumeSliders[ i ].SetId( i );
		m_aVolumeSliders[ i ].SetCallbackUserData( this );

		// $Barnyard: FUNCTION 004436a0
		m_aVolumeSliders[ i ].SetArrowFocusedCallback( []( void* a_pUserData, TINT a_iSliderID ) {
			AAudioOptionsState* pState = TSTATICCAST( AAudioOptionsState, a_pUserData );
			pState->m_oGridMenu.SetFocusAt( &pState->m_aTextItems[ a_iSliderID ] );
			pState->m_iFocusedItemId = a_iSliderID;
		} );
		m_oGridMenu.AddMenuGridItem( m_aVolumeSliders[ i ] );
	}

	// Update side elements
	m_aTextItems[ AUDIOOPTIONS_EFFECTS ].SetSideElements( TNULL, TNULL, &m_aTextItems[ AUDIOOPTIONS_SPEECH ], &m_aTextItems[ AUDIOOPTIONS_MUSIC ] );
	m_aTextItems[ AUDIOOPTIONS_MUSIC ].SetSideElements( TNULL, TNULL, &m_aTextItems[ AUDIOOPTIONS_EFFECTS ], &m_aTextItems[ AUDIOOPTIONS_SPEECH ] );
	m_aTextItems[ AUDIOOPTIONS_SPEECH ].SetSideElements( TNULL, TNULL, &m_aTextItems[ AUDIOOPTIONS_MUSIC ], &m_aTextItems[ AUDIOOPTIONS_EFFECTS ] );

	m_oGridMenu.ReflowChildren();

	// Center text elements
	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
	{
		m_aTextItems[ i ].SetTransform(
		    m_aTextItems[ i ].GetWidth() * 0.5f + m_aTextItems[ i ].GetTransform().GetTranslation().x,
		    m_aTextItems[ i ].GetTransform().GetTranslation().y
		);

		m_aTextItems[ i ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPCENTER );
	}

	m_oDialog.AddChildTail( m_oGridMenu );
	m_oGridMenu.SetFocusAt( &m_aTextItems[ 0 ] );

	m_oDialogTitle.SetText( g_pLocaleManager->GetString( 302 ) );
	m_bFocusedElementBloated = TFALSE;

	// Remember the volumes so they can be restored if the menu is cancelled
	AOptions* pOptions = AOptions::GetSingleton();

	m_aVolumes[ AUDIOOPTIONS_EFFECTS ] = m_aOriginalVolumes[ AUDIOOPTIONS_EFFECTS ] = pOptions->GetSfxVolume();
	m_aVolumes[ AUDIOOPTIONS_MUSIC ] = m_aOriginalVolumes[ AUDIOOPTIONS_MUSIC ] = pOptions->GetMusicVolume();
	m_aVolumes[ AUDIOOPTIONS_SPEECH ] = m_aOriginalVolumes[ AUDIOOPTIONS_SPEECH ] = pOptions->GetVOVolume();

	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
	{
		TINT iNumLitBars = TMath::CeilToInt( m_aVolumes[ i ] * ( 1.0f / 51.0f ) );

		for ( TINT k = 0; k < AVolumeSlider::GetNumBars(); k++ )
			m_aVolumeSliders[ i ].GetBar( k ).SetColour( ( k < iNumLitBars ) ? 0xFFFFFFFF : 0xFF000000 );
	}

	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetTexts( 1050, 1051, -1 );

	BaseClass::OnInsertion();

	m_oDialog.SetDimensions( m_oGridMenu.GetWidth() + 128.0f, m_oGridMenu.GetHeight() + 150.0f );

	if ( !AFrontEndMainMenuState2::ms_bIsInserted )
	{
		// The music samples aren't always loaded in game, they are needed for the music preview
		if ( ASoundManager::GetSingleton()->FindWaveBank( TPS8( music ) )->GetNumLoads() <= 0 )
		{
			AMusicManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );
			ASoundManager::GetSingleton()->LoadWaveBankSamples( TPS8( music ), AWaveBank::LOADFLAGS_NONE, -1 );
			ASoundManager::GetSingleton()->LoadSoundBankSamples( TPS8( music ) );
			m_bLoadedMusicSamples = TTRUE;
		}

		m_iMusicCue = ASoundManager::GetSingleton()->PlayCueEx( soundbank::MUSIC_MISSION, 1.0f, TTRUE, 0.0f, -1 );
	}
}

// $Barnyard: FUNCTION 00443240
void AAudioOptionsState::OnRemoval()
{
	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
	{
		m_aTextItems[ i ].RemoveSelf();
		m_aVolumeSliders[ i ].RemoveElements();
	}

	m_oGridMenu.RemoveSelf();

	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetTexts( 1047, 1048, -1 );

	if ( !AFrontEndMainMenuState2::ms_bIsInserted )
	{
		if ( m_iMusicCue != -1 )
			ASoundManager::GetSingleton()->StopCue( m_iMusicCue );

		if ( m_bLoadedMusicSamples == TTRUE )
		{
			ASoundManager::GetSingleton()->UnloadWaveBankSamples( TPS8( music ) );
			AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
		}
	}

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00442be0
void AAudioOptionsState::OnActivate()
{
	m_iFocusedItemId = 0;
	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 00443630
void AAudioOptionsState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );
}

// $Barnyard: FUNCTION 00443060
void AAudioOptionsState::OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus )
{
	m_iFocusedItemId = TSTATICCAST( AGUI2Button, &a_rNewFocus )->GetId();
}

// $Barnyard: FUNCTION 004436d0
void AAudioOptionsState::OnMenuClose()
{
	if ( m_iFocusedItemId == -1 )
	{
		// Cancelled, restore the volumes
		if ( m_aOriginalVolumes[ AUDIOOPTIONS_EFFECTS ] != m_aVolumes[ AUDIOOPTIONS_EFFECTS ] ||
		     m_aOriginalVolumes[ AUDIOOPTIONS_MUSIC ] != m_aVolumes[ AUDIOOPTIONS_MUSIC ] ||
		     m_aOriginalVolumes[ AUDIOOPTIONS_SPEECH ] != m_aVolumes[ AUDIOOPTIONS_SPEECH ] )
		{
			AOptions* pOptions = AOptions::GetSingleton();
			pOptions->SetSfxVolume( m_aOriginalVolumes[ AUDIOOPTIONS_EFFECTS ] );
			pOptions->SetMusicVolume( m_aOriginalVolumes[ AUDIOOPTIONS_MUSIC ] );
			pOptions->SetVOVolume( m_aOriginalVolumes[ AUDIOOPTIONS_SPEECH ] );
			pOptions->ApplySettings();
		}

		AOptionsState::ms_bSettingsChanged = m_bSettingsChangedOnInsertion;
	}

	BaseClass::Remove();
}

// $Barnyard: FUNCTION 00443660
void AAudioOptionsState::OnMenuGridItemFocused( AGUI2MenuGridItem& a_rOldFocus, AGUI2MenuGridItem& a_rNewFocus )
{
	// NOTE: only the labels can be focused (the sliders are disabled), both keep their id at 0x70
	m_iFocusedItemId = TSTATICCAST( AGUI2MenuGridTextItem, &a_rNewFocus )->GetId();

	// The music preview is only heard while the music volume is focused
	if ( m_iMusicCue != -1 )
		ASoundManager::GetSingleton()->PauseCue( m_iMusicCue, m_iFocusedItemId != AUDIOOPTIONS_MUSIC );
}

// $Barnyard: FUNCTION 00443170
TBOOL AVolumeSlider::ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
	{
		// Arrows are hit-tested in OnUpdate
		m_bMouseDirty = TTRUE;
		return TFALSE;
	}

	if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN ||
	     a_pInputEvent->GetDoodad() != TInputDeviceMouse::BUTTON_1 ||
	     m_iLastPressedArrowIndex == -1 )
		return TFALSE;

	// Clicking an arrow emulates a press of the Left/Right key
	TInputDeviceKeyboard* pKeyboard = AInputHandler::GetSingleton()->GetKeyboardDevice();
	TINT                  iKey      = ( m_iLastPressedArrowIndex == 0 ) ? TInputDeviceKeyboard::KEY_LEFT : TInputDeviceKeyboard::KEY_RIGHT;

	TInputInterface::GetSingleton()->SendEvent( pKeyboard, iKey, TInputInterface::EVENT_TYPE_GONE_DOWN );
	TInputInterface::GetSingleton()->SendEvent( pKeyboard, iKey, TInputInterface::EVENT_TYPE_GONE_UP );
	return TTRUE;
}

// $Barnyard: FUNCTION 00442720
AVolumeSlider::AVolumeSlider()
{
	m_bMouseDirty            = TFALSE;
	m_iLastPressedArrowIndex = -1;
}

// $Barnyard: FUNCTION 004427c0
// $Barnyard: FUNCTION 00443090
AVolumeSlider::~AVolumeSlider()
{
}

// $Barnyard: FUNCTION 00442840
TBOOL AVolumeSlider::Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TCHAR* a_szBarIcon )
{
	// Setup left arrow
	T2GUITextureSection* pLeftArrow = T2GUITextureSectionManager::GetTextureSection( a_szLeftArrow );
	TVALIDPTR( pLeftArrow );

	m_aArrows[ 0 ].SetDimensions( pLeftArrow->GetWidth(), pLeftArrow->GetHeight() );
	m_aArrows[ 0 ].SetTextureSection( pLeftArrow );
	m_aArrows[ 0 ].SetTransform( 0.0f, 0.0f );
	m_aArrows[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	AddChildTail( m_aArrows[ 0 ] );

	// Setup bar icons
	T2GUITextureSection* pBarIcon = T2GUITextureSectionManager::GetTextureSection( a_szBarIcon );
	TVALIDPTR( pBarIcon );

	TFLOAT fBarWidth  = pBarIcon->GetWidth();
	TFLOAT fBarHeight = pBarIcon->GetHeight();
	TFLOAT fBarX      = pLeftArrow->GetWidth();

	for ( TINT i = 0; i < TARRAYSIZE( m_aBars ); i++ )
	{
		m_aBars[ i ].SetDimensions( fBarWidth, fBarHeight );
		m_aBars[ i ].SetTextureSection( pBarIcon );
		m_aBars[ i ].SetTransform( fBarX, 0.0f );
		m_aBars[ i ].GetTransform().Scale( ms_fBarScale, ms_fBarScale );
		m_aBars[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		AddChildTail( m_aBars[ i ] );

		fBarX += ms_fBarScale * fBarWidth;
	}

	// Setup right arrow
	T2GUITextureSection* pRightArrow = T2GUITextureSectionManager::GetTextureSection( a_szRightArrow );
	TVALIDPTR( pRightArrow );

	m_aArrows[ 1 ].SetDimensions( pRightArrow->GetWidth(), pRightArrow->GetHeight() );
	m_aArrows[ 1 ].SetTextureSection( pRightArrow );
	m_aArrows[ 1 ].SetTransform( fBarX, 0.0f );
	m_aArrows[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	AddChildTail( m_aArrows[ 1 ] );

	SetDimensions( fBarX + pRightArrow->GetWidth(), m_aBars[ 0 ].GetHeight() * ms_fBarScale );
	return TTRUE;
}

// NOTE: folded with AGUI2MenuItem::SetAlpha (0063cb00)
void AVolumeSlider::SetAlpha( TFLOAT a_fAlpha )
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
		it->SetAlpha( a_fAlpha );

	T2GUIElement::SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 004430b0
void AVolumeSlider::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bMouseDirty )
	{
		// Check if any of the arrow rectangles was pressed
		m_iLastPressedArrowIndex = -1;
		T2GUITransform oElementInvTransform;

		TVector2 vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();

		for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
		{
			// Get inverse transform of the current UI element
			m_aArrows[ i ].GetInvScreenTransform( oElementInvTransform );

			// Get cursor position that is relative to the UI element
			TVector2 vecElementRelCursorPos;
			oElementInvTransform.Transform( vecElementRelCursorPos, vecCursorPos );

			if ( m_aArrows[ i ].IsPointInside( vecElementRelCursorPos.x, vecElementRelCursorPos.y ) )
			{
				TVALIDPTR( m_fnArrowFocusCallback );
				m_fnArrowFocusCallback( m_pCallbackUserData, m_iID );
				m_iLastPressedArrowIndex = i;
			}
		}

		m_bMouseDirty = TFALSE;
	}
}
