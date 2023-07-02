#pragma once
#include "AGameState.h"

class AAppInitState :
	public Toshi::TGenericClassDerived<AAppInitState, AGameState, "AAppInitState", TMAKEVERSION(1, 0), TTRUE>
{
public:
	virtual UpdateResult OnUpdate(float deltaTime) override;
	virtual void OnInsertion() override;
	virtual void OnRemoval() override;
};