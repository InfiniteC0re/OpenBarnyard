#pragma once
#include "THookedSingleton.h"

#include TOSHI_MULTIRENDER(TRenderInterface)

class THookedRenderD3DInterface :
	public THookedSingleton<Toshi::TRenderD3DInterface, 0x007ce6c8>
{

};
