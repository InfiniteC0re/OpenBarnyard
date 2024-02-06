#pragma once
#include <Toshi2/T2Allocator.h>
#include <Toshi/Locale/T2Locale.h>

class ALocaleManager : public Toshi::T2Locale
{
public:
	constexpr static const char* s_pLangDataFileNames[] = {
		"Data/Locale/eng.trb",
		"Data/Locale/eng-uk.trb",
		"Data/Locale/jpn.trb",
		"Data/Locale/ger.trb",
		"Data/Locale/ita.trb",
		"Data/Locale/spa.trb",
		"Data/Locale/fre.trb",
		"Data/Locale/nld.trb"
	};

	enum Lang_ : Lang
	{
		Lang_UNKNOWN = -1,
		Lang_English,
		Lang_EnglishUK,
		Lang_Japanese,
		Lang_German,
		Lang_Italian,
		Lang_Spanish,
		Lang_French,
		Lang_Netherlandic,
		Lang_NUMOF
	};

	constexpr static size_t NUM_LOCALES = TARRAYSIZE(s_pLangDataFileNames);
	TSTATICASSERT(Lang_NUMOF == NUM_LOCALES);

public:
	ALocaleManager() : T2Locale(Lang_NUMOF, 307200, Toshi::T2Allocator::s_GlobalAllocator.Malloc(307200))
	{

	}

	virtual const char* GetLanguageFilename(Lang a_eLang) override;

	LocaleString GetString(int a_iNumString) const
	{
		TASSERT(a_iNumString >= 0 && a_iNumString < GetNumStrings());
		return m_StringTable->Strings[a_iNumString];
	}

	int GetNumStrings() const
	{
		return T2Locale::GetNumStrings();
	}

	const char* GetCurrentLanguageName()
	{
		return GetLanguageName(m_LangId);
	}

	static const char* GetLanguageName(Lang a_eLang)
	{
		switch (a_eLang)
		{
		case Lang_Japanese: return "JPN";
		case Lang_German: return "DEU";
		case Lang_Italian: return "ITA";
		case Lang_Spanish: return "ESL";
		case Lang_French: return "FRE";
		case Lang_Netherlandic: return "NLD";
		default: return "ENG";
		}
	}

	static ALocaleManager* Interface()
	{
		return TSTATICCAST(ALocaleManager*, T2Locale::GetSingletonSafe());
	}

private:
};
