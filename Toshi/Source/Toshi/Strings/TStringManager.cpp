#include "ToshiPCH.h"
#include "TStringManager.h"
#include <string>

namespace Toshi {

	wchar_t* TStringManager::StringCharToUnicode(wchar_t* a_UnicodeString, const char* a_CharString, uint32_t a_iLength)
	{
		TASSERT((a_UnicodeString != TNULL) && (a_CharString != TNULL));

		auto iCharStringLength = String8Length(a_CharString);

		if (iCharStringLength < a_iLength || a_iLength == -1)
			a_iLength = iCharStringLength;

		for (size_t i = 0; i < a_iLength; i++)
			a_UnicodeString[i] = TSTATICCAST(wchar_t, a_CharString[i]);

		a_UnicodeString[a_iLength] = L'\0';
		return a_UnicodeString;
	}

	char* TStringManager::StringUnicodeToChar(char* a_CharString, const wchar_t* const& a_UnicodeString, uint32_t a_iLength)
	{
		TASSERT((a_UnicodeString != TNULL) && (a_CharString != TNULL));

		auto iUnicodeStringLength = String16Length(a_UnicodeString);

		if (iUnicodeStringLength < a_iLength || a_iLength == -1)
			a_iLength = iUnicodeStringLength;

		for (size_t i = 0; i < a_iLength; i++)
			a_CharString[i] = TSTATICCAST(char, a_UnicodeString[i]);

		a_CharString[a_iLength] = '\0';
		return a_CharString;
	}

}
