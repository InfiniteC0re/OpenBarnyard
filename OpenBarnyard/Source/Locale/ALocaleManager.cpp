#include "pch.h"
#include "ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ALocaleManager::ALocaleManager() :
    T2Locale( Lang_NUMOF, 307200, Toshi::T2Allocator::s_GlobalAllocator.Malloc( 307200 ) )
{
}

ALocaleManager::~ALocaleManager()
{
}

const TCHAR* ALocaleManager::GetLanguageFilename( Lang a_eLang )
{
    TASSERT( a_eLang >= 0 && a_eLang < Lang_NUMOF );
    return s_pLangDataFileNames[ a_eLang ];
}

const TCHAR* ALocaleManager::GetCurrentLanguageName()
{
    return GetLanguageName( m_LangId );
}

const TCHAR* ALocaleManager::GetVOLocaleDir( Lang a_eLang )
{
    switch ( a_eLang )
    {
        case Lang_German: return "GER/";
        case Lang_Spanish: return "SPA/";
        case Lang_French: return "FRE/";
        default: return "";
    }
}

const TCHAR* ALocaleManager::GetLanguageName( Lang a_eLang )
{
    switch ( a_eLang )
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
