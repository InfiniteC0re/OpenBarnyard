#include "pch.h"
#include "AMovieState.h"
#include "Locale/ALocaleManager.h"

using namespace Toshi;

AMovieState::AMovieState(const char* name, bool unk, bool unk2, bool unk3, AGameState* gameState) : AGameState()
{
	
}

void AMovieState::OnInsertion()
{
	
}

void AMovieState::PlayMovie(uint32_t unk)
{
	TString8 str;
	auto localeMgr = ALocaleManager::GetSingleton();
	localeMgr->GetCurSoundChannel();



	if (m_bUnk2)
	{
		str.Concat("_");
		str.Concat(localeMgr->GetCurLocaleCode());
	}

}
