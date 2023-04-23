#include "pch.h"
#include "AFrontEndMovieState.h"
#include "Movie/AMoviePlayer.h"
#include "AppBoot.h"

#include <Toshi2/T2GUI/T2GUI.h>


AGameState::UpdateResult AFrontEndMovieState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();
    AGameState::OnUpdate(deltaTime);
    AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingletonWeak();

    if (m_iAssetId == Asset_Legal || (m_iAssetId < 3 && pMoviePlayer->IsMoviePlaying()))
    {
        m_fUnknown -= deltaTime;
    }

    /*m_Test->SetTransform(0, 0, m_TestRotAngle);
    m_TestRotAngle += deltaTime * 2.5f;*/

    switch (m_iAssetId)
    {
    case Asset_THQLogo:
    case Asset_LogoMovie:
    case Asset_Intro:
        if (m_bSkip)
        {
            if (AApplication::g_oTheApp.m_bUnk4)
            {
                StopMovieIfPlaying();
                TIMPLEMENT("Start Rendering Frontend menu");
            }
        }
        else
        {
            if (m_fUnknown <= 0.0f)
            {
                if (AApplication::g_oTheApp.m_bUnk4)
                {
                    StopMovieIfPlaying();
                    TIMPLEMENT("Start Rendering Frontend menu");
                }
            }
        }
        break;
    default:
        if (m_fUnknown <= 0.0f)
        {
            if (AApplication::g_oTheApp.m_bUnk4)
            {
                StopMovieIfPlaying();
                TIMPLEMENT("Start Rendering Frontend menu");
            }
        }
        break;
    }
    switch (m_iAssetId)
    {
    case Asset_Legal:
        if (HasBackgroundStoppedShowing())
        {
            StartMovie(Asset_THQLogo);
        }
        break;
    case Asset_THQLogo:
        if (HasMovieStopped())
        {
            StartMovie(Asset_LogoMovie);
        }
        break;
    case Asset_LogoMovie:
        if (HasMovieStopped())
        {
            StartMovie(Asset_Intro);
        }
        break;
    case Asset_Intro:
        if (HasMovieStopped())
        {
            StopMovieIfPlaying();
            m_bSkip = false;
            TIMPLEMENT("Start Rendering Frontend menu");
        }
        break;
    }

    return UpdateResult_OK;
}

void AFrontEndMovieState::OnInsertion()
{
    auto pGUI = Toshi::T2GUI::GetSingleton();
    auto pGUIRenderer = pGUI->GetRenderer();
    auto pRootElement = pGUI->GetRootElement();

    m_Background.Create(s_Assets[m_iAssetId]);
    m_Background.SetVisible(TTRUE);

    //m_Test = new Toshi::T2GUIRectangle;
    //m_Test->Create(200, 200);
    //m_Test->SetTransform(0, 0, 0);
    //m_Test->SetColour(0x00FF0000); // ARGB
    //pRootElement->AddChildHead(sm_pLoadIconRect);
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

bool AFrontEndMovieState::HasBackgroundStoppedShowing()
{
    switch (m_iAssetId)
    {
    case Asset_THQLogo:
    case Asset_LogoMovie:
    case Asset_Intro:
        if (m_bSkip)
        {
            return true;
        }
    }
    return m_fUnknown <= 0.0f;
}

bool AFrontEndMovieState::HasMovieStopped()
{
    switch (m_iAssetId)
    {
    case Asset_THQLogo:
    case Asset_LogoMovie:
    case Asset_Intro:
        if (m_bSkip)
        {
            return true;
        }
    }
    return !AMoviePlayer::GetSingletonWeak()->IsMoviePlaying();
}

void AFrontEndMovieState::StopMovieIfPlaying()
{
    AMoviePlayer* pPlayer = AMoviePlayer::GetSingletonWeak();
    if (pPlayer->IsMoviePlaying())
    {
        pPlayer->StopMovie();
    }
}

void AFrontEndMovieState::StartMovie(Asset assetId)
{
    AMoviePlayer* pPlayer = AMoviePlayer::GetSingletonWeak();

    if (pPlayer->IsMoviePlaying())
    {
        pPlayer->StopMovie();
    }

    m_iAssetId = assetId;
    m_fUnknown = 5.0f;
    m_bSkip = false;
    m_Background.SetVisible(TFALSE);
    pPlayer->PlayMovie(s_Assets[assetId], 0, 0);
}
