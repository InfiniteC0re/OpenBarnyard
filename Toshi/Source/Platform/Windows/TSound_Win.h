#pragma once

#include "Toshi/Sound/TSound.h"
#include "fmod.hpp"

namespace Toshi
{
	class TSound_Win : TSound
	{
	public:
		bool Initialise(int maxchannels, int unk);
		bool m_bInitialised;
	};
	
}


