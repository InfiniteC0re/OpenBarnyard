#include "ToshiPCH.h"
#include "TStringManager.h"
#include <string>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TWCHAR* TStringManager::StringCharToUnicode( TWCHAR* a_UnicodeString, const TCHAR* a_CharString, TUINT32 a_iLength )
{
	TASSERT( ( a_UnicodeString != TNULL ) && ( a_CharString != TNULL ) );

	auto iCharStringLength = String8Length( a_CharString );

	if ( iCharStringLength < a_iLength || a_iLength == -1 )
		a_iLength = iCharStringLength;

	for ( TSIZE i = 0; i < a_iLength; i++ )
		a_UnicodeString[ i ] = TWCHAR( a_CharString[ i ] );

	a_UnicodeString[ a_iLength ] = L'\0';
	return a_UnicodeString;
}

TCHAR* TStringManager::StringUnicodeToChar( TCHAR* a_CharString, const TWCHAR* const& a_UnicodeString, TUINT32 a_iLength )
{
	TASSERT( ( a_UnicodeString != TNULL ) && ( a_CharString != TNULL ) );

	auto iUnicodeStringLength = String16Length( a_UnicodeString );

	if ( iUnicodeStringLength < a_iLength || a_iLength == -1 )
		a_iLength = iUnicodeStringLength;

	for ( TSIZE i = 0; i < a_iLength; i++ )
		a_CharString[ i ] = TCHAR( a_UnicodeString[ i ] );

	a_CharString[ a_iLength ] = '\0';
	return a_CharString;
}

TOSHI_NAMESPACE_END
