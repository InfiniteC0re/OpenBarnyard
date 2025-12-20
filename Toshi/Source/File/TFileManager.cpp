/**
 * @file TFileManager.cpp
 * File system manager implementation for the Toshi engine
 * 
 * This file implements the core file system manager that provides:
 * - File system mounting and unmounting
 * - File creation and management
 * - Path resolution and validation
 * - System path handling
 */

#include "ToshiPCH.h"
#include "TFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TFileManager::TFileManager()
    : m_WorkingDirectory( "/" ), m_ValidatedCount( 0 )
{
	InvalidateSystemPath();
}

TFileManager::~TFileManager()
{
	Destroy();
}

// $Barnyard: FUNCTION 006d9230
void TFileManager::Destroy()
{
	{
		auto pFileSystem = TFileManager::FindFileSystem( "mem" );
		if ( pFileSystem ) delete pFileSystem;
	}

	{
		auto pFileSystem = TFileManager::FindFileSystem( "null" );
		if ( pFileSystem ) delete pFileSystem;
	}

	{
		auto pFileSystem = TFileManager::FindFileSystem( "abs" );
		if ( pFileSystem ) delete pFileSystem;
	}

	{
		auto pFileSystem = TFileManager::FindFileSystem( "local" );
		if ( pFileSystem ) delete pFileSystem;
	}

	DestroyCommon();
}

// $Barnyard: FUNCTION 006be290
void TFileManager::MountFileSystem( TFileSystem* a_pFileSystem )
{
	TASSERT( TFileManager::FindFileSystem( a_pFileSystem->GetName() ) == TNULL );
	TASSERT( !a_pFileSystem->IsLinked() );

	m_Invalidated.InsertTail( a_pFileSystem );
	InvalidateSystemPath();
}

// $Barnyard: FUNCTION 006be260
TFileSystem* TFileManager::FindFileSystem( const TString8& name )
{
	TFileSystem* pFileSystem = TFileManager::FindFileSystem( m_Validated, name );

	if ( pFileSystem == TNULL )
	{
		pFileSystem = TFileManager::FindFileSystem( m_Invalidated, name );
	}

	return pFileSystem;
}

// $Barnyard: FUNCTION 006be200
TFileSystem* TFileManager::FindFileSystem( TDList<TFileSystem>& a_rList, const TString8& name )
{
	T2_FOREACH( a_rList, pFileSystem )
	{
		if ( pFileSystem->GetName() == name )
		{
			return pFileSystem;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006be4b0
TFile* TFileManager::CreateFile( const TString8& a_sName, TFILEMODE flags )
{
	TPROFILER_SCOPE();
	TASSERT( a_sName.Length() > 0, "Name can't be empty" );

	ValidateSystemPath();
	TINT pos = a_sName.Find( ':', 0 );

	if ( pos >= 0 )
	{
		TString8 fsName;
		TString8 fileName;

		fsName.Copy( a_sName, pos );
		fileName.Copy( a_sName.GetString( pos + 1 ) );

		TFileSystem* pFileSystem = FindFileSystem( fsName );

		if ( pFileSystem != TNULL )
		{
			return pFileSystem->CreateFile( fileName, flags );
		}
	}

	T2_FOREACH( m_Validated, pFileSystem )
	{
		TFile* pFile = pFileSystem->CreateFile( a_sName, flags );

		if ( pFile != TNULL )
		{
			return pFile;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006be5c0
TBOOL TFileManager::GetFileInfo( const TString8& a_strPath, TNativeFileInfo& a_rNativeInfo )
{
	TASSERT( a_strPath.Length() > 0, "Path can't be empty" );

	ValidateSystemPath();
	TINT pos = a_strPath.Find( ':', 0 );

	if ( pos >= 0 )
	{
		TString8 fsName;
		TString8 fileName;

		fsName.Copy( a_strPath, pos );
		fileName.Copy( a_strPath.GetString( pos + 1 ) );

		TFileSystem* pFileSystem = FindFileSystem( fsName );

		if ( pFileSystem != TNULL )
		{
			return pFileSystem->ToNative( fileName, a_rNativeInfo );
		}
	}

	// If there are few validated file systems or even zero, create a file to know what filesystem we should use
	if ( m_ValidatedCount != 1 )
	{
		TFile* pFile = CreateFile( a_strPath, TFILEMODE_READ | TFILEMODE_NOBUFFER );

		if ( pFile != TNULL )
		{
			TFileSystem* pFileSystem = pFile->GetFileSystem();
			pFileSystem->DestroyFile( pFile );

			return pFileSystem->ToNative( a_strPath, a_rNativeInfo );
		}

		return TFALSE;
	}

	TFileSystem* pFileSystem = m_Validated.Begin();
	return pFileSystem->ToNative( a_strPath, a_rNativeInfo );
}

// $Barnyard: FUNCTION 006be370
void TFileManager::ValidateSystemPath()
{
	if ( !m_IsValidated )
	{
		// Move all file systems to the invalidated list
		for ( auto pFS = m_Validated.Tail(); pFS != m_Validated.End(); pFS = m_Validated.Tail() )
		{
			pFS->Remove();
			m_Invalidated.InsertHead( pFS );
		}

		m_ValidatedCount = 0;

		TString8     fsName;
		TSysPathIter pathIter( m_SysPath );

		TBOOL hasPath = pathIter.First( fsName );

		while ( hasPath )
		{
			TFileSystem* pFS = FindFileSystem( m_Invalidated, fsName );

			if ( pFS != TNULL )
			{
				pFS->Remove();
				m_Validated.InsertTail( pFS );
				m_ValidatedCount += 1;
			}

			hasPath = pathIter.Next( fsName );
		}

		m_IsValidated = TTRUE;
	}
}

TFileManager::TSysPathIter::TSysPathIter( const TString8& str )
    : m_String( str ), m_Position( -1 )
{
}

TFileManager::TSysPathIter::TSysPathIter( const TSysPathIter& other )
    : m_String( other.m_String ), m_Position( other.m_Position )
{
}

TBOOL TFileManager::TSysPathIter::First( TString8& path )
{
	if ( m_String.Length() > 0 )
	{
		m_Position = m_String.Find( ';', 0 );
		path.Copy( m_String, m_Position );

		return TTRUE;
	}
	else
	{
		m_Position = -1;
		return TFALSE;
	}
}

TBOOL TFileManager::TSysPathIter::Next( TString8& path )
{
	if ( m_Position >= 0 )
	{
		TINT strStart = m_Position + 1;
		m_Position    = m_String.Find( ';', strStart );

		path.Copy(
		    m_String.GetString( strStart ),
		    ( m_Position >= 0 ) ? ( m_Position - strStart ) : -1
		);

		return TTRUE;
	}
	else
	{
		return TFALSE;
	}
}

// $Barnyard: FUNCTION 006be1c0
TString8 TFileManager::MakeAbsolutePath( const TString8& a_rcRelativePath ) const
{
	return TFile::ConcatPath( m_WorkingDirectory, a_rcRelativePath );
}

// $Barnyard: FUNCTION 006be140
void TFileManager::CreateCommon()
{
	TFileManager::CreateSingleton();
	ms_oMutex.Create();
}

// $Barnyard: FUNCTION 006be320
void TFileManager::DestroyCommon()
{
	ms_oMutex.Destroy();
	delete TFileManager::GetSingletonSafe();
}

TOSHI_NAMESPACE_END
