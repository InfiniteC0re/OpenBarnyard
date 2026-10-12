#pragma once
#include "HUD/AButtonSlamHudElement.h"

//-----------------------------------------------------------------------------
// Shot power bar of Barn Pool ("Pool_Power_Bar"): a horizontal AButtonSlamHudElement
// without brackets. Name from the Wii build (AGUI2NewPoolPower).
// NOTE: retail has no own vtable or TClass (the ctor doesn't store a vtable), so
// the class has no virtual methods and uses the TClass of AButtonSlamHudElement.
// Used by ABarnYardPoolMiniGameState (+0xebc) and 0048e6b0.
// ctor 00646880, dtor 00646920, size 0x4d8.
//-----------------------------------------------------------------------------
class AGUI2NewPoolPower
    : public AButtonSlamHudElement
{
public:
	AGUI2NewPoolPower();
	~AGUI2NewPoolPower();

	// Adds the bar to the HUD manager with the mask a_eMask (hidden), sets up the
	// textures and empties it. a_fUnused and a_pUnused aren't used.
	TBOOL Create( TFLOAT a_fUnused, Toshi::T2GUIElement* a_pUnused, Mask a_eMask );

	// Shows or hides the bar
	void SetBarVisible( TBOOL a_bVisible );

	// Sets the value (clamped to 0.0f..1.0f) the bar fills towards. With a_bImmediate
	// the bar is set to it right away.
	void SetValue( TFLOAT a_fValue, TBOOL a_bImmediate );

	TBOOL IsBarVisible() const { return m_bBarVisible; }

	Toshi::T2GUIRectangle& GetBarElement() { return m_oBar; }

public:
	TINT  m_iUnk1;       // 0x4cc, never read
	TINT  m_iUnk2;       // 0x4d0, never read
	TBOOL m_bBarVisible; // 0x4d4
};

TSTATICASSERT( sizeof( AGUI2NewPoolPower ) == 0x4d8 );
