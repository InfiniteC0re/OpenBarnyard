#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AGUI2Menu.h"

#include <Toshi/TEvent.h>

//-----------------------------------------------------------------------------
// Message box with up to four buttons (TClass 0078184c, vtable 0071a330).
// The buttons are input commands (AInputCommand_OK, _Yes, _No...), their text
// is the localised name of the command. Once a button is chosen the popup
// closes and, unless it's removed on response, throws a ResponseEvent every
// frame until a listener pops it.
//-----------------------------------------------------------------------------
class AMessagePopupState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AMessagePopupState, ABYardMenuState );

	static constexpr TINT MAX_NUM_BUTTONS = 4;

	struct ResponseEvent
	{
		AInputCommand eButton; // input command of the chosen button
	};

public:
	AMessagePopupState( const TWCHAR* a_wszTitle, const TWCHAR* a_wszMessage, const Toshi::TPString8& a_rcUnused, TBOOL a_bRemoveOnResponse, AInputCommand a_eButton1, AInputCommand a_eButton2, AInputCommand a_eButton3, AInputCommand a_eButton4, TBOOL a_bUnk1, TFLOAT a_fUnk2, TUINT32 a_uiHUDFlags );
	AMessagePopupState( const TWCHAR* a_wszTitle, const TWCHAR* a_wszMessage, const Toshi::TPString8& a_rcUnused, TINT a_iUnused, TBOOL a_bRemoveOnResponse, AInputCommand a_eButton1, AInputCommand a_eButton2, AInputCommand a_eButton3, AInputCommand a_eButton4, TBOOL a_bUnk1, TFLOAT a_fUnk2, TUINT32 a_uiHUDFlags );
	~AMessagePopupState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuOpen() OVERRIDE;
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE {} // DOES_NOTHING
	virtual void OnMenuClose() OVERRIDE {}                                     // DOES_NOTHING

	Toshi::TEmitter<AMessagePopupState, ResponseEvent>& GetResponseEmitter() { return m_oResponseEmitter; }

	// The message is resized to a_fWidth * the width of the dialog once activated
	void SetMessageWidth( TFLOAT a_fWidth )
	{
		m_bResizeMessage = TTRUE;
		m_fMessageWidth  = a_fWidth;
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: chooses a button as if the player had clicked it
	void Dev_ChooseButton( TINT a_iButton )
	{
		m_iSelectedButton = a_iButton;
		m_eMenuState      = MENUSTATE_MENU_DISAPPEAR;
	}
#endif

	void SetUnk19b0( TBOOL a_bUnk )
	{
		m_bUnk19b0 = a_bUnk;
	}
	void SetUnk19b4( TFLOAT a_fUnk ) { m_fUnk19b4 = a_fUnk; }
	void SetUnk19b8( TFLOAT a_fUnk ) { m_fUnk19b8 = a_fUnk; }

private:
	void Setup( const TWCHAR* a_wszTitle, const TWCHAR* a_wszMessage );

	static void OnMenuButtonActivated( void* a_pUserData, AGUI2MenuItem* a_pMenuItem );

private:
	Toshi::TEmitter<AMessagePopupState, ResponseEvent> m_oResponseEmitter;                          // 0x7d8
	AGUI2TextBox                                       m_oUnkTextBox;                               // 0x7e4
	AGUI2TextBox                                       m_oMessage;                                  // 0x85c
	AGUI2Button                                        m_aButtons[ MAX_NUM_BUTTONS ];               // 0x8d4
	TBOOL                                              m_bRemoveOnResponse;                         // 0x10e4
	AInputCommand                                      m_aButtonCommands[ MAX_NUM_BUTTONS ];        // 0x10e8
	TWCHAR                                             m_awszButtonTexts[ MAX_NUM_BUTTONS ][ 256 ]; // 0x10f8
	TINT                                               m_iUnk18f8;                                  // 0x18f8
	TFLOAT                                             m_fUnk18fc;                                  // 0x18fc
	TBOOL                                              m_bUnk1900;                                  // 0x1900
	AGUI2Menu                                          m_oButtonMenu;                               // 0x1904
	TBOOL                                              m_bResizeMessage;                            // 0x19a8
	TFLOAT                                             m_fMessageWidth;                             // 0x19ac
	TBOOL                                              m_bUnk19b0;                                  // 0x19b0
	TFLOAT                                             m_fUnk19b4;                                  // 0x19b4
	TFLOAT                                             m_fUnk19b8;                                  // 0x19b8
	TUINT32                                            m_uiHUDFlags;                                // 0x19bc
	TINT                                               m_iUnk19c0;                                  // 0x19c0
	TINT                                               m_iSelectedButton;                           // 0x19c4
};

TSTATICASSERT( sizeof( AMessagePopupState ) == 0x19c8 );
