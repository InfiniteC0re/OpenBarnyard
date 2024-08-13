#pragma once
#include "THookedSingleton.h"

#include <Core/TMemory.h>

class THookedMemory :
	public THookedSingleton<Toshi::TMemory, 0x007ce1d4>
{
public:
};
