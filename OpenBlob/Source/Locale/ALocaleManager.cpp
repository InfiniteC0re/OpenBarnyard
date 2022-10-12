#include "pch.h"
#include "ALocaleManager.h"

ALocaleManager::ALocaleManager()
{
	m_SomeTRB.Destroy();
	Read(LOCALE_LANG_INVALID);
}

const char* ALocaleManager::GetLocaleFilePath(uint32_t langid)
{
	TASSERT(langid < LOCALE_LANG_NUMOF, "langid is out of LOCALE_LANG_NUMOF");
	TASSERT(langid > LOCALE_LANG_INVALID, "langid is invalid");

	return s_LocaleFiles[langid];
}

void ALocaleManager::Read(uint32_t langid)
{
	if (langid != m_LangId && (m_Locale.Destroy(), langid > LOCALE_LANG_INVALID))
	{
		const char* localeFile = GetLocaleFilePath(langid);
		m_Unk2 = m_Unk1;
	}
}
