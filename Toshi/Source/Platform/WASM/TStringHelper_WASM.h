#pragma once
#include <stdarg.h>
#include <stddef.h>

// The MSVC string functions the game uses. The libc works with 32-bit wide characters, the game with UTF-16
class TStringHelper_WASM
{
public:
	static char* ToUpper( char* a_szString );
	static char* ToLower( char* a_szString );
	static char* GetTimeString( char* a_szBuffer );
	static char* IntToString( int a_iValue, char* a_szBuffer, int a_iRadix );
	static unsigned long long GetTimestamp();
	static size_t WideLength( const wchar_t* a_wszString );
	static int WideCompare( const wchar_t* a_wszA, const wchar_t* a_wszB );
	static int WideCompareN( const wchar_t* a_wszA, const wchar_t* a_wszB, size_t a_uiSize );
	static int WideCompareNoCase( const wchar_t* a_wszA, const wchar_t* a_wszB );
	static int WideCompareNoCaseN( const wchar_t* a_wszA, const wchar_t* a_wszB, size_t a_uiSize );
	static wchar_t* WideCopy( wchar_t* a_wszDst, const wchar_t* a_wszSrc );
	static wchar_t* WideCopyN( wchar_t* a_wszDst, const wchar_t* a_wszSrc, size_t a_uiSize );
	static wchar_t* WideConcat( wchar_t* a_wszDst, const wchar_t* a_wszSrc );
	static wchar_t* WideConcatN( wchar_t* a_wszDst, const wchar_t* a_wszSrc, size_t a_uiSize );
	static wchar_t* WideFindChar( const wchar_t* a_wszString, wchar_t a_cChar );
	static wchar_t* WideFindLastChar( const wchar_t* a_wszString, wchar_t a_cChar );
	static wchar_t* WideFindString( const wchar_t* a_wszString, const wchar_t* a_wszFind );
	static wchar_t* WideToUpper( wchar_t* a_wszString );
	static wchar_t* WideToLower( wchar_t* a_wszString );
	static int WideToInt( const wchar_t* a_wszString );
	static double WideToFloat( const wchar_t* a_wszString );
	static wchar_t* WideFromInt( int a_iValue, wchar_t* a_wszBuffer, int a_iRadix );
	static int WideFormatN( wchar_t* a_wszBuffer, size_t a_uiSize, const wchar_t* a_wszFormat, va_list a_Args );
	static int WideFormat( wchar_t* a_wszBuffer, const wchar_t* a_wszFormat, va_list a_Args );
};
