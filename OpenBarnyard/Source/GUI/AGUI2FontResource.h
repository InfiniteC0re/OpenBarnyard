#pragma once
#include <Toshi/File/TTRB.h>

struct AGUI2FontResource
{
	char* szFileName;
	Toshi::TTRB oTRB;
	TUINT uiRefCount;
};
