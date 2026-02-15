#pragma once
#include <File/TTRB.h>

TOSHI_NAMESPACE_START

struct T2GUIFontResource
{
	TCHAR* szFileName;
	TTRB   oTRB;
	TUINT  uiRefCount;
};

TOSHI_NAMESPACE_END
