#include "pch.h"
#include "AHackedQuickSave.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/ASimPersistData.h"
#include "Terrain/ATerrainInterface.h"
#include "Helpers/ATreasureManager.h"
#include "Helpers/ARegrowthManager.h"
#include "Helpers/AGateManager.h"
#include "Helpers/AReplayManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Player/APlayerProfileManager.h"
#include "Quests/AQuestManager.h"
#include "Units/AUnit.h"
#include "Units/AUnitType.h"
#include "Units/AAnimalPopulationManager.h"
#include "Units/AInteractionEventManager.h"
#include "Units/AInteractionManager.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AInventoryComponent.h"
#include "Units/AInventoryItem.h"
#include "Units/AUnitProfileComponent.h"
#include "Units/AUnitGameEventComponent.h"
#include "Units/AInteractionComponent.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Toshi/TStringManager.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: at 0x007ce238 in retail; sized up to the next global (0x007ce638)
// Also used as a buffer of 0x200 16-bit characters by ReadWString (0062e7d0)
static TCHAR s_szReadBuffer[ 0x400 ];

TPSTRING8_DECLARE( honeycomb );
TPSTRING8_DECLARE( royaljelly );
TPSTRING8_DECLARE( Hand );
TPSTRING8_DECLARE( Equip );

// NOTE: GetClass 0062dfa0, CreateObject 0062f660, CreateObjectInPlace 0062f6c0
TDEFINE_CLASS( AHackedQuickSave );

// Components of the player unit saved by SavePlayer (0x007797dc)
static TClass* const s_apPlayerComponents[] = {
	&TGetClass( AUnitProfileComponent ),
	&TGetClass( AUnitGameEventComponent ),
	&TGetClass( AInteractionComponent ),
};

// NOTE: retail writes the address of the TClass of the value's type. These are the addresses
// of the retail classes so that the saves stay compatible with the original game.
static constexpr TUINT32 PROPTYPE_UNDEF     = 0;
static constexpr TUINT32 PROPTYPE_INT       = 0x007cdfd4;
static constexpr TUINT32 PROPTYPE_UINT32    = 0x007cdf74;
static constexpr TUINT32 PROPTYPE_FLOAT     = 0x007ce094;
static constexpr TUINT32 PROPTYPE_BOOL      = 0x007ce064;
static constexpr TUINT32 PROPTYPE_TPSTRING8 = 0x007ce0c4;

static TUINT32 GetPropertyTypeId( const TClass* a_pType )
{
	if ( a_pType == PPropertyValue::TYPE_INT ) return PROPTYPE_INT;
	if ( a_pType == PPropertyValue::TYPE_UINT32 ) return PROPTYPE_UINT32;
	if ( a_pType == PPropertyValue::TYPE_FLOAT ) return PROPTYPE_FLOAT;
	if ( a_pType == PPropertyValue::TYPE_BOOL ) return PROPTYPE_BOOL;
	if ( a_pType == PPropertyValue::TYPE_TPSTRING8 ) return PROPTYPE_TPSTRING8;
	if ( a_pType == PPropertyValue::TYPE_UNDEF ) return PROPTYPE_UNDEF;

	// Types the stream doesn't know
	return TUINT32( TUINTPTR( a_pType ) );
}

// NOTE: inlined into AHackedQuickSave::AHackedQuickSave (0062f510)
AHackedQuickSave::Buffer::Buffer( TUINT a_uiSize )
{
	m_uiSize = a_uiSize;
	m_eFlags = FLAGS_NONE;
	m_pData  = TSTATICCAST( TCHAR, TMalloc( m_uiSize ) );
	TUtil::MemClear( m_pData, m_uiSize );
	m_uiCursor = 0;
	m_eFlags   = FLAGS_NONE;
}

// $Barnyard: FUNCTION 0062e4f0
AHackedQuickSave::Buffer::~Buffer()
{
	TFree( m_pData );
}

// $Barnyard: FUNCTION 0062e690
void AHackedQuickSave::Buffer::WriteString( const TPString8& a_rcString )
{
	const TCHAR* szString = a_rcString.GetString();
	TINT         iLength  = TStringManager::String8Length( szString );
	TCHAR*       pDest    = m_pData + m_uiCursor;

	TStringManager::String8Copy( pDest, szString, iLength + 1 );
	pDest[ iLength ] = '\0';

	m_uiCursor += iLength + 1;
}

// $Barnyard: FUNCTION 0062f220
void AHackedQuickSave::Buffer::ReadString( TPString8& a_rOutString )
{
	s_szReadBuffer[ 0 ] = '\0';

	const TCHAR* pSource = m_pData + m_uiCursor;
	TINT         iSize   = TStringManager::String8Length( pSource ) + 1;

	TStringManager::String8Copy( s_szReadBuffer, pSource, iSize );
	s_szReadBuffer[ iSize - 1 ] = '\0';
	m_uiCursor += iSize;

	a_rOutString = TPString8( s_szReadBuffer );
}

// $Barnyard: FUNCTION 0062e5c0
void AHackedQuickSave::Buffer::WriteVector4( const TVector4& a_rcVector )
{
	TUtil::MemCopy( m_pData + m_uiCursor, &a_rcVector.x, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( m_pData + m_uiCursor, &a_rcVector.y, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( m_pData + m_uiCursor, &a_rcVector.z, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( m_pData + m_uiCursor, &a_rcVector.w, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
}

// $Barnyard: FUNCTION 0062e900
void AHackedQuickSave::Buffer::ReadVector4( TVector4& a_rOutVector )
{
	TUtil::MemCopy( &a_rOutVector.x, m_pData + m_uiCursor, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( &a_rOutVector.y, m_pData + m_uiCursor, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( &a_rOutVector.z, m_pData + m_uiCursor, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( &a_rOutVector.w, m_pData + m_uiCursor, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
}

// $Barnyard: FUNCTION 0062e520
void AHackedQuickSave::Buffer::WriteVector3( const TVector4& a_rcVector )
{
	TUtil::MemCopy( m_pData + m_uiCursor, &a_rcVector.x, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( m_pData + m_uiCursor, &a_rcVector.y, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( m_pData + m_uiCursor, &a_rcVector.z, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
}

// $Barnyard: FUNCTION 0062e840
void AHackedQuickSave::Buffer::ReadVector3( TVector4& a_rOutVector )
{
	TUtil::MemCopy( &a_rOutVector.x, m_pData + m_uiCursor, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( &a_rOutVector.y, m_pData + m_uiCursor, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
	TUtil::MemCopy( &a_rOutVector.z, m_pData + m_uiCursor, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
}

// $Barnyard: FUNCTION 0062e0f0
void AHackedQuickSave::Buffer::WriteInt( TINT a_iValue )
{
	TUtil::MemCopy( m_pData + m_uiCursor, &a_iValue, sizeof( TINT ) );
	m_uiCursor += sizeof( TINT );
}

// NOTE: folded with WriteInt (0062e0f0) in retail
void AHackedQuickSave::Buffer::WriteFloat( TFLOAT a_fValue )
{
	TUtil::MemCopy( m_pData + m_uiCursor, &a_fValue, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
}

// $Barnyard: FUNCTION 0062e2e0
void AHackedQuickSave::Buffer::ReadInt( TINT& a_rOutValue )
{
	TUtil::MemCopy( &a_rOutValue, m_pData + m_uiCursor, sizeof( TINT ) );
	m_uiCursor += sizeof( TINT );
}

// $Barnyard: FUNCTION 0062e320
void AHackedQuickSave::Buffer::ReadFloat( TFLOAT& a_rOutValue )
{
	TUtil::MemCopy( &a_rOutValue, m_pData + m_uiCursor, sizeof( TFLOAT ) );
	m_uiCursor += sizeof( TFLOAT );
}

// $Barnyard: FUNCTION 0062e120
void AHackedQuickSave::Buffer::WriteString( const TCHAR* a_szString )
{
	TINT   iLength = TStringManager::String8Length( a_szString );
	TCHAR* pDest   = m_pData + m_uiCursor;

	TStringManager::String8Copy( pDest, a_szString, iLength + 1 );
	pDest[ iLength ] = '\0';

	m_uiCursor += iLength + 1;
}

// $Barnyard: FUNCTION 0062e1f0
void AHackedQuickSave::Buffer::ReadString( TCHAR* a_szOutString )
{
	const TCHAR* pSource = m_pData + m_uiCursor;
	TINT         iLength = TStringManager::String8Length( pSource );

	TStringManager::String8Copy( a_szOutString, pSource, iLength + 1 );
	a_szOutString[ iLength ] = '\0';

	m_uiCursor += iLength + 1;
}

// $Barnyard: FUNCTION 0062e390
TBOOL AHackedQuickSave::Buffer::CheckString( const TCHAR* a_szString )
{
	s_szReadBuffer[ 0 ] = '\0';

	const TCHAR* pSource = m_pData + m_uiCursor;
	TINT         iLength = TStringManager::String8Length( pSource );

	TStringManager::String8Copy( s_szReadBuffer, pSource, iLength + 1 );
	s_szReadBuffer[ iLength ] = '\0';
	m_uiCursor += iLength + 1;

	return TStringManager::String8Compare( s_szReadBuffer, a_szString ) == 0;
}

// $Barnyard: FUNCTION 0062e290
void AHackedQuickSave::Buffer::ReadUInt( TUINT& a_rOutValue )
{
	TUtil::MemCopy( &a_rOutValue, m_pData + m_uiCursor, sizeof( TUINT ) );
	m_uiCursor += sizeof( TUINT );
}

// $Barnyard: FUNCTION 0062e0b0
void AHackedQuickSave::Buffer::WriteUInt8( TUINT8 a_uiValue )
{
	m_pData[ m_uiCursor ] = TCHAR( a_uiValue );
	m_uiCursor += sizeof( TUINT8 );
}

// $Barnyard: FUNCTION 0062e0d0
void AHackedQuickSave::Buffer::WriteUInt16( TUINT16 a_uiValue )
{
	TUtil::MemCopy( m_pData + m_uiCursor, &a_uiValue, sizeof( TUINT16 ) );
	m_uiCursor += sizeof( TUINT16 );
}

// $Barnyard: FUNCTION 0062e220
void AHackedQuickSave::Buffer::ReadUInt8( TUINT8& a_rOutValue )
{
	a_rOutValue = TUINT8( m_pData[ m_uiCursor ] );
	m_uiCursor += sizeof( TUINT8 );
}

// $Barnyard: FUNCTION 0062e240
void AHackedQuickSave::Buffer::ReadUInt16( TUINT16& a_rOutValue )
{
	TUtil::MemCopy( &a_rOutValue, m_pData + m_uiCursor, sizeof( TUINT16 ) );
	m_uiCursor += sizeof( TUINT16 );
}

// $Barnyard: FUNCTION 0062e1d0
void AHackedQuickSave::Buffer::WriteBool( const TBOOL& a_rcValue )
{
	m_pData[ m_uiCursor ] = TCHAR( a_rcValue );
	m_uiCursor += sizeof( TUINT8 );
}

// $Barnyard: FUNCTION 0062e370
TBOOL AHackedQuickSave::Buffer::ReadBool( TBOOL& a_rOutValue )
{
	TCHAR cValue = m_pData[ m_uiCursor ];
	m_uiCursor += sizeof( TUINT8 );
	a_rOutValue = cValue != 0;

	return cValue != 0;
}

// $Barnyard: FUNCTION 0062e260
TINT16 AHackedQuickSave::Buffer::ReadInt16( TINT16& a_rOutValue )
{
	TUINT16 uiValue = TUINT16( TUINT8( m_pData[ m_uiCursor ] ) | ( TUINT8( m_pData[ m_uiCursor + 1 ] ) << 8 ) );
	m_uiCursor += sizeof( TINT16 );
	a_rOutValue = TINT16( uiValue );

	return TINT16( uiValue );
}

// $Barnyard: FUNCTION 0062e9f0
TBOOL AHackedQuickSave::Buffer::PeekString( TCHAR* a_szOutString, TINT a_iMaxLength )
{
	const TCHAR* pSource = m_pData + m_uiCursor;
	TINT         iMax    = TMath::Min( TINT( m_uiSize - m_uiCursor ), a_iMaxLength );

	for ( TINT i = 0; i < iMax; i++ )
	{
		a_szOutString[ i ] = pSource[ i ];

		if ( pSource[ i ] == '\0' )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0062f4a0
TBOOL AHackedQuickSave::Buffer::CheckString( const TPString8& a_rcString, TBOOL a_bUnused )
{
	TPString8 strString;
	ReadString( strString );

	return a_rcString == strString;
}

// $Barnyard: FUNCTION 0062e160
void AHackedQuickSave::Buffer::WriteWString( const TWCHAR* a_wszString )
{
	TINT   iSize = TStringManager::String16Length( a_wszString ) + 1;
	TCHAR* pDest = m_pData + m_uiCursor;

	for ( TINT i = 0; i < iSize - 1; i++ )
	{
		*pDest++ = TCHAR( a_wszString[ i ] & 0xff );
		*pDest++ = TCHAR( a_wszString[ i ] >> 8 );
	}

	pDest[ 0 ] = '\0';
	pDest[ 1 ] = '\0';

	m_uiCursor += iSize * 2;
}

// $Barnyard: FUNCTION 0062e1b0
void AHackedQuickSave::Buffer::WriteWString( const TString16& a_rcString )
{
	WriteWString( a_rcString.GetString( 0 ) );
}

// $Barnyard: FUNCTION 0062e7d0
void AHackedQuickSave::Buffer::ReadWString( TString16& a_rOutString )
{
	TWCHAR* wszBuffer = TREINTERPRETCAST( TWCHAR*, s_szReadBuffer );
	wszBuffer[ 0 ]    = L'\0';

	TINT iRead = AHackedQuickSave::ReadWString( m_pData + m_uiCursor, wszBuffer, sizeof( s_szReadBuffer ) / sizeof( TWCHAR ) );
	m_uiCursor += iRead * 2;

	a_rOutString = wszBuffer;
}

// $Barnyard: FUNCTION 0062e6e0
void AHackedQuickSave::Buffer::WritePropertyValue( const PPropertyValue& a_rcValue )
{
	const TClass* pType  = a_rcValue.GetType();
	TUINT32       uiType = GetPropertyTypeId( pType );

	TUtil::MemCopy( m_pData + m_uiCursor, &uiType, sizeof( uiType ) );
	m_uiCursor += sizeof( uiType );

	if ( pType == PPropertyValue::TYPE_UNDEF )
		return;

	if ( pType == PPropertyValue::TYPE_INT )
		WriteInt( a_rcValue.GetInteger() );
	else if ( pType == PPropertyValue::TYPE_UINT32 )
		WriteInt( TINT( a_rcValue.GetUINT32() ) );
	else if ( pType == PPropertyValue::TYPE_FLOAT )
		WriteFloat( a_rcValue.GetFloat() );
	else if ( pType == PPropertyValue::TYPE_BOOL )
	{
		m_pData[ m_uiCursor ] = TCHAR( a_rcValue.GetBoolean() );
		m_uiCursor += sizeof( TUINT8 );
	}
	else if ( pType == PPropertyValue::TYPE_TPSTRING8 )
		WriteString( a_rcValue.GetTPString8() );
}

// $Barnyard: FUNCTION 0062f2f0
void AHackedQuickSave::Buffer::ReadPropertyValue( PPropertyValue& a_rOutValue )
{
	TUINT32 uiType;
	TUtil::MemCopy( &uiType, m_pData + m_uiCursor, sizeof( uiType ) );
	m_uiCursor += sizeof( uiType );

	if ( uiType == PROPTYPE_UNDEF )
	{
		a_rOutValue.SetUndef();
	}
	else if ( uiType == PROPTYPE_INT )
	{
		TINT iValue;
		ReadInt( iValue );
		a_rOutValue.SetInteger( iValue );
	}
	else if ( uiType == PROPTYPE_UINT32 )
	{
		TUINT uiValue;
		ReadUInt( uiValue );
		a_rOutValue.SetUINT32( uiValue );
	}
	else if ( uiType == PROPTYPE_FLOAT )
	{
		TFLOAT fValue;
		ReadFloat( fValue );
		a_rOutValue.SetFloat( fValue );
	}
	else if ( uiType == PROPTYPE_BOOL )
	{
		// NOTE: the original sets the value as an integer
		TBOOL bValue = m_pData[ m_uiCursor ] != 0;
		m_uiCursor += sizeof( TUINT8 );
		a_rOutValue.SetInteger( bValue );
	}
	else if ( uiType == PROPTYPE_TPSTRING8 )
	{
		TPString8 strValue;
		ReadString( strValue );
		a_rOutValue.SetTPString8( strValue );
	}
}

//-----------------------------------------------------------------------------
// AHackedQuickSave
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0062f510
AHackedQuickSave::AHackedQuickSave()
{
	m_iCurrentBuffer = BUFFER_WORLD;

	for ( TINT i = 0; i < BUFFER_NUMOF; i++ )
		m_apBuffers[ i ] = new Buffer( ms_auiBufferSizes[ i ] );
}

// $Barnyard: FUNCTION 0062e470
// $Barnyard: FUNCTION 0062ea40
AHackedQuickSave::~AHackedQuickSave()
{
	for ( TINT i = 0; i < BUFFER_NUMOF; i++ )
	{
		if ( m_apBuffers[ i ] )
			delete m_apBuffers[ i ];

		m_apBuffers[ i ] = TNULL;
	}
}

// $Barnyard: FUNCTION 0062dfb0
TBOOL AHackedQuickSave::SaveToMemory( void* a_pData, TINT a_iSize )
{
	TCHAR* pDest = TSTATICCAST( TCHAR, a_pData );
	TUtil::MemClear( pDest, a_iSize );

	if ( a_iSize != SAVE_DATA_SIZE )
		return TFALSE;

	for ( TINT i = 0; i < BUFFER_NUMOF; i++ )
	{
		// Flags and cursor
		TUtil::MemCopy( pDest, &m_apBuffers[ i ]->m_eFlags, 8 );
		pDest += 8;

		TUtil::MemCopy( pDest, m_apBuffers[ i ]->m_pData, ms_auiBufferSizes[ i ] );
		pDest += ms_auiBufferSizes[ i ];
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0062e020
TBOOL AHackedQuickSave::LoadFromMemory( const void* a_pData, TINT a_iSize )
{
	if ( a_iSize != SAVE_DATA_SIZE )
		return TFALSE;

	const TCHAR* pSource = TSTATICCAST( const TCHAR, a_pData );

	for ( TINT i = 0; i < BUFFER_NUMOF; i++ )
	{
		TUtil::MemCopy( &m_apBuffers[ i ]->m_eFlags, pSource, 8 );
		pSource += 8;

		TUtil::MemCopy( m_apBuffers[ i ]->m_pData, pSource, ms_auiBufferSizes[ i ] );
		pSource += ms_auiBufferSizes[ i ];
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0062e080
TUINT AHackedQuickSave::GetBufferDataOffset( BUFFER a_eBuffer )
{
	TUINT uiOffset = 0;

	for ( TINT i = 0; i < a_eBuffer; i++ )
		uiOffset += ms_auiBufferSizes[ i ] + 8;

	return uiOffset + 8;
}

// $Barnyard: FUNCTION 0062e430
TINT AHackedQuickSave::ReadInt( const void* a_pData )
{
	const TUINT8* pData = TSTATICCAST( const TUINT8, a_pData );
	return TINT( pData[ 0 ] | ( pData[ 1 ] << 8 ) | ( *TREINTERPRETCAST( const TUINT16*, pData + 2 ) << 16 ) );
}

// $Barnyard: FUNCTION 0062e3e0
TINT AHackedQuickSave::ReadWString( const void* a_pData, TWCHAR* a_wszOutString, TINT a_iMaxLength )
{
	const TUINT8* pData = TSTATICCAST( const TUINT8, a_pData );
	TINT          iRead = 0;

	if ( a_iMaxLength <= 0 )
		return 0;

	TWCHAR wcChar;

	do
	{
		wcChar = TWCHAR( pData[ 0 ] | ( pData[ 1 ] << 8 ) );
		pData += 2;
		a_wszOutString[ iRead++ ] = wcChar;
	} while ( wcChar != L'\0' && iRead < a_iMaxLength );

	if ( iRead > 0 )
		a_wszOutString[ iRead - 1 ] = L'\0';

	return iRead;
}

// $Barnyard: FUNCTION 0062ea60
// $BarnyardWiiJP: FUNCTION 803e017c
TBOOL AHackedQuickSave::SaveTime()
{
	BUFFER eOldBuffer = m_iCurrentBuffer;
	m_iCurrentBuffer  = BUFFER_TIME;

#ifdef BARNYARD_LATEST
	if ( ms_iSavedDayPhase != SAVEDDAYPHASE_NIGHT_BARN )
#endif
		ms_iSavedDayPhase = AGameTimeManager::GetSingleton()->GetCurrentDayPhase();

	GetCurrentBuffer()->m_uiCursor = 0;
	AGameTimeManager::GetSingleton()->OnSerialize( GetCurrentBuffer() );
	AAnimalPopulationManager::GetSingleton()->SaveSpawnState( GetCurrentBuffer() );
	GetCurrentBuffer()->m_eFlags |= Buffer::FLAGS_SAVED;

	m_iCurrentBuffer = eOldBuffer;
	return TTRUE;
}

// $Barnyard: FUNCTION 0062ebb0
TBOOL AHackedQuickSave::LoadTime()
{
	BUFFER eOldBuffer = m_iCurrentBuffer;

	// NOTE: the original checks the current buffer, not BUFFER_TIME
	if ( GetCurrentBuffer()->m_eFlags & Buffer::FLAGS_SAVED )
	{
		m_iCurrentBuffer = BUFFER_TIME;

		m_apBuffers[ BUFFER_TIME ]->m_uiCursor = 0;
		AGameTimeManager::GetSingleton()->OnDeserialize( GetCurrentBuffer() );
		AAnimalPopulationManager::GetSingleton()->LoadSpawnState( GetCurrentBuffer() );
		GetCurrentBuffer()->m_eFlags &= ~Buffer::FLAGS_SAVED;

		m_iCurrentBuffer = eOldBuffer;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0062ead0
TBOOL AHackedQuickSave::SaveProfile()
{
	BUFFER eOldBuffer = m_iCurrentBuffer;
	m_iCurrentBuffer  = BUFFER_PROFILE;

	m_apBuffers[ BUFFER_PROFILE ]->m_uiCursor = 0;
	APlayerProfileManager::GetSingleton()->SaveMainProfile( GetCurrentBuffer() );
	AQuestManager::GetSingleton()->Save( GetCurrentBuffer() );
	AInteractionEventManager::GetSingleton()->Save( GetCurrentBuffer() );

	AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetHumanPlayerUnit( 0 );
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	pInventory->Save( GetCurrentBuffer() );

	// NOTE: the original calls RETURNS_TRUE (00413920) with the buffer here

	ASimPersistData::GetSingleton()->Save( GetCurrentBuffer() );
	AInteractionManager::GetSingleton()->SavePersistentLatches( GetCurrentBuffer() );
	GetCurrentBuffer()->m_eFlags |= Buffer::FLAGS_SAVED;

	m_iCurrentBuffer = eOldBuffer;
	return TTRUE;
}

// $Barnyard: FUNCTION 0062eee0
TBOOL AHackedQuickSave::LoadProfile()
{
	BUFFER eOldBuffer = m_iCurrentBuffer;

	// NOTE: the original checks the current buffer, not BUFFER_PROFILE
	if ( GetCurrentBuffer()->m_eFlags & Buffer::FLAGS_SAVED )
	{
		m_iCurrentBuffer                          = BUFFER_PROFILE;
		m_apBuffers[ BUFFER_PROFILE ]->m_uiCursor = 0;

		// Honeycombs the player has now, they are given back after the profile is loaded
		APlayerSimProfile* pProfile      = APlayerProfileManager::GetSingleton()->GetMainProfile();
		TBOOL              bRoyalJelly   = pProfile->HasRoyalJelly();
		TINT               iNumHoneycomb = pProfile->GetNumHoneycombs();
		pProfile->SetNumHoneycombs( 0 );
		pProfile->SetRoyalJelly( TFALSE );

		APlayerProfileManager::GetSingleton()->LoadMainProfile( GetCurrentBuffer() );
		AQuestManager::GetSingleton()->Load( GetCurrentBuffer() );
		AInteractionEventManager::GetSingleton()->Load( GetCurrentBuffer() );

		AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetHumanPlayerUnit( 0 );
		AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
		pInventory->Load( GetCurrentBuffer() );

		if ( iNumHoneycomb > 0 )
		{
			// Put away whatever is in the hand
			AInventoryComponent* pHand = pInventory->GetCategory( TPS8( Hand ) );

			for ( TINT i = pHand->GetNumSlots() - 1; i >= 0; i-- )
			{
				AInventoryItem* pItem = pHand->GetItem( i );

				if ( pItem )
					pInventory->MoveItemToUseCategory( pItem, pItem->GetCount() );
			}

			pInventory->AddItems( TPS8( honeycomb ), iNumHoneycomb, AInventoryComponentInterface::ms_strDefaultInventory );
			pInventory->TransferItems( TPS8( honeycomb ), pHand, 1 );

			if ( bRoyalJelly )
				pInventory->AddItems( TPS8( royaljelly ), 1, AInventoryComponentInterface::ms_strDefaultInventory );
		}

		AInventoryComponent* pEquip = pInventory->GetCategory( TPS8( Equip ) );

		for ( TINT i = pEquip->GetNumItems() - 1; i >= 0; i-- )
			pInventory->MoveItemToUseCategory( pEquip->GetItem( i ), 1 );

		AInventoryComponent* pHand = pInventory->GetCategory( TPS8( Hand ) );

		for ( TINT i = pHand->GetNumSlots() - 1; i >= 0; i-- )
		{
			AInventoryItem* pItem = pHand->GetItem( i );

			if ( pItem )
				pInventory->MoveItemToUseCategory( pItem, pItem->GetCount() );
		}

		// NOTE: the original calls RETURNS_TRUE (00413920) with the buffer here

		ASimPersistData::GetSingleton()->Load( GetCurrentBuffer() );
		AInteractionManager::GetSingleton()->LoadPersistentLatches( GetCurrentBuffer() );
		GetCurrentBuffer()->m_eFlags &= ~Buffer::FLAGS_SAVED;

		m_iCurrentBuffer = eOldBuffer;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0062ec20
TBOOL AHackedQuickSave::SavePlayer()
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetHumanPlayerUnit( 0 );

	if ( pPlayer )
	{
		GetCurrentBuffer()->WriteString( pPlayer->GetType()->GetName() );
		pPlayer->GetSteer()->Serialise( GetCurrentBuffer() );

		for ( TINT i = 0; i < TARRAYSIZE( s_apPlayerComponents ); i++ )
		{
			AUnitComponent* pComponent = pPlayer->FindComponent( s_apPlayerComponents[ i ] );

			if ( pComponent )
				pComponent->Save( GetCurrentBuffer() );
		}

		if ( AGameLoader::IsMainGameLoaded() )
		{
			ASimPersistData::GetSingleton()->SetTerrainVIS( ATerrainInterface::GetSingleton()->GetSectionIndexAtPoint( *pPlayer->GetSteer()->GetPosition(), TTRUE ) );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0062f5e0
TBOOL AHackedQuickSave::LoadPlayer()
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetHumanPlayerUnit( 0 );

	if ( pPlayer )
	{
		TBOOL bResult = GetCurrentBuffer()->CheckString( pPlayer->GetType()->GetName(), TTRUE );

		if ( !bResult )
			return bResult;

		pPlayer->GetSteer()->Deserialise( GetCurrentBuffer() );

		for ( TINT i = 0; i < TARRAYSIZE( s_apPlayerComponents ); i++ )
		{
			AUnitComponent* pComponent = pPlayer->FindComponent( s_apPlayerComponents[ i ] );

			if ( pComponent )
				pComponent->Load( GetCurrentBuffer() );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0062ecd0
TBOOL AHackedQuickSave::SaveRegrowth( TBOOL a_bSecond )
{
	Buffer* pBuffer = GetSingleton()->GetBuffer( a_bSecond ? BUFFER_REGROWTH2 : BUFFER_REGROWTH );

	pBuffer->m_uiCursor = 0;
	ARegrowthManager::GetSingleton()->Save( pBuffer );
	pBuffer->m_eFlags |= Buffer::FLAGS_SAVED;

	return TFALSE;
}

// $Barnyard: FUNCTION 0062ed10
TBOOL AHackedQuickSave::LoadRegrowth( TBOOL a_bSecond )
{
	Buffer* pBuffer = GetSingleton()->GetBuffer( a_bSecond ? BUFFER_REGROWTH2 : BUFFER_REGROWTH );

	if ( pBuffer->m_eFlags & Buffer::FLAGS_SAVED )
	{
		pBuffer->m_uiCursor = 0;
		ARegrowthManager::GetSingleton()->Load( pBuffer );
		pBuffer->m_eFlags &= ~Buffer::FLAGS_SAVED;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0062ed60
void AHackedQuickSave::ResetBuffers( TBOOL a_bAll )
{
	for ( TINT i = BUFFER_WORLD; i <= BUFFER_PROFILE; i++ )
	{
		m_apBuffers[ i ]->m_eFlags &= ~Buffer::FLAGS_SAVED;
		m_apBuffers[ i ]->m_uiCursor = 0;
	}

	if ( a_bAll )
	{
		for ( TINT i = BUFFER_OFFERS; i < BUFFER_NUMOF; i++ )
		{
			m_apBuffers[ i ]->m_eFlags &= ~Buffer::FLAGS_SAVED;
			m_apBuffers[ i ]->m_uiCursor = 0;
		}
	}

	m_iCurrentBuffer = BUFFER_WORLD;
}

// $Barnyard: FUNCTION 0062edc0
void AHackedQuickSave::SaveGame()
{
	// NOTE: the original looks for the first saved buffer here but doesn't use the result

	for ( TINT i = BUFFER_WORLD; i <= BUFFER_PROFILE; i++ )
	{
		m_apBuffers[ i ]->m_eFlags &= ~Buffer::FLAGS_SAVED;
		m_apBuffers[ i ]->m_uiCursor = 0;
	}

	// SaveTime (inlined)
	m_iCurrentBuffer               = BUFFER_TIME;
	ms_iSavedDayPhase              = AGameTimeManager::GetSingleton()->GetCurrentDayPhase();
	GetCurrentBuffer()->m_uiCursor = 0;
	AGameTimeManager::GetSingleton()->OnSerialize( GetCurrentBuffer() );
	AAnimalPopulationManager::GetSingleton()->SaveSpawnState( GetCurrentBuffer() );
	GetCurrentBuffer()->m_eFlags |= Buffer::FLAGS_SAVED;

	m_iCurrentBuffer = BUFFER_WORLD;
	SaveProfile();

	ATreasureManager::GetSingleton()->Save( GetCurrentBuffer() );
	ARegrowthManager::GetSingleton()->Save( GetCurrentBuffer() );
	SavePlayer();
	AAnimalPopulationManager::GetSingleton()->SaveUnits( GetCurrentBuffer() );

	AReplayManager::GetSingleton()->Save( GetCurrentBuffer() );

	AGateManager::GetSingleton()->Save( GetCurrentBuffer() );
	m_apBuffers[ BUFFER_WORLD ]->m_eFlags |= Buffer::FLAGS_SAVED;
}

// $Barnyard: FUNCTION 0062f700
void AHackedQuickSave::LoadGame()
{
	TINT i = 0;

	while ( !( m_apBuffers[ i ]->m_eFlags & Buffer::FLAGS_SAVED ) )
	{
		// Nothing has been saved
		if ( ++i >= 3 )
			return;
	}

	for ( TINT j = BUFFER_WORLD; j <= BUFFER_PROFILE; j++ )
		m_apBuffers[ j ]->m_uiCursor = 0;

	m_iCurrentBuffer = BUFFER_WORLD;
	LoadProfile();

	ATreasureManager::GetSingleton()->Load( GetCurrentBuffer() );
	ARegrowthManager::GetSingleton()->Load( GetCurrentBuffer() );
	LoadPlayer();
	AAnimalPopulationManager::GetSingleton()->LoadUnits( GetCurrentBuffer() );

	AReplayManager::GetSingleton()->Load( GetCurrentBuffer() );

	AGateManager::GetSingleton()->Load( GetCurrentBuffer() );
	m_apBuffers[ BUFFER_WORLD ]->m_eFlags &= ~Buffer::FLAGS_SAVED;
}
