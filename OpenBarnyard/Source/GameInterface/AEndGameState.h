#pragma once
#include "AGameState.h"

//-----------------------------------------------------------------------------
// Purpose: the end of the game. ABarnExploreMinigameState transitions to it
// (AGameLoader::SetTransitionToGameState) once the player is the champion of
// the barn late enough in the story. It loads the ss10/ss11 terrains and every
// time it gets activated it pushes the next step: the credits, the "SS11"
// scripted sequence, a save of the game, a message and finally the front end.
// Created by name. TClass 00783be8, vtable 00723bf0, size 0xa8.
//-----------------------------------------------------------------------------
class AEndGameState
    : public AGameState
{
public:
	TDECLARE_CLASS( AEndGameState, AGameState );

	enum STEP : TINT
	{
		STEP_CREDITS,
		STEP_SSEQ,
		STEP_SAVE,
		STEP_MESSAGE,
		STEP_FRONTEND,
	};

public:
	AEndGameState();
	~AEndGameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// NOTE: OnUpdate (+0x4c) and OnRemoval (+0x54) are thunks to the AGameState ones
	virtual TBOOL ShouldPlacePlayerAtStartGoal() OVERRIDE { return TFALSE; } // +0x28, RETURNS_FALSE
	virtual void  OnInsertion() OVERRIDE;                                    // +0x50
	virtual void  OnActivate() OVERRIDE;                                     // +0x60

private:
	TINT m_iStep; // 0xa4, STEP
};

TSTATICASSERT( sizeof( AEndGameState ) == 0xa8 );
