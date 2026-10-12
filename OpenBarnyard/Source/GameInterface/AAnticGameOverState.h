#pragma once
#include "AGameState.h"
#include "GUI/AFade.h"
#include "MiniGame/AMiniGameState.h"

#include <Toshi/TEvent.h>

//-----------------------------------------------------------------------------
// Game over screen of a minigame (TClass 00781cd4, vtable 0071b790).
// Pushed by the minigame (gatecrash: UpdateGameOver 0046dd50) with the results
// returned by AMiniGameState::GetResults().
//
// Flow (OnUpdate):
//  STATE_FADING_OUT: fades the screen and the sound out. Once the fade is over
//                    (OnFadeOver) the minigame is hidden, the "bkg_End_wood"
//                    picture is shown and the screen fades back in.
//  STATE_FADED_OUT:  lets the minigame prepare itself (cow tipping showdown only).
//  STATE_SHOW_RESULTS: adds the results child state: AAnticScoreState, or
//                    AAnticHighScoreState in story mode.
//  STATE_RESULTS:    once the child state has gone, either removes itself, lets
//                    the owning minigame continue, or restarts the minigame
//                    (AGameLoader::RestartMiniGame) which shows the instruction
//                    screen again. Quitting to the front end goes through the
//                    start (pause) menu.
//-----------------------------------------------------------------------------
class AAnticGameOverState
    : public AGameState
{
public:
	TDECLARE_CLASS( AAnticGameOverState, AGameState );

	enum STATE : TINT
	{
		STATE_FADING_OUT,
		STATE_FADED_OUT,
		STATE_SHOW_RESULTS,
		STATE_RESULTS,
	};

public:
	AAnticGameOverState( AMiniGameState::S1* a_pResults, TBOOL a_bRemoveWhenDone );
	~AAnticGameOverState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;

private:
	TBOOL OnFadeOver( AFade* a_pFade, TBOOL* a_pThrowFadeOver );

private:
	STATE                                               m_eState;            // 0xa4
	AMiniGameState::S1*                                 m_pResults;          // 0xa8
	TBOOL                                               m_bUnk;              // 0xac
	AMiniGameState*                                     m_pMiniGame;         // 0xb0
	TBOOL                                               m_bRemoveWhenDone;   // 0xb4
	Toshi::TListener<AFade, TBOOL, AAnticGameOverState> m_oFadeOverListener; // 0xb8
	TFLOAT                                              m_fVolume;           // 0xcc
};

TSTATICASSERT( sizeof( AAnticGameOverState ) == 0xd0 );
