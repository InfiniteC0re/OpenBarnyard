#include "ToshiPCH.h"
#include "TSystem.h"

namespace Toshi
{
	int TSystem::StringCompareNoCase(const char* const& str1, const char* const& str2, size_t size)
	{
		if (size != -1) {
			return _strnicmp(str1, str2, size);
		}

		return _stricmp(str1, str2);
	}

	const char* TSystem::StringCopy(char* dst, const char* source, int count)
	{
		if (count != -1)
		{
			return strncpy(dst, source, count);
		}

		char currentChar = '\0';

		do
		{
			currentChar = *source;
			source++;
			*dst = currentChar;
			dst++;
		} while (currentChar != '\0');

		return dst;
	}

	size_t TSystem::StringLength(const char* const& str)
	{
		const char* currentChar = str;
		while (*currentChar != 0) { currentChar++; }
		return currentChar - str;
	}

	size_t TSystem::StringLength(const wchar_t* const& wstr)
	{
		// In de Blob own StringLength func
		return wcslen(wstr);
	}

	void* TSystem::MemCopy(void* dst, const void* const& src, size_t size)
	{
		//JPOG implemented their own memcpy
		return memcpy(dst, src, size);
	}

	const char* TSystem::StringUnicodeToChar(char* a_CharString, const wchar_t* const& a_UnicodeString, uint32_t a_iLength)
	{
		TASSERT(a_UnicodeString != TNULL && a_CharString != TNULL, "Invalid string");

		size_t strLen = StringLength(a_UnicodeString);
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