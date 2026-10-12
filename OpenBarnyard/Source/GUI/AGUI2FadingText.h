#pragma once
#include "GUI/AGUI2TextBox.h"

#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// A text box with its own copy of the text that fades out a second after the
// text was set. Plain AGUI2TextBox (no own vtable), size 0x174.
// NOTE: invented name, the original one is unknown.
// Used by the Whack-a-Racc minigame for the "Level N" banner.
//-----------------------------------------------------------------------------
class AGUI2FadingText
    : public AGUI2TextBox
{
public:
	static constexpr TINT MAX_TEXT_LENGTH = 100;

public:
	// Also attaches the element to the root element
	AGUI2FadingText();
	~AGUI2FadingText();

	// Attaches the element to a_pParent (TNULL is the root element), a quarter of its height down
	void Create( Toshi::T2GUIFont* a_pFont, Toshi::T2GUIElement* a_pParent, Toshi::T2GUIATTACHMENT a_eAnchor, Toshi::T2GUIATTACHMENT a_ePivot );

	// Shows a copy of a_wszText. a_uiColour is unused (the text is white).
	void SetText( const TWCHAR* a_wszText, TUINT32 a_uiColour );

	// Shows a copy of the locale string a_iLocaleId. a_iUnused is unused.
	void SetLocaleText( TINT a_iLocaleId, TINT a_iUnused );

	void Update( TFLOAT a_fDeltaTime );

public:
	TWCHAR            m_wszBuffer[ MAX_TEXT_LENGTH ]; // 0x78
	TFLOAT            m_fUnk1;                        // 0x140
	TFLOAT            m_fTime;                        // 0x144, time since the text was set (after m_fDelay)
	TFLOAT            m_fUnk2;                        // 0x148, 3.0
	TFLOAT            m_fUnk3;                        // 0x14c, 0.1
	TFLOAT            m_fFadeTime;                    // 0x150, the text fades out after this time
	TFLOAT            m_fUnk4;                        // 0x154, 0.5
	TFLOAT            m_fUnk5;                        // 0x158, 1.0
	TFLOAT            m_fDelay;                       // 0x15c
	Toshi::TVector2   m_vUnk6;                        // 0x160
	Toshi::TVector2   m_vUnk7;                        // 0x168
	Toshi::T2GUIFont* m_pTextFont;                    // 0x170, TNULL uses "Rekord26"
};

TSTATICASSERT( sizeof( AGUI2FadingText ) == 0x174 );
