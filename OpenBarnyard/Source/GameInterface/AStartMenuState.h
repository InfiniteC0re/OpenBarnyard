#pragma once
#include "ABYardMenuState.h"
#include "AMessagePopupState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Button.h"

#include <Toshi/TEvent.h>

//-----------------------------------------------------------------------------
// The in-game pause menu (TClass 007818b0, vtable 0071a440), pushed by
// AGameStateController::StartMenuIfPossible when START is pressed.
// Pauses the game while it's shown. Restart and Help are only there in a
// minigame. Quit asks for a confirmation (AMessagePopupState), then leaves the
// minigame (or the story) through AGameStateController's transition flags.
//-----------------------------------------------------------------------------
class AStartMenuState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AStartMenuState, ABYardMenuState );

	enum BUTTON : TINT
	{
		BUTTON_RESUME,
		BUTTON_RESTART,
		BUTTON_OPTIONS,
		BUTTON_HELP,
		BUTTON_QUIT,
		BUTTON_NUMOF
	};

	// Replaces the default handling of the quit confirmation (Yes leaves the game)
	using t_QuitCallback = TBOOL( __stdcall* )( AStartMenuState* a_pStartMenu, void* a_pUserData, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

	using PopupListener = Toshi::TListener<AMessagePopupState, AMessagePopupState::ResponseEvent, AStartMenuState>;

public:
	AStartMenuState();
	~AStartMenuState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuOpen() OVERRIDE {} // DOES_NOTHING
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

	// Changes the text of the quit button (AGopherShopGame uses 332)
	static void SetQuitText( TINT a_iTextId ) { ms_aButtonTexts[ BUTTON_QUIT ] = a_iTextId; }

	// NOTE: the original takes the user data first
	// $Barnyard: FUNCTION 0042f080
	void SetQuitCallback( t_QuitCallback a_fnCallback, void* a_pUserData )
	{
		m_fnQuitCallback        = a_fnCallback;
		m_pQuitCallbackUserData = a_pUserData;
	}

private:
	// Shows the latest hint of the story in a popup
	void ShowHint();

	TBOOL OnQuitResponse( AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

	static TBOOL __stdcall OnQuitPopupResponse( AStartMenuState* a_pStartMenu, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );
	static TBOOL __stdcall OnHelpPopupResponse( AStartMenuState* a_pStartMenu, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

public:
	// Locale strings of the buttons
	inline static TINT ms_aButtonTexts[ BUTTON_NUMOF ] = { 335, 334, 294, 295, 333 }; // 0x0077344c

private:
	// Set by OnQuitResponse when the player has confirmed
	inline static TBOOL ms_bQuitConfirmed; // 0x007818ac

private:
	AGUI2TextBox   m_oTitle;                   // 0x7d8
	AGUI2Button    m_aButtons[ BUTTON_NUMOF ]; // 0x850
	TINT           m_iActivatedButton;         // 0x1264
	PopupListener  m_oQuitListener;            // 0x1268
	PopupListener  m_oHelpListener;            // 0x127c
	t_QuitCallback m_fnQuitCallback;           // 0x1290
	void*          m_pQuitCallbackUserData;    // 0x1294
	TBOOL          m_bHasHint;                 // 0x1298
};

TSTATICASSERT( sizeof( AStartMenuState ) == 0x129c );
