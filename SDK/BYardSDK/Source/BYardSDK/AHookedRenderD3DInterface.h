#pragma once
#include "AHookedSingleton.h"

#include TOSHI_MULTIRENDER(TRenderInterface)

class AHookedRenderD3DInterface :
	public AHookedSingleton<Toshi::TRenderD3DInterface, 0x007ce6c8>
{

};
