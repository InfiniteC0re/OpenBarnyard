#pragma once
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIElement.h>

class AUnit;

//-----------------------------------------------------------------------------
// "P1".."P4" / "Com" marker floating above a minigame player.
// Plain T2GUIElement (no TClass), vtable 0071c550.
//-----------------------------------------------------------------------------
class AMiniGamePlayerFloat
    : public Toshi::T2GUIElement
{
public:
	// Height above the unit's position the marker floats at.
	static constexpr TFLOAT HEIGHT_OFFSET = 2.0f;

public:
	AMiniGamePlayerFloat();
	~AMiniGamePlayerFloat();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;
	virtual void SetAlpha( TFLOAT a_fAlpha ) override;

	// Binds the float to a player slot and its unit and attaches it to the GUI root.
	TBOOL SetPlayer( TINT a_iSlot, TBOOL a_bHuman, AUnit* a_pUnit );

	// TRUE makes the float shrink away (set by AMiniGameState::OnUpdate once the start HUD hides).
	void  SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }
	TBOOL IsEnabled() const { return m_bEnabled; }

public:
	TWCHAR       m_wszLabel[ 8 ]; // 0x54, "P%d" or "Com"
	AGUI2TextBox m_oText;         // 0x64
	AUnit*       m_pUnit;         // 0xdc
	TFLOAT       m_fHeightOffset; // 0xe0
	TFLOAT       m_fScale;        // 0xe4, 1.0f, goes down to 0 once m_bEnabled is set
	TBOOL        m_bEnabled;      // 0xe8
};

TSTATICASSERT( sizeof( AMiniGamePlayerFloat ) == 0xec );
