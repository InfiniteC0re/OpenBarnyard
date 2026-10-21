#pragma once
#include <Toshi/TString8.h>

#include "Render/TBackend.h"

class ATexture
{
public:
	// $Barnyard: FUNCTION 00615ca0
	ATexture() {}

	// $Barnyard: FUNCTION 00615c90
	~ATexture() {}

public:
	Toshi::T2Texture* pTexture;
	Toshi::TString8   Name;
};
