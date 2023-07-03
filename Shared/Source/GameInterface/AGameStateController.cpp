#include "pch.h"
#include "AGameStateController.h"
#include "AppBoot.h"

void AGameStateController::Create()
{
	TIMPLEMENT();

	AGameState* pDummyState = new AGameState;
	PushState(pDummyState);
}

void AGameStateController::Update(float deltaTime)
{
	AGameState* pCurrentGameState = GetCurrentGameState();
	AGameState::UpdateResult updResult = pCurrentGameState->OnUpdate(deltaTime);

	deltaTime = Toshi::TMath::Min(deltaTime, 0.1f);

	if (updResult == AGameState::UpdateResult_Remove)
	{
		pCurrentGameState->RemoveSelf();
	}

	TTODO("AGameStateController::FUN_005c6190(deltaTime);");
	TTODO("AGameStateController::FUN_005c62c0(deltaTime);");
}

void AGameStateController::PushState(AGameState* pGameState)
{
	TASSERT(m_bStatePushing == TFALSE);
	m_bStatePushing = TTRUE;

	// Suspend and deactivate the previous layer
	if (m_oStateStack.Size() > 0)
	{
		AGameState* pCurrentGameState = m_oStateStack.Back();

		pCurrentGameState->Suspend();
		pCurrentGameState->Deactivate();
	}

	// Push the game state to the vector
	m_oStateStack.PushBack(pGameState);

	pGameState->Insert();
	pGameState->Activate();

	m_bStatePushing = TFALSE;
}

void AGameStateController::PopState(AGameState* pState)
{
	TASSERT(m_oStateStack.Size() > 1);
	AGameState* pOldState = GetCurrentGameState();

	TASSERT(pState == pOldState);
	m_oStateStack.PopBack();

	pOldState->Deactivate();
	pOldState->Remove();

	if (m_oStateStack.Size() == 1)
	{
		// Leave the application if the only game state is left
		AApplication::g_oTheApp.Destroy();
	}
	else
	{
		AGameState* pNewState = GetCurrentGameState();
		pNewState->Resume(pOldState);
		pNewState->Activate();
	}

	delete pOldState;
}

void AGameStateController::ResetStack()
{
	TBOOL shouldDeactivate = TTRUE;

	while (1 < m_oStateStack.Size())
	{
		auto& state = m_oStateStack.Back();
		
		if (shouldDeactivate && HASFLAG(state->GetState() & AGameState::State_Activated))
		{
			state->OnDeactivate();
			state->RemoveState(AGameState::State_Activated);
		}

		state->Remove();
		m_oStateStack.PopBack();
	}

	TASSERT(m_oStateStack.Size() == 1);
}

void AGameStateController::PopCurrentState()
{
	AGameState* pCurrentState = GetCurrentGameState();

	if (m_oStateStack.Size() > 1)
	{
		PopState(pCurrentState);
	}
}
