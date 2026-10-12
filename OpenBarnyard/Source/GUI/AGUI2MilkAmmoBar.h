#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: row of five milk bottles ("Joy_Ride_Bottle") showing the milk ammo
// of the player's launcher. Used by ASimPlayerMainHudElement.
// vtable 0073a478: +0x00 dtor 006428b0, +0x04 Tick 00642710.
//-----------------------------------------------------------------------------
class AGUI2MilkAmmoBar
    : public Toshi::T2GUIElement
{
public:
	static constexpr TINT NUM_BOTTLES = 5;

public:
	AGUI2MilkAmmoBar();
	~AGUI2MilkAmmoBar();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	// Creates the bottles, each one a_fHeight high and moved up or down by a_fOffsetY.
	// The empty bottles fade down to a_fMinAlpha.
	void Create( TFLOAT a_fHeight, TFLOAT a_fOffsetY, TFLOAT a_fMinAlpha );

	// Shows floor( a_iAmmo / a_iMaxAmmo * NUM_BOTTLES ) full bottles.
	void SetAmmo( TINT a_iAmmo, TINT a_iMaxAmmo );

	// Fades every bottle out when a_bDisabled is set (the launcher can't be used).
	void SetDisabled( TBOOL a_bDisabled ) { m_bDisabled = a_bDisabled; }

public:
	Toshi::T2GUIRectangle m_aBottles[ NUM_BOTTLES ]; // 0x54
	TINT                  m_iNumFull;                // 0x2ac
	TFLOAT                m_fMinAlpha;               // 0x2b0
	TBOOL                 m_bCreated;                // 0x2b4
	TBOOL                 m_bDisabled;               // 0x2b5
};

TSTATICASSERT( sizeof( AGUI2MilkAmmoBar ) == 0x2b8 );
