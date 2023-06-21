#include "pch.h"
#include "AMovieState.h"
#include "Locale/ALocaleManager.h"
#include <Movie/AMoviePlayer.h>
#include "AAssetStreaming.h"
#include <AppBoot.h>

using namespace Toshi;

AMovieState::AMovieState(const char* name, TBOOL a_bUseSoundChannels, TBOOL bUseLocalePostFix, TBOOL unk3, AGameState* gameState) : AGameState(), m_stringVector(&T2Allocator::s_GlobalAllocator, 1, 0)
{
    m_bUseSoundChannels = a_bUseSoundChannels;
    m_bUseLocalePostFix = bUseLocalePostFix;
    m_flags = 0;
    m_fDeltaTime = 0.0f;

    m_stringVector.PushBack(new TString8(name));
}

AGameState::UpdateResult AMovieState::OnUpdate(float deltaTime)
{
    TIMPLEMENT();
    TTODO("Update AInputHelper");
    m_fDeltaTime += deltaTime;

    switch (m_flags)
    {
    case 0:
        if (!AAssetStreaming::GetSingletonWeak()->HasActiveJobs())
        {
            m_bRenderWorld = AApplication::g_oTheApp.m_bRenderWorld;
            AApplication::g_oTheApp.SetRenderWorld(TFALSE);
            TIMPLEMENT_D("AMovieState::PauseSounds(1,0)");
            PlayMovie(0);
        }
        break;
    case 1:

    default:
        break;
    }

    return UpdateResult();
}

void AMovieState::OnInsertion()
{
    AInputManager2::GetSingleton()->SetContext(AInputMap::UNK4);
}

void AMovieState::PlayMovie(uint8_t unk)
{
    TString8 str = **m_stringVector.Begin();
    ALocaleManager& localeMgr = ALocaleManager::Instance();

    if (m_bUseLocalePostFix)
    {
        str.Concat("_");
        str.Concat(localeMgr.GetCurLocaleCode());
    }

    int soundChannel = 0;

    if (m_bUseSoundChannels)
    {
        soundChannel = localeMgr.GetCurSoundChannel();
    }

    TBOOL bRes = AMoviePlayer::GetSingletonWeak()->PlayMovie(str, soundChannel, unk);
    m_fDeltaTime = 0.0f;
    m_flags = !bRes ? 3 : 1;
}
