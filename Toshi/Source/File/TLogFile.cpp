#include "ToshiPCH.h"
#include "TLogFile.h"
#include "File/TFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TLogFile::TLogFile()
{
	m_pFile          = TNULL;
	m_iTotalLogCount = 0;
	m_bIsSimpleMode  = TFALSE;
	m_curLevel       = 0;
	*m_LevelString   = 0;
	*m_typeCounts    = 0;
	m_unk2           = TNULL;
	m_unk3           = 0;
}

TLogFile::~TLogFile()
{
	TASSERT( TNULL == m_pFile, "TFile must be NULL" );
	Close();
}

TLogFile::Error TLogFile::Create( const TCHAR* fileName, const TCHAR* str2, TBOOL writeExisting )
{
	TASSERT( TNULL == m_pFile );

	if ( m_pFile != TNULL )
	{
		return Error::AlreadyCreated;
	}

	TIMPLEMENT_D( "uVar1 = FUN_008289A0(fileName);" );
	TIMPLEMENT_D( "FUN_00828A40(uVar1);" );

	TFILEMODE fileMode = writeExisting ?
	    TFILEMODE_NOBUFFER | TFILEMODE_WRITE :
	    TFILEMODE_NOBUFFER | TFILEMODE_WRITE | TFILEMODE_CREATENEW;

	m_pFile = TFile::Create( fileName, fileMode );

	if ( m_pFile == TNULL )
	{
		return Error::OpenFile;
	}

	if ( writeExisting )
	{
		m_pFile->Seek( 0, TSEEK_END );
	}

	m_iTotalLogCount = 0;
	*m_LevelString   = 0;
	m_curLevel       = 0;

	m_pFile->CPrintf( "Log created [%s]:[%s]: %s\n", fileName, str2, TUtil::GetTime() );
	m_pFile->CPrintf( "Compilation: %s\n", __TIMESTAMP__ );

	return Error::OK;
}

void TLogFile::Close()
{
	m_curLevel = 0;
	RecalcLevel();

	for ( int i = TUtil::LogType_Info; i < TUtil::LogType_NUMOF; i++ )
	{
		Print( "%s count = %d\n", TUtil::LogTypeToString( TUtil::LogType( i ) ), m_typeCounts[ i ] );
	}

	if ( m_pFile != TNULL )
	{
		m_pFile->Destroy();
		m_pFile = TNULL;
	}
}

void TLogFile::RecalcLevel()
{
	m_curLevel = TMath::Min<TUINT32>( m_curLevel, cLevelMax );
	m_curLevel = TMath::Max<TUINT32>( m_curLevel, 0 );

	for ( TSIZE i = 0; i < m_curLevel; i++ )
	{
		m_LevelString[ i ] = 9;
	}

	m_LevelString[ m_curLevel ] = 0;
}

void TLogFile::Print( const TCHAR* format, ... )
{
	if ( m_pFile != TNULL )
	{
		va_list args;
		va_start( args, format );

		TCHAR str[ 1024 ];
		T2String8::FormatV( str, sizeof( str ), format, args );

		va_end( args );

		if ( m_bAllowIndentation )
		{
			m_pFile->CPrintf( "%s%s", m_LevelString, str );
		}
		else
		{
			m_pFile->CPrintf( str );
		}
	}
}

void TLogFile::Log( TUtil::LogType type, const TCHAR* str1, const TCHAR* str2, const TCHAR* format, ... )
{
	if ( m_pFile != TNULL )
	{
		if ( m_bIsSimpleMode )
		{
			if ( m_bAllowIndentation )
			{
				m_pFile->CPrintf(
				    "%d [%s]: %s",
				    m_iTotalLogCount,
				    TUtil::LogTypeToString( type ),
				    m_LevelString
				);
			}
		}
		else
		{
			TCHAR strTime[ 128 ];
			_strtime( strTime );

			m_pFile->CPrintf(
			    "%d [%s] [%s]: %s: %s: %s",
			    m_iTotalLogCount,
			    TUtil::LogTypeToString( type ),
			    strTime,
			    str1,
			    str2 != TNULL ? str2 : "",
			    !m_bAllowIndentation ? m_LevelString : ""
			);
		}

		va_list args;
		va_start( args, format );
		T2String8::FormatV( TStringManager::GetTempString8(), format, args );
		m_pFile->CPrintf( TStringManager::GetTempString8() );
		va_end( args );

		m_typeCounts[ type ]++;
		m_iTotalLogCount++;
	}
}

void TLogFile::Down()
{
	m_curLevel--;
	RecalcLevel();
}

void TLogFile::Up()
{
	m_curLevel++;
	RecalcLevel();
}

TOSHI_NAMESPACE_END
