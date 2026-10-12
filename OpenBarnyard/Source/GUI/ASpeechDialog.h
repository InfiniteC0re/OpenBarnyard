#pragma once
#include "AGUI2Dialog.h"
#include "AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

class ASpeechDialogManager;

//-----------------------------------------------------------------------------
// Purpose: The speech bubble of ASpeechDialogManager. A dialog with the name of
// the speaking unit, the text (scrolled up when it doesn't fit, the player can
// scroll it with the up/down keys once it's done) and the "press to continue"
// prompt that bobs up and down. The arrows show that the text can be scrolled.
// NOTE: original name from the Wii build, the class has no TClass.
// vtable 0x0071bcc8, size 0x770.
//-----------------------------------------------------------------------------
class ASpeechDialog
{
public:
	ASpeechDialog();
	virtual ~ASpeechDialog(); // +0x00

	// Sets the dialog up, a_szNameFont is the font of the name, a_pTextFont the one of the text
	void Create( const TCHAR* a_szNameFont, Toshi::T2GUIFont* a_pTextFont );

	// Clears the text and moves it back to the top
	void Reset();

	// Adds the dialog to the GUI
	void Show();

	// Clears the texts and removes the dialog from the GUI
	void Hide();

	// Shows or hides the whole dialog
	void SetVisible( TBOOL a_bVisible );

	// Sets the text, scrolls it up when it gets taller than the dialog
	void SetText( const TWCHAR* a_wszText, TUINT32 a_uiColour );

	// Sets the name of the speaking unit
	void SetName( const TWCHAR* a_wszName, TUINT32 a_uiColour );

	// Scales the whole dialog (opening and closing animations)
	void SetScale( TFLOAT a_fScale );

	// Scrolls the text and moves the prompt
	void Update( TFLOAT a_fDeltaTime );

	void ShowPrompt() { m_oPrompt.Show(); }
	void HidePrompt() { m_oPrompt.Hide(); }

	void SetOwner( void* a_pOwner ) { m_pOwner = a_pOwner; }

private:
	AGUI2Dialog           m_oDialog;         // 0x004
	Toshi::T2GUIFont*     m_pFont;           // 0x4bc, font of the text
	AGUI2TextBox          m_oText;           // 0x4c0
	AGUI2TextBox          m_oName;           // 0x538
	TFLOAT                m_fTime;           // 0x5b0
	AGUI2TextBox          m_oPrompt;         // 0x5b4
	TFLOAT                m_fPromptBob;      // 0x62c
	TWCHAR                m_wszPrompt[ 32 ]; // 0x630
	void*                 m_pOwner;          // 0x670, ASpeechDialogManager::Speech
	TBOOL                 m_bScrollDone;     // 0x674, the text is scrolled to m_fScrollTarget
	TFLOAT                m_fScrollTarget;   // 0x678
	TFLOAT                m_fTextHeight;     // 0x67c
	Toshi::T2GUIRectangle m_aArrows[ 2 ];    // 0x680, up and down
};

TSTATICASSERT( sizeof( ASpeechDialog ) == 0x770 );
