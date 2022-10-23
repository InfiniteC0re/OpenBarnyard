#include "ToshiPCH.h"
#include "TStringManager.h"
#include <string>

namespace Toshi
{
    const char* TStringManager::StringUnicodeToChar(char* a_CharString, const wchar_t* const& a_UnicodeString, uint32_t a_iLength)
	{
		TASSERT(a_UnicodeString != TNULL && a_CharString != TNULL, "Invalid string");

		size_t strLen = String16Length(a_UnicodeString);
		if (strLen < a_iLength || a_iLength == -1) { a_iLength = (uint32_t)strLen; }

		uint32_t index = 0;
		if (a_iLength > 0)
		{
			do
			{
				a_CharString[index] = (char)a_UnicodeString[index];
				index++;
			} while (index < a_iLength);
		}

		a_CharString[index] = '\0';
		return a_CharString;
	}
}