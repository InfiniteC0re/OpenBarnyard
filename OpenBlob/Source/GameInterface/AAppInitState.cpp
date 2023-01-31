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
        auto pGUI = Toshi::T2GUI::GetSingleton();
        auto pRootElement = pGUI->GetRootElement();

        Toshi::T2GUIRectangle* pRectangle = new Toshi::T2GUIRectangle;
        pRectangle->Create(200, 200);
        pRectangle->SetTransform(0, 0, 0);
        pRectangle->SetColour(0x00FF0000); // ARGB
        pRootElement->AddChildHead(pRectangle);

        AGameStateController::GetSingleton()->PushState(new AFrontEndMovieState);
        TIMPLEMENT_D("The game");
    }
}
