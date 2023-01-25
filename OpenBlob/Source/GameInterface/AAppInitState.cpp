#include "pch.h"
#include "AppBoot.h"
#include "AAppInitState.h"
#include "Render/ARenderer.h"

AGameState::UpdateResult AAppInitState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();

    ARenderer::GetSingletonWeak()->SetBackgroundColour(64, 64, 64);
    return AGameState::OnUpdate(deltaTime);

    return UpdateResult_Remove;
}

void AAppInitState::OnInsertion()
{
    TIMPLEMENT();
    AGameState::OnInsertion();
}

void AAppInitState::OnRemoval()
{
    AGameState::OnRemoval();

    if (AApplication::g_oTheApp.ShouldLoadModelViewState())
    {
        TIMPLEMENT_D("AModelViewState");
    }
    else
    {
        TIMPLEMENT_D("The game");
    }
}
