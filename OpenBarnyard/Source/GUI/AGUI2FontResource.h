#pragma once
#include <File/TTRB.h>

struct AGUI2FontResource
{
    TCHAR*      szFileName;
    Toshi::TTRB oTRB;
    TUINT       uiRefCount;
};
