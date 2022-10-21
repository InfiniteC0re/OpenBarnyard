#include "pch.h"
#include "ALocaleManager.h"

ALocaleManager::ALocaleManager()
{
	m_SomeTRB.Close();
	Read(LOCALE_LANG_INVALID);
}

const char* ALocaleManager::GetLocaleFilePath(int32_t langid)
{
	TASSERT(langid < LOCALE_LANG_NUMOF, "langid is out of LOCALE_LANG_NUMOF");
	TASSERT(langid > LOCALE_LANG_INVALID, "langid is invalid");

	return s_LocaleFiles[langid];
}

void ALocaleManager::Read(int32_t langid)
{
	TWIP();

	if (langid != m_LangId && (m_Locale.Close(), LOCALE_LANG_INVALID < langid))
	{
		m_Locale.Open(GetLocaleFilePath(langid));
		m_LocaleStrings = m_Locale.GetSymb<LocaleStrings>("LocaleStrings");

		m_Unk2 = m_Unk1;
		m_LangId = langid;
	}
}
