#include "pch.h"
#include "AOptions.h"
#include <Toshi/Sound/TSound.h>

using namespace Toshi;

void AOptions::SoundThings(TBOOL unk, float fUnk, float fUnk2)
{
	if (TSound::GetSingletonWeak()->IsInitialised())
	{
		TMath::Clip(m_Options->unk2, 0, 10);
		TMath::Clip(m_Options->m_iVolume, 0, 10);
		float fUnk2 = 0.0f <= fUnk ? fUnk : m_Options->unk2 / 10.0f;
		float fVolume = 0.0f <= fUnk2 ? fUnk2 : m_Options->m_iVolume / 10.0f;
	}
}
