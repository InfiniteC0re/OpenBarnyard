#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi
{
	class TSystem
	{
	public:
		static uint32_t StringLength(const char* const& str);
		static uint32_t StringLength(const wchar_t* const& wstr);
		static void* MemCopy(void* dst, const void* const& src, size_t size);
		static const char* StringUnicodeToChar(char*, wchar_t*, int);
	};
}


