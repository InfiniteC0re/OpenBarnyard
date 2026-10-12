#include "ToshiPCH.h"
#include "TMemFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TMemFile::TMemFile( TMemFileSystem* a_pFileSystem )
    : TFile( a_pFileSystem )
{
	m_pData   = TNULL;
	m_pCursor = TNULL;
	m_uiSize  = 0;
}

// $Barnyard: FUNCTION 006dc760
TMemFile::~TMemFile()
{
	// NOTE: the buffer is released by TMemFileSystem::DestroyFile
}

// $Barnyard: FUNCTION 006dc790
TBOOL TMemFile::Open( const TString8& a_rcFileName, TFILEMODE a_eFileMode )
{
	TFile* pFile = TFileManager::GetSingleton()->CreateFile( a_rcFileName, a_eFileMode );

	if ( !pFile )
		return TFALSE;

	TBOOL bResult = TTRUE;
	m_uiSize      = pFile->GetSize();

	if ( TINT( m_uiSize ) > 0 )
	{
		m_pData   = TSTATICCAST( TUINT8, TMemalign( 64, m_uiSize, TNULL ) );
		m_pCursor = m_pData;

		if ( pFile->Read( m_pData, m_uiSize ) != m_uiSize )
		{
			TFree( m_pData );
			m_pData = TNULL;
			bResult = TFALSE;
		}
	}

	pFile->Destroy();
	return bResult;
}

// $Barnyard: FUNCTION 006dc810
TSIZE TMemFile::Read( void* a_pDst, TSIZE a_uiSize )
{
	if ( TINT( a_uiSize ) <= 0 )
		return -1;

	if ( m_pCursor + a_uiSize > m_pData + m_uiSize )
		return -1;

	TUtil::MemCopy( a_pDst, m_pCursor, a_uiSize );
	m_pCursor += a_uiSize;

	return a_uiSize;
}

// $Barnyard: FUNCTION 006dc630
TSIZE TMemFile::Write( const void* a_pSrc, TSIZE a_uiSize )
{
	// Memory files are read-only, pretend everything was written
	return a_uiSize;
}

// $Barnyard: FUNCTION 006dc850
TBOOL TMemFile::Seek( TINT a_iOffset, TSEEK a_eOrigin )
{
	// NOTE: seeking out of range moves the cursor back to the start of the file and still succeeds
	if ( a_eOrigin == TSEEK_SET )
	{
		if ( a_iOffset <= TINT( m_uiSize ) )
			m_pCursor = m_pData + a_iOffset;
		else
			m_pCursor = m_pData;
	}
	else if ( a_eOrigin == TSEEK_CUR )
	{
		if ( m_pCursor + a_iOffset <= m_pData + m_uiSize )
			m_pCursor += a_iOffset;
		else
			m_pCursor = m_pData;
	}
	else if ( a_eOrigin == TSEEK_END )
	{
		m_pCursor = m_pData + m_uiSize;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006dc6b0
TSIZE TMemFile::Tell()
{
	return TSIZE( m_pCursor - m_pData );
}

// $Barnyard: FUNCTION 006dc640
TCHAR TMemFile::GetCChar()
{
	if ( m_pCursor >= m_pData + m_uiSize )
		return -1;

	return *( m_pCursor++ );
}

// $Barnyard: FUNCTION 006dc660
TWCHAR TMemFile::GetWChar()
{
	// NOTE: not implemented in retail, always returns 0xFFFF
	return L'\xFFFF';
}

// NOTE: folded with TNativeFile::PutCChar (006dc670)
TINT TMemFile::PutCChar( TCHAR a_cCharacter )
{
	if ( Write( &a_cCharacter, 1 ) != 1 )
		return -1;

	return a_cCharacter & 0xFF;
}

// NOTE: folded with TNativeFile::PutWChar (006dc690)
TINT TMemFile::PutWChar( TWCHAR a_wcCharacter )
{
	if ( Write( &a_wcCharacter, 2 ) != 2 )
		return -1;

	return a_wcCharacter & 0xFFFF;
}

// NOTE: folded with TNativeFile::VCPrintf (006dd1b0)
TINT TMemFile::VCPrintf( const TCHAR* a_szFormat, va_list a_vargs )
{
	TCHAR str[ 512 ];

	TINT iResult = T2String8::FormatV( str, TARRAYSIZE( str ), a_szFormat, a_vargs );

	if ( iResult < 0 )
		iResult = TARRAYSIZE( str );

	return Write( str, iResult );
}

// NOTE: folded with TNativeFile::VWPrintf (006dd200)
TINT TMemFile::VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs )
{
	TWCHAR str[ 512 ];

	TINT iResult = T2String16::FormatV( str, TARRAYSIZE( str ), a_wszFormat, a_vargs );

	if ( iResult < 0 )
		iResult = TARRAYSIZE( str );

	TINT iWritten = TINT( Write( str, iResult * sizeof( TWCHAR ) ) );

	if ( iWritten >= 0 )
		iWritten /= 2;

	return iWritten;
}

// $Barnyard: FUNCTION 006dc6c0
TMemFileSystem::TMemFileSystem( const TCHAR* a_szName )
    : TFileSystem( a_szName )
{
	TFileManager::GetSingleton()->MountFileSystem( this );
	m_pLocalFileSystem = TFileManager::GetSingleton()->FindFileSystem( "local" );
}

// $Barnyard: FUNCTION 006dc8b0
TMemFileSystem::~TMemFileSystem()
{
	UnmountFileSystem();
}

// $Barnyard: FUNCTION 006dc8e0
TFile* TMemFileSystem::CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode )
{
	TMemFile* pFile = new TMemFile( this );

	if ( !pFile->Open( a_rcFileName, a_eFileMode ) )
	{
		delete pFile;
		return TNULL;
	}

	return pFile;
}

// $Barnyard: FUNCTION 006dc720
void TMemFileSystem::DestroyFile( TFile* a_pFile )
{
	if ( a_pFile )
	{
		TMemFile* pMemFile = TSTATICCAST( TMemFile, a_pFile );

		if ( pMemFile->m_pData )
		{
			TFree( pMemFile->m_pData );
			pMemFile->m_pData = TNULL;
		}

		pMemFile->m_uiSize  = 0;
		pMemFile->m_pCursor = TNULL;

		delete pMemFile;
	}
}

// $Barnyard: FUNCTION 006dc5f0
TString8 TMemFileSystem::MakeInternalPath( const TString8& a_rcPath )
{
	return m_pLocalFileSystem->MakeInternalPath( a_rcPath );
}

TOSHI_NAMESPACE_END
