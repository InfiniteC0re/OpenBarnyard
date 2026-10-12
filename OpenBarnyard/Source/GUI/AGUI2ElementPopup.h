#pragma once
#include <GUI/T2GUIElement.h>

//-----------------------------------------------------------------------------
// Makes another element (usually a text box) pop in and out: the element is
// attached as a child and scaled with an overshooting curve, and it can pulse
// while it's shown. Plain T2GUIElement (no TClass), size 0x70.
// vtable 0071fa68: +0x00 dtor 004ab250, +0x04 Tick 00637090,
// +0x48 SetShown 00637030, +0x4c SetShownInstantly 00637050.
// NOTE: invented name, the original one is unknown.
// Used by the BarnTender minigame (ATapperMiniGameState).
//-----------------------------------------------------------------------------
class AGUI2ElementPopup
    : public Toshi::T2GUIElement
{
public:
	// Bits of m_uiEffects (indices for SetEffect)
	enum EFFECT : TINT
	{
		EFFECT_PULSE = 1, // pulses while fully shown (enabled by Create)
	};

public:
	// NOTE: inlined into its users in the original (T2GUIElement ctor, m_uiEffects = 0)
	AGUI2ElementPopup();
	~AGUI2ElementPopup();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Starts popping in (TTRUE) or out (TFALSE)
	virtual void SetShown( TBOOL a_bShown );

	// Shows/hides the element without animating
	virtual void SetShownInstantly( TBOOL a_bShown );

	// Attaches a_pElement to the centre of this element, takes its size and
	// hides everything (call SetShown to pop it in)
	void Create( Toshi::T2GUIElement* a_pElement );

	// Enables/disables an EFFECT
	void SetEffect( TINT a_iEffect, TBOOL a_bEnable );

private:
	void Setup();

public:
	Toshi::T2GUIElement* m_pElement;     // 0x54
	TFLOAT               m_fTargetScale; // 0x58, 0.0 or 1.0
	TFLOAT               m_fScale;       // 0x5c, moves to m_fTargetScale
	TFLOAT               m_fPulsePhase;  // 0x60
	TFLOAT               m_fPulseSpeed;  // 0x64
	TUINT                m_uiEffects;    // 0x68, bits are EFFECT
	TBOOL                m_bShown;       // 0x6c
};

TSTATICASSERT( sizeof( AGUI2ElementPopup ) == 0x70 );
