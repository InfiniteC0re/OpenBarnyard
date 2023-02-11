#include "pch.h"
#include "AFrontEndMovieState.h"
#include "Movie/AMoviePlayer.h"
#include "AppBoot.h"

#include <Toshi2/T2GUI/T2GUI.h>


AGameState::UpdateResult AFrontEndMovieState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();
    AGameState::OnUpdate(deltaTime);

    /*m_Test->SetTransform(0, 0, m_TestRotAngle);
    m_TestRotAngle += deltaTime * 2.5f;*/

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
    auto pGUI = Toshi::T2GUI::GetSingleton();
    auto pGUIRenderer = pGUI->GetRenderer();
    auto pRootElement = pGUI->GetRootElement();

    //m_Background.Create(s_Assets[0]);
    //m_Background.SetVisible(TFALSE);

    //m_Test = new Toshi::T2GUIRectangle;
    //m_Test->Create(200, 200);
    //m_Test->SetTransform(0, 0, 0);
    //m_Test->SetColour(0x00FF0000); // ARGB
    //pRootElement->AddChildHead(m_Test);

    //Toshi::T2GUIRectangle* pRectangle2 = new Toshi::T2GUIRectangle;
    //pRectangle2->Create(100, 100);
    //pRectangle2->SetTransform(0, 0, 0);
    //pRectangle2->SetMaterial(pGUIRenderer->CreateMaterial("loadicon.tga"));
    //m_Test->AddChildHead(pRectangle2);

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
    AGameState::OnActivate();

    auto pRootElement = Toshi::T2GUI::GetSingletonWeak()->GetRootElement();
    pRootElement->AddChildHead(&m_Background);

    float fWidth, fHeight;
    pRootElement->GetDimensions(fWidth, fHeight);
    m_Background.SetDimensions(fWidth, fHeight);

    AApplication::g_oTheApp.SetRenderWorld(false);
}

void AFrontEndMovieState::OnDeactivate()
{
    AGameState::OnDeactivate();
    AMoviePlayer* pPlayer = AMoviePlayer::GetSingleton();

    if (pPlayer->IsMoviePlaying())
    {
        pPlayer->StopMovie();
    }

    m_Background.Unlink();
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
    //m_Background.SetVisible(TFALSE);
    pPlayer->PlayMovie(s_Assets[assetId], TNULL, 0);
}
