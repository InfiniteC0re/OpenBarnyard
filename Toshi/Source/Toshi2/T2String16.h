#pragma once
#include "Toshi/Math/TMathInline.h"

namespace Toshi
{
	class T2String16
	{
	public:

		static int FormatV(wchar_t* a_pcString, int size, const wchar_t* a_pcFormat, va_list args)
		{
			int iResult = _vsnwprintf(a_pcString, size, a_pcFormat, args);
			TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
			a_pcString[size - 1] = '\0';
			return iResult;
		}

		static int FormatV(wchar_t* a_pcString, const wchar_t* a_pcFormat, va_list args)
		{
			int iResult = _vswprintf(a_pcString, a_pcFormat, args);
			TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
			return iResult;
		}

		static int Format(wchar_t* a_pcString, int size, const wchar_t* a_pcFormat, ...)
		{
			TString8 buffer2;
			va_list args;

			va_start(args, a_pcFormat);

			int iResult = _vsnwprintf(a_pcString, size, a_pcFormat, args);
			TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support vsprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
			return iResult;
		}

		static int Format(wchar_t* a_pcString, const wchar_t* a_pcFormat, ...)
		{
			TString8 buffer2;
			va_list args;

			va_start(args, a_pcFormat);

			int iResult = _vswprintf(a_pcString, a_pcFormat, args);
			TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support vsprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
			return iResult;
		}


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

		static Toshi::T2Allocator* GetDefaultAllocatorCB()
		{
			return &T2Allocator::s_GlobalAllocator;
		}

	public:
		static Toshi::T2Allocator* sm_pDefaultAllocatorCB;
		static wchar_t ms_aScratchMem[512];
	};
}
