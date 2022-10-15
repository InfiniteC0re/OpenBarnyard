#include "pch.h"
#include "ALocaleManager.h"

// 00981a20
constinit ALocaleManager::Platform ALocaleManager::s_Platform = 2;

ALocaleManager::ALocaleManager()
{
    // 005e1d30
	m_SomeTRB.Destroy();
	Read(LOCALE_LANG_INVALID);
}

const char* ALocaleManager::GetLocaleFilePath(int32_t langid)
{
    // 005e2190
	TASSERT(langid < LOCALE_LANG_NUMOF, "langid is out of LOCALE_LANG_NUMOF");
	TASSERT(langid > LOCALE_LANG_INVALID, "langid is invalid");

	return s_LocaleFiles[langid];
}

void ALocaleManager::Read(int32_t langid)
{
    // 00662e30
	if (langid != m_LangId && (m_Locale.Destroy(), LOCALE_LANG_INVALID < langid))
	{
		m_Locale.Open(GetLocaleFilePath(langid));
		m_StringTable = m_Locale.GetSymb<LocaleStrings>("LocaleStrings");

		m_Unk2 = m_Unk1;
		m_LangId = langid;
	}
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

const wchar_t* ALocaleManager::GetString(int stringid)
{
	TASSERT(stringid >= 0 && stringid < m_StringTable->Count, "stringid is out of bounds");
    
    stringid = FixStringIdPlatform(stringid);
    TTODO("FUN_005e23c0");

	return m_StringTable->Strings[stringid];
}
