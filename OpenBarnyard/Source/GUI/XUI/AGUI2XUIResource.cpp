#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIResource.h"
#  include "AGUI2XUIScene.h"
#  include "AGUI2XUIVisual.h"

#  include <File/TFile.h>
#  include <Toshi/T2String8.h>
#  include <Toshi/T2String16.h>
#  include <Toshi/TStringManager.h>
#  include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $BarnyardWiiJP: FUNCTION 8040f3a8
AGUI2XUIResource::AGUI2XUIResource()
    : m_vecUnk1( GetGlobalAllocator(), 64, 64 )
    , m_vecUnk2( GetGlobalAllocator(), 64, 64 )
    , m_vecUnk3( GetGlobalAllocator(), 64, 64 )
{
	m_apStrings    = TNULL;
	m_pVectors     = TNULL;
	m_pQuats       = TNULL;
	m_pCustData    = TNULL;
	m_pRoot        = TNULL;
	m_iNumStrings  = 0;
	m_uiNumVectors = 0;
	m_uiNumQuats   = 0;
	// NOTE: m_uiCustSize isn't initialised by the original
	m_pStringTable    = TNULL;
	m_oIDStack.m_iTop = 0;
}

// $BarnyardWiiJP: FUNCTION 8040f464
AGUI2XUIResource::~AGUI2XUIResource()
{
	for ( TINT i = 0; i < m_iNumStrings; i++ )
		delete[] m_apStrings[ i ];

	delete[] m_apStrings;
	delete[] m_pVectors;
	delete[] m_pQuats;

	if ( m_pRoot )
		delete m_pRoot;

	delete m_pStringTable;
	delete[] m_pCustData;
}

// $BarnyardWiiJP: FUNCTION 8040f60c
XURXUIElementData* AGUI2XUIResource::FindScene( const TWCHAR* a_wszName )
{
	for ( TUINT32 i = 0; i < m_pRoot->GetNumChildren(); i++ )
	{
		XURXUIElementData* pData = TSTATICCAST( XURXUIElementData, m_pRoot->GetChild( i ) );

		if ( pData->GetUIClass() == &TGetClass( AGUI2XUIScene ) && T2String16::Compare( m_apStrings[ pData->m_usId ], a_wszName ) == 0 )
			return pData;
	}

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 8040f6b0
XURXUIElementData* AGUI2XUIResource::FindScene( TUINT32 a_uiIndex )
{
	if ( a_uiIndex < m_pRoot->GetNumChildren() )
		return TSTATICCAST( XURXUIElementData, m_pRoot->GetChild( a_uiIndex ) );

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 8040f6d8
XURXUIElementData* AGUI2XUIResource::FindVisual( const TWCHAR* a_wszName )
{
	for ( TUINT32 i = 0; i < m_pRoot->GetNumChildren(); i++ )
	{
		XURXUIElementData* pData = TSTATICCAST( XURXUIElementData, m_pRoot->GetChild( i ) );

		if ( pData->GetUIClass() == &TGetClass( AGUI2XUIVisual ) && T2String16::Compare( m_apStrings[ pData->m_usId ], a_wszName ) == 0 )
			return pData;
	}

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 8040f77c
TBOOL AGUI2XUIResource::Load( const TCHAR* a_szFileName, const TCHAR* a_szStringTableFileName, TBOOL a_bIsTRB, TTRB* a_pTRB )
{
	// NOTE: the pointers aren't reset after this, so a failed load leaves them dangling
	for ( TINT i = 0; i < m_iNumStrings; i++ )
		delete[] m_apStrings[ i ];

	delete[] m_apStrings;
	delete[] m_pVectors;
	delete[] m_pQuats;

	if ( m_pRoot )
		delete m_pRoot;

	if ( a_szStringTableFileName )
	{
		m_pStringTable = new AGUI2XUIStringTable();

		if ( !m_pStringTable->Load( a_szStringTableFileName, a_bIsTRB, a_pTRB ) )
		{
			delete m_pStringTable;
			m_pStringTable = TNULL;
		}
	}

	if ( !T2String8::FindString( a_szFileName, a_bIsTRB ? ".trb" : ".xur" ) )
		return TFALSE;

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

		TINT  iNameLength = iLength - iNameStart - 4;
		TCHAR szSymbolName[ 128 ];

		// <name>_<name>_xur
		T2String8::Copy( szSymbolName, a_szFileName + iNameStart, iNameLength );
		szSymbolName[ iNameLength ]     = '_';
		szSymbolName[ iNameLength + 1 ] = '\0';
		T2String8::Copy( &szSymbolName[ iNameLength + 1 ], a_szFileName + iNameStart, iNameLength );
		T2String8::Copy( &szSymbolName[ 2 * iNameLength + 1 ], "_xur" );
		TStringManager::String8ToLowerCase( szSymbolName );

		if ( a_pTRB )
			pData = TSTATICCAST( TUINT8, a_pTRB->GetSymbolAddress( szSymbolName ) );

		if ( !pData )
		{
			// <name>_xur
			T2String8::Copy( szSymbolName, &szSymbolName[ iNameLength + 1 ], iNameLength + 4 );
			szSymbolName[ iNameLength + 4 ] = '\0';

			if ( oTRB.Load( a_szFileName ) != TTRB::ERROR_OK )
				return TFALSE;

			pData = TSTATICCAST( TUINT8, oTRB.GetSymbolAddress( szSymbolName ) );

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

	for ( TINT i = 0; i < m_oHeader.m_usNumSections; i++ )
	{
		XURSection& rSection = m_oHeader.m_pSections[ i ];
		TUINT8*     pSection = pData + rSection.uiOffset;
		TBOOL       bResult  = TTRUE;

		switch ( rSection.uiName )
		{
			case XURSECTION_STRN:
				bResult = ReadStringSection( pSection, rSection.uiSize );
				break;
			case XURSECTION_VECT:
				bResult = ReadVectorSection( pSection, rSection.uiSize );
				break;
			case XURSECTION_QUAT:
				bResult = ReadQuatSection( pSection, rSection.uiSize );
				break;
			case XURSECTION_CUST:
				bResult = ReadCustSection( pSection, rSection.uiSize );
				break;
			case XURSECTION_DATA:
				bResult = ReadDataSection( pSection );
				break;
		}

		if ( !bResult )
		{
			if ( !a_bIsTRB )
				TFree( pData );

			return TFALSE;
		}
	}

	if ( !a_bIsTRB )
		TFree( pData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8040fcc0
TBOOL AGUI2XUIResource::ReadHeader( TUINT8* a_pData )
{
	TUINT8* pSections = a_pData + 20;

	m_oHeader.m_uiFileID      = XURReader::PeekUInt32( a_pData );
	m_oHeader.m_uiVersion     = XURReader::PeekUInt32( a_pData + 4 );
	m_oHeader.m_usFlagsHigh   = XURReader::PeekUInt16( a_pData + 8 );
	m_oHeader.m_usFlags       = XURReader::PeekUInt16( a_pData + 10 );
	m_oHeader.m_uiFileSize    = XURReader::PeekUInt32( a_pData + 14 );
	m_oHeader.m_usNumSections = XURReader::PeekUInt16( a_pData + 18 );
	m_oHeader.m_pSections     = new XURSection[ XURReader::PeekUInt16( a_pData + 18 ) ];

	// Skip the extended header if the section table doesn't follow
	TUINT32 uiFirstSection = XURReader::PeekUInt32( pSections );

	if ( uiFirstSection != XURSECTION_STRN && uiFirstSection != XURSECTION_VECT && uiFirstSection != XURSECTION_QUAT &&
	     uiFirstSection != XURSECTION_CUST && uiFirstSection != XURSECTION_DATA )
		pSections += 40;

	for ( TINT i = 0; i < m_oHeader.m_usNumSections; i++ )
	{
		m_oHeader.m_pSections[ i ].uiName   = XURReader::ReadUInt32( pSections );
		m_oHeader.m_pSections[ i ].uiOffset = XURReader::ReadUInt32( pSections );
		m_oHeader.m_pSections[ i ].uiSize   = XURReader::ReadUInt32( pSections );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8040feb4
TBOOL AGUI2XUIResource::ReadStringSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	T2DynamicVector<TWCHAR*> vecStrings( GetGlobalAllocator(), 64, 64 );
	TWCHAR                   wszBuffer[ 276 ];

	// The first string is always an empty one
	TWCHAR* wszEmpty = new TWCHAR[ 1 ];
	wszEmpty[ 0 ]    = L'\0';
	vecStrings.PushBack( wszEmpty );

	while ( a_uiSize != 0 )
	{
		TUINT16 usLength = XURReader::ReadUInt16( a_pData );
		a_uiSize -= 2;

		for ( TUINT16 i = 0; i < usLength; i++ )
			wszBuffer[ i ] = XURReader::ReadUInt16( a_pData );

		wszBuffer[ usLength ] = L'\0';

		TWCHAR* wszString = new TWCHAR[ usLength + 1 ];
		T2String16::Copy( wszString, wszBuffer, usLength + 1 );
		a_uiSize -= usLength * 2;
		vecStrings.PushBack( wszString );
	}

	m_iNumStrings = vecStrings.Size();
	m_apStrings   = new TWCHAR*[ m_iNumStrings ];
	TUtil::MemCopy( m_apStrings, vecStrings.Data(), m_iNumStrings * sizeof( TWCHAR* ) );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80410104
TBOOL AGUI2XUIResource::ReadVectorSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	T2DynamicVector<TVector4> vecVectors( GetGlobalAllocator(), 64, 64 );

	while ( a_uiSize != 0 )
	{
		TVector4 vec;
		vec.x = XURReader::ReadFloat( a_pData );
		vec.y = XURReader::ReadFloat( a_pData );
		vec.z = XURReader::ReadFloat( a_pData );
		vec.w = 1.0f;

		vecVectors.PushBack( vec );
		a_uiSize -= 12;
	}

	m_uiNumVectors = vecVectors.Size();
	m_pVectors     = new TVector4[ m_uiNumVectors ];
	TUtil::MemCopy( m_pVectors, vecVectors.Data(), m_uiNumVectors * sizeof( TVector4 ) );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 804102b0
TBOOL AGUI2XUIResource::ReadQuatSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	T2DynamicVector<TQuaternion> vecQuats( GetGlobalAllocator(), 64, 64 );

	while ( a_uiSize != 0 )
	{
		TQuaternion quat;
		quat.x = XURReader::ReadFloat( a_pData );
		quat.y = XURReader::ReadFloat( a_pData );
		quat.z = XURReader::ReadFloat( a_pData );
		quat.w = XURReader::ReadFloat( a_pData );

		vecQuats.PushBack( quat );
		a_uiSize -= 16;
	}

	m_uiNumQuats = vecQuats.Size();
	m_pQuats     = new TQuaternion[ m_uiNumQuats ];
	TUtil::MemCopy( m_pQuats, vecQuats.Data(), m_uiNumQuats * sizeof( TQuaternion ) );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041046c
TBOOL AGUI2XUIResource::ReadCustSection( TUINT8* a_pData, TUINT32 a_uiSize )
{
	m_pCustData = new TUINT8[ a_uiSize ];
	TUtil::MemCopy( m_pCustData, a_pData, a_uiSize );
	m_uiCustSize = a_uiSize;

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 804104d8
TBOOL AGUI2XUIResource::ReadDataSection( TUINT8* a_pData )
{
	TUINT16 usClassIndex = XURReader::ReadUInt16( a_pData );
	m_pRoot              = CreateObjectData( *this, usClassIndex );

	XURXUIObjectData::OBJECTFLAGS ucFlags = XURReader::ReadUInt8( a_pData );
	m_pRoot->Load( *this, a_pData );

	if ( ucFlags & XURXUIObjectData::OBJECTFLAGS_CHILDREN )
		m_pRoot->LoadChildren( *this, a_pData );

	if ( ucFlags & XURXUIObjectData::OBJECTFLAGS_NAMED_FRAMES )
	{
		m_pRoot->LoadNamedFrames( *this, a_pData );

		if ( ucFlags & XURXUIObjectData::OBJECTFLAGS_CHILDREN )
			m_pRoot->LoadTimelines( *this, a_pData );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 804105a8
AGUI2XUIScene* AGUI2XUIResource::CreateScene( TUINT32 a_uiIndex )
{
	XURXUIElementData* pData = FindScene( a_uiIndex );

	if ( pData )
		return CreateScene( m_apStrings[ pData->m_usId ] );

	return TNULL;
}

// $BarnyardWiiJP: FUNCTION 804105fc
AGUI2XUIScene* AGUI2XUIResource::CreateScene( const TWCHAR* a_wszName )
{
	XURXUISceneData* pData = TSTATICCAST( XURXUISceneData, FindScene( a_wszName ) );

	if ( !pData )
		return TNULL;

	AGUI2XUIScene* pScene = TNULL;

	if ( pData->m_usClassOverride != 0 )
	{
		// The scene uses its own class
		const TWCHAR* wszClassName = m_apStrings[ pData->m_usClassOverride ];

		TCHAR szClassName[ 136 ];
		TStringManager::StringUnicodeToChar( szClassName, wszClassName, T2String16::Length( wszClassName ) );

		const TClass* pClass = TClass::Find( szClassName, &TGetClass( AGUI2XUIControl ) );

		if ( pClass )
		{
			TObject* pObject = pClass->CreateObject();
			pScene           = pObject ? TSTATICCAST( AGUI2XUIScene, pObject ) : TNULL;
		}
	}
	else
	{
		pScene = new AGUI2XUIScene();
	}

	pScene->Create( *this, pData, TTRUE );
	PopID();

	return pScene;
}

// $BarnyardWiiJP: FUNCTION 80410708
AGUI2XUIVisual* AGUI2XUIResource::CreateVisual( const TWCHAR* a_wszName )
{
	XURXUIElementData* pData = FindVisual( a_wszName );

	if ( !pData )
		return TNULL;

	AGUI2XUIVisual* pVisual = new AGUI2XUIVisual();
	pVisual->Create( *this, pData, TTRUE );
	PopID();

	return pVisual;
}

// $BarnyardWiiJP: FUNCTION 804107ac
const TVector4& AGUI2XUIResource::GetVector( TINT a_iIndex )
{
	if ( a_iIndex == -1 )
		return TVector4::VEC_ZERO;

	return m_pVectors[ a_iIndex ];
}

// $BarnyardWiiJP: FUNCTION 804107d0
const TQuaternion& AGUI2XUIResource::GetQuat( TINT a_iIndex )
{
	// NOTE: the original returns another (0, 0, 0, 1) vector here
	if ( a_iIndex == -1 )
		return TQuaternion::IDENTITY;

	return m_pQuats[ a_iIndex ];
}

// $BarnyardWiiJP: FUNCTION 804107f4
void AGUI2XUIResource::PushID( const TWCHAR* a_wszId )
{
	m_oIDStack.Push( a_wszId );

	if ( m_strIDPath.Length() != 0 )
		m_strIDPath.Concat( L"." );

	m_strIDPath.Concat( a_wszId );
}

// $BarnyardWiiJP: FUNCTION 80410858
void AGUI2XUIResource::PopID()
{
	const TWCHAR* wszId = m_oIDStack.Top();
	m_oIDStack.Pop();

	TINT iNewLength = m_strIDPath.Length() - T2String16::Length( wszId ) - 1;
	m_strIDPath     = m_strIDPath.Mid( 0, TMath::Max( 0, iNewLength ) );
}

// $BarnyardWiiJP: FUNCTION 804108f4
const TWCHAR* AGUI2XUIResource::LookupStringTable()
{
	if ( !m_pStringTable )
		return TNULL;

	return m_pStringTable->Lookup( m_strIDPath.GetString() );
}

// $BarnyardWiiJP: FUNCTION 80412764
XURXUIObjectData* AGUI2XUIResource::CreateObjectData( AGUI2XUIResource& a_rResource, TUINT16 a_usClassIndex )
{
	if ( a_usClassIndex == 0 )
		return TNULL;

	const TWCHAR* wszClassName = a_rResource.m_apStrings[ a_usClassIndex ];

	if ( T2String16::Compare( wszClassName, L"XuiCanvas" ) == 0 ) return new XURXUICanvasData();
	if ( T2String16::Compare( wszClassName, L"XuiScene" ) == 0 ) return new XURXUISceneData();
	if ( T2String16::Compare( wszClassName, L"XuiFigure" ) == 0 ) return new XURXUIFigureData();
	if ( T2String16::Compare( wszClassName, L"XuiVisual" ) == 0 ) return new XURXUIVisualData();
	if ( T2String16::Compare( wszClassName, L"XuiImage" ) == 0 ) return new XURXUIImageData();
	if ( T2String16::Compare( wszClassName, L"XuiText" ) == 0 ) return new XURXUITextData();
	if ( T2String16::Compare( wszClassName, L"XuiButton" ) == 0 ) return new XURXUIButtonData();
	if ( T2String16::Compare( wszClassName, L"XuiGroup" ) == 0 ) return new XURXUIGroupData();
	if ( T2String16::Compare( wszClassName, L"XuiImagePresenter" ) == 0 ) return new XURXUIImagePresenterData();
	if ( T2String16::Compare( wszClassName, L"XuiTextPresenter" ) == 0 ) return new XURXUITextPresenterData();

	return TNULL;
}

#endif // BARNYARD_LATEST
