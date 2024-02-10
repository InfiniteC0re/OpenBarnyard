#include "ToshiPCH.h"
#include "T2String16.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TWCHAR T2String16::ms_aScratchMem[512];

	TINT T2String16::FormatV(TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, va_list args)
	{
		TINT iResult = _vsnwprintf(a_pcString, size, a_pcFormat, args);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
		a_pcString[size - 1] = '\0';
		return iResult;
	}

	TINT T2String16::FormatV(TWCHAR* a_pcString, const TWCHAR* a_pcFormat, va_list args)
	{
		TINT iResult = _vswprintf(a_pcString, a_pcFormat, args);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
		return iResult;
	}

	TINT T2String16::Format(TWCHAR* a_pcString, const TWCHAR* a_pcFormat, ...)
	{
		va_list args;

		va_start(args, a_pcFormat);

		TINT iResult = _vswprintf(a_pcString, a_pcFormat, args);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support vsprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
		return iResult;
	}

	TINT T2String16::Format(TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, ...)
	{
		va_list args;

		va_start(args, a_pcFormat);

		TINT iResult = _vsnwprintf(a_pcString, size, a_pcFormat, args);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support vsprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
		return iResult;
	}

	TINT T2String16::Compare(const TWCHAR* str1, const TWCHAR* str2, size_t size)
	{
		if (size != -1) return wcsncmp(str1, str2, size);
		return wcscmp(str1, str2);
	}

	TINT T2String16::CompareNoCase(const TWCHAR* str1, const TWCHAR* str2, size_t size)
	{
		if (size != -1) {
			return _wcsnicmp(str1, str2, size);
		}

		return _wcsicmp(str1, str2);
	}

	TWCHAR* T2String16::Copy(TWCHAR* dst, const TWCHAR* src, size_t size)
	{
		if (size != -1) return wcsncpy(dst, src, size);
		return wcscpy(dst, src);
	}

	TWCHAR* T2String16::CopySafe(TWCHAR* dst, const TWCHAR* src, size_t size)
	{
		size_t srcLen = Length(src);
		size = TMath::Min(size - 1, srcLen);
		wcsncpy(dst, src, size);
		dst[size] = L'\x0000';
		return dst;
	}

	const TWCHAR* T2String16::FindChar(const TWCHAR* str, TWCHAR character)
	{
		while (TTRUE)
		{
			if (*str == L'\x0000') return TNULL;
			if (*str == character) return str;
			str++;
		}

		return TNULL;
	}

	const TWCHAR* T2String16::FindString(const TWCHAR* str, TWCHAR* substr)
	{
		return wcsstr(str, substr);
	}

	size_t T2String16::Length(const TWCHAR* str)
	{
		if (str != TNULL) return wcslen(str);
		return -1;
	}

}