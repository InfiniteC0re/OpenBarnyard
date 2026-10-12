#include "pch.h"
#include "AMessagePopupState.h"
#include "Input/AInputMapManager.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUIFontManager.h>
#include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AMessagePopupState );

// $Barnyard: FUNCTION 0042e970
AMessagePopupState::AMessagePopupState( const TWCHAR* a_wszTitle, const TWCHAR* a_wszMessage, const TPString8& a_rcUnused, TBOOL a_bRemoveOnResponse, AInputCommand a_eButton1, AInputCommand a_eButton2, AInputCommand a_eButton3, AInputCommand a_eButton4, TBOOL a_bUnk1, TFLOAT a_fUnk2, TUINT32 a_uiHUDFlags )
    : m_oResponseEmitter( this )
    , m_bRemoveOnResponse( a_bRemoveOnResponse )
    , m_fUnk18fc( a_fUnk2 )
    , m_bUnk1900( a_bUnk1 )
    , m_bResizeMessage( TFALSE )
    , m_fMessageWidth( 1.0f )
    , m_bUnk19b0( TTRUE )
    , m_fUnk19b4( 0.05f )
    , m_fUnk19b8( 0.18f )
    , m_uiHUDFlags( a_uiHUDFlags )
    , m_iUnk19c0( 0 )
    , m_iSelectedButton( -1 )
{
	m_aButtonCommands[ 0 ] = a_eButton1;
	m_aButtonCommands[ 1 ] = a_eButton2;
	m_aButtonCommands[ 2 ] = a_eButton3;
	m_aButtonCommands[ 3 ] = a_eButton4;

	m_HUDParams.SetFlags( a_uiHUDFlags );
	Setup( a_wszTitle, a_wszMessage );
}

// $Barnyard: FUNCTION 0042ead0
AMessagePopupState::AMessagePopupState( const TWCHAR* a_wszTitle, const TWCHAR* a_wszMessage, const TPString8& a_rcUnused, TINT a_iUnused, TBOOL a_bRemoveOnResponse, AInputCommand a_eButton1, AInputCommand a_eButton2, AInputCommand a_eButton3, AInputCommand a_eButton4, TBOOL a_bUnk1, TFLOAT a_fUnk2, TUINT32 a_uiHUDFlags )
    : m_oResponseEmitter( this )
    , m_bRemoveOnResponse( a_bRemoveOnResponse )
    , m_fUnk18fc( a_fUnk2 )
    , m_bUnk1900( a_bUnk1 )
    , m_bResizeMessage( TFALSE )
    , m_fMessageWidth( 1.0f )
    , m_bUnk19b0( TTRUE )
    , m_fUnk19b4( 0.05f )
    , m_fUnk19b8( 0.18f )
    , m_uiHUDFlags( a_uiHUDFlags )
    , m_iUnk19c0( 0 )
    , m_iSelectedButton( -1 )
{
	m_aButtonCommands[ 0 ] = a_eButton1;
	m_aButtonCommands[ 1 ] = a_eButton2;
	m_aButtonCommands[ 2 ] = a_eButton3;
	m_aButtonCommands[ 3 ] = a_eButton4;

	Setup( a_wszTitle, a_wszMessage );
}

// $Barnyard: FUNCTION 0042e140
// $Barnyard: FUNCTION 0042e4b0
AMessagePopupState::~AMessagePopupState()
{
	// NOTE: the original destroys the emitter in its destructor (0x006c46e0), disconnecting the listeners
	m_oResponseEmitter.Destroy();
}

// $Barnyard: FUNCTION 0042e690
void AMessagePopupState::OnMenuButtonActivated( void* a_pUserData, AGUI2MenuItem* a_pMenuItem )
{
	AMessagePopupState* pPopup = TSTATICCAST( AMessagePopupState, a_pUserData );

	pPopup->m_iSelectedButton = TSTATICCAST( AGUI2Button, a_pMenuItem )->GetId();
	pPopup->m_eMenuState      = MENUSTATE_MENU_DISAPPEAR;
}

// $Barnyard: FUNCTION 0042e6b0
void AMessagePopupState::Setup( const TWCHAR* a_wszTitle, const TWCHAR* a_wszMessage )
{
	m_oDialogTitle.SetText( a_wszTitle );

	if ( a_wszTitle == TNULL )
		m_oDialogTitleBackground.Hide();

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	// Message
	m_oMessage.Create( pFont, m_oDialog.GetWidth() * 0.8f );
	m_oMessage.SetText( a_wszMessage );
	m_oMessage.SetColour( 0xffffffff );
	m_oMessage.SetShadow( TTRUE, 0xa0000000 );
	m_oMessage.SetAlpha( 0.0f );
	m_oMessage.SetShadowAlpha( 0.0f );
	m_oMessage.SetShadowOffset( m_oMessage.GetScale() * 5.0f, m_oMessage.GetScale() * 5.0f );
	m_oMenu.AddChildTail( m_oMessage );

	// Buttons
	TINT iDefaultButton = 0;

	for ( TINT i = 0; i < MAX_NUM_BUTTONS; i++ )
	{
		AInputCommand eCommand = m_aButtonCommands[ i ];

		if ( eCommand == AInputCommand_Unknown || eCommand == AInputCommand_Empty )
			continue;

		const TWCHAR* wszCommandName = TNULL;
		AInputMapManager::GetSingleton()->GetCommandLocalisedName( eCommand, wszCommandName );
		TStringManager::String16Format( m_awszButtonTexts[ i ], 256, L"%s", wszCommandName );

		AGUI2Button& rButton = m_aButtons[ i ];
		rButton.Create( pFont, pFont->GetTextWidth( m_awszButtonTexts[ i ] ), m_awszButtonTexts[ i ], 40.0f );
		rButton.SetFocusedColour( 0xffffffff );
		rButton.SetFocusedScale( 1.0f );
		rButton.SetEnabledColour( 0xffb56f22 );
		rButton.SetDisabledColour( 0xff4b4b4b );
		rButton.SetId( i );
		rButton.OnFocusLost();
		rButton.GetTextBox().SetShadow( TTRUE, 0xa0000000 );
		rButton.GetTextBox().SetShadowOffset( m_oMessage.GetScale() * 5.0f, m_oMessage.GetScale() * 5.0f );
		m_oButtonMenu.AddMenuItem( rButton );

		// Cancelling buttons are focused by default
		if ( m_aButtonCommands[ i ] == AInputCommand_Cancel || m_aButtonCommands[ i ] == AInputCommand_No || m_aButtonCommands[ i ] == AInputCommand_Quit )
			iDefaultButton = i;
	}

	m_oButtonMenu.SetFocus( TTRUE );
	m_oButtonMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	m_oButtonMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oButtonMenu.SetSelectSound( soundbank::UI_MENUOK );
	m_oButtonMenu.SetNegativeSound( soundbank::UI_NEGATIVE );
	m_oButtonMenu.ReflowChildrenHorizontally();

	TFLOAT fMenuWidth, fMenuHeight;
	m_oButtonMenu.GetDimensions( fMenuWidth, fMenuHeight );

	m_oButtonMenu.SetItemActivationCallback( OnMenuButtonActivated );
	m_oButtonMenu.SetCallbackUserData( this );
	m_oButtonMenu.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oButtonMenu.SetFocusAt( m_aButtons[ iDefaultButton ] );
	m_oButtonMenu.SetMenuAlpha( 0.0f, -1.0f );
}

// $Barnyard: FUNCTION 0042e3f0
TBOOL AMessagePopupState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( !m_oButtonMenu.IsFocused() )
		return ABYardMenuState::ProcessInput( a_pInputEvent );

	// NOTE: AGUI2Menu::ProcessInputEvent is inlined here in the original
	return m_oButtonMenu.ProcessInputEvent( a_pInputEvent );
}

// $Barnyard: FUNCTION 0042e410
TBOOL AMessagePopupState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && a_pInputEvent->GetDoodad() == TInputDeviceKeyboard::KEY_ESCAPE )
	{
		// Escape chooses the cancelling button if there's one
		for ( TINT i = 0; i < MAX_NUM_BUTTONS; i++ )
		{
			AInputCommand eCommand = m_aButtonCommands[ i ];

			if ( eCommand == AInputCommand_Cancel || eCommand == AInputCommand_Quit || eCommand == AInputCommand_No )
			{
				m_eMenuState      = MENUSTATE_MENU_DISAPPEAR;
				m_iSelectedButton = i;
				return TTRUE;
			}
		}

		return TTRUE;
	}

	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE && m_oButtonMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) )
		return TTRUE;

	return ABYardMenuState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 0042e4d0
TBOOL AMessagePopupState::OnUpdate( TFLOAT a_fDeltaTime )
{
	switch ( m_eMenuState )
	{
		case MENUSTATE_MENU_APPEAR:
		case MENUSTATE_MENU_DISAPPEAR:
			m_oMessage.SetAlpha( m_fMenuOpacity );
			m_oMessage.SetShadowAlpha( 0.6f * m_fMenuOpacity );
			m_oButtonMenu.SetMenuAlpha( m_fMenuOpacity, 0.6f * m_fMenuOpacity );
			return ABYardMenuState::OnUpdate( a_fDeltaTime );
		case MENUSTATE_MENU_VISIBLE:
		{
			// The focused button pulses
			AGUI2MenuItem*  pFocused       = m_oButtonMenu.GetFocusedMenuItem();
			T2GUITransform& rTransform     = pFocused->GetTransform();
			TVector2        vecTranslation = rTransform.GetTranslation();

			rTransform.SetIdentity();

			TFLOAT fScale = TMath::Sin( TMath::TWO_PI * m_fTotalTime ) * 0.05f + 1.0f;
			rTransform.Scale( fScale, fScale );
			rTransform.SetTranslation( vecTranslation );

			m_oButtonMenu.Update( a_fDeltaTime );
			return ABYardMenuState::OnUpdate( a_fDeltaTime );
		}
		case MENUSTATE_DIALOG_DISAPPEAR:
			m_oMessage.SetAlpha( 0.0f );
			m_oMessage.SetShadowAlpha( 0.0f );
			m_oButtonMenu.SetMenuAlpha( 0.0f, -1.0f );
			return ABYardMenuState::OnUpdate( a_fDeltaTime );
		case MENUSTATE_NOTHING:
			// The popup has closed
			if ( m_bRemoveOnResponse )
			{
				Remove();
			}
			else
			{
				ResponseEvent oEvent;
				oEvent.eButton = m_aButtonCommands[ m_iSelectedButton ];
				m_oResponseEmitter.Throw( &oEvent );
			}

			return TTRUE;
	}

	return ABYardMenuState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0042e1f0
void AMessagePopupState::OnInsertion()
{
	m_bFocusedElementBloated = TFALSE;
	ABYardMenuState::OnInsertion();

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );
	m_oMessage.Create( pFont, m_oDialog.GetWidth() * 0.8f );

	m_oButtonMenu.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oMessage.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oMenu.AddChildTail( m_oButtonMenu );

	// Make the dialog fit the message and the buttons
	m_oMenu.SetHeight( m_oButtonMenu.GetHeight() + m_oButtonMenu.GetHeight() + m_oMessage.GetHeight() );
	m_oDialog.SetDimensions( m_oDialog.GetWidth(), m_oMenu.GetHeight() + 128.0f );
}

// $Barnyard: FUNCTION 0042e2c0
void AMessagePopupState::OnRemoval()
{
	m_oMessage.RemoveSelf();

	for ( TINT i = 0; i < MAX_NUM_BUTTONS; i++ )
		m_aButtons[ i ].RemoveSelf();

	m_oMessage.RemoveSelf();
	m_oButtonMenu.RemoveSelf();

	ABYardMenuState::OnRemoval();
}

// $Barnyard: FUNCTION 0042e330
void AMessagePopupState::OnActivate()
{
	ABYardMenuState::OnActivate();
	m_HUDParams.SetFlags( m_uiHUDFlags );

	if ( m_bResizeMessage == TTRUE )
		m_oMessage.SetWidth( m_oDialog.GetWidth() * m_fMessageWidth );
}

// $Barnyard: FUNCTION 0042e390
void AMessagePopupState::OnMenuOpen()
{
	m_oMessage.SetAlpha( m_fMenuOpacity );
	m_oMessage.SetShadowAlpha( 0.6f * m_fMenuOpacity );
	m_oButtonMenu.SetMenuAlpha( m_fMenuOpacity, 0.6f * m_fMenuOpacity );
}
