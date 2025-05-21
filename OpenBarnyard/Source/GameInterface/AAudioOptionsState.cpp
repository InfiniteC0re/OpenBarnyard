#include "pch.h"
#include "AAudioOptionsState.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "GUI/AGUI2FontManager.h"
#include "GUI/AGUI2TextureSectionManager.h"
#include "Locale/ALocaleManager.h"
#include "GUI/AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AAudioOptionsState );

// $Barnyard: FUNCTION 00442a70
AAudioOptionsState::AAudioOptionsState()
{
}

AAudioOptionsState::~AAudioOptionsState()
{
}

// $Barnyard: FUNCTION 004435d0
TBOOL AAudioOptionsState::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	TIMPLEMENT();

	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE )
	{
		if ( m_oGridMenu.ProcessInputEvent( a_pInputEvent ) )
			return TTRUE;
	}

	return BaseClass::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 00443370
TBOOL AAudioOptionsState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	TIMPLEMENT();

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
			a_pInputEvent->StartRepeat();
			return TTRUE;
		}
		else if ( a_eInputCommand == AInputCommand_Right )
		{
			a_pInputEvent->StartRepeat();
			return TTRUE;
		}

		if ( m_oGridMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) )
			return TTRUE;
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00443760
void AAudioOptionsState::OnInsertion()
{
	TIMPLEMENT();

	AGUI2Font* pFont = AGUI2FontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	m_oGridMenu.CreateGrid( AUDIOOPTIONS_NUMOF, 2, 40.0f, 20.0f );
	m_oGridMenu.SetCallbackUserData( this );
	m_oGridMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	m_oGridMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oGridMenu.SetSelectSound( soundbank::UI_MENUOK );
	m_oGridMenu.BlockActivation( TTRUE );

	m_oGridMenu.SetItemActivationCallback( []( void* a_pUserData, AGUI2MenuGridItem* a_pItem ) {
		TSTATICCAST( AAudioOptionsState, a_pUserData )->m_eMenuState = MENUSTATE_MENU_DISAPPEAR;
	} );
	m_oGridMenu.SetItemFocusCallback( []( void* a_pUserData, AGUI2MenuGridItem* a_pOldFocus, AGUI2MenuGridItem* a_pNewFocus ) {
		TSTATICCAST( AAudioOptionsState, a_pUserData )->OnMenuGridItemFocused( *a_pOldFocus, *a_pNewFocus );
	} );

	// Create sliders
	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
	{
		// Initialise label
		const TWCHAR* wszText = ALocaleManager::GetSingleton()->GetString( AUDIOOPTIONS_TEXTS[ i ] );

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

		m_aTextItems[ i ].SetAttachment( AGUI2ATTACHMENT_TOPLEFT, AGUI2ATTACHMENT_TOPCENTER );
	}

	// ...
	m_oDialog.AddChildTail( m_oGridMenu );
	m_oGridMenu.SetFocusAt( &m_aTextItems[ 0 ] );

	m_oDialogTitle.SetText( ALocaleManager::GetSingleton()->GetString( 302 ) );
	m_bFocusedElementBloated = TFALSE;

	BaseClass::OnInsertion();

	m_oDialog.SetDimensions( m_oGridMenu.GetWidth() + 128.0f, m_oGridMenu.GetHeight() + 150.0f );
}

// $Barnyard: FUNCTION 00443240
void AAudioOptionsState::OnRemoval()
{
	TIMPLEMENT();
	for ( TINT i = 0; i < AUDIOOPTIONS_NUMOF; i++ )
	{
		m_aTextItems[ i ].RemoveSelf();
	}

	m_oGridMenu.RemoveSelf();
	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00443630
void AAudioOptionsState::OnMenuItemActivated( AGUI2MenuItem& a_rMenuItem )
{
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, &a_rMenuItem ) );
}

// $Barnyard: FUNCTION 00443060
void AAudioOptionsState::OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 004436d0
void AAudioOptionsState::OnMenuClose()
{
	TIMPLEMENT();

	BaseClass::Remove();
}

// $Barnyard: FUNCTION 00443660
void AAudioOptionsState::OnMenuGridItemFocused( AGUI2MenuGridItem& a_rOldFocus, AGUI2MenuGridItem& a_rNewFocus )
{
	TIMPLEMENT();
	m_iFocusedItemId = -1;
}

// $Barnyard: FUNCTION 00442720
AVolumeSlider::AVolumeSlider()
{
	m_bMouseDirty            = TFALSE;
	m_iLastPressedArrowIndex = -1;
}

// $Barnyard: FUNCTION 004427c0
AVolumeSlider::~AVolumeSlider()
{
}

// $Barnyard: FUNCTION 00442840
TBOOL AVolumeSlider::Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TCHAR* a_szBarIcon )
{
	// Setup left arrow
	AGUI2TextureSection* pLeftArrow = AGUI2TextureSectionManager::GetTextureSection( a_szLeftArrow );
	TVALIDPTR( pLeftArrow );

	m_aArrows[ 0 ].SetDimensions( pLeftArrow->GetWidth(), pLeftArrow->GetHeight() );
	m_aArrows[ 0 ].SetTextureSection( pLeftArrow );
	m_aArrows[ 0 ].SetTransform( 0.0f, 0.0f );
	m_aArrows[ 0 ].SetAttachment( AGUI2ATTACHMENT_MIDDLELEFT, AGUI2ATTACHMENT_MIDDLELEFT );
	AddChildTail( m_aArrows[ 0 ] );

	// Setup bar icons
	AGUI2TextureSection* pBarIcon = AGUI2TextureSectionManager::GetTextureSection( a_szBarIcon );
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
		m_aBars[ i ].SetAttachment( AGUI2ATTACHMENT_MIDDLELEFT, AGUI2ATTACHMENT_MIDDLELEFT );
		AddChildTail( m_aBars[ i ] );

		fBarX += ms_fBarScale * fBarWidth;
	}

	// Setup right arrow
	AGUI2TextureSection* pRightArrow = AGUI2TextureSectionManager::GetTextureSection( a_szRightArrow );
	TVALIDPTR( pRightArrow );

	m_aArrows[ 1 ].SetDimensions( pRightArrow->GetWidth(), pRightArrow->GetHeight() );
	m_aArrows[ 1 ].SetTextureSection( pRightArrow );
	m_aArrows[ 1 ].SetTransform( fBarX, 0.0f );
	m_aArrows[ 1 ].SetAttachment( AGUI2ATTACHMENT_MIDDLELEFT, AGUI2ATTACHMENT_MIDDLELEFT );
	AddChildTail( m_aArrows[ 1 ] );

	SetDimensions( fBarX + pRightArrow->GetWidth(), m_aBars[ 0 ].GetHeight() * ms_fBarScale );
	return TTRUE;
}

// $Barnyard: FUNCTION 004430b0
void AVolumeSlider::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bMouseDirty )
	{
		// Check if any of the arrow rectangles was pressed
		m_iLastPressedArrowIndex = -1;
		AGUI2Transform oElementInvTransform;

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
