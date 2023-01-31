#include "pch.h"
#include "AppBoot.h"
#include "AAppInitState.h"
#include "Render/ARenderer.h"
#include "Movie/AMoviePlayer.h"
#include "Movie/ADX11MoviePlayer.h"
#include "GameInterface/AFrontEndMovieState.h"

#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi2/T2GUI/T2GUIRectangle.h>

AGameState::UpdateResult AAppInitState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();

    //ARenderer::GetSingletonWeak()->SetBackgroundColour(64, 64, 64);
    AGameState::OnUpdate(deltaTime);

    // Create singletons here
    ADX11MoviePlayer::Create()->OnCreate();

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
        AGameStateController::GetSingleton()->PushState(new AFrontEndMovieState);
        TIMPLEMENT_D("The game");
    }
}
