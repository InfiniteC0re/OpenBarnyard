#include "pch.h"
#include "AFrontEndMovieState.h"
#include "Movie/AMoviePlayer.h"
#include "ImGui/AImGui.h"
#include "AppBoot.h"

#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi/Shaders/SysShader/TSysShaderHAL.h>

#ifdef TOSHI_DEBUG
// Used to output debug info
#include "Movie/ADX11MoviePlayer.h"
#endif // TOSHI_DEBUG

AGameState::UpdateResult AFrontEndMovieState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();
    AGameState::OnUpdate(deltaTime);
    AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingletonWeak();

    if (m_iAssetId == Asset_Legal || (m_iAssetId < 3 && pMoviePlayer->IsMoviePlaying()))
    {
        m_fBackgroundLeftTime -= deltaTime;
    }

    //m_Test->SetTransform(0, 0, m_TestRotAngle);
    //m_TestRotAngle += deltaTime * 2.5f;

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
                TIMPLEMENT_D("Start Rendering Frontend menu");
            }
        }
        else
        {
            if (m_fBackgroundLeftTime <= 0.0f)
            {
                if (AApplication::g_oTheApp.m_bUnk4)
                {
                    StopMovieIfPlaying();
                    TIMPLEMENT_D("Start Rendering Frontend menu");
                }
            }
        }
        break;
    default:
        if (m_fBackgroundLeftTime <= 0.0f)
        {
            if (AApplication::g_oTheApp.m_bUnk4)
            {
                StopMovieIfPlaying();
                TIMPLEMENT_D("Start Rendering Frontend menu");
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
            m_bSkip = TFALSE;
            TIMPLEMENT_D("Start Rendering Frontend menu");
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

    auto pSysShader = Toshi::TSysShaderHAL::GetSingletonWeak();
    auto pTestMaterial = pSysShader->CreateMaterial();
    pTestMaterial->Create(Toshi::TSysMaterial::BlendMode::Default);

    m_Test = new Toshi::T2GUIRectangle;
    m_Test->Create(200, 200);
    m_Test->SetTransform(0, 0, 0);
    m_Test->SetColour(0x00FF0000); // ARGB
    pRootElement->AddChildHead(sm_pLoadIconRect);
    pRootElement->AddChildHead(m_Test);

    Toshi::T2GUIRectangle* pRectangle2 = new Toshi::T2GUIRectangle;
    pRectangle2->Create(100, 100);
    pRectangle2->SetTransform(0, 0, 0);
    pRectangle2->SetColour(0x0000FF00);
    //pRectangle2->SetMaterial(pGUIRenderer->CreateMaterial("loadicon.tga"));
    m_Test->AddChildHead(pRectangle2);

    delete pRectangle2;
    delete m_Test;

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

    AApplication::g_oTheApp.SetRenderWorld(TFALSE);
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
    AApplication::g_oTheApp.SetRenderWorld(TTRUE);
}

TBOOL AFrontEndMovieState::HasBackgroundStoppedShowing()
{
    switch (m_iAssetId)
    {
    case Asset_THQLogo:
    case Asset_LogoMovie:
    case Asset_Intro:
        if (m_bSkip)
        {
            return TTRUE;
        }
    }

    return m_fBackgroundLeftTime <= 0.0f;
}

TBOOL AFrontEndMovieState::HasMovieStopped()
{
    switch (m_iAssetId)
    {
    case Asset_THQLogo:
    case Asset_LogoMovie:
    case Asset_Intro:
        if (m_bSkip)
        {
            return TTRUE;
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
    m_fBackgroundLeftTime = 5.0f;
    m_bSkip = TFALSE;
    m_Background.SetVisible(TFALSE);
    pPlayer->PlayMovie(s_Assets[assetId], 0, 0);
}

#ifdef TOSHI_DEBUG
void AFrontEndMovieState::DEBUG_RenderImGui()
{
    auto pMoviePlayer = TSTATICCAST(ADX11MoviePlayer*, AMoviePlayer::GetSingletonWeak());

    unsigned int audioPosition = 0;

    if (pMoviePlayer->m_pChannel)
        pMoviePlayer->m_pChannel->getPosition(&audioPosition, FMOD_TIMEUNIT_MS);

    AIMGUI_FORMAT("Video file: %s", pMoviePlayer->m_CurrentFileName);
    AIMGUI_FORMAT("Current asset: %s", s_Assets[m_iAssetId]);
    AIMGUI_FORMAT("Video position: %.2lf", pMoviePlayer->m_Position);
    AIMGUI_FORMAT("Audio position: %u", audioPosition);
    AIMGUI_FORMAT("Audio delay: %d", int(pMoviePlayer->m_Position - audioPosition));
    AIMGUI_FORMAT("Background image left time: %.2f", m_fBackgroundLeftTime);
}
#endif // TOSHI_DEBUG
