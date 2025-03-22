#include "ToshiPCH.h"
#include "Toshi/Platform.h"
#include "Toshi/TString8.h"
#include "Toshi/TString16.h"
#include "Toshi/T2String.h"
#include "ToshiTools/T2DynamicVector.h"

#include <Windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void ThreadSleep( TUINT dwMilliseconds )
{
	Sleep( dwMilliseconds );
}

void FixPathSlashes( TString8& a_rPath )
{
	for ( TINT i = 0; i < a_rPath.Length(); i++ )
	{
		if ( a_rPath[ i ] == '/' )
			a_rPath[ i ] = '\\';
	}
}

void FixPathSlashesBack( TString8& a_rPath )
{
	for ( TINT i = 0; i < a_rPath.Length(); i++ )
	{
		if ( a_rPath[ i ] == '\\' )
			a_rPath[ i ] = '/';
	}
}

TString8 Platform_UnicodeToUTF8( const TWCHAR* a_wszUnicodeString )
{
	TINT iStrLen;
	if ( a_wszUnicodeString && ( iStrLen = T2String16::Length( a_wszUnicodeString ) ) )
	{
		TINT sizeRequired = WideCharToMultiByte( CP_UTF8, 0, a_wszUnicodeString, -1, NULL, 0, NULL, NULL );

		if ( sizeRequired > 0 )
		{
			T2DynamicVector<TCHAR> utf8String;
			utf8String.SetSize( sizeRequired );

			TINT bytesConverted = WideCharToMultiByte( CP_UTF8, 0, a_wszUnicodeString, -1, &utf8String[ 0 ], sizeRequired, NULL, NULL );

			if ( bytesConverted != 0 )
				return &utf8String[ 0 ];
		}
	}

	return "";
}

TString16 Platform_UTF8ToUnicode( const TCHAR* a_pchUTF8String )
{
	TINT iStrLen; 
	if ( a_pchUTF8String && ( T2String8::Length( a_pchUTF8String ) ) )
	{
		TINT sizeRequired = MultiByteToWideChar( CP_UTF8, 0, a_pchUTF8String, -1, NULL, 0 );

		if ( sizeRequired > 0 )
		{
			T2DynamicVector<TWCHAR> utf16String;
			utf16String.SetSize( sizeRequired );

			TINT bytesConverted = MultiByteToWideChar( CP_UTF8, 0, a_pchUTF8String, -1, &utf16String[ 0 ], sizeRequired );

			if ( bytesConverted != 0 )
				return &utf16String[ 0 ];
		}
	}

	return L"";
}

TOSHI_NAMESPACE_END
