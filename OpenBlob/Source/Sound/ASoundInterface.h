#pragma once

#include "Toshi/Sound/TSound.h"

class ASoundInterface
{
	void GetEventCategory(int index)
	{
		auto sound = Toshi::TSound::GetSingletonWeak();
		auto system = sound->GetSystem();
		
	}
};

