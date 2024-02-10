#pragma once
#include "Toshi/T2Allocator.h"
#include "Math/TMathInline.h"

#include <cstdarg>

namespace Toshi
{
	class T2String16
	{
	public:
		static TINT Format(TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, ...);
		static TINT Format(TWCHAR* a_pcString, const TWCHAR* a_pcFormat, ...);
		
		static TINT FormatV(TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, va_list args);
		static TINT FormatV(TWCHAR* a_pcString, const TWCHAR* a_pcFormat, va_list args);

		static TINT Compare(const TWCHAR* str1, const TWCHAR* str2, size_t size);
		static TINT CompareNoCase(const TWCHAR* str1, const TWCHAR* str2, size_t size);

		static TWCHAR* Copy(TWCHAR* dst, const TWCHAR* src, size_t size = -1);
		static TWCHAR* CopySafe(TWCHAR* dst, const TWCHAR* src, size_t size);

		static const TWCHAR* FindChar(const TWCHAR* str, TWCHAR character);
		static const TWCHAR* FindString(const TWCHAR* str, TWCHAR* substr);

		static size_t Length(const TWCHAR* str);

		static T2Allocator* GetDefaultAllocatorCB()
		{
			return &T2Allocator::s_GlobalAllocator;
		}

	public:
		static inline T2Allocator* sm_pDefaultAllocatorCB = GetDefaultAllocatorCB();
		static TWCHAR ms_aScratchMem[512];
	};
}
