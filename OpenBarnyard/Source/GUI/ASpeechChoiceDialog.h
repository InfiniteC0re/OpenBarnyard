#pragma once
#include "AGUI2Dialog.h"
#include "AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: The choice dialog of ASpeechDialogManager, lists up to MAX_NUM_CHOICES
// answers and highlights the selected one with a bobbing arrow.
// NOTE: original name from the Wii build, the class has no TClass. Nothing
// fills the choices nor sets the font on PC, the dialog is never shown.
// vtable 0x0071bccc, size 0x828.
//-----------------------------------------------------------------------------
class ASpeechChoiceDialog
{
public:
	static constexpr TINT MAX_NUM_CHOICES = 6;

public:
	ASpeechChoiceDialog();
	virtual ~ASpeechChoiceDialog(); // +0x00

	// Highlights the choice a_iSelection
	void SetSelection( TINT a_iSelection );

	// Clears the choices and removes the dialog from the GUI
	void Hide();

	// Moves the arrow to the selected choice
	void Update( TFLOAT a_fDeltaTime );

	// Scales the whole dialog (opening and closing animations)
	void SetScale( TFLOAT a_fScale );

	TINT GetSelection() const { return m_iSelection; }

private:
	AGUI2Dialog           m_oDialog;                     // 0x004
	Toshi::T2GUIFont*     m_pFont;                       // 0x4bc, never set
	AGUI2TextBox          m_aChoices[ MAX_NUM_CHOICES ]; // 0x4c0
	TINT                  m_iSelection;                  // 0x790
	TINT                  m_iNumChoices;                 // 0x794
	TUINT32               m_uiColour;                    // 0x798
	TUINT32               m_uiSelectedColour;            // 0x79c
	TFLOAT                m_fTime;                       // 0x7a0
	TINT                  m_iUnk7A4;                     // 0x7a4
	Toshi::T2GUIRectangle m_oArrow;                      // 0x7a8
	TFLOAT                m_fArrowBob;                   // 0x820
	TFLOAT                m_fLineScale;                  // 0x824
};

TSTATICASSERT( sizeof( ASpeechChoiceDialog ) == 0x828 );
