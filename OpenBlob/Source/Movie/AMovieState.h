#pragma once

#include "GameInterface/AGameState.h"
#include "Input/AInputManager2.h"

class AMovieState : public AGameState
{
	AMovieState();

	virtual void Unk1();
	virtual bool Unk2();
	virtual void Unk3(void*, void*);
	virtual UpdateResult OnUpdate(float deltaTime);
	virtual void OnInsertion();
	virtual void OnRemoval();
	virtual void OnSuspend();
	virtual void OnResume(AGameState* pOldState);
	virtual void OnActivate();
	virtual void OnDeactivate();
	virtual bool CanActivateConsoleState();
	virtual bool CheckForControllerRemoval();
	virtual bool PauseOnControllerReinserted();
};

