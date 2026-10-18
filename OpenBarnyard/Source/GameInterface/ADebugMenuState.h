#pragma once

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include "AGameState.h"

#  include <TGUI/TGUIEvents.h>
#  include <Plugins/PGUI/PGUIMenuTree.h>
#  include <Toshi/TEvent.h>

class PGUIMenuTreeItem;

//-----------------------------------------------------------------------------
// Purpose: the in-game debug menu (development, not in Final builds). The
// DebugMenu command of the input maps (` on the keyboard, Select on the pad
// in NormalBarnyard) pushes it on top of the current game state and closes
// it. It pauses the game like the pause menu and holds the input of the
// states under it.
// It's a menu tree of the legacy TOSHI GUI (TGUI/PGUI, hosted by AGUISystem)
// made from the menu templates of Data/GUI/main.gui (CurrentMenu,
// CurrentPopupMenu, NextMenuTemplate): the arrows (the pad) move, Return
// selects or opens a submenu, Escape/Backspace go back.
// It holds the development toggles (ADebugOverlay, the layers of
// AWorldDebugDraw, the TGUI demo), the tools (the interior network editor)
// and a few actions that are safe at any time (cheats, money, time of day,
// weather).
//-----------------------------------------------------------------------------
class ADebugMenuState
    : public AGameState
{
public:
	TDECLARE_CLASS( ADebugMenuState, AGameState );

	enum ACTION
	{
		ACTION_NONE,
		ACTION_CLOSE,
		ACTION_TOGGLE_OVERLAY,
		ACTION_TOGGLE_LAYER, // iValue: AWorldDebugDraw::LAYER
		ACTION_TOGGLE_TGUI_DEMO,
		ACTION_NETWORK_EDITOR,
		ACTION_CHEAT,        // iValue: ACheatActivator::CHEAT
		ACTION_ADD_MONEY,    // iValue: pick ups to add
		ACTION_SET_MONEY,    // iValue: pick ups
		ACTION_DAY_PHASE,    // iValue: AGameTimeManager::DAYPHASE
		ACTION_TIME_SCALE,   // iValue: scale of the game time in the sim
		ACTION_WEATHER,      // iValue: AWeatherManager::WEATHERTYPE
	};

	static constexpr TINT MAX_ITEMS    = 64;
	static constexpr TINT LABEL_LENGTH = 48;

	using SelectListener = Toshi::TListener<PGUIMenuTree, Toshi::TGUIActionEvent, ADebugMenuState>;
	using CloseListener  = Toshi::TListener<PGUIMenuTree, PGUIMenuTree::CloseEvent, ADebugMenuState>;

public:
	ADebugMenuState( AInputMap* a_pUnderInputMap );
	~ADebugMenuState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnInsertion() override;
	virtual void  OnRemoval() override;

	// Called by AGameStateController::ProcessInput before the game state gets the event: the DebugMenu command opens
	// the menu on top of a_pGameState (the current state) or closes it. TTRUE if the event was used.
	static TBOOL ProcessDebugMenuInput( AGameState* a_pGameState, const Toshi::TInputInterface::InputEvent* a_pEvent );

private:
	// Can the menu open on top of a_pGameState
	static TBOOL CanOpen( AGameState* a_pGameState );

	// Is a_pEvent the DebugMenu command of a_pInputMap (or ` without an input map)
	static TBOOL IsDebugMenuEvent( AInputMap* a_pInputMap, const Toshi::TInputInterface::InputEvent* a_pEvent );

	TBOOL CreateMenu();
	void  DestroyMenu();

	PGUIMenuTreeItem* AddItem( PGUIMenuTreeItem* a_pParent, const TCHAR* a_szLabel, ACTION a_eAction, TINT a_iValue );
	void              UpdateLabels();
	// TTRUE if the menu was popped (the state is destroyed)
	TBOOL             DoAction( ACTION a_eAction, TINT a_iValue );

	// Makes the selected item visible: Button_MenuSel of main.gui looks like Button_MenuCurr
	static void HighlightSelectedStyle();

	static TBOOL __stdcall OnMenuSelect( ADebugMenuState* a_pState, PGUIMenuTree* a_pMenuTree, Toshi::TGUIActionEvent* a_pEvent );
	static TBOOL __stdcall OnMenuClose( ADebugMenuState* a_pState, PGUIMenuTree* a_pMenuTree, PGUIMenuTree::CloseEvent* a_pEvent );

private:
	struct Item
	{
		PGUIMenuTreeItem* pItem;
		ACTION            eAction;
		TINT              iValue;
		const TCHAR*      szLabel;
		TWCHAR            wszLabel[ LABEL_LENGTH ]; // the item keeps the pointer
	};

	AInputMap*     m_pUnderInputMap; // input map of the state the menu is on top of (the DebugMenu command)
	PGUIMenuTree*  m_pMenuTree;
	Item           m_aItems[ MAX_ITEMS ];
	TINT           m_iNumItems;
	ACTION         m_ePendingAction;
	TINT           m_iPendingValue;
	TBOOL          m_bWasPaused;
	TBOOL          m_bClose;
	SelectListener m_oSelectListener;
	CloseListener  m_oCloseListener;
};

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
