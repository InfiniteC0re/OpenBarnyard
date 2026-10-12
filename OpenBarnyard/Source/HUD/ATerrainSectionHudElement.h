#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

class ATerrainSection;

//-----------------------------------------------------------------------------
// Purpose: the name of the area of the farm the player walks into, faded in
// at the top of the screen for a few seconds. Created by AHudManager.
// TClass 0078221c, vtable 0071c80c.
//-----------------------------------------------------------------------------
class ATerrainSectionHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ATerrainSectionHudElement, AHudElement );

	static constexpr TFLOAT SHOW_TIME = 2.0f; // 0x00773738

public:
	ATerrainSectionHudElement();
	~ATerrainSectionHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;

	// Fades a_wszText in (TNULL hides the text). Does nothing while the player interacts.
	void SetText( const TWCHAR* a_wszText );

public:
	AGUI2TextBox     m_oText;         // 0x14
	TBOOL            m_bShowText;     // 0x8c
	TFLOAT           m_fShowTimeLeft; // 0x90
	TBOOL            m_bFadingIn;     // 0x94
	ATerrainSection* m_pSection;      // 0x98, the area of the terrain the player is in
};

TSTATICASSERT( sizeof( ATerrainSectionHudElement ) == 0x9c );
