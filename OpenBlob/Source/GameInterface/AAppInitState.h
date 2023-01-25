#pragma once
#include "AGameState.h"

class AAppInitState : public AGameState
{
public:
	virtual UpdateResult OnUpdate(float deltaTime) override;
	virtual void OnInsertion() override;
	virtual void OnRemoval() override;
};