#include "ToshiPCH.h"
#include "TNullFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TNullFile::TNullFile( TNullFileSystem* a_pFileSystem )
    : TFile( a_pFileSystem )
{
	m_iBytesWritten = 0;
}

// $Barnyard: FUNCTION 006dca80
TNullFile::~TNullFile()
{
}

// $Barnyard: FUNCTION 006dc990
TSIZE TNullFile::Read( void* a_pDst, TSIZE a_uiSize )
{
	return -1;
}

// $Barnyard: FUNCTION 006dc9a0
TSIZE TNullFile::Write( const void* a_pSrc, TSIZE a_uiSize )
{
	m_iBytesWritten += a_uiSize;
	return a_uiSize;
}

// $Barnyard: FUNCTION 006dc9b0
TCHAR TNullFile::GetCChar()
{
	TCHAR cCharacter;
	if ( Read( &cCharacter, 1 ) != 1 )
		return -1;

	return cCharacter;
}

// $Barnyard: FUNCTION 006dc9d0
TWCHAR TNullFile::GetWChar()
{
	TWCHAR wcCharacter;
	if ( Read( &wcCharacter, 2 ) != 2 )
		return L'\xFFFF';

	return wcCharacter;
}

// NOTE: folded with TNativeFile::PutCChar (006dc670)
TINT TNullFile::PutCChar( TCHAR a_cCharacter )
{
	if ( Write( &a_cCharacter, 1 ) != 1 )
		return -1;

	return a_cCharacter & 0xFF;
}

// NOTE: folded with TNativeFile::PutWChar (006dc690)
TINT TNullFile::PutWChar( TWCHAR a_wcCharacter )
{
	if ( Write( &a_wcCharacter, 2 ) != 2 )
		return -1;

	return a_wcCharacter & 0xFFFF;
}

// NOTE: folded with TNativeFile::VCPrintf (006dd1b0)
TINT TNullFile::VCPrintf( const TCHAR* a_szFormat, va_list a_vargs )
{
	TCHAR str[ 512 ];

	TINT iResult = T2String8::FormatV( str, TARRAYSIZE( str ), a_szFormat, a_vargs );

	if ( iResult < 0 )
		iResult = TARRAYSIZE( str );

	return Write( str, iResult );
}

// NOTE: folded with TNativeFile::VWPrintf (006dd200)
TINT TNullFile::VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs )
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

// $Barnyard: FUNCTION 006dc9f0
TNullFileSystem::TNullFileSystem( const TCHAR* a_szName )
    : TFileSystem( a_szName )
{
	TFileManager::GetSingleton()->MountFileSystem( this );
	s_pFileSystem = this;
}

// $Barnyard: FUNCTION 006dcab0
TNullFileSystem::~TNullFileSystem()
{
	UnmountFileSystem();
	s_pFileSystem = TNULL;
}

// $Barnyard: FUNCTION 006dca20
TFile* TNullFileSystem::CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode )
{
	return new TNullFile( this );
}

// $Barnyard: FUNCTION 006dca60
void TNullFileSystem::DestroyFile( TFile* a_pFile )
{
	if ( a_pFile )
		delete a_pFile;
}

// $Barnyard: FUNCTION 006dc950
TString8 TNullFileSystem::MakeInternalPath( const TString8& a_rcPath )
{
	return TString8();
}

TOSHI_NAMESPACE_END
