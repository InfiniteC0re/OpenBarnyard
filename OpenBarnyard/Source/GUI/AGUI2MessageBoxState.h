#pragma once
#include "GameInterface/AGameState.h"
#include "AGUI2Dialog.h"
#include "AGUI2TextBox.h"
#include "AGUI2Menu.h"
#include "AGUI2Button.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUIFont.h>

//-----------------------------------------------------------------------------
// Pop up with a text and up to MAX_NUM_BUTTONS buttons, drawn in an AGUI2Dialog.
// Removes itself once a button is chosen and writes the index of the focused
// button to the result pointer (unless the cancel button was chosen).
// Used by ACheatActivator ("Cheat activated!") and AMiscFunctions.
//-----------------------------------------------------------------------------
class AGUI2MessageBoxState
    : public AGameState
{
public:
	TDECLARE_CLASS( AGUI2MessageBoxState, AGameState );

	static constexpr TINT MAX_NUM_BUTTONS = 5;

public:
	AGUI2MessageBoxState();
	~AGUI2MessageBoxState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL ShouldPlacePlayerAtStartGoal() OVERRIDE { return TFALSE; } // +0x28, RETURNS_FALSE
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Creates the dialog of a_fWidth x a_fHeight with the fonts and the border texture sections,
	// a_bDarkenScreen puts a black rectangle over the whole screen behind it.
	void Create(
	    TFLOAT       a_fWidth,
	    TFLOAT       a_fHeight,
	    const TCHAR* a_szTextFont,
	    const TCHAR* a_szButtonFont,
	    const TCHAR* a_szTopLeft,
	    const TCHAR* a_szTopCenter,
	    const TCHAR* a_szTopRight,
	    const TCHAR* a_szMiddleLeft,
	    const TCHAR* a_szMiddleCenter,
	    const TCHAR* a_szMiddleRight,
	    const TCHAR* a_szBottomLeft,
	    const TCHAR* a_szBottomCenter,
	    const TCHAR* a_szBottomRight,
	    TBOOL        a_bDarkenScreen
	);

	// Adds a button with the text a_wszText, returns TFALSE if there are MAX_NUM_BUTTONS already
	TBOOL AddButton( const TWCHAR* a_wszText );

	void SetText( const TWCHAR* a_wszText );
	void SetTextColour( TUINT32 a_uiColour );
	void SetDialogColour( TUINT32 a_uiColour );

	void SetButtonColours( TUINT32 a_uiFocusedColour, TUINT32 a_uiEnabledColour );
	void SetButtonScales( TFLOAT a_fFocusedScale, TFLOAT a_fScale );

	void SetTextShadowEnabled( TBOOL a_bEnabled );
	void SetTextShadowColour( TUINT32 a_uiColour );
	void SetTextShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY );
	void SetTextShadowAlpha( TFLOAT a_fAlpha );

	// Index of the focused button or -1
	TINT GetFocusedButton();

	// $Barnyard: FUNCTION 00640ae0
	AGUI2TextBox* GetTextBox() { return &m_oText; }

	// $Barnyard: FUNCTION 00640af0
	void SetAutoSize( TBOOL a_bAutoSize ) { m_bAutoSize = a_bAutoSize; }

	// $Barnyard: FUNCTION 00640b00
	void SetSize( TFLOAT a_fWidth, TFLOAT a_fHeight )
	{
		m_fWidth  = a_fWidth;
		m_fHeight = a_fHeight;
	}

	// $Barnyard: FUNCTION 00640b20
	void SetPadding( TFLOAT a_fPaddingX, TFLOAT a_fPaddingY )
	{
		m_fPaddingX = a_fPaddingX;
		m_fPaddingY = a_fPaddingY;
	}

private:
	static void OnButtonActivated( void* a_pUserData, AGUI2MenuItem* a_pItem );

private:
	TFLOAT                m_fPaddingX;                   // 0xa4
	TFLOAT                m_fPaddingY;                   // 0xa8
	TFLOAT                m_fWidth;                      // 0xac
	TFLOAT                m_fHeight;                     // 0xb0
	TBOOL                 m_bAutoSize;                   // 0xb4
	TFLOAT                m_fMenuX;                      // 0xb8, only used without the dialog
	TFLOAT                m_fMenuY;                      // 0xbc, only used without the dialog
	TINT*                 m_pResult;                     // 0xc0, receives the chosen button, no setter in retail
	Toshi::T2GUIElement*  m_pRootElement;                // 0xc4, the dialog after Create
	TINT                  m_iNumButtons;                 // 0xc8
	TBOOL                 m_bUseDialog;                  // 0xcc
	Toshi::T2GUIRectangle m_oBackground;                 // 0xd0, used instead of the dialog (never in retail)
	AGUI2Dialog           m_oDialog;                     // 0x148
	Toshi::T2GUIFont*     m_pTextFont;                   // 0x600
	AGUI2TextBox          m_oText;                       // 0x604
	TINT                  m_iCancelButton;               // 0x67c, -1 if none, no setter in retail
	Toshi::T2GUIFont*     m_pButtonFont;                 // 0x684
	AGUI2Menu             m_oMenu;                       // 0x688
	AGUI2Button           m_aButtons[ MAX_NUM_BUTTONS ]; // 0x72c
	TBOOL                 m_bDarkenScreen;               // 0x1140
	Toshi::T2GUIRectangle m_oScreenRect;                 // 0x1144
};
