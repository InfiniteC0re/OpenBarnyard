#include "pch.h"
#include "AOptions.h"
#include <Toshi/Sound/TSound.h>

using namespace Toshi;

void AOptions::SoundThings(TBOOL unk, float a_fPitch, float a_fVolume)
{
	if (TSound::GetSingletonWeak()->IsInitialised())
	{
		TMath::Clip(m_Options->m_iPitch, 0, 10);
		TMath::Clip(m_Options->m_iVolume, 0, 10);
		float fUnk2 = 0.0f <= a_fPitch ? a_fPitch : m_Options->m_iPitch / 10.0f;
		float fVolume = 0.0f <= a_fVolume ? a_fVolume : m_Options->m_iVolume / 10.0f;
	}
}
