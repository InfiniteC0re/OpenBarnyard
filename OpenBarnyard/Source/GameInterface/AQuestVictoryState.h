#pragma once
#include "AGameState.h"
#include "GUI/AGUI2Dialog.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Menu.h"
#include "GUI/AGUI2Button.h"

#include <GUI/T2GUIFont.h>
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: a cut "Mission accomplished. Save?" Yes/No dialog that pauses the
// game and saves the game state when Yes is chosen.
// PC retail only has the TClass (size 0xb04, parent AGameState). Recovered from
// the PS2 build (SLUS_212.77, vtable 0x50b000, size 0xb1c), which keeps every
// virtual but not the constructor (see Docs/PS2DeadCode.md).
// NOTE: nothing creates it in any build (no constructor, no CreateObject caller,
// no FSM reference), so it's not wired into the quest flow.
//-----------------------------------------------------------------------------
class AQuestVictoryState
    : public AGameState
{
public:
	TDECLARE_CLASS( AQuestVictoryState, AGameState );

public:
	AQuestVictoryState();
	~AQuestVictoryState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

private:
	// AGUI2Menu activation callback (a_pUserData is the state)
	static void MenuItemActivationCallback( void* a_pUserData, AGUI2MenuItem* a_pItem );

	void OnMenuItemActivated( AGUI2MenuItem* a_pItem );

private:
	Toshi::T2GUIFont*     m_pFont;      // PS2: 0xa8, "Rekord26"
	Toshi::T2GUIFont*     m_pSmallFont; // PS2: 0xac, "Rekord18"
	AGUI2Dialog           m_oDialog;    // PS2: 0xb0
	AGUI2TextBox          m_oText;      // PS2: 0x568, "Mission accomplished. Save?"
	Toshi::T2GUIRectangle m_oContent;   // PS2: 0x5e4, transparent, holds the text and the menu
	AGUI2Menu             m_oMenu;      // PS2: 0x65c
	AGUI2Button           m_oYes;       // PS2: 0x6fc
	AGUI2Button           m_oNo;        // PS2: 0x908
	TBOOL                 m_bDone;      // PS2: 0xb14
	TBOOL                 m_bSave;      // PS2: 0xb18
};

TSTATICASSERT( sizeof( AQuestVictoryState ) == 0xb04 );
