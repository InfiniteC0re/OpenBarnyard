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

	constexpr static size_t NUM_LOCALES = sizeof(s_pLangDataFileNames) / sizeof(*s_pLangDataFileNames);
	static_assert(Lang_NUMOF == NUM_LOCALES);

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

private:
};
