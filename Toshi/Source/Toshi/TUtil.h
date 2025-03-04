#pragma once
#include "Toshi/TSingleton.h"
#include "Toshi/TEvent.h"
#include "Thread/TMutex.h"

TOSHI_NAMESPACE_START

class TPString8Pool;
class TLogFile;

class TUtil : public TSingleton<TUtil>
{
public:
	//-----------------------------------------------------------------------------
	// Logging
	//-----------------------------------------------------------------------------

	enum LogType
	{
		LogType_Info,
		LogType_Warning,
		LogType_Error,
		LogType_Critical,
		LogType_NUMOF
	};

	static constexpr const TCHAR* kTypeStrings[] = {
		"Info",
		"Warning",
		"Error",
		"Critical"
	};

	static constexpr const TCHAR* LogTypeToString( LogType a_eType )
	{
		TSTATICASSERT( Toshi::TUtil::LogType_NUMOF == TARRAYSIZE( kTypeStrings ) );

		return a_eType >= LogType_Info && a_eType < LogType_NUMOF ?
		    kTypeStrings[ a_eType ] :
		    "UNKNOWN";
	}

	struct LogEvent
	{
		constexpr LogEvent( TLogFile* a_pFile, LogType a_eType, const TCHAR* a_szString )
		    : m_pFile( a_pFile ), m_eType( a_eType ), m_szString( a_szString )
		{}

		TLogFile*    m_pFile;
		LogType      m_eType;
		const TCHAR* m_szString;
	};

	// Note: this wasn't in the original engine
	struct TOSHIParams
	{
		TOSHIParams() noexcept {}

		const TCHAR* szCommandLine   = "";
		const TCHAR* szLogFileName   = "toshi";
		const TCHAR* szLogAppName    = "Toshi";
		const TCHAR* szLogAppDirName = "Kernel";
		TBOOL        bLogToConsole   = TTRUE;
		TINT         iUnused1        = 0;
		TINT         iUnused2        = 0;
	};

public:
	TUtil();
	~TUtil() = default;

private:
	TLogFile*                 m_pDefaultLogFile;
	TLogFile*                 m_pCurrentLogFile;
	TEmitter<TUtil, LogEvent> m_LogEmitter;

public:
	//-----------------------------------------------------------------------------
	// Static members
	//-----------------------------------------------------------------------------

	static TBOOL ToshiCreate( const TOSHIParams& a_rToshiParams );
	static void  ToshiDestroy();

	static const TCHAR* GetTime();

	static void  MemSet( void* ptr, TINT value, TSIZE size ) { std::memset( ptr, value, size ); }
	static void* MemCopy( void* dst, const void* src, TSIZE size ) { return std::memcpy( dst, src, size ); }
	static void  MemClear( void* ptr, TSIZE size ) { std::memset( ptr, 0, size ); }
	static TINT  MemCompare( const void* ptr1, const void* ptr2, TSIZE size ) { return std::memcmp( ptr1, ptr2, size ); }
	static void* MemMove( void* dst, const void* src, TSIZE size ) { return std::memmove( dst, src, size ); }

	template <class T>
	static void Fill( T* a_pStart, T* a_pEnd, const T& a_rcValue = T() )
	{
		for ( ; a_pStart != a_pEnd; a_pStart++ )
			*a_pStart = a_rcValue;
	}

	//-----------------------------------------------------------------------------
	// Logging
	//-----------------------------------------------------------------------------

	static void Log( const TCHAR* a_szFormat, ... );
	static void Log( LogType a_eLogType, const TCHAR* a_szFormat, ... );
	static void TrimLog( const TCHAR* fileExtension, TSIZE trimTo );

	static void LogDown();
	static void LogUp();

	static void LogConsole( const TCHAR* a_szFormat, ... );
	static void LogSet( TLogFile* a_logFile );

	static TLogFile*                         GetCurrentLogFile() { return ms_pSingleton->m_pCurrentLogFile; }
	static TEmitter<TUtil, TUtil::LogEvent>& GetLogEmitter() { return ms_pSingleton->m_LogEmitter; }

	static TMutex& GetGlobalMutex() { return ms_oGlobalMutex; }

	static TPString8Pool* SetTPStringPool( TPString8Pool* a_pStringPool ) { return std::exchange( *ms_poStringPool, a_pStringPool ); }

	static TPString8Pool* GetTPStringPool()
	{
		TASSERT( ms_poStringPool != TNULL );
		return *ms_poStringPool;
	}

	static const TOSHIParams& GetToshiParameters()
	{
		return ms_oToshiParams;
	}

private:
	static void Create();
	static void CreateKernelInterface();

	static void LogInitialise();
	static void CreateTPStringPool();
	static void DestroyTPStringPool();

private:
	inline static TPString8Pool** ms_poStringPool;
	inline static TMutex          ms_oGlobalMutex;
	inline static TOSHIParams     ms_oToshiParams;

public:
#pragma region CRC

// Source: https://lentz.com.au/blog/tag/crc-table-generator

/*
			LICENCE

			This package may be freely distributed provided the files remain together,
			in their original unmodified form.
			All files, executables and sourcecode remain the copyrighted property of
			Arjen G. Lentz and LENTZ SOFTWARE-DEVELOPMENT.
			Licence for any use granted, provided this notice & CRC.DOC are included.
			For executable applications, credit should be given in the appropriate
			places in the program and documentation.
			These notices must be retained in any copies of any part of this
			documentation and/or software.

			Any use of, or operation on (including copying/distributing) any of
			the above mentioned files implies full and unconditional acceptance of
			this licence and disclaimer.
		*/

/* ------------------------------------------------------------------------- */
/* CRC-32 CCITT                                                              */
/* ------------------------------------------------------------------------- */
#define CRC32POLY        ( 0xEDB88320L ) /* Generator polynomial number       */
#define CRC32POST( crc ) ( ~( crc ) )    /* CRC Postconditioning before xmit  */

#define crc32upd( crctab, crc, c ) \
	( ( crctab )[ ( (TINT)( crc ) ^ ( c ) ) & 0xff ] ^ ( ( crc ) >> 8 ) )

#define CRC_TABSIZE ( 256 ) /* Normal 256-entry table            */

	//=============================================================================

	inline static TUINT32 s_aiCRC32LUT[ CRC_TABSIZE ] = {};

	static void    CRCInitialise();
	static TUINT32 CRC32( TBYTE* buffer, TUINT32 len );

#pragma endregion
};

TOSHI_NAMESPACE_END
