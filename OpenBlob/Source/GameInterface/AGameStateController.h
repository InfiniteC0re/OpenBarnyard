#pragma once
#include "AGameState.h"

#include "Toshi/Utils/TSingleton.h"
#include "Toshi2/T2Vector.h"

class AGameStateController : public Toshi::TSingleton<AGameStateController>
{
public:
	AGameStateController()
	{
		m_bStatePushing = false;
	}

	AGameState* GetCurrentGameState()
	{
		return m_oStateStack.Back();
	}

	void Create();
	void Update(float deltaTime);
	void PushState(AGameState* pGameState);
	void PopState(AGameState* pState);
	void PopCurrentState();

private:
	Toshi::T2Vector<AGameState*, 10> m_oStateStack;
	bool m_bStatePushing;
};