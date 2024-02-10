#pragma once
#include <Toshi/TString8.h>

#ifdef TOSHI_SKU_WINDOWS
#include "Platform/DX8/T2Texture_DX8.h"
#endif // TOSHI_SKU_WINDOWS

class ATexture
{
public:
	Toshi::T2Texture* pTexture;
	Toshi::TString8 Name;
};