#include "ToshiPCH.h"
#include "T2CharTraits.h"
#include "T2String16.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TWCHAR T2String16::ms_aScratchMem[ T2String16::SCRATCH_MEM_SIZE ];

TINT T2String16::FormatV( TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, va_list args )
{
	TINT iResult = _vsnwprintf( a_pcString, size, a_pcFormat, args );
	TASSERT( iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem" );
	a_pcString[ size - 1 ] = '\0';
	return iResult;
}

TINT T2String16::FormatV( TWCHAR* a_pcString, const TWCHAR* a_pcFormat, va_list args )
{
	TINT iResult = _vswprintf( a_pcString, a_pcFormat, args );
	TASSERT( iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem" );
	return iResult;
}

TINT T2String16::Format( TWCHAR* a_pcString, const TWCHAR* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, a_pcFormat, args );
	va_end( args );

	return iResult;
}

TINT T2String16::Format( TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, size, a_pcFormat, args );
	va_end( args );

	return iResult;
}

TINT T2String16::Compare( const TWCHAR* str1, const TWCHAR* str2, TSIZE size )
{
	if ( size != -1 )
		return wcsncmp( str1, str2, size );

	return wcscmp( str1, str2 );
}

TINT T2String16::CompareNoCase( const TWCHAR* str1, const TWCHAR* str2, TSIZE size )
{
	if ( size != -1 )
	{
		return _wcsnicmp( str1, str2, size );
	}

	return _wcsicmp( str1, str2 );
}

TWCHAR* T2String16::Copy( TWCHAR* dst, const TWCHAR* src, TSIZE size )
{
	if ( size != -1 )
		return wcsncpy( dst, src, size );

	return wcscpy( dst, src );
}

TWCHAR* T2String16::CopySafe( TWCHAR* dst, const TWCHAR* src, TSIZE size )
{
	TSIZE srcLen = Length( src );
	size         = TMath::Min( size - 1, srcLen );
	wcsncpy( dst, src, size );
	dst[ size ] = L'\0';
	return dst;
}

TWCHAR* T2String16::Concat( TWCHAR* dst, const TWCHAR* src, TSIZE size /*= -1 */ )
{
	if ( size != -1 )
		return wcsncat( dst, src, size );

	return wcscat( dst, src );
}

const TWCHAR* T2String16::FindChar( const TWCHAR* str, TWCHAR character )
{
	while ( TTRUE )
	{
		if ( *str == L'\0' ) return TNULL;
		if ( *str == character ) return str;
		str++;
	}

	return TNULL;
}

TWCHAR* T2String16::FindChar( TWCHAR* str, TWCHAR character )
{
	while ( TTRUE )
	{
		if ( *str == L'\0' ) return TNULL;
		if ( *str == character ) return str;
		str++;
	}

	return TNULL;
}

const TWCHAR* T2String16::FindString( const TWCHAR* str, const TWCHAR* substr )
{
	return wcsstr( str, substr );
}

TWCHAR* T2String16::FindString( TWCHAR* str, const TWCHAR* substr )
{
	return wcsstr( str, substr );
}

// $Barnyard: FUNCTION 006bb3c0
TSIZE T2String16::Length( const TWCHAR* str )
{
	if ( str != TNULL ) return wcslen( str );
	return -1;
}

TBOOL T2String16::IsLowerCase( const TWCHAR* str )
{
	while ( *str != L'\0' && T2Char16::IsLowerCase( *str ) )
		str++;

	return ( *str == L'\0' );
}

TBOOL T2String16::IsUpperCase( const TWCHAR* str )
{
	while ( *str != L'\0' && T2Char16::IsUpperCase( *str ) )
		str++;

	return ( *str == L'\0' );
}

void T2String16::ToLowerCase( TWCHAR* str )
{
	while ( *str )
	{
		*str = T2Char16::ToLowerCase( *str );
		str++;
	}
}

void T2String16::ToUpperCase( TWCHAR* str )
{
	while ( *str )
	{
		*str = T2Char16::ToUpperCase( *str );
		str++;
	}
}

void T2String16::IntToString( TINT value, TWCHAR* dst, TINT unused, TINT radix )
{
	_itow( value, dst, radix );
}

void T2String16::IntToString( TINT value, TWCHAR* dst, TINT radix )
{
	_itow( value, dst, radix );
}

TINT T2String16::StringToInt( const TWCHAR* src )
{
	return _wtoi( src );
}

TFLOAT T2String16::StringToFloat( const TWCHAR* src )
{
	return (TFLOAT)_wtof( src );
}

TWCHAR* T2String16::SkipSpaces( TWCHAR* str )
{
	TWCHAR* pszCursor = str;
	while ( T2Char16::IsSpace( *pszCursor ) ) pszCursor++;

	return pszCursor;
}

const TWCHAR* T2String16::SkipSpaces( const TWCHAR* str )
{
	const TWCHAR* pszCursor = str;
	while ( T2Char16::IsSpace( *pszCursor ) ) pszCursor++;

	return pszCursor;
}

// $Barnyard: FUNCTION 006bb450
TWCHAR* T2String16::CreateCopy( const TWCHAR* a_wszString, TSIZE a_uiSize )
{
	TSIZE uiSize = ( a_uiSize == -1 ) ? T2String16::Length( a_wszString ) : a_uiSize;

	TWCHAR* wszBuffer = new TWCHAR[ uiSize + 1 ];
	TUtil::MemCopy( wszBuffer, a_wszString, uiSize * sizeof( TWCHAR ) );
	wszBuffer[ uiSize ] = L'\0';

	return wszBuffer;
}

TOSHI_NAMESPACE_END
