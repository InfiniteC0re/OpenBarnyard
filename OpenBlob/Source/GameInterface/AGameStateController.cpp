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
	if (m_GameStates.Size() > 0)
	{
		AGameState* pCurrentGameState = m_GameStates.Back();

		pCurrentGameState->Suspend();
		pCurrentGameState->Deactivate();
	}

	// Push the game state to the vector
	m_GameStates.PushBack(pGameState);

	pGameState->Insert();
	pGameState->Activate();

	m_bStatePushing = TFALSE;
}

void AGameStateController::PopState(AGameState* pState)
{
	TASSERT(m_GameStates.Size() > 1);
	AGameState* pOldState = GetCurrentGameState();

	TASSERT(pState == pOldState);
	m_GameStates.PopBack();

	pOldState->Deactivate();
	pOldState->Remove();

	if (m_GameStates.Size() == 1)
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

void AGameStateController::PopCurrentState()
{
	AGameState* pCurrentState = GetCurrentGameState();

	if (m_GameStates.Size() > 1)
	{
		PopState(pCurrentState);
	}
}
