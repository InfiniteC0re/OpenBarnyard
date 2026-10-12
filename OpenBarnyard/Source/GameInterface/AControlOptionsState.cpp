#include "pch.h"
#include "AControlOptionsState.h"
#include "AOptionsState.h"
#include "AOptions.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
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

TDEFINE_CLASS( AControlOptionsState );

// $Barnyard: FUNCTION 004447c0
AControlOptionsState::AControlOptionsState()
{
	m_iFocusedItemId = 0;

	// The settings are only applied once the menu is closed
	m_bInvertMouse = AOptions::GetSingleton()->IsMouseInverted();
	m_fSensitivity = AOptions::GetSingleton()->GetSensitivity();
}

// $Barnyard: FUNCTION 00443f00
// $Barnyard: FUNCTION 004443e0
AControlOptionsState::~AControlOptionsState()
{
}

// $Barnyard: FUNCTION 004446b0
TBOOL AControlOptionsState::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE )
	{
		if ( m_oGridMenu.ProcessInputEvent( a_pInputEvent ) )
			return TTRUE;

		for ( TINT i = 0; i < NUM_TOGGLES; i++ )
		{
			if ( m_aToggleSliders[ i ].ProcessInputEvent( a_pInputEvent ) )
				return TTRUE;
		}

		if ( m_oSensitivitySlider.GetSlider().ProcessInputEvent( a_pInputEvent ) )
			return TTRUE;
	}

	return BaseClass::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 004444e0
TBOOL AControlOptionsState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( a_eInputCommand == AInputCommand_Cancel )
	{
		// Cancelled, the changes are discarded
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );
		m_eMenuState     = MENUSTATE_MENU_DISAPPEAR;
		m_iFocusedItemId = -1;
		return TTRUE;
	}

	if ( m_iFocusedItemId == CONTROLOPTIONS_SENSITIVITY )
	{
		// NOTE: the sensitivity changes by 0.1f and the bounds are checked on the accumulated float. From the default 0.5
		// it goes 0.4, 0.3, 0.2, 0.10000002 and then to ~0.0 (mouse look stops working), and up to 1.0000001
		if ( a_eInputCommand == AInputCommand_Left )
		{
			if ( m_fSensitivity > 0.1f )
			{
				m_fSensitivity -= 0.1f;
				ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
				m_oSensitivitySlider.GetSlider().SetValue( m_fSensitivity );
				a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			}

			return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
		}
		else if ( a_eInputCommand == AInputCommand_Right )
		{
			if ( m_fSensitivity < 1.0f )
			{
				m_fSensitivity += 0.1f;
				ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
				m_oSensitivitySlider.GetSlider().SetValue( m_fSensitivity );
				a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			}

			return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
		}
	}
	else if ( m_iFocusedItemId == CONTROLOPTIONS_INVERTMOUSE && ( a_eInputCommand == AInputCommand_Left || a_eInputCommand == AInputCommand_Right ) )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );

		// Switch between "On" and "Off"
		m_bInvertMouse = !m_bInvertMouse;
		m_aToggleSliders[ 0 ].SetText( g_pLocaleManager->GetString( m_bInvertMouse ? 1097 : 1096 ) );

		a_pInputEvent->StartRepeat( 0.5f, 0.1f );
		return TTRUE;
	}

	if ( a_eInputCommand != AInputCommand_Left && a_eInputCommand != AInputCommand_Right && m_oGridMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) )
		return TTRUE;

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

void AControlOptionsState::UpdateLabelScale( AGUI2MenuGridTextItem& a_rLabel, TFLOAT a_fDeltaTime )
{
	// NOTE: inlined twice in AControlOptionsState::OnUpdate
	T2GUITransform& rTransform = a_rLabel.GetTransform();

	TFLOAT   fScaleX      = rTransform.GetScaleX();
	TFLOAT   fScaleY      = rTransform.GetScaleY();
	TVector2 vTranslation = rTransform.GetTranslation();
	rTransform.SetIdentity();

	// Grow the focused label and shrink the others back
	if ( &a_rLabel == m_oGridMenu.GetFocusedMenuItem() )
	{
		fScaleX = TMath::LERPClamped( fScaleX, ms_fFocusedLabelScale, a_fDeltaTime * 10.0f );
		fScaleY = TMath::LERPClamped( fScaleY, ms_fFocusedLabelScale, a_fDeltaTime * 10.0f );
	}
	else
	{
		fScaleX = TMath::LERPClamped( fScaleX, ms_fLabelScale, a_fDeltaTime * 20.0f );
		fScaleY = TMath::LERPClamped( fScaleY, ms_fLabelScale, a_fDeltaTime * 20.0f );
	}

	T2GUITransform oScale;
	oScale.Set( TVector2( fScaleX, 0.0f ), TVector2( 0.0f, fScaleY ), TVector2::VEC_ZERO );
	rTransform.PreMultiply( oScale );
	rTransform.SetTranslation( vTranslation );
}

// $Barnyard: FUNCTION 00443fc0
TBOOL AControlOptionsState::OnUpdate( TFLOAT a_fDeltaTime )
{
	UpdateLabelScale( m_aToggleTexts[ 0 ], a_fDeltaTime );
	UpdateLabelScale( m_oSensitivityText, a_fDeltaTime );

	m_oGridMenu.Update( a_fDeltaTime );
	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004448f0
void AControlOptionsState::OnInsertion()
{
	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	m_oGridMenu.CreateGrid( 2, 2, 40.0f, 20.0f );
	m_oGridMenu.SetCallbackUserData( this );
	m_oGridMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	m_oGridMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oGridMenu.SetSelectSound( soundbank::UI_MENUOK );
	m_oGridMenu.BlockActivation( TTRUE );

	// NOTE: same code as the AAudioOptionsState callback (004448c0)
	m_oGridMenu.SetItemActivationCallback( []( void* a_pUserData, AGUI2MenuGridItem* a_pItem ) {
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
		TSTATICCAST( AControlOptionsState, a_pUserData )->m_eMenuState = MENUSTATE_MENU_DISAPPEAR;
	} );

	// NOTE: same code as the AAudioOptionsState callback (00443070)
	m_oGridMenu.SetItemFocusCallback( []( void* a_pUserData, AGUI2MenuGridItem* a_pOldFocus, AGUI2MenuGridItem* a_pNewFocus ) {
		TSTATICCAST( AControlOptionsState, a_pUserData )->OnMenuGridItemFocused( *a_pOldFocus, *a_pNewFocus );
	} );

	m_aToggleSliders[ 0 ].Create( "left_arrow", "left_arrow2", g_pLocaleManager->GetString( m_bInvertMouse ? 1097 : 1096 ) );

	// Shrink the labels if they are too wide
	TFLOAT fMaxTextWidth = 0.0f;
	fMaxTextWidth        = TMath::Max( fMaxTextWidth, pFont->GetTextWidth( g_pLocaleManager->GetString( 305 ), 1.0f ) );
	fMaxTextWidth        = TMath::Max( fMaxTextWidth, pFont->GetTextWidth( g_pLocaleManager->GetString( 286 ), 1.0f ) );

	ms_fLabelScale        = ( fMaxTextWidth >= ms_fMaxLabelWidth ) ? ms_fMaxLabelWidth / fMaxTextWidth : 1.0f;
	ms_fFocusedLabelScale = ms_fLabelScale * 1.1f;

	// $Barnyard: FUNCTION 00444720
	auto fnArrowFocused = []( void* a_pUserData, TINT a_iSliderID ) {
		AControlOptionsState* pState = TSTATICCAST( AControlOptionsState, a_pUserData );

		if ( a_iSliderID >= NUM_TOGGLES )
			pState->m_oGridMenu.SetFocusAt( &pState->m_oSensitivityText );
		else
			pState->m_oGridMenu.SetFocusAt( &pState->m_aToggleTexts[ a_iSliderID ] );

		pState->m_iFocusedItemId = a_iSliderID;
	};

	auto fnCreateLabel = [ pFont ]( AGUI2MenuGridTextItem& a_rLabel, const TWCHAR* a_wszText, TINT a_iID ) {
		// NOTE: inlined twice in AControlOptionsState::OnInsertion
		a_rLabel.GetTextBox().SetScale( ms_fLabelScale );
		a_rLabel.Create( pFont, pFont->GetTextWidth( a_wszText, ms_fLabelScale ), a_wszText, 10.0f );
		a_rLabel.SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		a_rLabel.SetEnabledColour( AGUI2STYLE_COLOR_SECONDARY );
		a_rLabel.SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		a_rLabel.SetFocusedScale( 1.0f );
		a_rLabel.OnFocusLost();
		a_rLabel.SetId( a_iID );
		a_rLabel.SetTextShadow( TTRUE, 0xa0000000, 0.6f );
		a_rLabel.SetTextShadowOffset( 5.0f );
	};

	// Invert mouse
	fnCreateLabel( m_aToggleTexts[ 0 ], g_pLocaleManager->GetString( 305 ), CONTROLOPTIONS_INVERTMOUSE );
	m_oGridMenu.AddMenuGridItem( m_aToggleTexts[ 0 ] );

	m_aToggleSliders[ 0 ].SetEnabled( TFALSE );
	m_aToggleSliders[ 0 ].SetId( CONTROLOPTIONS_INVERTMOUSE );
	m_aToggleSliders[ 0 ].SetCallbackUserData( this );
	m_aToggleSliders[ 0 ].SetArrowFocusedCallback( fnArrowFocused );
	m_oGridMenu.AddMenuGridItem( m_aToggleSliders[ 0 ] );

	// Mouse sensitivity
	fnCreateLabel( m_oSensitivityText, g_pLocaleManager->GetString( 286 ), CONTROLOPTIONS_SENSITIVITY );
	m_oGridMenu.AddMenuGridItem( m_oSensitivityText );

	AGUI2Slider& rSlider = m_oSensitivitySlider.GetSlider();
	rSlider.Create( "left_arrow", "left_arrow2", "Statue_GoldStar", 5 );
	m_oSensitivitySlider.SetDimensions( rSlider.GetWidth(), rSlider.GetHeight() );
	m_oSensitivitySlider.AddChildTail( rSlider );
	rSlider.SetValue( m_fSensitivity );

	m_oSensitivitySlider.SetEnabled( TFALSE );
	rSlider.SetId( CONTROLOPTIONS_SENSITIVITY );
	rSlider.SetCallbackUserData( this );
	rSlider.SetArrowFocusedCallback( fnArrowFocused );
	m_oGridMenu.AddMenuGridItem( m_oSensitivitySlider );

	m_aToggleTexts[ 0 ].SetSideElements( TNULL, TNULL, &m_oSensitivityText, &m_oSensitivityText );
	m_oSensitivityText.SetSideElements( TNULL, TNULL, &m_aToggleTexts[ 0 ], &m_aToggleTexts[ 0 ] );
	m_oGridMenu.SetFocusAt( &m_aToggleTexts[ 0 ] );

	// The second column is sized relatively to the first one
	TFLOAT fMaxLabelWidth  = 0.0f;
	TFLOAT fMaxWidgetWidth = 0.0f;
	fMaxLabelWidth         = TMath::Max( fMaxLabelWidth, m_aToggleTexts[ 0 ].GetWidth() );
	fMaxWidgetWidth        = TMath::Max( fMaxWidgetWidth, m_aToggleSliders[ 0 ].GetWidth() );
	fMaxLabelWidth         = TMath::Max( fMaxLabelWidth, m_oSensitivityText.GetWidth() );
	fMaxWidgetWidth        = TMath::Max( fMaxWidgetWidth, m_oSensitivitySlider.GetWidth() );

	if ( fMaxLabelWidth <= fMaxWidgetWidth )
		m_oGridMenu.SetColumnWidth( 1, fMaxLabelWidth / fMaxWidgetWidth );
	else
		m_oGridMenu.SetColumnWidth( 1, fMaxWidgetWidth / fMaxLabelWidth );

	m_oGridMenu.ReflowChildren();

	// Center the labels in the first column
	m_aToggleTexts[ 0 ].SetTransform(
	    fMaxLabelWidth * 0.5f + m_aToggleTexts[ 0 ].GetTransform().GetTranslation().x,
	    m_aToggleTexts[ 0 ].GetTransform().GetTranslation().y
	);
	m_aToggleTexts[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPCENTER );

	m_oSensitivityText.SetTransform(
	    fMaxLabelWidth * 0.5f + m_oSensitivityText.GetTransform().GetTranslation().x,
	    m_oSensitivityText.GetTransform().GetTranslation().y
	);
	m_oSensitivityText.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPCENTER );

	m_oDialog.AddChildTail( m_oGridMenu );

	m_oDialogTitle.SetText( g_pLocaleManager->GetString( 285 ) );
	m_bFocusedElementBloated = TFALSE;

	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetTexts( 1050, 1051, -1 );

	BaseClass::OnInsertion();

	m_oDialog.SetDimensions( m_oGridMenu.GetWidth() + 128.0f, m_oGridMenu.GetHeight() + 150.0f );
}

// $Barnyard: FUNCTION 00444400
void AControlOptionsState::OnRemoval()
{
	for ( TINT i = 0; i < NUM_TOGGLES; i++ )
		m_aToggleSliders[ i ].RemoveElements();

	for ( TINT i = 0; i < NUM_TOGGLES; i++ )
		m_aToggleTexts[ i ].RemoveSelf();

	m_oGridMenu.RemoveSelf();
	m_oSensitivityText.RemoveSelf();
	m_oSensitivitySlider.GetSlider().RemoveElements();
	m_oSensitivitySlider.GetSlider().RemoveSelf();
	m_oSensitivitySlider.RemoveSelf();

	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetTexts( 1047, 1048, -1 );

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00443fb0
void AControlOptionsState::OnActivate()
{
	m_iFocusedItemId = 0;
	BaseClass::OnActivate();
}

// NOTE: folded with AAudioOptionsState::OnMenuItemActivated (00443630)
void AControlOptionsState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );
}

// $Barnyard: FUNCTION 00444210
void AControlOptionsState::OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus )
{
	m_iFocusedItemId = TSTATICCAST( AGUI2Button, &a_rNewFocus )->GetId();
}

// $Barnyard: FUNCTION 00444760
void AControlOptionsState::OnMenuClose()
{
	if ( m_iFocusedItemId != -1 )
	{
		// Closed with OK, apply the new settings
		AOptions* pOptions = AOptions::GetSingleton();
		pOptions->SetMouseInverted( m_bInvertMouse );
		pOptions->SetSensitivity( m_fSensitivity );
		pOptions->ApplySettings();

		AOptionsState::ms_bSettingsChanged = TTRUE;
	}

	BaseClass::Remove();
}

// $Barnyard: FUNCTION 00444220
void AControlOptionsState::OnMenuGridItemFocused( AGUI2MenuGridItem& a_rOldFocus, AGUI2MenuGridItem& a_rNewFocus )
{
	// NOTE: only the labels can be focused (the sliders are disabled), all of them keep their id at 0x70
	m_iFocusedItemId = TSTATICCAST( AGUI2MenuGridTextItem, &a_rNewFocus )->GetId();
}

// $Barnyard: FUNCTION 00443bf0
AToggleSlider::AToggleSlider()
{
	m_bMouseDirty            = TFALSE;
	m_iLastPressedArrowIndex = -1;
}

// $Barnyard: FUNCTION 00443c70
// $Barnyard: FUNCTION 00444230
AToggleSlider::~AToggleSlider()
{
}

// $Barnyard: FUNCTION 00443ce0
TBOOL AToggleSlider::Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TWCHAR* a_wszText )
{
	TFLOAT fHeight = 0.0f;

	// Setup left arrow (the sections are square)
	T2GUITextureSection* pLeftArrow = T2GUITextureSectionManager::GetTextureSection( a_szLeftArrow );
	TFLOAT               fLeftSize  = pLeftArrow->GetWidth();

	m_aArrows[ 0 ].SetDimensions( fLeftSize, fLeftSize );
	m_aArrows[ 0 ].SetTextureSection( pLeftArrow );
	m_aArrows[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	AddChildTail( m_aArrows[ 0 ] );

	if ( fLeftSize > 0.0f )
		fHeight = fLeftSize;

	// Setup the text, it's a bit wider than the text itself
	m_oTextBox.Create( T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY ), 0.0f );
	m_oTextBox.SetTextAndFitWidth( a_wszText );
	m_oTextBox.SetWidth( m_oTextBox.GetWidth() * 1.35f );
	m_oTextBox.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTextBox.SetColour( AGUI2STYLE_COLOR_PRIMARY );
	AddChildTail( m_oTextBox );

	TFLOAT fWidth = m_oTextBox.GetWidth() + fLeftSize;

	// Setup right arrow
	T2GUITextureSection* pRightArrow = T2GUITextureSectionManager::GetTextureSection( a_szRightArrow );
	TFLOAT               fRightSize  = pRightArrow->GetWidth();

	m_aArrows[ 1 ].SetDimensions( fRightSize, fRightSize );
	m_aArrows[ 1 ].SetTextureSection( pRightArrow );
	m_aArrows[ 1 ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	AddChildTail( m_aArrows[ 1 ] );

	fWidth += fRightSize;

	if ( fRightSize > fHeight )
		fHeight = fRightSize;

	SetDimensions( fWidth, fHeight );
	return TTRUE;
}

// $Barnyard: FUNCTION 00444310
TBOOL AToggleSlider::ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
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
	TINT                  iKey      = ( m_iLastPressedArrowIndex != 0 ) ? TInputDeviceKeyboard::KEY_RIGHT : TInputDeviceKeyboard::KEY_LEFT;

	TInputInterface::GetSingleton()->SendEvent( pKeyboard, iKey, TInputInterface::EVENT_TYPE_GONE_DOWN );
	TInputInterface::GetSingleton()->SendEvent( pKeyboard, iKey, TInputInterface::EVENT_TYPE_GONE_UP );
	return TTRUE;
}

// $Barnyard: FUNCTION 00444250
void AToggleSlider::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bMouseDirty == TTRUE )
	{
		// Check if any of the arrow rectangles is hovered
		m_iLastPressedArrowIndex = -1;
		T2GUITransform oElementInvTransform;

		TVector2 vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();

		for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
		{
			// Get cursor position that is relative to the arrow
			m_aArrows[ i ].GetInvScreenTransform( oElementInvTransform );

			TVector2 vecElementRelCursorPos;
			oElementInvTransform.Transform( vecElementRelCursorPos, vecCursorPos );

			if ( m_aArrows[ i ].IsPointInside( vecElementRelCursorPos.x, vecElementRelCursorPos.y ) )
			{
				m_fnArrowFocusCallback( m_pCallbackUserData, m_iID );
				m_iLastPressedArrowIndex = i;
			}
		}

		m_bMouseDirty = TFALSE;
	}

	AGUI2MenuGridItem::OnUpdate( a_fDeltaTime );
}

// NOTE: folded with AGUI2MenuItem::SetAlpha (0063cb00)
void AToggleSlider::SetAlpha( TFLOAT a_fAlpha )
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
		it->SetAlpha( a_fAlpha );

	T2GUIElement::SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 00443e80
// $Barnyard: FUNCTION 004443c0
ACameraSlider::~ACameraSlider()
{
}

// $Barnyard: FUNCTION 00443ee0
void ACameraSlider::SetAlpha( TFLOAT a_fAlpha )
{
	m_oSlider.SetAlpha( a_fAlpha );
	T2GUIElement::SetAlpha( a_fAlpha );
}
