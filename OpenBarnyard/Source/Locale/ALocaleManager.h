#pragma once
#include <Toshi/T2Allocator.h>
#include <T2Locale/T2Locale.h>

class ALocaleManager : public Toshi::T2Locale
{
public:
    constexpr static const TCHAR* s_pLangDataFileNames[] = {
        "Data/Locale/eng.trb",
        "Data/Locale/eng-uk.trb",
        "Data/Locale/jpn.trb",
        "Data/Locale/ger.trb",
        "Data/Locale/ita.trb",
        "Data/Locale/spa.trb",
        "Data/Locale/fre.trb",
        "Data/Locale/nld.trb" };

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

    constexpr static TSIZE NUM_LOCALES = TARRAYSIZE( s_pLangDataFileNames );
    TSTATICASSERT( Lang_NUMOF == NUM_LOCALES );

public:
    ALocaleManager();
    ~ALocaleManager();

    virtual const TCHAR* GetLanguageFilename( Lang a_eLang ) override;

    const TCHAR* GetCurrentLanguageName();
    const TCHAR* GetVOLocaleDir( Lang a_eLang );

public:
    static const TCHAR* GetLanguageName( Lang a_eLang );

    static ALocaleManager* Interface() { return TSTATICCAST( ALocaleManager, ms_pSingleton ); }
};
