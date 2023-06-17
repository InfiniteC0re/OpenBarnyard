#include "pch.h"
#include "ALocaleManager.h"

using namespace Toshi;

void ALocaleManager::Create()
{
    auto localeManager = ALocaleManager::CreateSingleton<ALocaleManager>();

    auto langid = GetOSLanguage();
    localeManager->SetLanguage(langid);
}

void ALocaleManager::Destroy()
{
    if (T2Locale::ms_pSingleton != TNULL)
        delete T2Locale::GetSingletonWeak();
}

const char* ALocaleManager::GetLocaleCode(Lang code)
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

const char* ALocaleManager::GetVOLocaleCode(Lang code)
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

int ALocaleManager::GetSoundChannel(Lang code)
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

ALocaleManager::ALocaleManager() :
    m_pLocaleBuffer(TMalloc(BUFFER_SIZE)),
    T2Locale(LOCALE_LANG_NUMOF, BUFFER_SIZE, m_pLocaleBuffer)
{
    m_Format.SetMemoryFunctions(TRBAllocator, TRBDeallocator, this);
    InitialiseOverrideTexts();
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
    TBOOL changeLang = langid != m_LangId;

    if (changeLang)
    {
        m_Format.Close();

        if (langid > LOCALE_LANG_INVALID)
        {
            m_Format.Load(s_LocaleFormatFiles[langid]);
            m_LocaleFormat = m_Format.CastSymbol<LocaleFormat>("LocaleFormat");
        }
    }

    T2Locale::SetLanguage(langid);
}

ALocaleManager::LocaleString ALocaleManager::GetString(LocaleId stringid)
{
    TASSERT(stringid >= 0 && stringid < m_StringTable->m_numstrings);
    stringid = FixStringIdPlatform(stringid);

    LocaleString overrideString = GetOverwrittenLocaleString(stringid, m_LangId);
    return (overrideString == TNULL) ? m_StringTable->Strings[stringid] : overrideString;
}

ALocaleManager::LocaleString ALocaleManager::GetOverwrittenFormatString(LocaleId stringid, LocaleString fallbackString)
{
    LocaleString resultString = fallbackString;

    if (m_LangId != -1)
    {
        auto foundString = FindOverwrittenFormatString(stringid, m_LangId);

        if (foundString)
            resultString = foundString;
    }
    else
    {
        return TNULL;
    }

    return resultString;
}

ALocaleManager::LocaleId ALocaleManager::FixStringIdPlatform(Lang stringid)
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

ALocaleManager::LocaleString ALocaleManager::FindOverwrittenLocaleString(LocaleId stringid, Lang langid)
{
    for (size_t i = 0; i < NUM_OF_OVERRIDES; i++)
    {
        if (s_Overrides[i].m_StringId == stringid)
            return s_Overrides[i].m_LocaleStrings[langid];
    }

    return TNULL;
}

ALocaleManager::LocaleString ALocaleManager::FindOverwrittenFormatString(LocaleId stringid, Lang langid)
{
    for (size_t i = 0; i < NUM_OF_OVERRIDES; i++)
    {
        if (s_Overrides[i].m_StringId == stringid &&
            s_Overrides[i].m_LocaleStrings[langid] != TNULL)
        {
            return s_Overrides[i].m_FormatStrings[langid];
        }
    }

    return TNULL;
}

void ALocaleManager::InitialiseOverrideTexts()
{
    static TBOOL s_bAreOverridesInitialized = TFALSE;

    if (!s_bAreOverridesInitialized)
    {
        s_bAreOverridesInitialized = TTRUE;
        Toshi::TUtil::MemClear(s_Overrides, sizeof(s_Overrides));

        // Japanese
        static wchar_t s_Str1[] = L"@を長押しして弱点を突き、侵入しよう。";
        static wchar_t s_Str2[] = L"この2ブロックにはかつてミュージックバーとダンスホールがあった。*「今じゃ聞こえるのはグレーディアンたちがマシーンを増設するタップ音だけ」*活気を取り戻して、ブロブ！建物を緑に塗ろう。";
        static wchar_t s_EmptyFormat[] = L"";

        s_Overrides[0].m_StringId = 520;
        s_Overrides[0].m_LocaleStrings[12] = s_Str1;
        s_Overrides[0].m_FormatStrings[12] = s_EmptyFormat;

        for (size_t i = 0; i < LOCALE_LANG_NUMOF; i++)
        {
            wchar_t* string = s_Overrides[0].m_LocaleStrings[i];

            if (string != TNULL)
            {
                auto character = wcsrchr(string, L'@');

                if (character)
                    *character = L'\x84';
            }
        }

        s_Overrides[1].m_StringId = 985;
        s_Overrides[1].m_LocaleStrings[12] = s_Str2;
        s_Overrides[1].m_FormatStrings[12] = s_EmptyFormat;
    }
}
