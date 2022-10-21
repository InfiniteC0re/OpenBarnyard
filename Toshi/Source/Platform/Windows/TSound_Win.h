#pragma once

#include "Toshi/Sound/TSound.h"

namespace Toshi
{

	class TSound;

	class TSound_Win : TSound
	{
	public:
		bool SetSpeakerType(int maxchannels, int unk);
		bool m_bInitialised;
	};
	
}


