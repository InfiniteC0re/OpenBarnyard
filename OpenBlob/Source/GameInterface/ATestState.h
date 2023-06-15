#pragma once
#include "AGameState.h"

class ATestState :
	public Toshi::TGenericClassDerived<ATestState, AGameState, "ATestState", TMAKEVERSION(1, 0), true>
{
public:
	virtual UpdateResult OnUpdate(float deltaTime) override;
	virtual void OnRemoval() override;
	virtual void OnInsertion() override;

private:
	Toshi::T2GUIRectangle m_Menu;
};