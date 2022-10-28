#pragma once
#include "Toshi/Sound/TSound.h"

namespace Toshi
{
	class TSound_Win : public TSound
	{
	public:
		bool SetSpeakerType(int maxchannels, int unk);
		bool m_bInitialised;
	};
	
}


