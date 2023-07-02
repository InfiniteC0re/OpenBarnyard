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
#include "Input/AInputHelper.h"

AGameState::UpdateResult AFrontEndMovieState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();
    m_InputHelper.Update(deltaTime);

    if (m_iAssetId != Asset_Legal)
    {
        if (m_InputHelper.IsJustDown(0x2A))
        {
            // Commenting this cause handling input is not finished atm and it would skip all instantly
            // m_bSkip = true;
        }
    }

    AGameState::OnUpdate(deltaTime);
    AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingletonWeak();

    if (m_iAssetId == Asset_Legal || (m_iAssetId < 3 && pMoviePlayer->IsMoviePlaying()))
    {
        m_fBackgroundLeftTime -= deltaTime;
    }

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

    AInputManager2::GetSingletonWeak()->SetContext(AInputMap::INPUTCONTEXT_UNK8);
    m_InputHelper.AddMapping(0x2A, TFALSE, -1.0f); // SPACE
    m_InputHelper.AddMapping(0x2B, TFALSE, -1.0f); // RETURN

    m_Background.Create(s_Assets[m_iAssetId]);
    m_Background.SetVisible(TTRUE);

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
    AIMGUI_FORMAT("Video position: %.0lfms", pMoviePlayer->m_Position);
    AIMGUI_FORMAT("Audio position: %ums", audioPosition);
    AIMGUI_FORMAT("Audio delay: %dms", int(pMoviePlayer->m_Position - audioPosition));
    AIMGUI_FORMAT("Background image left time: %.2fs", m_fBackgroundLeftTime);

    if (ImGui::Button("Reset state"))
    {
        m_iAssetId = Asset_Legal;
        m_fBackgroundLeftTime = 5.0f;
        m_Background.SetVisible(TTRUE);
        StopMovieIfPlaying();
    }
}
#endif // TOSHI_DEBUG
