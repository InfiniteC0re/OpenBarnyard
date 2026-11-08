#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <wchar.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>

#undef PAGE_SIZE
#undef PAGE_SHIFT

#include "TStringHelper_WASM.h"

// The Win32 types that shared code uses for retail layouts
using DWORD = unsigned long;
using WORD  = unsigned short;
using BYTE  = unsigned char;
using BOOL  = int;
using UINT  = unsigned int;
using LONG  = long;
using ULONG = unsigned long;
using INT   = int;
using FLOAT = float;
using CHAR  = char;
using WCHAR = wchar_t;

#define TRUE  1
#define FALSE 0

#ifndef MAX_PATH
#  define MAX_PATH 260
#endif

using HANDLE    = void*;
using HMODULE   = void*;
using HDC       = void*;
using HWND      = void*;
using HICON     = void*;
using LPCSTR    = const char*;
using LPSTR     = char*;
using LPVOID    = void*;
using ULONG_PTR = unsigned long;
using SIZE_T    = size_t;

union LARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		LONG  HighPart;
	};
	long long QuadPart;
};

struct FILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
};

using LONGLONG = long long;
using LPBYTE   = unsigned char*;
using HKEY     = void*;
using HGLOBAL  = void*;

union ULARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		DWORD HighPart;
	};
	unsigned long long QuadPart;
};

#define LOGPIXELSY 90
#define ERROR_SUCCESS 0
#define HKEY_CURRENT_USER ( (HKEY)1 )
#define KEY_READ 1
#define KEY_WRITE 2
#define KEY_ALL_ACCESS 3
#define REG_NONE 0
#define REG_SZ 1
#define REG_DWORD 4
#define REG_OPTION_NON_VOLATILE 0
#define MB_OK 0
#define MB_ICONWARNING 0
#define MB_ICONERROR 0
#define TIMER_ALL_ACCESS 0
#define INFINITE 0xFFFFFFFF
#define INVALID_FILE_ATTRIBUTES DWORD( -1 )
#define GMEM_MOVEABLE 0
#define CF_TEXT 1
#define YieldProcessor()
#define __CRTDECL
#define CALLBACK
#define WINAPI

#define _stricmp   strcasecmp
#define _strnicmp  strncasecmp
#define stricmp    strcasecmp
#define strnicmp   strncasecmp
#define _snprintf  snprintf
#define _vsnprintf vsnprintf
#define _finite    isfinite
#define _strdup    strdup
#define _TRUNCATE  ( size_t( -1 ) )
#define strtok_s   strtok_r
#define _vsnprintf_s( DST, SIZE, COUNT, FORMAT, ARGS ) vsnprintf( DST, SIZE, FORMAT, ARGS )
#define __time32_t time_t
#define _time32 time
#define _localtime32 localtime

void          OutputDebugStringA( const char* a_szString );
const char*   GetCommandLineA();
DWORD         GetTickCount();
void          Sleep( DWORD a_uiMilliseconds );
BOOL          QueryPerformanceCounter( LARGE_INTEGER* a_pCounter );
BOOL          QueryPerformanceFrequency( LARGE_INTEGER* a_pFrequency );
DWORD         GetModuleFileNameA( HMODULE a_hModule, char* a_szBuffer, DWORD a_uiSize );
DWORD         GetTempPathA( DWORD a_uiSize, char* a_szBuffer );

// Settings live in the registry on Windows, here there's nothing to read so the defaults stay
inline LONG RegOpenKeyExA( HKEY, const char*, DWORD, DWORD, HKEY* ) { return 1; }
inline LONG RegCreateKeyExA( HKEY, const char*, DWORD, char*, DWORD, DWORD, void*, HKEY*, DWORD* ) { return 1; }
inline LONG RegQueryValueExA( HKEY, const char*, DWORD*, DWORD*, unsigned char*, DWORD* ) { return 1; }
inline LONG RegSetValueExA( HKEY, const char*, DWORD, DWORD, const unsigned char*, DWORD ) { return 1; }
inline LONG RegCloseKey( HKEY ) { return 0; }

int MessageBoxA( HWND a_hWnd, const char* a_szText, const char* a_szCaption, UINT a_uiType );

inline BOOL   IsDebuggerPresent() { return FALSE; }
inline HANDLE GetProcessHeap() { return nullptr; }
inline void*  HeapAlloc( HANDLE, DWORD, size_t a_uiSize ) { return malloc( a_uiSize ); }

// A frame limit isn't needed, the page paces the frames
inline HANDLE CreateWaitableTimerExW( void*, const wchar_t*, DWORD, DWORD ) { return nullptr; }
inline BOOL   SetWaitableTimer( HANDLE, const LARGE_INTEGER*, LONG, void*, void*, BOOL ) { return FALSE; }
inline DWORD  WaitForSingleObject( HANDLE, DWORD ) { return 0; }
inline UINT   timeBeginPeriod( UINT ) { return 0; }

inline BOOL    OpenClipboard( HWND ) { return FALSE; }
inline BOOL    EmptyClipboard() { return FALSE; }
inline BOOL    CloseClipboard() { return FALSE; }
inline HANDLE  SetClipboardData( UINT, HANDLE ) { return nullptr; }
inline HGLOBAL GlobalAlloc( UINT, size_t ) { return nullptr; }
inline void*   GlobalLock( HGLOBAL ) { return nullptr; }
inline BOOL    GlobalUnlock( HGLOBAL ) { return FALSE; }
inline HGLOBAL GlobalFree( HGLOBAL ) { return nullptr; }

inline HANDLE GetCurrentProcess() { return nullptr; }
inline BOOL   GetProcessTimes( HANDLE, FILETIME* a_pCreation, FILETIME*, FILETIME*, FILETIME* ) { *a_pCreation = {}; return TRUE; }
void          GetSystemTimeAsFileTime( FILETIME* a_pTime );
inline BOOL   CreateDirectoryA( const char*, void* ) { return TRUE; }
inline DWORD  GetFileAttributesA( const char* ) { return INVALID_FILE_ATTRIBUTES; }

// The game's save files go to the folder the page keeps in IndexedDB
FILE* WASM_fopen( const char* a_szPath, const char* a_szMode );
int   WASM_fclose( FILE* a_pFile );

#define fopen  WASM_fopen
#define fclose WASM_fclose

inline HDC    GetDC( HWND ) { return nullptr; }
inline int    ReleaseDC( HWND, HDC ) { return 1; }
inline int    GetDeviceCaps( HDC, int ) { return 96; }
unsigned long GetCurrentThreadId();

#define _strupr  TStringHelper_WASM::ToUpper
#define _strlwr  TStringHelper_WASM::ToLower
#define _strtime TStringHelper_WASM::GetTimeString
#define _itoa    TStringHelper_WASM::IntToString
#define __rdtsc  TStringHelper_WASM::GetTimestamp

#define wcslen     TStringHelper_WASM::WideLength
#define wcscmp     TStringHelper_WASM::WideCompare
#define wcsncmp    TStringHelper_WASM::WideCompareN
#define _wcsicmp   TStringHelper_WASM::WideCompareNoCase
#define _wcsnicmp  TStringHelper_WASM::WideCompareNoCaseN
#define wcscpy     TStringHelper_WASM::WideCopy
#define wcsncpy    TStringHelper_WASM::WideCopyN
#define wcscat     TStringHelper_WASM::WideConcat
#define wcsncat    TStringHelper_WASM::WideConcatN
#define wcschr     TStringHelper_WASM::WideFindChar
#define wcsrchr    TStringHelper_WASM::WideFindLastChar
#define wcsstr     TStringHelper_WASM::WideFindString
#define _wcsupr    TStringHelper_WASM::WideToUpper
#define _wcslwr    TStringHelper_WASM::WideToLower
#define _wtoi      TStringHelper_WASM::WideToInt
#define _wtof      TStringHelper_WASM::WideToFloat
#define _itow      TStringHelper_WASM::WideFromInt
#define _vsnwprintf TStringHelper_WASM::WideFormatN
#define _vswprintf  TStringHelper_WASM::WideFormat
