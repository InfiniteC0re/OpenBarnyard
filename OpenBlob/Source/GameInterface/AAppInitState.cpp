#include "pch.h"
#include "AppBoot.h"
#include "AAppInitState.h"
#include "Render/ARenderer.h"
#include "Movie/AMoviePlayer.h"
#include "Movie/ADX11MoviePlayer.h"
#include "GameInterface/AFrontEndMovieState.h"

#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi2/T2GUI/T2GUIRectangle.h>
#include "AAssetStreaming.h"
#include "ALevelInformation.h"
#include "Options/AOptions.h"
#include "../../ARandom.h"

using namespace Toshi;

AGameState::UpdateResult AAppInitState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();

    if (AAssetStreaming::GetSingletonWeak()->HasActiveJobs())
    {
        ARenderer::GetSingletonWeak()->SetBackgroundColour(0, 0, 0);
        return AGameState::OnUpdate(deltaTime);
    }

    TString8 str;
    str = str.Format("data/%s.trb", AApplication::g_oTheApp.GetLevelInformationFileName());

    ALevelInformation* levelInformation = ALevelInformation::CreateSingleton();
    levelInformation->Create(str);

    AOptions::CreateSingleton();
    ARandom::CreateSingleton();

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
