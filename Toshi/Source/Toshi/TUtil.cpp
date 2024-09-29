#include "ToshiPCH.h"
#include "TUtil.h"
#include "Toshi/TSystem.h"
#include "Toshi/TError.h"
#include "Toshi/T2String.h"
#include "File/TLogFile.h"
#include "Render/TModelManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TUtil::LogInitialise()
{
#ifndef TOSHI_NO_LOGS
	TLogFile* logfile                        = new TLogFile;
	TUtil::GetSingleton()->m_pDefaultLogFile = logfile;
	TUtil::GetSingleton()->m_pCurrentLogFile = logfile;

	TrimLog( "*.log", 9 );

	time_t seconds;
	time( &seconds );
	tm* time = gmtime( &seconds );

	T2FormatString256 filename;

	filename.Format(
	    "Logs\\%s_%d%02d%02d_%02d_%02d_%02d.log",
	    ms_oToshiParams.szLogFileName,
	    time->tm_year + 1900,
	    time->tm_mon + 1,
	    time->tm_mday,
	    time->tm_hour,
	    time->tm_min,
	    time->tm_sec
	);

	GetCurrentLogFile()->Create( filename.Get(), "Toshi 2.0", TFALSE );
	GetCurrentLogFile()->AllowIndentation( TTRUE );
	GetCurrentLogFile()->SetSimpleMode( TTRUE );
#endif // TOSHI_NO_LOGS
}

void TUtil::CreateTPStringPool()
{
	TASSERT( ms_poStringPool == TNULL );
	ms_poStringPool  = new TPString8Pool*;
	*ms_poStringPool = TNULL;
}

void TUtil::DestroyTPStringPool()
{
	if ( ms_poStringPool )
	{
		delete ms_poStringPool;
		ms_poStringPool = TNULL;
	}
}

void TUtil::Log( const TCHAR* a_szFormat, ... )
{
	if ( IsSingletonCreated() && GetCurrentLogFile() )
	{
		auto pLogFile = GetCurrentLogFile();

		T2FormatString2048 formatString;

		va_list args;
		va_start( args, a_szFormat );
		formatString.FormatV( a_szFormat, args );
		va_end( args );

		pLogFile->Log( LogType_Info, ms_oToshiParams.szLogAppName, ms_oToshiParams.szLogAppDirName, formatString.Get() );
		GetLogEmitter().Throw( LogEvent( pLogFile, LogType_Info, formatString.Get() ) );
	}
}

void TUtil::Log( LogType a_eLogType, const TCHAR* a_szFormat, ... )
{
	auto pLogFile = GetCurrentLogFile();

	if ( pLogFile )
	{
		T2FormatString2048 formatString;

		va_list args;
		va_start( args, a_szFormat );
		formatString.FormatV( a_szFormat, args );
		va_end( args );

		pLogFile->Log( a_eLogType, ms_oToshiParams.szLogAppName, ms_oToshiParams.szLogAppDirName, formatString.Get() );
		GetLogEmitter().Throw( LogEvent( pLogFile, a_eLogType, formatString.Get() ) );
	}
}

void TUtil::LogDown()
{
	TUtil::GetSingletonSafe()->m_pCurrentLogFile->Down();
}

void TUtil::LogUp()
{
	TUtil::GetSingletonSafe()->m_pCurrentLogFile->Up();
}

void TUtil::LogConsole( const TCHAR* a_szFormat, ... )
{
	auto pLogFile = GetCurrentLogFile();

	if ( pLogFile )
	{
		T2FormatString2048 formatString;

		va_list args;
		va_start( args, a_szFormat );
		formatString.FormatV( a_szFormat, args );
		va_end( args );

		OutputDebugStringA( formatString.Get() );
		printf( formatString.Get() );
	}
}

void TUtil::LogSet( TLogFile* a_pLogFile )
{
	Log( "Changing log file." );
	TUtil* pUtil = TUtil::GetSingleton();

	if ( pUtil->m_pCurrentLogFile )
	{
		pUtil->m_pCurrentLogFile->Close();
	}

	pUtil->m_pCurrentLogFile = ( !a_pLogFile ) ? pUtil->m_pDefaultLogFile : a_pLogFile;
}

TUtil::TUtil()
    : m_LogEmitter( this )
{
	m_pDefaultLogFile = TNULL;
	m_pCurrentLogFile = TNULL;
}

TBOOL TUtil::ToshiCreate( const TOSHIParams& a_rToshiParams )
{
	ms_oToshiParams = a_rToshiParams;

	TSystemManager::Create();
	CreateKernelInterface();
	TFileManager::Create();
	CreateTPStringPool();
	Create();

	return TTRUE;
}

void TUtil::CreateKernelInterface()
{
	TKernelInterface::CreateSingleton();
}

void TUtil::ToshiDestroy()
{
	TIMPLEMENT();
#ifdef TOSHI_SKU_WINDOWS
	ReleaseMutex( ms_hGlobalMutex );
#endif

	TModelManager::Uninitialise();
}

void TUtil::Create()
{
	TUtil::CreateSingleton()->LogInitialise();
	TUtil::CRCInitialise();
}

// Source: https://lentz.com.au/blog/tag/crc-table-generator
void TUtil::CRCInitialise()
{
	TUINT32 crc;

	for ( TINT i = 0; i < CRC_TABSIZE; i++ )
	{
		crc = i;
		for ( TINT j = 8; j > 0; j-- )
		{
			if ( crc & 1 )
				crc = ( crc >> 1 ) ^ CRC32POLY;
			else
				crc >>= 1;
		}
		s_aiCRC32LUT[ i ] = crc;
	}
}

// Source: https://lentz.com.au/blog/tag/crc-table-generator
TUINT32 TUtil::CRC32( TBYTE* buffer, TUINT32 len )
{
	TUINT32 crc = 0;

	while ( len-- )
		crc = crc32upd( s_aiCRC32LUT, crc, *buffer++ );

	return CRC32POST( crc );
}

TOSHI_NAMESPACE_END
