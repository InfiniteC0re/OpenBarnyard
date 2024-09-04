#include "pch.h"
#include "ABYardMenuState.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2FontManager.h"
#include "GUI/AGUI2TextureSectionManager.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ABYardMenuState )

ABYardMenuState::ABYardMenuState()
{
	m_eMenuState             = MENUSTATE_NOTHING;
	m_fOscillatorOffset      = 0.0f;
	m_fMenuOpacity           = 0;
	m_fTotalTime             = 0.0f;
	m_bFocusedElementBloated = TTRUE;
	m_bIgnoreInputs          = TFALSE;
	m_pButtonRotations       = TNULL;
	m_bHasRectangle2         = TFALSE;
}

ABYardMenuState::~ABYardMenuState()
{
	if ( m_pButtonRotations )
		delete[] m_pButtonRotations;
}

TBOOL ABYardMenuState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && !m_bIgnoreInputs )
	{
		if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
		{
			// Make sure menu updates mouse state
			m_oMenu.SetMouseStateDirty();
		}
		else if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT &&
		          a_pInputEvent->GetDoodad() == TInputDeviceMouse::BUTTON_1 &&
		          m_oMenu.GetHoveredMenuItem() != TNULL )
		{
			// LMB was pressed and the menu has a hovered item
			return m_oMenu.TriggerButtonPress( *m_oMenu.GetHoveredMenuItem() );
		}

		return TFALSE;
	}

	return BaseClass::ProcessInput( a_pInputEvent );
}

TBOOL ABYardMenuState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && !m_bIgnoreInputs )
	{
		if ( a_eInputCommand == AInputCommand_Cancel )
			m_eMenuState = MENUSTATE_MENU_DISAPPEAR;
		else if ( !m_oMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) )
			return TFALSE;

		return TTRUE;
	}

	return TFALSE;
}

static TFLOAT s_fOscillatorPeriod    = TMath::PI / 1.5f;
static TFLOAT s_fOscillatorAmplitude = 2.0f - TMath::Sin( s_fOscillatorPeriod );

TBOOL ABYardMenuState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_fTotalTime += a_fDeltaTime;

	switch ( m_eMenuState )
	{
		case MENUSTATE_DIALOG_APPEAR:
			// Dialog appear animation
			m_fOscillatorOffset += a_fDeltaTime * 6.0f;

			if ( m_fOscillatorOffset <= s_fOscillatorPeriod )
			{
				TFLOAT fScale = s_fOscillatorAmplitude * TMath::Sin( m_fOscillatorOffset );
				m_oDialog.GetTransform().SetFromScale( fScale, fScale );
			}
			else
			{
				m_fOscillatorOffset = s_fOscillatorPeriod;
				m_eMenuState        = MENUSTATE_MENU_APPEAR;
				m_fMenuOpacity      = 0;
				m_oDialog.GetTransform().SetIdentity();
			}

			// Increase dialog alpha
			SetDialogOpacity( m_oDialog.GetAlpha() + a_fDeltaTime * 5.0f );
			break;

		case MENUSTATE_DIALOG_DISAPPEAR:
			// Dialog disappear animation
			if ( m_fOscillatorOffset >= 0.0f )
			{
				m_fOscillatorOffset -= a_fDeltaTime * 6.0f;

				TFLOAT fScale = s_fOscillatorAmplitude * TMath::Sin( TMath::Max( m_fOscillatorOffset, 0.0f ) );
				m_oDialog.GetTransform().SetFromScale( fScale, fScale );

				if ( fScale < 0.5f )
				{
					// Decrease dialog alpha
					SetDialogOpacity( m_oDialog.GetAlpha() - a_fDeltaTime * 5.0f );
				}
			}
			else
			{
				m_fOscillatorOffset = 0.0;
				m_eMenuState        = MENUSTATE_NOTHING;
				m_fMenuOpacity      = 1.0f;

				m_oRootElement.Hide();
				OnMenuHidden();

				return TTRUE;
			}

			break;
		case MENUSTATE_MENU_APPEAR:
			// Menu fade-in animation
			m_fMenuOpacity += a_fDeltaTime * 5.0f;

			if ( m_fMenuOpacity >= 1.0f )
			{
				m_fMenuOpacity      = 1.0f;
				m_eMenuState        = MENUSTATE_MENU_VISIBLE;
				m_fOscillatorOffset = 0.0f;

				OnMenuVisible();
			}

			// Update menu opacity
			UpdateMenuOpacity();
			return BaseClass::OnUpdate( a_fDeltaTime );

		case MENUSTATE_MENU_DISAPPEAR:
			// Menu fade-out animation
			m_fMenuOpacity -= a_fDeltaTime * 5.0f;

			if ( m_fMenuOpacity > 0.0f )
			{
				UpdateMenuOpacity();
				return BaseClass::OnUpdate( a_fDeltaTime );
			}
			else
			{
				// Fade-out animation is over
				m_fMenuOpacity = 0.0f;
				m_oMenu.SetMenuAlpha( 0.0f, 0.0f );

				if ( m_oDialog.IsVisible() )
				{
					// Can show dialog
					m_eMenuState        = MENUSTATE_DIALOG_DISAPPEAR;
					m_fOscillatorOffset = s_fOscillatorPeriod;
					return BaseClass::OnUpdate( a_fDeltaTime );
				}
				else
				{
					// Can't show dialog
					m_oRootElement.Hide();
					m_eMenuState = MENUSTATE_NOTHING;
					OnMenuHidden();

					return TTRUE;
				}
			}

			break;
		case MENUSTATE_NOTHING:
			// Closed?
			return BaseClass::OnUpdate( a_fDeltaTime );
		case MENUSTATE_MENU_VISIBLE:
			// Menu is open and can be used
			if ( m_bFocusedElementBloated )
			{
				TFLOAT fScale = 1.0f + 0.05f * ( m_fTotalTime * ( TMath::PI * 2 ) );
				m_oMenu.GetFocusedMenuItem()->GetTransform().SetScale( fScale, fScale );
			}

			m_oMenu.Update( a_fDeltaTime );
			return BaseClass::OnUpdate( a_fDeltaTime );
	}

	return BaseClass::OnUpdate( a_fDeltaTime );
}

TPSTRING8_DECLARE( DialogInputMap );

void ABYardMenuState::OnInsertion()
{
	ms_fTimeSinceInsertion = 0.0f;

	if ( m_bHasRectangle2 )
	{
		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

		m_oRectangle2.SetDimensions( fWidth, fHeight );
		m_oRectangle2.SetColour( 0x7f000000 );

		AGUI2::GetRootElement()->AddChildTail( m_oRectangle2 );
	}

	SetInputMap( TPS8( DialogInputMap ) );
	AGUI2Font*           pFont        = AGUI2FontManager::FindFont( AGUI2FONT_PRIMARY );
	AGUI2TextureSection* pPanelTexSec = AGUI2TextureSectionManager::GetTextureSection( "Panel_01" );

	TVALIDPTR( pFont );
	TVALIDPTR( pPanelTexSec );

	// Setup textbox
	TFLOAT fTextBoxWidth = ( m_oTitle.GetText() != TNULL ) ? pFont->GetTextWidth( m_oTitle.GetText() ) : pPanelTexSec->GetWidth();
	m_oTitle.Create( pFont, fTextBoxWidth );
	m_oTitle.SetColour( 0xfffff0af );
	m_oTitle.SetTransform( 0.0f, 2.0f, 0.0f );

	AGUI2Transform oTransform;
	oTransform.m_aRotations[ 0 ]  = TVector2( 1.0f, 0.0f );
	oTransform.m_aRotations[ 1 ]  = TVector2( 0.0f, 1.0f );
	oTransform.m_vecTranslation.x = 0.0f;
	m_oTitle.GetTransform().PreMultiply( oTransform );

	m_oTitle.SetShadow( TTRUE, 0xa0000000 );
	m_oTitle.SetShadowAlpha( 0.6f );
	m_oTitle.SetShadowOffset( 5.0f, 5.0f );

	// Setup textbox background
	m_oTitleBackground.SetTextureSection( pPanelTexSec );
	m_oTitleBackground.SetDimensions( TMath::Max( m_oTitle.GetWidth() * 1.25f, pPanelTexSec->GetWidth() ), pPanelTexSec->GetHeight() );
	m_oTitleBackground.SetAttachment( AGUI2ATTACHMENT_TOPCENTER, AGUI2ATTACHMENT_TOPCENTER );
	m_oTitle.AddChildTail( m_oTitleBackground );

	// Setup menu
	m_oMenu.SetFocus( TTRUE );
	m_oMenu.SetItemActivationCallback( []( void* a_pUserData, AGUI2MenuItem* a_pItem ) {
		TSTATICCAST( ABYardMenuState, a_pUserData )->OnMenuItemActivated( *a_pItem );
	} );
	m_oMenu.SetItemFocusCallback( []( void* a_pUserData, AGUI2MenuItem* a_pOldFocus, AGUI2MenuItem* a_pNewFocus ) {
		TSTATICCAST( ABYardMenuState, a_pUserData )->OnMenuItemFocused( *a_pOldFocus, *a_pNewFocus );
	} );
	m_oMenu.SetCallbackUserData( this );
	m_oMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	m_oMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oMenu.SetSelectSound( soundbank::UI_MENUOK );
	m_oMenu.SetNegativeSound( soundbank::UI_NEGATIVE );
	m_oMenu.SetVerticalFlow();

	// Setup dialog
	TFLOAT fMenuWidth, fMenuHeight;
	m_oMenu.GetDimensions( fMenuWidth, fMenuHeight );

	if ( fMenuHeight < 128.0f )
		fMenuHeight = 128.0f;

	if ( fMenuWidth < 384.0f )
		fMenuWidth = 384.0f;

	m_oDialog.Create();
	m_oDialog.SetDialogSize( fMenuWidth + 128.0f, fMenuHeight + 128.0f );

	AGUI2Transform oDialogTransform;
	oDialogTransform.SetZero();
	m_oDialog.GetTransform().PreMultiply( oDialogTransform );

	// Add the elements to the GUI root
	m_oRootElement.AddChildTail( m_oDialog );

	TFLOAT fGUIRootWidth, fGUIRootHeight;
	AGUI2::GetRootElement()->GetDimensions( fGUIRootWidth, fGUIRootHeight );
	m_oRootElement.SetDimensions( fGUIRootWidth, fGUIRootHeight );

	m_oRootElement.AddChildTail( m_oMenu );
	m_oDialog.AddChildTail( m_oTitle );
	AGUI2::GetRootElement()->AddChildTail( m_oRootElement );

	BaseClass::OnInsertion();
}

void ABYardMenuState::OnRemoval()
{
	if ( m_bHasRectangle2 )
		m_oRectangle2.RemoveSelf();

	m_oTitle.RemoveSelf();
	m_oTitleBackground.RemoveSelf();
	m_oDialog.RemoveSelf();
	m_oMenu.RemoveSelf();
	m_oRootElement.RemoveSelf();

	BaseClass::OnRemoval();
}

void ABYardMenuState::OnActivate()
{
	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );

	m_eMenuState = MENUSTATE_DIALOG_APPEAR;
	m_oRootElement.Show();

	// Apply additional transform to the dialog window (apparently, no transform is applied)
	AGUI2Transform oDialogTransform( AGUI2Transform::Uninitialised );
	oDialogTransform.SetZero();

	m_oDialog.GetTransform().PreMultiply( oDialogTransform );

	m_oMenu.SetMenuAlpha( 0.0f );
	m_fMenuOpacity      = 0;
	m_fOscillatorOffset = 0.0f;
	m_bIgnoreInputs     = TFALSE;

	m_HUDParams.SetFlags( 64 );
}

void ABYardMenuState::OnDeactivate()
{
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
	m_oRootElement.Hide();
}

void ABYardMenuState::OnMenuVisible()
{
}

void ABYardMenuState::OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus )
{
	if ( m_bFocusedElementBloated )
	{
		// Reset transform of the element saving it's translation
		TVector2 vecTranslation = a_rOldFocus.GetTransform().GetTranslation();

		a_rOldFocus.GetTransform().SetZero();
		a_rOldFocus.GetTransform().PreMultiply( 1.0f, 1.0f );
		a_rOldFocus.GetTransform().SetTranslation( vecTranslation );
	}
}

void ABYardMenuState::OnMenuHidden()
{
}

void ABYardMenuState::UpdateMenuOpacity()
{
	TMath::Clip( m_fMenuOpacity, 0.0f, 1.0f );
	m_oMenu.SetMenuAlpha( m_fMenuOpacity, m_fMenuOpacity * 0.6f );
}

void ABYardMenuState::SetDialogOpacity( TFLOAT a_fOpacity )
{
	TMath::Clip( a_fOpacity, 0.0f, 1.0f );
	m_oDialog.SetAlpha( a_fOpacity );
}
