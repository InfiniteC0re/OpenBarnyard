#pragma once
#include "T2CharTraits.h"
#include "T2String8.h"
#include "T2String16.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Helper class that gives you the required methods to operate
// with strings of different char types using specified char traits
//-----------------------------------------------------------------------------
template <typename CharT, typename TCharTraits = T2CharTraits<CharT>>
class T2StringTraits
{
public:
	using CharTraits = TCharTraits;
	using CharType   = typename TCharTraits::CharType;
	using UCharType  = typename TCharTraits::UCharType;

public:
	static TINT FormatV( CharType* a_pcString, TINT size, const CharType* a_pcFormat, va_list args ) { TUNREACHABLE(); }
	static TINT FormatV( CharType* a_pcString, const CharType* a_pcFormat, va_list args ) { TUNREACHABLE(); }

	static TINT Format( CharType* a_pcString, TINT size, const CharType* a_pcFormat, ... ) { TUNREACHABLE(); }
	static TINT Format( CharType* a_pcString, const CharType* a_pcFormat, ... ) { TUNREACHABLE(); }

	static TINT Compare( const CharType* str1, const CharType* str2, TSIZE size = -1 ) { TUNREACHABLE(); }
	static TINT CompareNoCase( const CharType* str1, const CharType* str2, TSIZE size = -1 ) { TUNREACHABLE(); }

	static CharType* Copy( CharType* dst, const CharType* src, TSIZE size = -1 ) { TUNREACHABLE(); }
	static CharType* CopySafe( CharType* dst, const CharType* src, TSIZE size ) { TUNREACHABLE(); }

	static CharType* Concat( CharType* dst, const CharType* src, TSIZE size = -1 ) { TUNREACHABLE(); }

	static CharType*       FindChar( CharType* str, CharType character ) { TUNREACHABLE(); }
	static const CharType* FindChar( const CharType* str, CharType character ) { TUNREACHABLE(); }
	static CharType*       FindString( CharType* str, const CharType* substr ) { TUNREACHABLE(); }
	static const CharType* FindString( const CharType* str, const CharType* substr ) { TUNREACHABLE(); }

	static TSIZE Length( const CharType* str ) { TUNREACHABLE(); }

	static TBOOL IsLowerCase( const CharType* str ) { TUNREACHABLE(); }
	static TBOOL IsUpperCase( const CharType* str ) { TUNREACHABLE(); }

	static void ToLowerCase( CharType* str ) { TUNREACHABLE(); }
	static void ToUpperCase( CharType* str ) { TUNREACHABLE(); }

	static void IntToString( TINT value, CharType* dst, TINT unused, TINT radix ) { TUNREACHABLE(); }
	static void IntToString( TINT value, CharType* dst, TINT radix ) { TUNREACHABLE(); }

	static TINT   StringToInt( const CharType* src ) { TUNREACHABLE(); }
	static TFLOAT StringToFloat( const CharType* src ) { TUNREACHABLE(); }

	static CharType*       SkipSpaces( CharType* str ) { TUNREACHABLE(); }
	static const CharType* SkipSpaces( const CharType* str ) { TUNREACHABLE(); }
};

//-----------------------------------------------------------------------------
// TCHAR - 1 byte character
//-----------------------------------------------------------------------------

template <>
TINLINE TINT T2StringTraits<TCHAR>::FormatV( CharType* a_pcString, TINT size, const CharType* a_pcFormat, va_list args )
{
	return T2String8::FormatV( a_pcString, size, a_pcFormat, args );
}

template <>
TINLINE TINT T2StringTraits<TCHAR>::FormatV( CharType* a_pcString, const CharType* a_pcFormat, va_list args )
{
	return T2String8::FormatV( a_pcString, a_pcFormat, args );
}

template <>
TINLINE TINT T2StringTraits<TCHAR>::Format( CharType* a_pcString, TINT size, const CharType* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, size, a_pcFormat, args );
	va_end( args );

	return iResult;
}

template <>
TINLINE TINT T2StringTraits<TCHAR>::Format( CharType* a_pcString, const CharType* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, a_pcFormat, args );
	va_end( args );

	return iResult;
}

template <>
TINLINE TINT T2StringTraits<TCHAR>::Compare( const CharType* str1, const CharType* str2, TSIZE size /*= -1 */ )
{
	return T2String8::Compare( str1, str2, size );
}

template <>
TINLINE TINT T2StringTraits<TCHAR>::CompareNoCase( const CharType* str1, const CharType* str2, TSIZE size /*= -1 */ )
{
	return T2String8::CompareNoCase( str1, str2, size );
}

template <>
TINLINE T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::Copy( CharType* dst, const CharType* src, TSIZE size /*= -1 */ )
{
	return T2String8::Copy( dst, src, size );
}

template <>
TINLINE T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::CopySafe( CharType* dst, const CharType* src, TSIZE size )
{
	return T2String8::CopySafe( dst, src, size );
}

template <>
TINLINE T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::Concat( CharType* dst, const CharType* src, TSIZE size /*= -1 */ )
{
	return T2String8::Concat( dst, src, size );
}

template <>
TINLINE T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::FindChar( CharType* str, CharType character )
{
	return T2String8::FindChar( str, character );
}

template <>
TINLINE const T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::FindChar( const CharType* str, CharType character )
{
	return T2String8::FindChar( str, character );
}

template <>
TINLINE T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::FindString( CharType* str, const CharType* substr )
{
	return T2String8::FindString( str, substr );
}

template <>
TINLINE const T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::FindString( const CharType* str, const CharType* substr )
{
	return T2String8::FindString( str, substr );
}

template <>
TINLINE TSIZE T2StringTraits<TCHAR>::Length( const CharType* str )
{
	return T2String8::Length( str );
}

template <>
TINLINE TBOOL T2StringTraits<TCHAR>::IsLowerCase( const CharType* str )
{
	return T2String8::IsLowerCase( str );
}

template <>
TINLINE TBOOL T2StringTraits<TCHAR>::IsUpperCase( const CharType* str )
{
	return T2String8::IsUpperCase( str );
}

template <>
TINLINE void T2StringTraits<TCHAR>::ToLowerCase( CharType* str )
{
	T2String8::ToLowerCase( str );
}

template <>
TINLINE void T2StringTraits<TCHAR>::ToUpperCase( CharType* str )
{
	T2String8::ToUpperCase( str );
}

template <>
TINLINE void T2StringTraits<TCHAR>::IntToString( TINT value, CharType* dst, TINT unused, TINT radix )
{
	T2String8::IntToString( value, dst, unused, radix );
}

template <>
TINLINE void T2StringTraits<TCHAR>::IntToString( TINT value, CharType* dst, TINT radix )
{
	T2String8::IntToString( value, dst, radix );
}

template <>
TINLINE TINT T2StringTraits<TCHAR>::StringToInt( const CharType* src )
{
	return T2String8::StringToInt( src );
}

template <>
TINLINE TFLOAT T2StringTraits<TCHAR>::StringToFloat( const CharType* src )
{
	return T2String8::StringToFloat( src );
}

template <>
TINLINE T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::SkipSpaces( CharType* str )
{
	return T2String8::SkipSpaces( str );
}

template <>
TINLINE const T2StringTraits<TCHAR>::CharType* T2StringTraits<TCHAR>::SkipSpaces( const CharType* str )
{
	return T2String8::SkipSpaces( str );
}

//-----------------------------------------------------------------------------
// TWCHAR - 2 bytes character
//-----------------------------------------------------------------------------

template <>
TINLINE TINT T2StringTraits<TWCHAR>::FormatV( CharType* a_pcString, TINT size, const CharType* a_pcFormat, va_list args )
{
	return T2String16::FormatV( a_pcString, size, a_pcFormat, args );
}

template <>
TINLINE TINT T2StringTraits<TWCHAR>::FormatV( CharType* a_pcString, const CharType* a_pcFormat, va_list args )
{
	return T2String16::FormatV( a_pcString, a_pcFormat, args );
}

template <>
TINLINE TINT T2StringTraits<TWCHAR>::Format( CharType* a_pcString, TINT size, const CharType* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, size, a_pcFormat, args );
	va_end( args );

	return iResult;
}

template <>
TINLINE TINT T2StringTraits<TWCHAR>::Format( CharType* a_pcString, const CharType* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, a_pcFormat, args );
	va_end( args );

	return iResult;
}

template <>
TINLINE TINT T2StringTraits<TWCHAR>::Compare( const CharType* str1, const CharType* str2, TSIZE size /*= -1 */ )
{
	return T2String16::Compare( str1, str2, size );
}

template <>
TINLINE TINT T2StringTraits<TWCHAR>::CompareNoCase( const CharType* str1, const CharType* str2, TSIZE size /*= -1 */ )
{
	return T2String16::CompareNoCase( str1, str2, size );
}

template <>
TINLINE T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::Copy( CharType* dst, const CharType* src, TSIZE size /*= -1 */ )
{
	return T2String16::Copy( dst, src, size );
}

template <>
TINLINE T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::CopySafe( CharType* dst, const CharType* src, TSIZE size )
{
	return T2String16::CopySafe( dst, src, size );
}

template <>
TINLINE T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::Concat( CharType* dst, const CharType* src, TSIZE size /*= -1 */ )
{
	return T2String16::Concat( dst, src, size );
}

template <>
TINLINE T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::FindChar( CharType* str, CharType character )
{
	return T2String16::FindChar( str, character );
}

template <>
TINLINE const T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::FindChar( const CharType* str, CharType character )
{
	return T2String16::FindChar( str, character );
}

template <>
TINLINE T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::FindString( CharType* str, const CharType* substr )
{
	return T2String16::FindString( str, substr );
}

template <>
TINLINE const T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::FindString( const CharType* str, const CharType* substr )
{
	return T2String16::FindString( str, substr );
}

template <>
TINLINE TSIZE T2StringTraits<TWCHAR>::Length( const CharType* str )
{
	return T2String16::Length( str );
}

template <>
TINLINE TBOOL T2StringTraits<TWCHAR>::IsLowerCase( const CharType* str )
{
	return T2String16::IsLowerCase( str );
}

template <>
TINLINE TBOOL T2StringTraits<TWCHAR>::IsUpperCase( const CharType* str )
{
	return T2String16::IsUpperCase( str );
}

template <>
TINLINE void T2StringTraits<TWCHAR>::ToLowerCase( CharType* str )
{
	T2String16::ToLowerCase( str );
}

template <>
TINLINE void T2StringTraits<TWCHAR>::ToUpperCase( CharType* str )
{
	T2String16::ToUpperCase( str );
}

template <>
TINLINE void T2StringTraits<TWCHAR>::IntToString( TINT value, CharType* dst, TINT unused, TINT radix )
{
	T2String16::IntToString( value, dst, unused, radix );
}

template <>
TINLINE void T2StringTraits<TWCHAR>::IntToString( TINT value, CharType* dst, TINT radix )
{
	T2String16::IntToString( value, dst, radix );
}

template <>
TINLINE TINT T2StringTraits<TWCHAR>::StringToInt( const CharType* src )
{
	return T2String16::StringToInt( src );
}

template <>
TINLINE TFLOAT T2StringTraits<TWCHAR>::StringToFloat( const CharType* src )
{
	return T2String16::StringToFloat( src );
}

template <>
TINLINE T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::SkipSpaces( CharType* str )
{
	return T2String16::SkipSpaces( str );
}

template <>
TINLINE const T2StringTraits<TWCHAR>::CharType* T2StringTraits<TWCHAR>::SkipSpaces( const CharType* str )
{
	return T2String16::SkipSpaces( str );
}

TOSHI_NAMESPACE_END
