#pragma once

#include "GameInterface/AGameState.h"
#include "Input/AInputManager2.h"

class AMovieState : public AGameState
{
	AMovieState(const char* name, bool unk, bool unk2, bool unk3, AGameState* gameState);

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
	virtual void PlayMovie(uint32_t unk);

	// T2DynamicArray

	bool m_bUnk; // 0x40
	bool m_bUnk2; // 0x41
};

