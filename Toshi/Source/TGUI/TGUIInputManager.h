#pragma once
#include <Input/TInputInterface.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: gets the input events of a screen (TGUIScreen::NotifyInputEvent
// gives them to the input manager at the top of its stack) with the input
// actions of their doodads (TGUIActionMap). The screen is its own first
// input manager.
//-----------------------------------------------------------------------------
class TGUIInputManager
{
public:
	// +0x08
	virtual TBOOL DoButtonPress( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) = 0;

	// +0x0C
	virtual TBOOL DoButtonRepeat( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) = 0;

	// +0x10
	virtual TBOOL DoButtonRelease( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) = 0;

	// +0x14
	virtual TBOOL DoAxisMove( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) = 0;

	// +0x18
	virtual TBOOL DoPointerMove( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) = 0;

	// +0x1C: Updates the widget under the pointer
	virtual void DoRolloverUpdate() = 0;
};

TOSHI_NAMESPACE_END
