#include "pch.h"
#include "ALocaleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ALocaleManager::ALocaleManager()
    : T2Locale( Lang_NUMOF, 307200, Toshi::GetGlobalAllocator()->Malloc( 307200 ) )
{
}

// $Barnyard: FUNCTION 00402730
ALocaleManager::~ALocaleManager()
{
}

// $Barnyard: FUNCTION 00402860
const TCHAR* ALocaleManager::GetLanguageFilename( Lang a_eLang )
{
	TASSERT( a_eLang >= 0 && a_eLang < Lang_NUMOF );
	return s_pLangDataFileNames[ a_eLang ];
}

// $Barnyard: FUNCTION 00402850
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

// $Barnyard: FUNCTION 004027e0
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
