#pragma once
#include "Toshi/T2Allocator.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Stores all common methods that let you operate with TCHAR* strings
//-----------------------------------------------------------------------------
class T2String8
{
public:
	static constexpr TSIZE SCRATCH_MEM_SIZE = 2048;

public:
	static TCHAR* CreateCopy( const TCHAR* a_pcString, TSIZE a_uiSize = -1 );

	static TINT FormatV( TCHAR* a_pcString, TINT size, const TCHAR* a_pcFormat, va_list args );
	static TINT FormatV( TCHAR* a_pcString, const TCHAR* a_pcFormat, va_list args );

	static TINT Format( TCHAR* a_pcString, TINT size, const TCHAR* a_pcFormat, ... );
	static TINT Format( TCHAR* a_pcString, const TCHAR* a_pcFormat, ... );

	static TINT Compare( const TCHAR* str1, const TCHAR* str2, TSIZE size = -1 );
	static TINT CompareNoCase( const TCHAR* str1, const TCHAR* str2, TSIZE size = -1 );

	static TCHAR* Copy( TCHAR* dst, const TCHAR* src, TSIZE size = -1 );
	static TCHAR* CopySafe( TCHAR* dst, const TCHAR* src, TSIZE size );

	static TCHAR* Concat( TCHAR* dst, const TCHAR* src, TSIZE size = -1 );

	static TCHAR*       FindChar( TCHAR* str, TCHAR character );
	static const TCHAR* FindChar( const TCHAR* str, TCHAR character );

	static TCHAR*       FindString( TCHAR* str, const TCHAR* substr );
	static const TCHAR* FindString( const TCHAR* str, const TCHAR* substr );

	static TSIZE Length( const TCHAR* str );

	static TBOOL IsLowerCase( const TCHAR* str );
	static TBOOL IsUpperCase( const TCHAR* str );

	static void ToLowerCase( TCHAR* str );
	static void ToUpperCase( TCHAR* str );

	static void IntToString( TINT value, TCHAR* dst, TINT unused, TINT radix );
	static void IntToString( TINT value, TCHAR* dst, TINT radix );

	static TINT   StringToInt( const TCHAR* src );
	static TFLOAT StringToFloat( const TCHAR* src );

	static TCHAR*       SkipSpaces( TCHAR* str );
	static const TCHAR* SkipSpaces( const TCHAR* str );

	static T2Allocator* GetDefaultAllocatorCB()
	{
		return GetGlobalAllocator();
	}

public:
	static inline T2Allocator* sm_pDefaultAllocatorCB = GetDefaultAllocatorCB();
	static TCHAR               ms_aScratchMem[ SCRATCH_MEM_SIZE ];
};

TOSHI_NAMESPACE_END
