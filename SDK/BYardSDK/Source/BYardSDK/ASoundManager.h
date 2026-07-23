#pragma once
#include "THookedSingleton.h"

class ASoundManager
    : public THookedSingleton<ASoundManager, 0x0078c4e0>
{
public:
	// Plays a cue by id and returns its playing index
	TINT PlayCue( TINT a_iCue );
};
