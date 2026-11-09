#include "ToshiPCH.h"
#include "TStringHelper_WASM.h"

#include <ctype.h>
#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/threading.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include <time.h>
#include <wctype.h>

#undef fopen
#undef fclose

EM_JS( void, WASM_SyncSaves, (), {
	postMessage( { bSyncSaves : true } );
} );

char* TStringHelper_WASM::ToUpper( char* a_szString )
{
	for ( char* p = a_szString; *p; p++ ) *p = char( toupper( TUINT8( *p ) ) );
	return a_szString;
}

char* TStringHelper_WASM::ToLower( char* a_szString )
{
	for ( char* p = a_szString; *p; p++ ) *p = char( tolower( TUINT8( *p ) ) );
	return a_szString;
}

char* TStringHelper_WASM::GetTimeString( char* a_szBuffer )
{
	const time_t iTime = time( TNULL );
	strftime( a_szBuffer, 9, "%H:%M:%S", localtime( &iTime ) );
	return a_szBuffer;
}

char* TStringHelper_WASM::IntToString( int a_iValue, char* a_szBuffer, int a_iRadix )
{
	sprintf( a_szBuffer, a_iRadix == 16 ? "%x" : "%d", a_iValue );
	return a_szBuffer;
}

void OutputDebugStringA( const char* a_szString )
{
	emscripten_console_log( a_szString );
}

EM_JS( char*, WASM_GetCommandLine, (), {
	var szSearch = typeof g_szPageSearch !== "undefined" ? g_szPageSearch : location.search;
	var szArgs   = "OpenBarnyard " + ( new URLSearchParams( szSearch ).get( "args" ) || "" );
	var iSize  = lengthBytesUTF8( szArgs ) + 1;
	var pArgs  = _malloc( iSize );
	stringToUTF8( szArgs, pArgs, iSize );
	return pArgs;
} );

const char* GetCommandLineA()
{
	static const char* s_szCommandLine = WASM_GetCommandLine();
	return s_szCommandLine;
}

DWORD GetTickCount()
{
	return DWORD( emscripten_get_now() );
}

void Sleep( DWORD a_uiMilliseconds )
{
	emscripten_thread_sleep( a_uiMilliseconds );
}

BOOL QueryPerformanceCounter( LARGE_INTEGER* a_pCounter )
{
	a_pCounter->QuadPart = (long long)( emscripten_get_now() * 1000.0 );
	return TRUE;
}

BOOL QueryPerformanceFrequency( LARGE_INTEGER* a_pFrequency )
{
	a_pFrequency->QuadPart = 1000000;
	return TRUE;
}

DWORD GetModuleFileNameA( HMODULE a_hModule, char* a_szBuffer, DWORD a_uiSize )
{
	strncpy( a_szBuffer, "\\OpenBarnyard.exe", a_uiSize );
	return DWORD( strlen( a_szBuffer ) );
}

DWORD GetTempPathA( DWORD a_uiSize, char* a_szBuffer )
{
	strncpy( a_szBuffer, "Temp/", a_uiSize );
	return DWORD( strlen( a_szBuffer ) );
}

int MessageBoxA( HWND a_hWnd, const char* a_szText, const char* a_szCaption, UINT a_uiType )
{
	emscripten_console_error( a_szText );
	return 1;
}

void GetSystemTimeAsFileTime( FILETIME* a_pTime )
{
	const unsigned long long uiTicks = (unsigned long long)( emscripten_get_now() * 10000.0 );
	a_pTime->dwLowDateTime           = DWORD( uiTicks );
	a_pTime->dwHighDateTime          = DWORD( uiTicks >> 32 );
}

static void MakeSavePath( const char* a_szPath, char* a_szOut, size_t a_uiSize )
{
	size_t uiOut = 0;

	if ( a_szPath[ 0 ] != '/' && a_szPath[ 0 ] != '\\' )
		a_szOut[ uiOut++ ] = '/';

	for ( const char* p = a_szPath; *p && uiOut + 1 < a_uiSize; p++ )
		a_szOut[ uiOut++ ] = *p == '\\' ? '/' : *p;

	a_szOut[ uiOut ] = '\0';
}

static TBOOL s_bSavesChanged;

FILE* WASM_fopen( const char* a_szPath, const char* a_szMode )
{
	char szPath[ 512 ];
	MakeSavePath( a_szPath, szPath, sizeof( szPath ) );

	if ( strchr( a_szMode, 'w' ) || strchr( a_szMode, 'a' ) )
		s_bSavesChanged = TTRUE;

	return fopen( szPath, a_szMode );
}

int WASM_fclose( FILE* a_pFile )
{
	const int iResult = fclose( a_pFile );

	if ( s_bSavesChanged )
	{
		s_bSavesChanged = TFALSE;
		WASM_SyncSaves();
	}

	return iResult;
}

intptr_t _findfirst( const char* a_szPath, _finddata_t* a_pData )
{
	char szPath[ 512 ];
	MakeSavePath( a_szPath, szPath, sizeof( szPath ) );

	struct stat oStat;

	if ( stat( szPath, &oStat ) != 0 )
		return -1;

	a_pData->attrib = S_ISDIR( oStat.st_mode ) ? _A_SUBDIR : 0;
	a_pData->size   = long( oStat.st_size );

	return 1;
}

int _findclose( intptr_t a_hFind )
{
	return 0;
}

int _mkdir( const char* a_szPath )
{
	char szPath[ 512 ];
	MakeSavePath( a_szPath, szPath, sizeof( szPath ) );

	return mkdir( szPath, 0777 );
}

unsigned long GetCurrentThreadId()
{
	return (unsigned long)pthread_self();
}

unsigned long long TStringHelper_WASM::GetTimestamp()
{
	return (unsigned long long)( emscripten_get_now() * 1000000.0 );
}

size_t TStringHelper_WASM::WideLength( const wchar_t* a_wszString )
{
	const wchar_t* p = a_wszString;
	while ( *p ) p++;
	return p - a_wszString;
}

int TStringHelper_WASM::WideCompare( const wchar_t* a_wszA, const wchar_t* a_wszB )
{
	while ( *a_wszA && *a_wszA == *a_wszB ) { a_wszA++; a_wszB++; }
	return int( TUINT16( *a_wszA ) ) - int( TUINT16( *a_wszB ) );
}

int TStringHelper_WASM::WideCompareN( const wchar_t* a_wszA, const wchar_t* a_wszB, size_t a_uiSize )
{
	for ( ; a_uiSize > 0; a_uiSize--, a_wszA++, a_wszB++ )
	{
		if ( *a_wszA != *a_wszB ) return int( TUINT16( *a_wszA ) ) - int( TUINT16( *a_wszB ) );
		if ( !*a_wszA ) return 0;
	}

	return 0;
}

static wchar_t ToLowerWide( wchar_t a_cChar )
{
	return ( a_cChar >= L'A' && a_cChar <= L'Z' ) ? wchar_t( a_cChar - L'A' + L'a' ) : a_cChar;
}

static wchar_t ToUpperWide( wchar_t a_cChar )
{
	return ( a_cChar >= L'a' && a_cChar <= L'z' ) ? wchar_t( a_cChar - L'a' + L'A' ) : a_cChar;
}

int TStringHelper_WASM::WideCompareNoCase( const wchar_t* a_wszA, const wchar_t* a_wszB )
{
	while ( *a_wszA && ToLowerWide( *a_wszA ) == ToLowerWide( *a_wszB ) ) { a_wszA++; a_wszB++; }
	return int( TUINT16( ToLowerWide( *a_wszA ) ) ) - int( TUINT16( ToLowerWide( *a_wszB ) ) );
}

int TStringHelper_WASM::WideCompareNoCaseN( const wchar_t* a_wszA, const wchar_t* a_wszB, size_t a_uiSize )
{
	for ( ; a_uiSize > 0; a_uiSize--, a_wszA++, a_wszB++ )
	{
		if ( ToLowerWide( *a_wszA ) != ToLowerWide( *a_wszB ) ) return int( TUINT16( ToLowerWide( *a_wszA ) ) ) - int( TUINT16( ToLowerWide( *a_wszB ) ) );
		if ( !*a_wszA ) return 0;
	}

	return 0;
}

wchar_t* TStringHelper_WASM::WideCopy( wchar_t* a_wszDst, const wchar_t* a_wszSrc )
{
	wchar_t* p = a_wszDst;
	while ( ( *p++ = *a_wszSrc++ ) ) {}
	return a_wszDst;
}

wchar_t* TStringHelper_WASM::WideCopyN( wchar_t* a_wszDst, const wchar_t* a_wszSrc, size_t a_uiSize )
{
	size_t i = 0;
	for ( ; i < a_uiSize && a_wszSrc[ i ]; i++ ) a_wszDst[ i ] = a_wszSrc[ i ];
	for ( ; i < a_uiSize; i++ ) a_wszDst[ i ] = L'\0';
	return a_wszDst;
}

wchar_t* TStringHelper_WASM::WideConcat( wchar_t* a_wszDst, const wchar_t* a_wszSrc )
{
	WideCopy( a_wszDst + WideLength( a_wszDst ), a_wszSrc );
	return a_wszDst;
}

wchar_t* TStringHelper_WASM::WideConcatN( wchar_t* a_wszDst, const wchar_t* a_wszSrc, size_t a_uiSize )
{
	wchar_t* p = a_wszDst + WideLength( a_wszDst );
	for ( ; a_uiSize > 0 && *a_wszSrc; a_uiSize-- ) *p++ = *a_wszSrc++;
	*p = L'\0';
	return a_wszDst;
}

wchar_t* TStringHelper_WASM::WideFindChar( const wchar_t* a_wszString, wchar_t a_cChar )
{
	for ( ;; a_wszString++ )
	{
		if ( *a_wszString == a_cChar ) return const_cast<wchar_t*>( a_wszString );
		if ( !*a_wszString ) return TNULL;
	}
}

wchar_t* TStringHelper_WASM::WideFindLastChar( const wchar_t* a_wszString, wchar_t a_cChar )
{
	const wchar_t* pFound = TNULL;

	for ( ;; a_wszString++ )
	{
		if ( *a_wszString == a_cChar ) pFound = a_wszString;
		if ( !*a_wszString ) return const_cast<wchar_t*>( pFound );
	}
}

wchar_t* TStringHelper_WASM::WideFindString( const wchar_t* a_wszString, const wchar_t* a_wszFind )
{
	const size_t uiLength = WideLength( a_wszFind );

	for ( ; ; a_wszString++ )
	{
		if ( WideCompareN( a_wszString, a_wszFind, uiLength ) == 0 ) return const_cast<wchar_t*>( a_wszString );
		if ( !*a_wszString ) return TNULL;
	}
}

wchar_t* TStringHelper_WASM::WideToUpper( wchar_t* a_wszString )
{
	for ( wchar_t* p = a_wszString; *p; p++ ) *p = ToUpperWide( *p );
	return a_wszString;
}

wchar_t* TStringHelper_WASM::WideToLower( wchar_t* a_wszString )
{
	for ( wchar_t* p = a_wszString; *p; p++ ) *p = ToLowerWide( *p );
	return a_wszString;
}

static void WideToNarrow( const wchar_t* a_wszString, char* a_szOut, size_t a_uiSize )
{
	size_t i = 0;
	for ( ; i + 1 < a_uiSize && a_wszString[ i ]; i++ ) a_szOut[ i ] = char( a_wszString[ i ] );
	a_szOut[ i ] = '\0';
}

int TStringHelper_WASM::WideToInt( const wchar_t* a_wszString )
{
	char szBuffer[ 64 ];
	WideToNarrow( a_wszString, szBuffer, sizeof( szBuffer ) );
	return atoi( szBuffer );
}

double TStringHelper_WASM::WideToFloat( const wchar_t* a_wszString )
{
	char szBuffer[ 64 ];
	WideToNarrow( a_wszString, szBuffer, sizeof( szBuffer ) );
	return atof( szBuffer );
}

wchar_t* TStringHelper_WASM::WideFromInt( int a_iValue, wchar_t* a_wszBuffer, int a_iRadix )
{
	char szBuffer[ 32 ];
	IntToString( a_iValue, szBuffer, a_iRadix );

	size_t i = 0;
	for ( ; szBuffer[ i ]; i++ ) a_wszBuffer[ i ] = wchar_t( szBuffer[ i ] );
	a_wszBuffer[ i ] = L'\0';

	return a_wszBuffer;
}

// %s takes a wide string and %S a narrow one, like the wide printf of MSVC
int TStringHelper_WASM::WideFormatN( wchar_t* a_wszBuffer, size_t a_uiSize, const wchar_t* a_wszFormat, va_list a_Args )
{
	size_t uiOut = 0;

	auto fnPut = [ & ]( wchar_t a_cChar ) {
		if ( uiOut + 1 < a_uiSize ) a_wszBuffer[ uiOut ] = a_cChar;
		uiOut++;
	};

	for ( const wchar_t* p = a_wszFormat; *p; p++ )
	{
		if ( *p != L'%' )
		{
			fnPut( *p );
			continue;
		}

		char   szSpec[ 32 ];
		size_t uiSpec    = 0;
		szSpec[ uiSpec++ ] = '%';
		p++;

		int iWidth     = 0;
		int iPrecision = -1;

		while ( *p && WideFindChar( L"-+ #0", *p ) && uiSpec < 16 )
			szSpec[ uiSpec++ ] = char( *p++ );

		if ( *p == L'*' )
		{
			iWidth = va_arg( a_Args, int );
			uiSpec += sprintf( szSpec + uiSpec, "%d", iWidth );
			p++;
		}
		else
		{
			while ( *p >= L'0' && *p <= L'9' && uiSpec < 20 )
			{
				iWidth             = iWidth * 10 + ( *p - L'0' );
				szSpec[ uiSpec++ ] = char( *p++ );
			}
		}

		if ( *p == L'.' )
		{
			szSpec[ uiSpec++ ] = '.';
			p++;
			iPrecision = 0;

			if ( *p == L'*' )
			{
				iPrecision = va_arg( a_Args, int );
				uiSpec += sprintf( szSpec + uiSpec, "%d", iPrecision );
				p++;
			}
			else
			{
				while ( *p >= L'0' && *p <= L'9' && uiSpec < 26 )
				{
					iPrecision         = iPrecision * 10 + ( *p - L'0' );
					szSpec[ uiSpec++ ] = char( *p++ );
				}
			}
		}

		TBOOL bLong = TFALSE;
		TBOOL bLongLong = TFALSE;
		TBOOL bShort = TFALSE;

		while ( *p == L'l' || *p == L'h' )
		{
			if ( *p == L'l' ) { bLongLong = bLong; bLong = TTRUE; }
			else bShort = TTRUE;
			p++;
		}

		const wchar_t cType = *p;
		char          szOut[ 128 ];

		switch ( cType )
		{
			case L'%':
				fnPut( L'%' );
				break;
			case L'c':
			case L'C':
				fnPut( wchar_t( va_arg( a_Args, int ) ) );
				break;
			case L's':
			case L'S':
			{
				const TBOOL bNarrow = ( cType == L'S' ) != bShort && !bLong;

				if ( bNarrow )
				{
					const char* szString = va_arg( a_Args, const char* );
					if ( !szString ) szString = "(null)";

					const int iLength = int( strlen( szString ) );
					const int iCount  = iPrecision >= 0 && iPrecision < iLength ? iPrecision : iLength;

					for ( int i = iCount; i < iWidth; i++ ) fnPut( L' ' );
					for ( int i = 0; i < iCount; i++ ) fnPut( wchar_t( TUINT8( szString[ i ] ) ) );
				}
				else
				{
					const wchar_t* wszString = va_arg( a_Args, const wchar_t* );
					if ( !wszString ) wszString = L"(null)";

					const int iLength = int( WideLength( wszString ) );
					const int iCount  = iPrecision >= 0 && iPrecision < iLength ? iPrecision : iLength;

					for ( int i = iCount; i < iWidth; i++ ) fnPut( L' ' );
					for ( int i = 0; i < iCount; i++ ) fnPut( wszString[ i ] );
				}
				break;
			}
			case L'd':
			case L'i':
			case L'u':
			case L'x':
			case L'X':
			case L'o':
			{
				if ( bLongLong )
				{
					szSpec[ uiSpec++ ] = 'l';
					szSpec[ uiSpec++ ] = 'l';
				}

				szSpec[ uiSpec++ ] = char( cType );
				szSpec[ uiSpec ]   = '\0';

				if ( bLongLong )
					snprintf( szOut, sizeof( szOut ), szSpec, va_arg( a_Args, long long ) );
				else
					snprintf( szOut, sizeof( szOut ), szSpec, va_arg( a_Args, int ) );

				for ( const char* q = szOut; *q; q++ ) fnPut( wchar_t( *q ) );
				break;
			}
			case L'f':
			case L'F':
			case L'g':
			case L'G':
			case L'e':
			case L'E':
			{
				szSpec[ uiSpec++ ] = char( cType );
				szSpec[ uiSpec ]   = '\0';
				snprintf( szOut, sizeof( szOut ), szSpec, va_arg( a_Args, double ) );

				for ( const char* q = szOut; *q; q++ ) fnPut( wchar_t( *q ) );
				break;
			}
			case L'\0':
				p--;
				break;
			default:
				fnPut( cType );
				break;
		}
	}

	if ( a_uiSize > 0 )
		a_wszBuffer[ uiOut < a_uiSize ? uiOut : a_uiSize - 1 ] = L'\0';

	return int( uiOut );
}

int TStringHelper_WASM::WideFormat( wchar_t* a_wszBuffer, const wchar_t* a_wszFormat, va_list a_Args )
{
	return WideFormatN( a_wszBuffer, size_t( -1 ) / 4, a_wszFormat, a_Args );
}
