#pragma once

#include "GameInterface/AGameState.h"
#include "Input/AInputManager2.h"
#include "Toshi2/T2DynamicVector.h"

class AMovieState : public AGameState
{
	AMovieState(const char* name, bool a_bUseSoundChannels, bool unk2, bool unk3, AGameState* gameState);

	//virtual void Unk1();
	//virtual bool Unk2();
	//virtual void Unk3(void*, void*);
	virtual UpdateResult OnUpdate(float deltaTime);
	virtual void OnInsertion();
	//virtual void OnRemoval();
	//virtual void OnSuspend();
	//virtual void OnResume(AGameState* pOldState);
	//virtual void OnActivate();
	//virtual void OnDeactivate();
	//virtual bool CanActivateConsoleState();
	//virtual bool CheckForControllerRemoval();
	//virtual bool PauseOnControllerReinserted();
	virtual void PlayMovie(uint8_t unk);

	Toshi::T2DynamicVector<Toshi::TString8*> m_stringVector; // 0x30
	bool m_bUseSoundChannels;                                // 0x40
	bool m_bUnk2;                                            // 0x41
	AGameState* m_pGameState;                                // 0x48
	uint32_t m_flags;                                        // 0x4C
	bool m_bRenderWorld;                                     // 0xD9
	float m_fDeltaTime;                                      // 0xDC
};

