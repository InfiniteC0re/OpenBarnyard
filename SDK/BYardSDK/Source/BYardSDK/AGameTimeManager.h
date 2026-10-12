#pragma once
#include "THookedSingleton.h"

class AGameTimeManager : public THookedSingleton<AGameTimeManager, 0x00783d3c>
{
public:
	using DAYPHASE = TINT;
	enum DAYPHASE_ : DAYPHASE
	{
		DAYPHASE_DAWN      = 0,
		DAYPHASE_MORNING   = 1,
		DAYPHASE_AFTERNOON = 2,
		DAYPHASE_DUSK      = 3,
		DAYPHASE_NIGHT     = 4,
		DAYPHASE_NUMOF     = 5,
	};

public:
	DAYPHASE GetCurrentDayPhase() const { return *(const DAYPHASE*)( (const TCHAR*)this + 0x34 ); }
};
