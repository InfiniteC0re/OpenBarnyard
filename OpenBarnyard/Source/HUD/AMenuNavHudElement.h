#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: the wooden bar at the bottom of the screen that tells which buttons
// select and go back in menus and sequences. Created by AHudManager.
// TClass 00782110, vtable 0071c458.
//-----------------------------------------------------------------------------
class AMenuNavHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( AMenuNavHudElement, AHudElement );

	static constexpr TINT NUM_TEXTS  = 3;
	static constexpr TINT MAX_LENGTH = 32;

public:
	AMenuNavHudElement();
	~AMenuNavHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  OnDetach() override;
	virtual void  Show() override;
	virtual void  Hide() override;

	// Sets the texts from the locale strings a_iLeft, a_iRight and a_iCentre (-1 hides the text)
	void SetTexts( TINT a_iLeft, TINT a_iRight, TINT a_iCentre );

	// Shows the default "select" and "back" texts
	void SetDefaultTexts();

	// Moves the left and right texts a_fSpacing (relative to a quarter of the bar) away from the centre
	void SetSpacing( TFLOAT a_fSpacing );

	// Moves the bar up or down
	void SetY( TFLOAT a_fY );

	void   SetTextScale( TFLOAT a_fScale );
	TFLOAT GetTextScale();

public:
	Toshi::T2GUIRectangle m_oBacking;                  // 0x14, "Wood_Thin"
	AGUI2TextBox          m_aTexts[ NUM_TEXTS ];       // 0x8c
	TWCHAR*               m_apwszTexts[ NUM_TEXTS ];   // 0x1f4, [ MAX_LENGTH ], TMalloc'd by OnAttach
	TFLOAT                m_fSpacing;                  // 0x200
	TBOOL                 m_abFixedWidth[ NUM_TEXTS ]; // 0x204
};

TSTATICASSERT( sizeof( AMenuNavHudElement ) == 0x208 );
