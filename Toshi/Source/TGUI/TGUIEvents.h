#pragma once
#include <Input/TInputInterface.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

TOSHI_NAMESPACE_START

class TGUIWidget;
class TGUIDisplayContext;

//-----------------------------------------------------------------------------
// Purpose: the events of the GUI emitters.
//-----------------------------------------------------------------------------

// Thrown by the input emitter of a widget for the input events it receives, a
// listener sets bHandled to stop the default handling (the actions of the widget)
struct TGUIWidgetInputEvent
{
	TInputInterface::InputEvent* pEvent;   // 0x0
	TBOOL                        bHandled; // 0x4
};

// Thrown by the paint emitter of a widget after its style is painted
struct TGUIWidgetPaintEvent
{
	TGUIDisplayContext* pDisplayContext; // 0x0
};

// Thrown by the paint emitter of a screen before (Phase_Begin) and after (Phase_End) the widgets are painted
struct TGUIScreenPaintEvent
{
	enum Phase : TINT
	{
		Phase_Begin = 0,
		Phase_End   = 1,
	};

	Phase ePhase; // 0x0
};

// Thrown by the focus change emitter of a screen when the focused widget changes
struct TGUIFocusChangeEvent
{
	TGUIWidget* pOldFocus; // 0x0
	TGUIWidget* pNewFocus; // 0x4
};

// Thrown by the rollover emitter of a screen when the widget under the pointer changes
struct TGUIRolloverEvent
{
	TGUIWidget* pOldWidget; // 0x0
	TGUIWidget* pNewWidget; // 0x4
};

// Thrown by the action emitter of the buttons (PGUI) when they're pressed and by the select emitter of the menu
// trees (PGUIMenuTree) when an item is selected
struct TGUIActionEvent
{
	void* pItem;   // 0x0 the selected PGUIMenuTreeItem, TNULL for the buttons
	TBOOL bAccept; // 0x4 PGUIMenuTree: a listener clears it to refuse the selection (the buttons don't set it)
};

// Thrown by the value change emitter of the PGUI widgets that have a value (sliders...)
struct TGUIValueChangeEvent
{
	PPropertyValue oValue; // 0x0
};

TOSHI_NAMESPACE_END
