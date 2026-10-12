#pragma once
#include "HUD/AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITransform.h>

//-----------------------------------------------------------------------------
// The Chicken Launch (CFP) power dial: a "dial_backing" with a "needle" that
// points at m_fValue (0.0f..1.0f), a popping text (scales up and fades out)
// and a second, static text below the dial. The dial bounces in when shown.
// AHudElement (mask 2), vtable 0071bea0, TClass.
// ctor 004506b0, dtor 00450780, size 0x250.
// Names from the Wii build (ACFPBashHudElement, ACFPBashHudElement::Needle).
// Used by AChickenFireMinigameState (+0xc6c).
//-----------------------------------------------------------------------------
class ACFPBashHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ACFPBashHudElement, AHudElement );

	static constexpr TINT MAX_POP_TEXT_LENGTH = 16;

	//-----------------------------------------------------------------------------
	// A rectangle that rotates around m_oPivotTransform (same as
	// AGameTimeHudElement::ClockHand). vtable 0071be58, size 0x90.
	//-----------------------------------------------------------------------------
	class Needle
	    : public Toshi::T2GUIRectangle
	{
	public:
		Needle() = default;
		~Needle();

		//-----------------------------------------------------------------------------
		// Toshi::T2GUIElement
		//-----------------------------------------------------------------------------
		virtual void PreRender() override;
		virtual void PostRender() override;

	public:
		Toshi::T2GUITransform m_oPivotTransform; // 0x78
	};

public:
	ACFPBashHudElement();
	~ACFPBashHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;

	// Sets m_fUnk20 (not used by the element itself)
	// $Barnyard: FUNCTION 004508a0
	void SetUnk20( TFLOAT a_fValue ) { m_fUnk20 = a_fValue; }

	// Shows a_wszText (TNULL hides it) below the dial
	void SetText( const TWCHAR* a_wszText );

	// Pops a_wszText (TNULL hides it) above the dial: it scales up and then fades out.
	// Does nothing if the text didn't change.
	void SetPopText( const TWCHAR* a_wszText );

public:
	TFLOAT                m_fValue;                            // 0x14, needle position, 0.0f..1.0f
	TFLOAT                m_fUnk18;                            // 0x18
	TFLOAT                m_fUnk1C;                            // 0x1c, set directly by AChickenFireMinigameState
	TFLOAT                m_fUnk20;                            // 0x20
	TFLOAT                m_fUnk24;                            // 0x24
	TINT                  m_iUnk28;                            // 0x28
	TFLOAT                m_fPopTextScale;                     // 0x2c
	TWCHAR                m_wszPopText[ MAX_POP_TEXT_LENGTH ]; // 0x30
	TFLOAT                m_fPopTextAlpha;                     // 0x50
	Toshi::T2GUIRectangle m_oDial;                             // 0x54
	Needle                m_oNeedle;                           // 0xcc
	AGUI2TextBox          m_oPopText;                          // 0x15c
	AGUI2TextBox          m_oText;                             // 0x1d4
	TFLOAT                m_fBouncePhase;                      // 0x24c
};

TSTATICASSERT( sizeof( ACFPBashHudElement::Needle ) == 0x90 );
TSTATICASSERT( sizeof( ACFPBashHudElement ) == 0x250 );
