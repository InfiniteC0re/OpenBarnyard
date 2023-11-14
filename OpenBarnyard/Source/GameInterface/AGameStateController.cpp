#include "pch.h"
#include "AGameStateController.h"

TOSHI_NAMESPACE_USING

AGameStateController::AGameStateController() :
	m_UnkVector(m_VectorData, sizeof(m_VectorData) / sizeof(*m_VectorData), 0)
{
	// Incomplete
	m_eFlags = 0;
}

void AGameStateController::AddGameState(AGameState* a_pGameState)
{
	TGlobalEmitter<AGameStateControllerEvent>::Throw(
		AGameStateControllerEvent(a_pGameState, AGameStateControllerEvent::Type_GameStateBeingAdded)
	);

	m_oStates.PushBack(a_pGameState);
	TTODO("Call virtual methods of AGameState");

	TGlobalEmitter<AGameStateControllerEvent>::Throw(
		AGameStateControllerEvent(a_pGameState, AGameStateControllerEvent::Type_GameStateAdded)
	);
}

void AGameStateController::PushState(AGameState* a_pGameState)
{
	if (m_oStates.Size() > 1)
	{
		TTODO("Call virtual methods of AGameState on the top of stack");

		TGlobalEmitter<AGameStateControllerEvent>::Throw(
			AGameStateControllerEvent(
				m_oStates.Back(),
				AGameStateControllerEvent::Type_GameStateSuspended
			)
		);
	}

	AddGameState(a_pGameState);
}
