#include "pch.h"
#include "ALocaleManager.h"

static constexpr const char* m_pLangDataFileNames[] =
{
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

static constexpr const char* s_LocaleFormatFiles[] =
{
        "Data/Locale/format_eng.trb",
        "Data/Locale/format_eng-uk.trb",
        "Data/Locale/format_ned.trb",
        "Data/Locale/format_ger.trb",
        "Data/Locale/format_ita.trb",
        "Data/Locale/format_spa.trb",
        "Data/Locale/format_las.trb",
        "Data/Locale/format_fre.trb",
        "Data/Locale/format_dan.trb",
        "Data/Locale/format_nor.trb",
        "Data/Locale/format_swe.trb",
        "Data/Locale/format_fin.trb",
        "Data/Locale/format_jpn.trb",
        "Data/Locale/format_kor.trb",
        "Data/Locale/format_pt-br.trb",
        "Data/Locale/format_zh.trb",
};

// 00981a20
constinit ALocaleManager::Platform ALocaleManager::s_Platform = Platform_PC;

static constexpr int32_t LOCALE_LANG_INVALID = -1;
static constexpr int32_t LOCALE_LANG_NUMOF = sizeof(m_pLangDataFileNames) / sizeof(*m_pLangDataFileNames);
static constexpr int32_t LOCALE_FORMAT_LANG_NUMOF = sizeof(s_LocaleFormatFiles) / sizeof(*s_LocaleFormatFiles);

static_assert(LOCALE_FORMAT_LANG_NUMOF == LOCALE_LANG_NUMOF);

void ALocaleManager::Create()
{
    TASSERT(T2Locale::s_Singleton == TNULL, "T2Locale is already created");

    auto localeManager = new ALocaleManager();

    auto langid = GetOSLanguage();
    localeManager->SetLanguage(langid);

    T2Locale::s_Singleton = localeManager;
}

void ALocaleManager::Destroy()
{
    if (T2Locale::s_Singleton != TNULL)
    {
        delete T2Locale::s_Singleton;
    }
}

const char* ALocaleManager::GetLocaleCode(int code)
{
    switch (code)
    {
    case 1:
        return "UK";
    case 2:
        return "NL";
    case 3:
        return "DE";
    case 4:
        return "IT";
    case 5:
        return "ES";
    case 6:
        return "LAS";
    case 7:
        return "FR";
    case 8:
        return "DA";
    case 9:
        return "NO";
    case 10:
        return "SW";
    case 11:
        return "FI";
    case 12:
        return "JP";
    case 13:
        return "KO";
    case 14:
        return "PT";
    case 15:
        return "ZH";
    default:
        return "US";
    }
}

const char* ALocaleManager::GetVOLocaleCode(int code)
{
    switch (code) {
    case 2:
        return "NL";
    case 3:
        return "DE";
    case 4:
        return "IT";
    case 7:
        return "FR";
    default:
        return "EN";
    }
}

int ALocaleManager::GetSoundChannel(int code)
{
    if (code == 2) {
        return 3;
    }
    if (code != 3) {
        if (code != 7) {
            return 0;
        }
        return 2;
    }
    return 1;
}

ALocaleManager::ALocaleManager() : T2Locale(LOCALE_LANG_NUMOF, 0x64000, nullptr)
{
    // 005e1ca0
    m_LocaleBuffer = T2Locale::m_Buffer;
    setlocale(LC_ALL, "");
    TTODO("de blob: FUN_005e2470");
}

const char* ALocaleManager::GetLanguageFilename(int32_t langid)
{
    // 005e2190
    TASSERT(langid < LOCALE_LANG_NUMOF, "langid is out of LOCALE_LANG_NUMOF");
    TASSERT(langid > LOCALE_LANG_INVALID, "langid is invalid");

    return m_pLangDataFileNames[langid];
}

void ALocaleManager::SetLanguage(Lang langid)
{
    bool changeLang = langid != m_LangId;

    if (changeLang)
    {
        m_Format.Close();

        if (langid > LOCALE_LANG_INVALID)
        {
            m_Format.Load(s_LocaleFormatFiles[langid]);
            m_LocaleFormat = m_Format.CastSymbol<LocaleFormat>("LocaleFormat");
            TTODO("reverse LocaleFormat symbol");
        }
    }

    T2Locale::SetLanguage(langid);
}

int ALocaleManager::FixStringIdPlatform(int stringid)
{
    // 005e2210
    if (s_Platform == Platform_Wii)
    {
        // PC => Wii
        if (stringid < 879)
        {
            switch (stringid)
            {
            case 860: return 491;
            case 861: return 493;
            case 862: return 525;
            case 863: return 739;
            case 864: return 521;
            case 865: return 535;
            case 878: return 565;
            }
        }
        else if (stringid == 983)
        {
            stringid = 905;
        }
        else
        {
            if (stringid == 1171) return 1071;
            if (stringid == 1267) return 1239;
        }

        return stringid;
    }

    // Wii => PC
    if (stringid < 566)
    {
        switch (stringid)
        {
        case 492:
        case 494:
        case 495:
        case 496:
        case 497:
        case 498:
        case 499:
        case 500:
        case 501:
        case 502:
        case 503:
        case 504:
        case 505:
        case 506:
        case 507:
        case 508:
        case 509:
        case 510:
        case 511:
        case 512:
        case 513:
        case 514:
        case 515:
        case 516:
        case 517:
        case 518:
        case 519:
        case 520:
        case 522:
        case 523:
        case 524:
        case 526:
        case 527:
        case 528:
        case 529:
        case 530:
        case 531:
        case 532:
        case 533:
        case 534: return stringid;
        case 491: return 860;
        case 493: return 861;
        case 521: return 864;
        case 525: return 862;
        case 535: return 865;
        default:  return 878;
        }
    }

    if (stringid < 1072)
    {
        if (stringid == 1071) return 1171;
        if (stringid == 739)  return 863;
        if (stringid == 905)  return 983;
    }
    else if (stringid == 1239)
    {
        return 1267;
    }

    return stringid;
}
