#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "MiniGame/AMiniGameState.h"

//-----------------------------------------------------------------------------
// High score table shown after the results of a single player minigame
// (TClass 00781d04, vtable 0071b810). Replaces AAnticScoreState and is replaced
// by AAnticTrophyState (AAnticTrophyState::AAnticTrophyState) once the player presses OK, or right
// away if no high score table is selected.
//-----------------------------------------------------------------------------
class AAnticHighScoreState
    : public AGameState
{
public:
	TDECLARE_CLASS( AAnticHighScoreState, AGameState );

public:
	AAnticHighScoreState( AMiniGameState::S1* a_pResults );
	~AAnticHighScoreState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;

private:
	TBOOL               m_bDone;             // 0xa4
	AMiniGameState::S1* m_pResults;          // 0xa8
	TWCHAR              m_wszPressOK[ 128 ]; // 0xac
	AGUI2TextBox        m_oPressOK;          // 0x1ac
};

TSTATICASSERT( sizeof( AAnticHighScoreState ) == 0x224 );
