#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIStringTable.h"
#  include "XURData.h"

#  include <File/TFile.h>
#  include <Toshi/T2String8.h>
#  include <Toshi/T2String16.h>
#  include <Toshi/TStringManager.h>
#  include <ToshiTools/T2DynamicVector.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Name of the TRB symbol that holds the string table (a global buffer in the original)
static TCHAR s_szSymbolName[ 128 ];

// $BarnyardWiiJP: FUNCTION 8041d210
AGUI2XUIStringTable::AGUI2XUIStringTable()
{
	m_uiMagic      = 0;
	m_ucUnk1       = 0;
	m_ucUnk2       = 0;
	m_uiSize       = 0;
	m_usNumStrings = 0;
	m_pEntries     = TNULL;
}

// $BarnyardWiiJP: FUNCTION 8041d230
AGUI2XUIStringTable::~AGUI2XUIStringTable()
{
	for ( TINT i = 0; i < m_usNumStrings; i++ )
	{
		delete[] m_pEntries[ i ].wszId;
		delete[] m_pEntries[ i ].wszString;
	}

	delete[] m_pEntries;
}

// $BarnyardWiiJP: FUNCTION 8041d2d4
TBOOL AGUI2XUIStringTable::Load( const TCHAR* a_szFileName, TBOOL a_bIsTRB, TTRB* a_pTRB )
{
	if ( !T2String8::FindString( a_szFileName, a_bIsTRB ? ".trb" : ".xus" ) )
		return TFALSE;

	for ( TINT i = 0; i < m_usNumStrings; i++ )
	{
		delete[] m_pEntries[ i ].wszId;
		delete[] m_pEntries[ i ].wszString;
	}

	delete[] m_pEntries;

	TTRB    oTRB;
	TUINT8* pData = TNULL;

	if ( a_bIsTRB )
	{
		// Get the file name without the path and the extension
		TINT iNameStart = 0;
		TINT iLength    = T2String8::Length( a_szFileName );

		for ( TINT i = iLength - 1; i >= 0; i-- )
		{
			if ( a_szFileName[ i ] == '/' || a_szFileName[ i ] == '\\' )
			{
				iNameStart = i + 1;
				break;
			}
		}

		TINT iNameLength = iLength - iNameStart - 4;

		// <name>_<name>
		T2String8::Copy( s_szSymbolName, a_szFileName + iNameStart, iNameLength );
		s_szSymbolName[ iNameLength ]     = '_';
		s_szSymbolName[ iNameLength + 1 ] = '\0';
		T2String8::Copy( &s_szSymbolName[ iNameLength + 1 ], a_szFileName + iNameStart, iNameLength );
		s_szSymbolName[ 2 * iNameLength + 1 ] = '\0';
		TStringManager::String8ToLowerCase( s_szSymbolName );

		if ( a_pTRB )
			pData = TSTATICCAST( TUINT8, a_pTRB->GetSymbolAddress( s_szSymbolName ) );

		if ( !pData )
		{
			// <name>
			T2String8::Copy( s_szSymbolName, &s_szSymbolName[ iNameLength + 1 ], iNameLength + 4 );
			s_szSymbolName[ iNameLength + 4 ] = '\0';

			if ( oTRB.Load( a_szFileName ) != TTRB::ERROR_OK )
				return TFALSE;

			pData = TSTATICCAST( TUINT8, oTRB.GetSymbolAddress( s_szSymbolName ) );

			if ( !pData )
				return TFALSE;
		}
	}

	if ( !pData )
	{
		TFile* pFile = TFile::Create( TString8( a_szFileName ), TFILEMODE_READ );

		if ( !pFile )
			return TFALSE;

		TSIZE uiFileSize = pFile->GetSize();
		pData            = TSTATICCAST( TUINT8, TMalloc( uiFileSize ) );
		pFile->Read( pData, uiFileSize );
		pFile->Destroy();
	}

	if ( !ReadHeader( pData ) )
		return TFALSE;

	ReadStrings( pData + 12 );

	if ( !a_bIsTRB )
		TFree( pData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041d5d8
TBOOL AGUI2XUIStringTable::ReadHeader( TUINT8* a_pData )
{
	m_uiMagic      = XURReader::PeekUInt32( a_pData );
	m_ucUnk1       = a_pData[ 4 ];
	m_ucUnk2       = a_pData[ 5 ];
	m_uiSize       = XURReader::PeekUInt32( a_pData + 6 );
	m_usNumStrings = XURReader::PeekUInt16( a_pData + 10 );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041d648
TBOOL AGUI2XUIStringTable::ReadStrings( TUINT8* a_pData )
{
	T2DynamicVector<TWCHAR*> vecStrings( GetGlobalAllocator(), 64, 64 );
	TWCHAR                   wszBuffer[ 272 ];

	// Each entry is the string followed by the id
	for ( TUINT32 i = 0; i < TUINT32( m_usNumStrings * 2 ); i++ )
	{
		TUINT16 usLength = XURReader::ReadUInt16( a_pData );

		for ( TUINT16 k = 0; k < usLength; k++ )
			wszBuffer[ k ] = XURReader::ReadUInt16( a_pData );

		wszBuffer[ usLength ] = L'\0';

		TWCHAR* wszString = new TWCHAR[ usLength + 1 ];
		T2String16::Copy( wszString, wszBuffer, usLength + 1 );
		vecStrings.PushBack( wszString );
	}

	m_pEntries = new Entry[ m_usNumStrings ];
	TUtil::MemCopy( m_pEntries, vecStrings.Data(), m_usNumStrings * sizeof( Entry ) );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041d864
const TWCHAR* AGUI2XUIStringTable::Lookup( const TWCHAR* a_wszId )
{
	for ( TINT i = 0; i < m_usNumStrings; i++ )
	{
		if ( T2String16::Compare( a_wszId, m_pEntries[ i ].wszId ) == 0 )
			return m_pEntries[ i ].wszString;
	}

	return TNULL;
}

#endif // BARNYARD_LATEST
