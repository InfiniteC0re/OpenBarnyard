#include "pch.h"
#include "AFrontEndMovieState.h"
#include "Movie/AMoviePlayer.h"
#include "AppBoot.h"

AGameState::UpdateResult AFrontEndMovieState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();
    AGameState::OnUpdate(deltaTime);

    AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingletonWeak();

    switch (m_iAssetId)
    {
    case Asset_Legal:
        StartMovie(Asset_Intro);
        break;
    }

    return UpdateResult_OK;
}

void AFrontEndMovieState::OnInsertion()
{
    TIMPLEMENT();
    AGameState::OnInsertion();
}

void AFrontEndMovieState::OnRemoval()
{
    TIMPLEMENT();
    AGameState::OnRemoval();
}

void AFrontEndMovieState::OnActivate()
{
    TIMPLEMENT();
    AGameState::OnActivate();
    AApplication::g_oTheApp.SetRenderWorld(false);
}

void AFrontEndMovieState::OnDeactivate()
{
    TIMPLEMENT();
    AGameState::OnDeactivate();
    AMoviePlayer* pPlayer = AMoviePlayer::GetSingleton();

    if (pPlayer->IsMoviePlaying())
    {
        pPlayer->StopMovie();
    }

    AApplication::g_oTheApp.SetRenderWorld(true);
}

void AFrontEndMovieState::StartMovie(Asset assetId)
{
    AMoviePlayer* pPlayer = AMoviePlayer::GetSingleton();

    if (pPlayer->IsMoviePlaying())
    {
        pPlayer->StopMovie();
    }

    m_iAssetId = assetId;
    m_fUnknown = 5.0f;
    m_bFlag1 = true;
    TTODO("(**(code **)(*(int *)&param_1_00->field_0x2c + 0x48))(0);");
    pPlayer->PlayMovie(s_Assets[assetId], TNULL, 0);
}
