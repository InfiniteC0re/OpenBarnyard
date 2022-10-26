#pragma once
#include "Toshi/Math/TMathInline.h"

namespace Toshi
{
	class T2String16
	{
	public:
		static int Compare(const wchar_t* str1, const wchar_t* str2, size_t size)
		{
			if (size != -1) return wcsncmp(str1, str2, size);
			return wcscmp(str1, str2);
		}

		static int CompareNoCase(const wchar_t* str1, const wchar_t* str2, size_t size)
		{
			if (size != -1) {
				return _wcsnicmp(str1, str2, size);
			}

			return _wcsicmp(str1, str2);
		}

		static wchar_t* Copy(wchar_t* dst, const wchar_t* src, size_t size)
		{
			if (size != -1) return wcsncpy(dst, src, size);
			return wcscpy(dst, src);
		}

		static wchar_t* CopySafe(wchar_t* dst, const wchar_t* src, size_t size)
		{
			size_t srcLen = Length(src);
			size = TMath::Min(size - 1, srcLen);
			wcsncpy(dst, src, size);
			dst[size] = L'\x0000';
		}

		static const wchar_t* FindChar(const wchar_t* str, wchar_t character)
		{
			while (true)
			{
				if (*str == L'\x0000') return nullptr;
				if (*str == character) return str;
				str++;
			}

			return nullptr;
		}

		static const wchar_t* FindString(const wchar_t* str, wchar_t* substr)
		{
			return wcsstr(str, substr);
		}

		static size_t Length(const wchar_t* str)
		{
			if (str != nullptr) return wcslen(str);
			return -1;
		}

	public:
		static wchar_t ms_aScratchMem[512];
	};
}
