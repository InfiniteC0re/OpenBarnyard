#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIElement.h>

//-----------------------------------------------------------------------------
// Purpose: the two lines of text at the bottom of the screen telling what the
// action button does (APlayerControlComponent). Created by AHudManager.
// TClass 00782080, vtable 0071c33c.
//-----------------------------------------------------------------------------
class AInteractionHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( AInteractionHudElement, AHudElement );

	static constexpr TINT NUM_LINES  = 2;
	static constexpr TINT MAX_LENGTH = 256;

public:
	AInteractionHudElement();
	~AInteractionHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;

	// Shows a_wszText on the line a_iLine (TNULL hides the line). The input tags of the text are replaced.
	// NOTE: a_iUnk1, a_iUnk2 and a_iUnk3 aren't used
	void SetText( const TWCHAR* a_wszText, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iLine, TUINT32 a_uiColour, TFLOAT a_fWidth );

	// Makes the lines as wide as 70% of the screen
	void UpdateWidth();

public:
	TBOOL                m_abShowLine[ NUM_LINES ];           // 0x14
	AGUI2TextBox         m_aLines[ NUM_LINES ];               // 0x18
	Toshi::T2GUIFont*    m_pFont;                             // 0x108, "Rekord18"
	Toshi::T2GUIElement* m_pParent;                           // 0x10c, the root of AGUI2 if not set
	TWCHAR               m_aTexts[ NUM_LINES ][ MAX_LENGTH ]; // 0x110
};

TSTATICASSERT( sizeof( AInteractionHudElement ) == 0x510 );
