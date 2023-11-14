#pragma once
#include "AGameState.h"
#include "AGameStateControllerEvent.h"
#include "GUI/AGUI2Rectangle.h"

#include <Toshi/Core/TTask.h>
#include <Toshi2/T2Vector.h>
#include <Toshi2/T2DynamicVector.h>

TOBJECT(AGameStateController, Toshi::TTask, TTRUE),
	public Toshi::TSingleton<AGameStateController>
{
public:
	AGameStateController();

	void PushState(AGameState* a_pGameState);

private:
	void AddGameState(AGameState* a_pGameState);

private:
	// ...
	Toshi::T2Vector<AGameState*, 10> m_oStates;
	void* m_VectorData[5];
	Toshi::T2DynamicVector<void*> m_UnkVector;
	// ...
	AGUI2Rectangle m_Rectangle;
	TUINT16 m_eFlags;
};
