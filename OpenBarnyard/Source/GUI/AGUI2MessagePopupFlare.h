#pragma once
#include "GUI/AGUI2AnythingPopup.h"
#include "GUI/AGUI2Flare.h"

#include <GUI/T2GUIElement.h>

//-----------------------------------------------------------------------------
// A text popup with three flares behind it. The text pops in at the centre
// of the screen and then flies to (and scales into) a target element, which
// is hidden while the popup is shown ("+100" flying into a score).
// Plain T2GUIElement (no TClass), size 0x43c.
// vtable 0073a420: +0x00 dtor 00641d70, +0x04 Tick 00641f20, +0x0c Render 00641cb0.
// Name from the Wii build (AGUI2MessagePopupFlare).
// Used by the BarnTender minigame (ATapperMiniGameState).
//-----------------------------------------------------------------------------
class AGUI2MessagePopupFlare
    : public Toshi::T2GUIElement
{
public:
	static constexpr TINT NUM_FLARES = 3;

	enum STATE : TINT
	{
		STATE_NONE,
		STATE_STARTING, // set by Show before Start
		STATE_FLARES,   // the flares are shown
		STATE_FLYING,   // the flares are fading out, the text flies to the target
	};

public:
	AGUI2MessagePopupFlare();
	~AGUI2MessagePopupFlare();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;
	virtual void Render() override;

	// Creates the flares
	void Create();

	// Shows a_wszText (not copied) in a_uiColour. The text then flies to a_pTarget (TNULL is
	// the centre of the screen) changing to a_fEndScale and a_uiEndColour. a_pTarget is hidden
	// until the text reaches it.
	void Show( const TWCHAR* a_wszText, TFLOAT a_fEndScale, Toshi::T2GUIElement* a_pTarget, TUINT32 a_uiColour, TUINT32 a_uiEndColour );

private:
	void Start();

	// Returns the scale from screen pixels to the GUI root element
	static void GetScreenToGUIScale( TFLOAT& a_rScaleX, TFLOAT& a_rScaleY );

public:
	AGUI2AnythingPopup   m_oPopup;                // 0x54
	AGUI2Flare           m_aFlares[ NUM_FLARES ]; // 0x114
	const TWCHAR*        m_wszText;               // 0x420
	TFLOAT               m_fFlareTime;            // 0x424, the flares fade out when it runs out
	TFLOAT               m_fEndScale;             // 0x428
	TUINT32              m_uiColour;              // 0x42c
	TUINT32              m_uiEndColour;           // 0x430
	STATE                m_eState;                // 0x434
	Toshi::T2GUIElement* m_pTarget;               // 0x438
};

TSTATICASSERT( sizeof( AGUI2MessagePopupFlare ) == 0x43c );
