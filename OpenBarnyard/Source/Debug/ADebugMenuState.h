#pragma once

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include "GameInterface/AGameState.h"

//-----------------------------------------------------------------------------
// Purpose: the game state behind the "Prime Debug Menu" (development, not in
// Final builds; the menu itself is PrimeDebug's PDebugMenu, filled by
// APrimeDebug). The DebugMenu command of the input maps (` on the keyboard,
// Select on the pad in NormalBarnyard) pushes it on top of the current game
// state and closes it; Home (Prime's own key) does the same through
// APrimeDebug. While it's on top the game is paused like by the pause menu,
// the states under it get no input and the menu reads the keyboard
// (PKeyboard): Up/Down select, Return/Escape go in/back, Left/Right (held:
// faster) change a value, digits type one in.
// The menu entries that change the game states (the network editor, the
// cheats, Close) ask the state to do it in its update.
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
		ACTION_NETWORK_EDITOR,
		ACTION_CHEAT, // iValue: ACheatActivator::CHEAT
	};

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

	// Called by AGameStateController::ProcessInput before the game state and the TGUI screen get the event: the
	// DebugMenu command opens the menu on top of a_pGameState (the current state) or closes it, the menu takes the
	// other events while it's open. TTRUE if the event was used.
	static TBOOL ProcessDebugMenuInput( AGameState* a_pGameState, const Toshi::TInputInterface::InputEvent* a_pEvent );

	// Opens the menu on top of the current state (Home), TFALSE if it can't open there
	static TBOOL Open();

	// The open menu, TNULL if it's closed
	static ADebugMenuState* GetOpenMenu() { return ms_pOpenMenu; }

	// Does a_eAction in the next update of the open menu (TFALSE if it's closed)
	static TBOOL RequestAction( ACTION a_eAction, TINT a_iValue );

private:
	// Can the menu open on top of a_pGameState
	static TBOOL CanOpen( AGameState* a_pGameState );

	// Is a_pEvent the DebugMenu command of a_pInputMap (or ` without an input map)
	static TBOOL IsDebugMenuEvent( AInputMap* a_pInputMap, const Toshi::TInputInterface::InputEvent* a_pEvent );

	// TTRUE if the menu was popped (the state is destroyed)
	TBOOL DoAction( ACTION a_eAction, TINT a_iValue );

private:
	AInputMap* m_pUnderInputMap; // input map of the state the menu is on top of (the DebugMenu command)
	ACTION     m_ePendingAction;
	TINT       m_iPendingValue;
	TBOOL      m_bWasPaused;
	TBOOL      m_bClose;

	static ADebugMenuState* ms_pOpenMenu;
};

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
