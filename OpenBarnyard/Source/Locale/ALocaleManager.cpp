#include "pch.h"
#include "ALocaleManager.h"

TOSHI_NAMESPACE_USING

const char* ALocaleManager::GetLanguageFilename(Lang a_eLang)
{
	TASSERT(a_eLang >= 0 && a_eLang < Lang_NUMOF);
	return s_pLangDataFileNames[a_eLang];
}
