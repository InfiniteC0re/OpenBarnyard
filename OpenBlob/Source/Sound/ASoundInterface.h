#pragma once

#include "Toshi/Sound/TSound.h"

class ASoundInterface : public Toshi::TSingleton<ASoundInterface>
{
	void GetEventCategory(int index)
	{
		auto sound = Toshi::TSound::GetSingleton();
		auto system = sound->GetSystem();
	}
};

