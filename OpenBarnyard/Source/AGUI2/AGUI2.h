#pragma once
#include "AGUI2TextureSection.h"

#include <Toshi/Core/TTask.h>

class AGUI2 : public Toshi::TTask
{
public:
	static AGUI2TextureSection* GetTextureSection(const char* a_szSectionName);

private:
	
};
