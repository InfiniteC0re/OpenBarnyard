#include "ToshiPCH.h"
#include "Toshi/Platform.h"
#include "Toshi/TString16.h"
#include "Toshi/THPTimer.h"
#include "Toshi/TDebugConsole.h"

#include <emscripten/console.h>
#include "Toshi/TUtil.h"
#include "File/TFile.h"
#include "File/TNullFile.h"
#include "File/TMemFile.h"
#include "Core/TMemory.h"
#include "Thread/TMutex.h"
#include "TNativeFile_WASM.h"
#include "TSystemDLL_WASM.h"
#include "ToshiTools/T2DynamicVector.h"

#include <emscripten.h>
#include <emscripten/threading.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void ThreadSleep( TUINT dwMilliseconds )
{
	usleep( dwMilliseconds * 1000 );
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
	if ( !a_wszUnicodeString )
		return "";

	T2DynamicVector<TCHAR> utf8String;

	for ( const TWCHAR* pChar = a_wszUnicodeString; *pChar; pChar++ )
	{
		TUINT32 uiCode = TUINT16( *pChar );

		if ( uiCode >= 0xD800 && uiCode < 0xDC00 && pChar[ 1 ] )
		{
			uiCode = 0x10000 + ( ( uiCode - 0xD800 ) << 10 ) + ( TUINT16( pChar[ 1 ] ) - 0xDC00 );
			pChar++;
		}

		if ( uiCode < 0x80 )
		{
			utf8String.PushBack( TCHAR( uiCode ) );
		}
		else if ( uiCode < 0x800 )
		{
			utf8String.PushBack( TCHAR( 0xC0 | ( uiCode >> 6 ) ) );
			utf8String.PushBack( TCHAR( 0x80 | ( uiCode & 0x3F ) ) );
		}
		else if ( uiCode < 0x10000 )
		{
			utf8String.PushBack( TCHAR( 0xE0 | ( uiCode >> 12 ) ) );
			utf8String.PushBack( TCHAR( 0x80 | ( ( uiCode >> 6 ) & 0x3F ) ) );
			utf8String.PushBack( TCHAR( 0x80 | ( uiCode & 0x3F ) ) );
		}
		else
		{
			utf8String.PushBack( TCHAR( 0xF0 | ( uiCode >> 18 ) ) );
			utf8String.PushBack( TCHAR( 0x80 | ( ( uiCode >> 12 ) & 0x3F ) ) );
			utf8String.PushBack( TCHAR( 0x80 | ( ( uiCode >> 6 ) & 0x3F ) ) );
			utf8String.PushBack( TCHAR( 0x80 | ( uiCode & 0x3F ) ) );
		}
	}

	utf8String.PushBack( '\0' );

	return &utf8String[ 0 ];
}

TString16 Platform_UTF8ToUnicode( const TCHAR* a_pchUTF8String )
{
	if ( !a_pchUTF8String )
		return L"";

	T2DynamicVector<TWCHAR> utf16String;

	for ( const TUINT8* pChar = TREINTERPRETCAST( const TUINT8*, a_pchUTF8String ); *pChar; )
	{
		TUINT32 uiCode;
		TINT    iExtra;

		if ( *pChar < 0x80 ) { uiCode = *pChar; iExtra = 0; }
		else if ( *pChar < 0xE0 ) { uiCode = *pChar & 0x1F; iExtra = 1; }
		else if ( *pChar < 0xF0 ) { uiCode = *pChar & 0x0F; iExtra = 2; }
		else { uiCode = *pChar & 0x07; iExtra = 3; }

		pChar++;

		for ( ; iExtra > 0 && *pChar; iExtra--, pChar++ )
			uiCode = ( uiCode << 6 ) | ( *pChar & 0x3F );

		if ( uiCode >= 0x10000 )
		{
			uiCode -= 0x10000;
			utf16String.PushBack( TWCHAR( 0xD800 + ( uiCode >> 10 ) ) );
			utf16String.PushBack( TWCHAR( 0xDC00 + ( uiCode & 0x3FF ) ) );
		}
		else
		{
			utf16String.PushBack( TWCHAR( uiCode ) );
		}
	}

	utf16String.PushBack( L'\0' );

	return &utf16String[ 0 ];
}

THPTimer::THPTimer()
{
	Reset();
}

THPTimer::~THPTimer()
{
	Reset();
}

static TUINT64 GetCurrentMicroseconds()
{
	return TUINT64( emscripten_get_now() * 1000.0 );
}

TUINT32 THPTimer::GetRaw32()
{
	return TUINT32( GetCurrentMicroseconds() );
}

int64_t THPTimer::GetRaw64()
{
	return GetCurrentMicroseconds();
}

void THPTimer::Reset()
{
	m_iCurrentTime = GetCurrentMicroseconds();
	m_iFrequency   = 1000000;
	m_iOldTime     = m_iCurrentTime;
	m_fDelta       = 0;
}

void THPTimer::Update()
{
	m_iOldTime     = m_iCurrentTime;
	m_iCurrentTime = GetCurrentMicroseconds();

	double ratio      = 1.0 / m_iFrequency;
	m_fDelta          = TFLOAT( ( m_iCurrentTime - m_iOldTime ) * ratio );
	m_fCurrentSeconds = TFLOAT( m_iCurrentTime * ratio );
}

TDebugConsole::TDebugConsole()
{
	m_Visible = TFALSE;
	m_Created = TFALSE;

	m_LogListener.Connect( TUtil::GetLogEmitter(), this, TDebugConsole::OnLog, 1 );
}

TBOOL TDebugConsole::OnLog( TDebugConsole* a_pCaller, TUtil* a_pOwner, TUtil::LogEvent* a_pLogEvent )
{
	emscripten_console_log( a_pLogEvent->m_szString );
	return TTRUE;
}

TDebugConsole::~TDebugConsole()
{
}

TBOOL TDebugConsole::Show( TBOOL state )
{
	return TFALSE;
}

TBOOL TDebugConsole::Toggle()
{
	return TFALSE;
}

TSystemDLLWASM::TSystemDLLWASM()
{
	m_pDLL = TNULL;
}

TSystemDLLWASM::TSystemDLLWASM( const TSystemDLLWASM& other )
{
	m_sFileName = other.m_sFileName;
	m_pDLL      = TNULL;
}

TSystemDLLWASM::~TSystemDLLWASM()
{
}

TBOOL TSystemDLLWASM::Load( const TString8& a_sFileName )
{
	m_sFileName = a_sFileName;
	return TFALSE;
}

void TSystemDLLWASM::Unload()
{
}

void* TSystemDLLWASM::GetAddress( const TString8& a_sSymbolName )
{
	return TNULL;
}

TSystemDLLWASM& TSystemDLLWASM::operator=( const TSystemDLLWASM& other )
{
	m_sFileName = other.m_sFileName;
	m_pDLL      = TNULL;
	return *this;
}

void TUtil::TrimLog( const TCHAR* fileName, TSIZE trimTo )
{
}

const TCHAR* TUtil::GetTime()
{
	time_t t   = time( NULL );
	TCHAR* str = ctime( &t );
	return strtok( str, "\n" );
}

#ifndef TMEMORY_USE_DLMALLOC

void* TMemory::SysAlloc( TSIZE a_uiSize )
{
	void* pMem = calloc( 1, a_uiSize );
	TVALIDPTR( pMem );

	return pMem;
}

void TMemory::SysFree( void* a_pMem )
{
	TASSERT( TNULL != a_pMem );
	free( a_pMem );
}

#endif // !TMEMORY_USE_DLMALLOC

TBOOL TFileManager::Create()
{
	CreateCommon();
	auto fileManager = TFileManager::GetSingletonSafe();

	( new TNativeFileSystem( "local" ) )->SetPrefix( "" );
	( new TNativeFileSystem( "abs" ) )->SetPrefix( "" );
	new TNullFileSystem( "null" );
	new TMemFileSystem( "mem" );

	fileManager->SetSystemPath( "local" );

	return TTRUE;
}

TOSHI_NAMESPACE_END

TOSHI_NAMESPACE_USING

static TCHAR  TDebug_ScratchMem[ 4096 ];
static TBOOL  TDebug_IsMutexCreated;
static TMutex TDebug_Mutex;

void TDebug_AcquireMutex()
{
	if ( !TDebug_IsMutexCreated )
	{
		TDebug_Mutex.Create();
		TDebug_IsMutexCreated = TTRUE;
	}

	TDebug_Mutex.Lock();
}

void TDebug_ReleaseMutex()
{
	TDebug_Mutex.Unlock();
}

TBOOL TDebug_IsValidAddress( const void* a_pPtr )
{
	return a_pPtr != TNULL &&
	    a_pPtr != (void*)0xcdcdcdcd &&
	    a_pPtr != (void*)0xdeaddead &&
	    a_pPtr != (void*)0xbeefbeef &&
	    a_pPtr >= (void*)0x4f;
}

void TDebug_FinalPrintf( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );
	TDebug_FinalVPrintf( a_szFormat, args );
	va_end( args );
}

void TDebug_FinalVPrintf( const TCHAR* a_szFormat, va_list a_Args )
{
	TDebug_FinalVSPrintf( a_szFormat, a_Args );
	TDebug_OutputDebugString( TDebug_ScratchMem );
}

void TDebug_FinalSPrintf( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );
	TDebug_FinalVSPrintf( a_szFormat, args );
	va_end( args );
}

void TDebug_FinalVSPrintf( const TCHAR* a_szFormat, va_list a_Args )
{
	TDebug_AcquireMutex();
	vsnprintf( TDebug_ScratchMem, sizeof( TDebug_ScratchMem ), a_szFormat, a_Args );
	TDebug_ReleaseMutex();
}

void TDebug_OutputDebugString( const TCHAR* a_szString )
{
	emscripten_console_log( a_szString );
}

void TDebug_BroadcastDebugString( const TCHAR* a_szString )
{
}

TINT TDebug_AssertHandler( const TCHAR* a_szExpression, const TCHAR* a_szFileName, TUINT a_uiLineNumber, const TCHAR* a_szDescription )
{
	TERROR( "Assertion (%s) failed ('%s', at line %u)\n", a_szExpression, a_szFileName, a_uiLineNumber );
	return 0;
}
