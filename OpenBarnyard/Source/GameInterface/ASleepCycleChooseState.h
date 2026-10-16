#pragma once
#include "AGameState.h"
#include "AMessagePopupState.h"

#include <Toshi/TEvent.h>

//-----------------------------------------------------------------------------
// Purpose: The player goes to bed. If quests are assigned, a popup asks if the
// player really wants to sleep (Yes: ASleepCycleState), otherwise the sleep
// starts right away.
// Created by name ("ASleepCycleChooseState") by the bed. AGameStateController::
// SaveCurrentGameState pushes it with m_bCannotSave set when quests are assigned,
// then it only shows a message that the game can't be saved now.
// TClass 00781b6c, vtable 0071b198, size 0xbc.
// Member names are invented.
//-----------------------------------------------------------------------------
class ASleepCycleChooseState
    : public AGameState
{
public:
	TDECLARE_CLASS( ASleepCycleChooseState, AGameState );

	using PopupListener = Toshi::TListener<AMessagePopupState, AMessagePopupState::ResponseEvent, ASleepCycleChooseState>;

public:
	ASleepCycleChooseState();
	~ASleepCycleChooseState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// NOTE: folded with ASleepCycleState::ProcessInput (0043fc30), it only calls AGameState::ProcessInput
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE { return AGameState::ProcessInput( a_pInputEvent ); }   // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50

	void SetCannotSave( TBOOL a_bCannotSave ) { m_bCannotSave = a_bCannotSave; }

private:
	static TBOOL __stdcall OnPopupResponse( ASleepCycleChooseState* a_pState, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

private:
	TBOOL         m_bStarted;       // 0xa4, the popup is shown (or the sleep started)
	TBOOL         m_bCannotSave;    // 0xa5, only tell that the game can't be saved
	PopupListener m_oPopupListener; // 0xa8
};
