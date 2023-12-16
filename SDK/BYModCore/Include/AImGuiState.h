#pragma once
#include <BYardSDK/AGameState.h>
#include <Toshi/Input/TInputInterface.h>

TOBJECT(AImGuiState, AGameState, TTRUE)
{
public:
	AImGuiState();

	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	virtual void OnActivate() override;
	virtual void OnDeactivate() override;
	virtual TBOOL ProcessInput(const Toshi::TInputInterface::InputEvent* a_pInputEvent) override;
};
