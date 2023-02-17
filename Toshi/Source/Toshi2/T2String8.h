#pragma once
#include <stdlib.h>
#include <Toshi/Strings/TString8.h>

namespace Toshi
{
	class T2String8
	{
	public:
		static constexpr size_t SCRATCH_MEM_SIZE = 2048;

	public:
		static int FormatV(char* a_pcString, int size, const char* a_pcFormat, va_list* args)
		{
			int iResult = _vsnprintf(a_pcString, size, a_pcFormat, *args);
			TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
			a_pcString[size - 1] = '\0';
			return iResult;
		}

		static int FormatV(char* a_pcString, const char* a_pcFormat, va_list* args)
		{
			int iResult = vsprintf(a_pcString, a_pcFormat, *args);
			TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
			return iResult;
		}

		static int Format(char* a_pcString, int size, const char* a_pcFormat, ...)
		{
			TString8 buffer2;
			va_list args;

			va_start(args, a_pcFormat);

			int iResult = _vsnprintf(a_pcString, size, a_pcFormat, args);
			TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support vsprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
			a_pcString[size - 1] = '\0';

			va_end(args);
			return iResult;
		}

		static int Format(char* a_pcString, const char* a_pcFormat, ...)
		{
			TString8 buffer2;
			va_list args;

			va_start(args, a_pcFormat);

			int iResult = vsprintf(a_pcString, a_pcFormat, args);
			TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support vsprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
			
			va_end(args);
			return iResult;
		}

		static int Compare(const char* str1, const char* str2, size_t size)
		{
			if (size != -1) return strncmp(str1, str2, size);
			return strcmp(str1, str2);
		}

		static int CompareNoCase(const char* str1, const char* str2, size_t size = -1)
		{
			if (size != -1) {
				return _strnicmp(str1, str2, size);
			}

			return _stricmp(str1, str2);
		}

		static char* Copy(char* dst, const char* src, size_t size)
		{
			if (size != -1) return strncpy(dst, src, size);
			return strcpy(dst, src);
		}

		static char* CopySafe(char* dst, const char* src, size_t size)
		{
			size_t srcLen = Length(src);
			size = TMath::Min(size - 1, srcLen);
			strncpy(dst, src, size);
			dst[size] = L'\x0000';
			return dst;
		}

		static const char* FindChar(const char* str, char character)
		{
			while (true)
			{
				if (*str == L'\x0000') return nullptr;
				if (*str == character) return str;
				str++;
			}

			return nullptr;
		}

		static const char* FindString(const char* str, char* substr)
		{
			return strstr(str, substr);
		}

		static size_t Length(const char* str)
		{
			if (str != nullptr) return strlen(str);
			return -1;
		}

		static bool IsLowerCase(const char* str)
		{
			while (*str != 0 && (islower(*str) != 0 || isalpha(*str) == 0))
			{
				str++;
			}
			return *str == 0;
		}

		static void ToLowerCase(char* str)
		{
			while (*str)
			{
				*str = static_cast<char>(tolower(*str));
				str++;
			}
		}

		static void IntToString(int value, char* dst, int unused, int radix)
		{
			_itoa(value, dst, radix);
		}

		static int StringToInt(const char* src)
		{
			return atoi(src);
		}

	public:
		static char ms_aScratchMem[SCRATCH_MEM_SIZE];
	};
}