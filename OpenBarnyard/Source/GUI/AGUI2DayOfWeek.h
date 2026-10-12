#pragma once
#include "AGUI2TextBox.h"

#include <GUI/T2GUIElement.h>

//-----------------------------------------------------------------------------
// Purpose: the name of the day that shows up for a few seconds at the top of
// the screen when a new day starts in the sim. Owned by ASimState (+0xa064).
// vtable 0071af08: +0x00 dtor 0043ce70, +0x04 Tick 00637f80.
//-----------------------------------------------------------------------------
class AGUI2DayOfWeek
    : public Toshi::T2GUIElement
{
public:
	AGUI2DayOfWeek() = default;
	~AGUI2DayOfWeek();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	// Creates the text and adds the element to the GUI
	void Create();

	// Removes the text
	void Destroy();

	// Shows the name of a_iWeekDay (0 is Monday, 6 is Sunday)
	void Show( TINT a_iWeekDay );

public:
	AGUI2TextBox m_oText;     // 0x54
	TFLOAT       m_fTimeLeft; // 0xcc, seconds before the text is hidden
	TFLOAT       m_fFadeTime; // 0xd0, the text fades out in the last m_fFadeTime seconds
};

TSTATICASSERT( sizeof( AGUI2DayOfWeek ) == 0xd4 );
