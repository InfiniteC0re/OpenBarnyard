#pragma once
#include <Toshi/TNodeList.h>
#include <Toshi/TArray.h>
#include <Input/TInputInterface.h>

TOSHI_NAMESPACE_START

class TGUIScreen;
class TGUIWidget;
class TGUIWidgetAction;

//-----------------------------------------------------------------------------
// Purpose: input context of a screen, implemented by the game: shows the
// commands (doodads) available for the focused widget (the "context
// commands", e.g. the button prompts) and gets the actions fired by the
// widgets.
//
// NOTE: follows Nicktoons, which has an unknown virtual after
// FireWidgetAction. The input mappings and the functions managing the context
// commands are only in JPOG.
//-----------------------------------------------------------------------------
class TGUIInputContext : public TObject
{
public:
	TDECLARE_CLASS( TGUIInputContext, TObject );

	// A command shown by the input context
	struct ContextCommandInfo : public TNodeList<ContextCommandInfo>::TNode
	{
		ContextCommandInfo( TINT a_iCommand )
		    : iCommand( a_iCommand )
		{}

		virtual ~ContextCommandInfo();

		TINT iCommand; // 0x0C the doodad
	};

	// An input mapping (sorted by doodad)
	struct InputMapping
	{
		TINT iDoodad;  // 0x0
		TINT iCommand; // 0x4
	};

public:
	TGUIInputContext();
	~TGUIInputContext();

	// +0x14: Shows the context commands of the focused widget of the screen (the actions of its widget class and the
	// actions of the widget)
	virtual void UpdateStatusDisplay();

	// +0x18: Called when a widget fires an action
	virtual void FireWidgetAction( TGUIWidget* a_pWidget, TGUIWidgetAction* a_pAction );

	// +0x1C
	// NOTE: unknown, does nothing in Nicktoons
	virtual void Unknown_0x1C();

	// +0x20: Shows a context command with a command ID (0 for the actions of the widget class) and a label
	virtual void SetContextCommand( ContextCommandInfo& a_rCommand, TINT a_iCommandID, const TWCHAR* a_wszLabel ) = 0;

	void AddContextCommand( TINT a_iCommand );
	void RemoveContextCommand( TINT a_iCommand );

	// Maps the doodad a_iDoodad to the command a_iCommand (a doodad can have several commands)
	void  AddInputMapping( TINT a_iCommand, TINT a_iDoodad );
	TBOOL IsInput( TINT a_iDoodad, TINT a_iCommand );

	// Index of the first mapping of the doodad of the event (-1 if there's none)
	TINT FindInputMapping( const TInputInterface::InputEvent* a_pEvent );

	// The command of the mapping a_rIndex if it's a mapping of the doodad of the event (moves a_rIndex to the next one), -1 otherwise
	TINT GetNextInputMapping( const TInputInterface::InputEvent* a_pEvent, TINT& a_rIndex );

	// $NicktoonsPS2: FUNCTION 00365138
	// $JPOG: FUNCTION TGuiInterface.dll+0000f650
	void SetScreen( TGUIScreen* a_pScreen ) { m_pScreen = a_pScreen; }

	TGUIScreen* GetScreen() const { return m_pScreen; }

	TNodeList<ContextCommandInfo>& GetContextCommands() { return m_oContextCommands; }

private:
	TGUIScreen*                   m_pScreen;          // 0x04
	TNodeList<ContextCommandInfo> m_oContextCommands; // 0x08
	TArray<InputMapping>          m_aInputMappings;   // 0x18
};

TOSHI_NAMESPACE_END
