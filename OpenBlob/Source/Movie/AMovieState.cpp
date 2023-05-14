#include "pch.h"
#include "AMovieState.h"
#include "Locale/ALocaleManager.h"
#include <Movie/AMoviePlayer.h>
#include "AAssetStreaming.h"
#include <AppBoot.h>

using namespace Toshi;

AMovieState::AMovieState(const char* name, bool a_bUseSoundChannels, bool unk2, bool unk3, AGameState* gameState) : AGameState(), m_stringVector(&T2Allocator::s_GlobalAllocator, 1, 0)
{
	m_bUseSoundChannels = a_bUseSoundChannels;
	m_bUnk2 = unk2;
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
			AApplication::g_oTheApp.SetRenderWorld(false);
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
	TString8 str = TString8(*m_stringVector.Begin());
	auto localeMgr = ALocaleManager::GetSingleton();

	if (m_bUnk2)
	{
		str.Concat("_");
		str.Concat(localeMgr->GetCurLocaleCode());
	}

	int soundChannel = 0;

	if (m_bUseSoundChannels)
	{
		soundChannel = localeMgr->GetCurSoundChannel();
	}

	bool bRes = AMoviePlayer::GetSingletonWeak()->PlayMovie(str, soundChannel, unk);
	m_fDeltaTime = 0.0f;
	m_flags = !bRes ? 3 : 1;
}
