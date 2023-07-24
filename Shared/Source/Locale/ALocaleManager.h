#pragma once
#include <Toshi/Core/Platform.h>
#include <Toshi/Locale/T2Locale.h>

class ALocaleManager :
    public Toshi::T2Locale
{
public:
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

    static constexpr int32_t LOCALE_LANG_INVALID = -1;
    static constexpr int32_t LOCALE_LANG_NUMOF = sizeof(m_pLangDataFileNames) / sizeof(*m_pLangDataFileNames);
    static constexpr int32_t LOCALE_FORMAT_LANG_NUMOF = sizeof(s_LocaleFormatFiles) / sizeof(*s_LocaleFormatFiles);

    static_assert(LOCALE_FORMAT_LANG_NUMOF == LOCALE_LANG_NUMOF);

    inline static constinit Toshi::Platform s_ePlatform = Toshi::Platform::PC;
    static constexpr size_t BUFFER_SIZE = 409600;

    struct Override
    {
        LocaleId m_StringId;
        wchar_t* m_LocaleStrings[LOCALE_LANG_NUMOF];
        wchar_t* m_FormatStrings[LOCALE_LANG_NUMOF];
    };

    static constexpr size_t NUM_OF_OVERRIDES = 2;
    inline static Override s_Overrides[NUM_OF_OVERRIDES];

    struct LocaleFormat
    {

    };

public:
    ALocaleManager();
    ALocaleManager(const ALocaleManager&) = delete;
    ALocaleManager(const ALocaleManager&&) = delete;

    virtual ~ALocaleManager() { };
    virtual const char* GetLanguageFilename(Lang langid) override;

    void SetLanguage(Lang langid);
    
    LocaleString GetString(LocaleId stringid);

    LocaleString GetOverwrittenFormatString(LocaleId stringid, LocaleString fallbackString);

    const char* GetCurLocaleCode() { return GetLocaleCode(m_LangId); }
    int GetCurSoundChannel() { return GetSoundChannel(m_LangId); }

public:
    static void Create();
    static void Destroy();
	static void SetPlatform(Toshi::Platform a_ePlatform) { s_ePlatform = a_ePlatform; }

    static const char* GetLocaleCode(Lang code);
    static const char* GetVOLocaleCode(Lang code);

    static int GetSoundChannel(Lang code);

    static int GetOSLanguage() { return 0; }
    static LocaleId FixStringIdPlatform(Lang stringid);
    static LocaleString FindOverwrittenLocaleString(LocaleId stringid, Lang langid);
    static LocaleString FindOverwrittenFormatString(LocaleId stringid, Lang langid);

    static LocaleString GetOverwrittenLocaleString(LocaleId stringid, Lang langid)
    {
        return (langid != -1) ? FindOverwrittenLocaleString(stringid, langid) : TNULL;
    }

    static ALocaleManager& Instance() { return *TSTATICCAST(ALocaleManager*, GetSingleton()); }

private:
    static void InitialiseOverrideTexts();

private:
    void* m_pLocaleBuffer;                // 0x2C
    Toshi::TTRB m_Format;                 // 0x30
    LocaleFormat* m_LocaleFormat;         // 0x44
};