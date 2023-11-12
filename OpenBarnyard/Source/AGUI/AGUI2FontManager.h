#pragma once
#include "AGUI2Font.h"
#include "AGUI2FontResource.h"

class AGUI2FontManager
{
public:
	static constexpr TUINT MAX_NUM_LOADED_FONTS = 1;

public:
	inline static AGUI2FontResource s_fontresources[MAX_NUM_LOADED_FONTS];
};
