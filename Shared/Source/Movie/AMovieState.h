#pragma once

#include "GameInterface/AGameState.h"
#include "Input/AInputManager2.h"
#include "Toshi2/T2DynamicVector.h"
#include "Input/AInputHelper.h"

class AMovieState : public AGameState
{
	AMovieState(const char* name, TBOOL a_bUseSoundChannels, TBOOL unk2, TBOOL unk3, AGameState* gameState);

	//virtual void Unk1();
	//virtual TBOOL Unk2();
	//virtual void Unk3(void*, void*);
	virtual UpdateResult OnUpdate(float deltaTime) override;
	virtual void OnInsertion() override;
	//virtual void OnRemoval();
	//virtual void OnSuspend();
	//virtual void OnResume(AGameState* pOldState);
	//virtual void OnActivate();
	//virtual void OnDeactivate();
	//virtual TBOOL CanActivateConsoleState();
	//virtual TBOOL CheckForControllerRemoval();
	//virtual TBOOL PauseOnControllerReinserted();
	virtual void PlayMovie(uint8_t unk);

	Toshi::T2DynamicVector<Toshi::TString8*> m_stringVector; // 0x30
	TBOOL m_bUseSoundChannels;                               // 0x40
	TBOOL m_bUseLocalePostFix;                               // 0x41
	AGameState* m_pGameState;                                // 0x48
	uint32_t m_flags;                                        // 0x4C
	AInputHelper m_InputHelper;                              // 0x24
	TBOOL m_bRenderWorld;                                    // 0xD9
	float m_fDeltaTime;                                      // 0xDC
};

