#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: Shows the place of the player in a race ("1" "st", "2" "nd"...) on a
// wood backing. Used by Joy Ride (and by the code at 00528610/0052a454).
// TClass 0x00781e3c (CreateObject 004573a0, CreateObjectInPlace 00457400), vtable 0x0071c604.
//-----------------------------------------------------------------------------
class ARacePlacingHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ARacePlacingHudElement, AHudElement );

public:
	ARacePlacingHudElement();
	~ARacePlacingHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() OVERRIDE;                      // +0x14
	virtual void  OnDetach() OVERRIDE;                      // +0x18
	virtual void  Show() OVERRIDE;                          // +0x1c
	virtual void  Hide() OVERRIDE;                          // +0x20
	virtual void  Update( TFLOAT a_fDeltaTime ) OVERRIDE {} // +0x24, DOES_NOTHING
	virtual void  Reset() OVERRIDE {}                       // +0x28, DOES_NOTHING

	// Sets the place (0 is first, 3 and above show the 4th suffix with no number)
	void SetPlacing( TINT a_iPlacing );

	TINT GetPlacing() const { return m_iPlacing; }

	Toshi::T2GUIRectangle& GetBacking() { return m_oBacking; }

private:
	TINT                  m_iPlacing;    // 0x14
	Toshi::T2GUIRectangle m_oBacking;    // 0x18, "Score_Wood_Thin"
	AGUI2TextBox          m_oNumberText; // 0x90
	AGUI2TextBox          m_oSuffixText; // 0x108
};

TSTATICASSERT( sizeof( ARacePlacingHudElement ) == 0x180 );
