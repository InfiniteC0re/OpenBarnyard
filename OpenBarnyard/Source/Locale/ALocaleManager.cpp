#include "pch.h"
#include "ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TCHAR* ALocaleManager::GetLanguageFilename(Lang a_eLang)
{
	TASSERT(a_eLang >= 0 && a_eLang < Lang_NUMOF);
	return s_pLangDataFileNames[a_eLang];
}
