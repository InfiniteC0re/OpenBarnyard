#pragma once
#include <Toshi/File/TTRB.h>

class ALocaleManager
{
public:
	struct LocaleStrings
	{
		uint32_t Count;
		wchar_t** Strings;
	};

public:
	ALocaleManager();
	virtual ~ALocaleManager() { };

	virtual const char* GetLocaleFilePath(uint32_t langid);

	void Read(uint32_t langid);

private:
	Toshi::TTRB m_Locale;
	uint32_t m_Unk1;
	uint32_t m_Unk2;
	uint32_t m_Unk3;
	uint32_t m_LangId;
	uint32_t m_Unk4;
	uint32_t m_Unk5;
	Toshi::TTRB m_SomeTRB;

private:
	static constexpr const char* s_LocaleFiles[] = {
		"Data/Locale/eng.trb",
		"Data/Locale/eng-uk.trb",
		"Data/Locale/ned.trb",
		"Data/Locale/ger.trb",
		"Data/Locale/ita.trb",
		"Data/Locale/spa.trb",
		"Data/Locale/las.trb",
		"Data/Locale/fre.trb",
		"Data/Locale/dan.trb",
		"Data/Locale/nor.trb",
		"Data/Locale/swe.trb",
		"Data/Locale/fin.trb",
		"Data/Locale/jpn.trb",
		"Data/Locale/kor.trb",
		"Data/Locale/pt-br.trb",
		"Data/Locale/zh.trb",
	};

	static constexpr uint32_t LOCALE_LANG_INVALID = -1;
	static constexpr uint32_t LOCALE_LANG_NUMOF = sizeof(s_LocaleFiles) / sizeof(*s_LocaleFiles);
};