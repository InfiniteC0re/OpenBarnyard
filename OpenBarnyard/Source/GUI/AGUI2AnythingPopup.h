#pragma once
#include "GUI/AGUI2TextBox.h"

#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// A text popup ("Level 2", "Well done!"...) that scales in, stays for a while
// and then pulses, fades out or moves away. Plain AGUI2TextBox (no own vtable).
// Name from the Wii build (AGUI2AnythingPopup), size 0xc0.
// Used by the Whack-a-Racc minigame and by AGUI2MessagePopupFlare (00641ac0).
//-----------------------------------------------------------------------------
class AGUI2AnythingPopup
    : public AGUI2TextBox
{
public:
	enum MODE : TINT
	{
		MODE_SCALE_AND_MOVE, // scales in, holds, then moves to m_vEndPos while scaling to m_fEndScale
		MODE_PULSE,          // pulses between m_fStartScale and m_fScale
		MODE_FADE,           // scales in, then fades out
	};

public:
	AGUI2AnythingPopup();
	~AGUI2AnythingPopup();

	// Shows a_wszText (TNULL hides the popup) in a_uiColour, attached to a_pParent
	// (TNULL attaches it to the root element). The text isn't copied.
	void SetText( const TWCHAR* a_wszText, TUINT32 a_uiColour, Toshi::T2GUIElement* a_pParent );

	// Sets the start position, a_rcPos is relative to the root element (in quarters of its size)
	const Toshi::TVector2& SetStartPosition( const Toshi::TVector2& a_rcPos );

	void Update( TFLOAT a_fDeltaTime );

	// Shows the popup again if it has a text
	// $Barnyard: FUNCTION 00641420
	void ShowIfHasText()
	{
		if ( m_wszPopupText ) Show();
	}

public:
	const TWCHAR*   m_wszPopupText;  // 0x78, TNULL when hidden
	MODE            m_eMode;         // 0x7c
	TFLOAT          m_fTime;         // 0x80, time since the popup appeared (after m_fDelay)
	TFLOAT          m_fScale;        // 0x84, scale once scaled in
	TFLOAT          m_fEndScale;     // 0x88, MODE_SCALE_AND_MOVE
	TFLOAT          m_fStartScale;   // 0x8c
	TFLOAT          m_fDuration;     // 0x90, scale in time
	TFLOAT          m_fPulseSpeed;   // 0x94, MODE_PULSE
	TFLOAT          m_fMoveDuration; // 0x98, MODE_SCALE_AND_MOVE
	TFLOAT          m_fDelay;        // 0x9c
	TFLOAT          m_fHoldTime;     // 0xa0, MODE_SCALE_AND_MOVE
	TUINT32         m_uiStartColour; // 0xa4
	TUINT32         m_uiEndColour;   // 0xa8
	Toshi::TVector2 m_vStartPos;     // 0xac
	Toshi::TVector2 m_vEndPos;       // 0xb4
	TBOOL           m_bCentered;     // 0xbc, attaches to the top centre of the parent
	TBOOL           m_bWhite;        // 0xbd, ignores the colour and shows the text white
	TBOOL           m_bAutoPosition; // 0xbe, a quarter of the parent's height down
};

TSTATICASSERT( sizeof( AGUI2AnythingPopup ) == 0xc0 );
