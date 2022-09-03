#pragma once

namespace Toshi
{
	namespace TSystem
	{
		int StringCompareNoCase(const char* const& str1, const char* const& str2, size_t size);
		const char* StringCopy(char*, const char*, int);
		size_t StringLength(const char* const& str);
		size_t StringLength(const wchar_t* const& wstr);
		void* MemCopy(void* dst, const void* const& src, size_t size);
		const char* StringUnicodeToChar(char*, const wchar_t* const&, uint32_t);
	};
}


