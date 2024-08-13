#include "ToshiPCH.h"
#include "TConsoleFile_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

class TConsoleFile : public TFile
{
public:
	TConsoleFile( TFileSystem* a_pFileSystem ) :
		TFile( a_pFileSystem )
	{
	}

	virtual TUINT Read( void* a_pDst, TUINT a_uiSize ) override
	{
		TASSERT( !"Console doesn't support this" );
		return 0;
	}

	virtual TUINT Write( const void* a_pSrc, TUINT a_uiSize ) override
	{
		TASSERT( !"Use Printf or VCPrintf" );
		return 0;
	}

	virtual TBOOL Seek( TINT a_iOffset, TSEEK a_eOrigin = TSEEK_CUR ) override
	{
		TASSERT( !"Console doesn't support this" );
		return 0;
	}

	virtual TUINT Tell() override
	{
		TASSERT( !"Console doesn't support this" );
		return 0;
	}

	virtual TUINT GetSize() override
	{
		TASSERT( !"Console doesn't support this" );
		return 0;
	}

	virtual TCHAR GetCChar() override
	{
		TASSERT( !"Console doesn't support this" );
		return '\0';
	}

	virtual TWCHAR GetWChar() override
	{
		TASSERT( !"Console doesn't support this" );
		return L'\0';
	}

	virtual TINT PutCChar( TCHAR a_cCharacter ) override
	{
		putchar( a_cCharacter );
		return a_cCharacter & 0xFF;
	}

	virtual TINT PutWChar( TWCHAR a_wcCharacter ) override
	{
		TASSERT( !"Console doesn't support this" );
		return a_wcCharacter & 0xFFFF;
	}

	virtual TINT CPrintf( const TCHAR* a_szFormat, ... ) override
	{
		va_list args;
		va_start( args, a_szFormat );

		TCHAR str[ 2048 ];
		T2String8::FormatV( str, sizeof( str ), a_szFormat, args );
		TINT iResult = printf( str );

		va_end( args );

		return iResult;
	}

	virtual TINT WPrintf( const TWCHAR* a_wszFormat, ... ) override
	{
		TASSERT( !"Console doesn't support this" );
		return 0;
	}

	virtual TINT VCPrintf( const TCHAR* a_szFormat, va_list a_vargs ) override
	{
		TCHAR str[ 2048 ];
		T2String8::FormatV( str, sizeof( str ), a_szFormat, a_vargs );
		TINT iResult = printf( str );

		return iResult;
	}

	virtual TINT VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs ) override
	{
		TASSERT( !"Console doesn't support this" );
		return 0;
	}
};

TConsoleFileSystem::TConsoleFileSystem( const TCHAR* a_szName ) :
	TFileSystem( a_szName )
{
	TFileManager::GetSingletonSafe()->MountFileSystem( this );
}

TFile* TConsoleFileSystem::CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode )
{
	return new TConsoleFile( this );
}

void TConsoleFileSystem::DestroyFile( TFile* a_pFile )
{
	if ( a_pFile != TNULL )
	{
		delete a_pFile;
	}
}

TString8 TConsoleFileSystem::MakeInternalPath( const TString8& a_rcPath )
{
	return TString8();
}

TOSHI_NAMESPACE_END
