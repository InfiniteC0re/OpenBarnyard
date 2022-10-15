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

	typedef int Platform;
	enum Platform_ : Platform
	{
		Platform_Wii,
		Platform_Unk,
		Platform_PC,
	};

public:
	ALocaleManager();
	virtual ~ALocaleManager() { };

	virtual const char* GetLocaleFilePath(int32_t langid);

	void Read(int32_t langid);
	const wchar_t* GetString(int stringid);

	static int FixStringIdPlatform(int stringid);

private:
	Toshi::TTRB m_Locale;
	uint32_t m_Unk1;
	uint32_t m_Unk2;
	LocaleStrings* m_StringTable;
	int32_t m_LangId;
	uint32_t m_Unk4;
	uint32_t m_Unk5;
	Toshi::TTRB m_SomeTRB;

	// 00981a20
	static constinit Platform s_Platform;

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

	static constexpr int32_t LOCALE_LANG_INVALID = -1;
	static constexpr int32_t LOCALE_LANG_NUMOF = sizeof(s_LocaleFiles) / sizeof(*s_LocaleFiles);
};