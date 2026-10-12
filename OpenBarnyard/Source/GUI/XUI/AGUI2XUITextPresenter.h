#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"
#  include "GUI/AGUI2TextBox.h"

#  include <GUI/T2GUIFont.h>

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUITextPresenter : AGUI2XUIElement
// Shows the Text of the control that owns the visual it is in.
//-----------------------------------------------------------------------------
class AGUI2XUITextPresenter
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUITextPresenter, AGUI2XUIElement );

public:
	AGUI2XUITextPresenter();
	~AGUI2XUITextPresenter();

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;

	// Inlined in AGUI2XUIVisual::SetPresenterData
	// NOTE: it also sets a halfword at 0x6C of the Wii AGUI2TextBox to -1
	void SetText( const TWCHAR* a_wszText ) { m_oTextBox.SetTextNoResize( a_wszText ); }

private:
	Toshi::T2GUIFont* m_pFont;    // 0x98
	AGUI2TextBox      m_oTextBox; // 0x9C
};

#endif // BARNYARD_LATEST
