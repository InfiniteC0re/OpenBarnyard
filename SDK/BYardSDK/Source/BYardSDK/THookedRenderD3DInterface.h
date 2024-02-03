#pragma once
#include "THookedSingleton.h"

#include "Platform/DX8/TRenderInterface_DX8.h"

class THookedRenderD3DInterface :
	public THookedSingleton<Toshi::TRenderD3DInterface, 0x007ce6c8>
{

};
