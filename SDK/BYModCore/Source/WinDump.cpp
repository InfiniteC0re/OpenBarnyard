#include "pch.h"

#include <windows.h>
#include <Dbghelp.h>
#include <tchar.h>

typedef BOOL( WINAPI* MINIDUMPWRITEDUMP )( HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam );

void create_minidump( struct _EXCEPTION_POINTERS* apExceptionInfo )
{
	HMODULE           mhLib = ::LoadLibraryA( _T("dbghelp.dll") );
	MINIDUMPWRITEDUMP pDump = ( MINIDUMPWRITEDUMP )::GetProcAddress( mhLib, "MiniDumpWriteDump" );

	HANDLE hFile = ::CreateFileA( _T("crash.dmp"), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId          = ::GetCurrentThreadId();
	ExInfo.ExceptionPointers = apExceptionInfo;
	ExInfo.ClientPointers    = FALSE;

	pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
	::CloseHandle( hFile );
}

LONG WINAPI unhandled_handler( struct _EXCEPTION_POINTERS* apExceptionInfo )
{
	create_minidump( apExceptionInfo );

	MessageBoxA( TNULL, "It looks like your game has unfortunately crashed!\n\nContact @infc0re on Discord attaching the crash.dmp file from your game directory.", "Barnyard Crashed", MB_OK );
	return EXCEPTION_CONTINUE_SEARCH;
}
