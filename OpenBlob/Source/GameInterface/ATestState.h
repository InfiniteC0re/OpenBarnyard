#pragma once
#include "AGameState.h"

class ATestState :
	public Toshi::TGenericClassDerived<ATestState, AGameState, "ATestState", TMAKEVERSION(1, 0), TTRUE>
{
public:
	virtual UpdateResult OnUpdate(float deltaTime) override;
	virtual void OnRemoval() override;
	virtual void OnInsertion() override;

#ifdef TOSHI_DEBUG
    virtual void DEBUG_RenderImGui() override;
#endif

private:
	Toshi::T2GUIRectangle m_Menu;
};