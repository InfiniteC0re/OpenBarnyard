#pragma once
#include "PrimeDebug.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// The string helpers of Prime the debug tools use (crString), on the C library
class PString
{
public:
	// NOTE: Prime's crStringCopy fails fatally when a_szSource doesn't fit; this one cuts it and returns TNULL
	static TCHAR* Copy( TCHAR* a_szDest, TSIZE a_uiDestSize, const TCHAR* a_szSource )
	{
		if ( a_uiDestSize == 0 )
			return TNULL;

		TSIZE uiLength = strlen( a_szSource );

		if ( uiLength >= a_uiDestSize )
		{
			memcpy( a_szDest, a_szSource, a_uiDestSize - 1 );
			a_szDest[ a_uiDestSize - 1 ] = '\0';
			return TNULL;
		}

		memcpy( a_szDest, a_szSource, uiLength + 1 );
		return a_szDest;
	}

	// $deBlob2: FUNCTION 0057d09b
	static TCHAR* CopyToLower( TCHAR* a_szDest, TINT a_iDestSize, const TCHAR* a_szSource )
	{
		for ( TINT i = 0; i < a_iDestSize - 1; i++ )
		{
			a_szDest[ i ] = TCHAR( tolower( a_szSource[ i ] ) );

			if ( a_szSource[ i ] == '\0' )
				return a_szDest;
		}

		a_szDest[ a_iDestSize - 1 ] = '\0';
		return a_szDest;
	}

	static void Concat( TCHAR* a_szDest, TSIZE a_uiDestSize, const TCHAR* a_szSource )
	{
		TSIZE uiLength = strlen( a_szDest );

		if ( uiLength + 1 < a_uiDestSize )
			Copy( a_szDest + uiLength, a_uiDestSize - uiLength, a_szSource );
	}

	static void Format( TCHAR* a_szDest, TINT a_iDestSize, const TCHAR* a_szFormat, ... )
	{
		va_list args;
		va_start( args, a_szFormat );
		FormatV( a_szDest, a_iDestSize, a_szFormat, args );
		va_end( args );
	}

	static void FormatV( TCHAR* a_szDest, TINT a_iDestSize, const TCHAR* a_szFormat, va_list a_Args )
	{
		_vsnprintf_s( a_szDest, a_iDestSize, _TRUNCATE, a_szFormat, a_Args );
	}

	// The first token of a_szString (strtok): the delimiters before it are skipped, the one after it is cut
	// $deBlob2: FUNCTION 0057cf74
	static TCHAR* Tokenize( TCHAR* a_szString, const TCHAR* a_szDelimiters )
	{
		TCHAR* pContext = TNULL;

		if ( a_szString == TNULL )
			return TNULL;

		return strtok_s( a_szString, a_szDelimiters, &pContext );
	}

	// $deBlob2: FUNCTION 0057e156
	static TINT ToInt( const TCHAR* a_szString, TCHAR** a_ppEnd, TINT a_iBase ) { return strtol( a_szString, a_ppEnd, a_iBase ); }

	// $deBlob2: FUNCTION 0057e15f
	static TFLOAT ToFloat( const TCHAR* a_szString, TCHAR** a_ppEnd ) { return TFLOAT( strtod( a_szString, a_ppEnd ) ); }
};
