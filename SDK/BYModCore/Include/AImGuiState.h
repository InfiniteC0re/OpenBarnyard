#pragma once
#include <BYardSDK/AGameState.h>
#include <Input/TInputInterface.h>

class AImGuiState : public AGameState
{
public:
	TDECLARE_CLASS(AImGuiState, AGameState);

public:
	AImGuiState();

	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	virtual void OnActivate() override;
	virtual void OnDeactivate() override;
	virtual TBOOL ProcessInput(const Toshi::TInputInterface::InputEvent* a_pInputEvent) override;
};
