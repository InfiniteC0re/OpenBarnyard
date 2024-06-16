#pragma once
#include "Toshi/T2Allocator.h"

namespace Toshi {

	//-----------------------------------------------------------------------------
	// Purpose: Stores all common methods that let you operate with TWCHAR* strings
	//-----------------------------------------------------------------------------
	class T2String16
	{
	public:
		static constexpr TSIZE SCRATCH_MEM_SIZE = 512;

	public:
		static TINT Format(TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, ...);
		static TINT Format(TWCHAR* a_pcString, const TWCHAR* a_pcFormat, ...);
		
		static TINT FormatV(TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, va_list args);
		static TINT FormatV(TWCHAR* a_pcString, const TWCHAR* a_pcFormat, va_list args);

		static TINT Compare(const TWCHAR* str1, const TWCHAR* str2, TSIZE size);
		static TINT CompareNoCase(const TWCHAR* str1, const TWCHAR* str2, TSIZE size);

		static TWCHAR* Copy(TWCHAR* dst, const TWCHAR* src, TSIZE size = -1);
		static TWCHAR* CopySafe(TWCHAR* dst, const TWCHAR* src, TSIZE size);

		static TWCHAR* Concat( TWCHAR* dst, const TWCHAR* src, TSIZE size = -1 );

		static TWCHAR* FindChar( TWCHAR* str, TWCHAR character );
		static const TWCHAR* FindChar( const TWCHAR* str, TWCHAR character );

		static TWCHAR* FindString( TWCHAR* str, const TWCHAR* substr );
		static const TWCHAR* FindString( const TWCHAR* str, const TWCHAR* substr );

		static TSIZE Length(const TWCHAR* str);

		static TBOOL IsLowerCase( const TWCHAR* str );
		static TBOOL IsUpperCase( const TWCHAR* str );

		static void ToLowerCase( TWCHAR* str );
		static void ToUpperCase( TWCHAR* str );

		static void IntToString( TINT value, TWCHAR* dst, TINT unused, TINT radix );
		static void IntToString( TINT value, TWCHAR* dst, TINT radix );

		static TINT StringToInt( const TWCHAR* src );
		static TFLOAT StringToFloat( const TWCHAR* src );

		static TWCHAR* SkipSpaces( TWCHAR* str );
		static const TWCHAR* SkipSpaces( const TWCHAR* str );

		static T2Allocator* GetDefaultAllocatorCB()
		{
			return &T2Allocator::s_GlobalAllocator;
		}

	public:
		static inline T2Allocator* sm_pDefaultAllocatorCB = GetDefaultAllocatorCB();
		static TWCHAR ms_aScratchMem[ SCRATCH_MEM_SIZE ];
	};

}
