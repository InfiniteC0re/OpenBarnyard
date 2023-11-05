#pragma once
#include "AGUI2Renderer.h"
#include "AGUI2TextureSection.h"

#include <Toshi/Core/TTask.h>

class AGUI2 : 
	public Toshi::TGenericClassDerived<AGUI2, Toshi::TTask, "AGUI2", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<AGUI2>
{
public:
	static AGUI2TextureSection* GetTextureSection(const char* a_szSectionName);
	static AGUI2Renderer* GetRenderer() { return TNULL; }

private:
	
};
