#pragma once
#include "AHudElement.h"
#include "GUI/T2GUIString.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// Purpose: HUD element with the four face buttons of a PlayStation pad
// ("button_triangle", "button_circle", "button_cross", "button_square").
// Leftover of the console versions: only its TClass creates it and nothing
// is ever added to the buttons or the container.
// TClass "AContextButtonHudElement", vtable 0071bf44, size 0x52c.
//-----------------------------------------------------------------------------
class AContextButtonHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( AContextButtonHudElement, AHudElement );

	static constexpr TINT NUM_BUTTONS = 4;

	struct Button
	{
		// $Barnyard: FUNCTION 00450e80
		Button();

		// $Barnyard: FUNCTION 00450ee0
		~Button();

		Toshi::T2GUIElement   oElement; // 0x000
		Toshi::T2GUIRectangle oIcon;    // 0x054
		Toshi::T2GUIString    oText;    // 0x0cc
	};

public:
	AContextButtonHudElement();
	~AContextButtonHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() OVERRIDE; // +0x14
	virtual void  OnDetach() OVERRIDE; // +0x18
	virtual void  Show() OVERRIDE;     // +0x1c
	virtual void  Hide() OVERRIDE;     // +0x20

private:
	// The texture sections of the buttons (0x00781f5c)
	static Toshi::T2GUITextureSection* ms_apButtonSections[ NUM_BUTTONS ];

private:
	TINT                m_iUnk14;                  // 0x014
	Button              m_aButtons[ NUM_BUTTONS ]; // 0x018
	Toshi::T2GUIElement m_oContainer;              // 0x4d8
};
