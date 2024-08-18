#include "ToshiPCH.h"
#include "THashTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

THashTable::t_ItemCompareFunc THashTable::DefaultItemCompareFunc = []( const void* a_pMem1, const void* a_pMem2, TSIZE a_iSize ) {
	return TUtil::MemCompare( a_pMem1, a_pMem2, a_iSize ) == 0;
};

THashTable::t_ItemHashFunc THashTable::DefaultItemHashFunc = []( const void* a_pMem, TSIZE a_iSize, TUINT32 a_uiMaxValue ) {
	const TBYTE* pBuffer = TREINTERPRETCAST( const TBYTE*, a_pMem );
	TUINT32      uiHash  = 0;

	while ( a_iSize != 0 )
	{
		uiHash = ( uiHash * 0x10 ) + *pBuffer++;
		if ( ( uiHash & 0xF0000000 ) != 0 )
		{
			uiHash = uiHash ^ ( uiHash & 0xF0000000 ) >> 24 ^ ( uiHash & 0xF0000000 );
		}
		a_iSize--;
	}

	return uiHash - ( uiHash / a_uiMaxValue ) * a_uiMaxValue;
};

THashTable::THashTable()
{
	m_iBucketSize         = 0;
	m_iItemSize           = 0;
	m_iItemCount          = 0;
	m_iItemCountTotal     = 0;
	m_iHashNodeCount      = 0;
	m_iHashNodeCountTotal = 0;
	m_pHashToBucketId     = TNULL;
	m_pBuckets            = TNULL;
	m_pItems              = TNULL;
	m_ItemHashFunc        = TNULL;
	m_ItemCompareFunc     = TNULL;
}

THashTable::~THashTable()
{
	Destroy();
}

void THashTable::DeleteBucketMemory()
{
	delete[] m_pHashToBucketId;
	m_pHashToBucketId = TNULL;
}

void THashTable::Destroy()
{
	DeleteBucketMemory();

	delete[] m_pBuckets;
	m_pBuckets = TNULL;

	TFree( m_pItems );
	m_pItems = TNULL;
}

THashTable::Bucket* THashTable::Insert( void* a_pData )
{
	TASSERT( 0 != m_iItemSize );

	if ( TNULL == GetHashToBucketIds() )
		return TNULL;

	THashTable::Bucket* pFoundBucket = Find( a_pData );
	return ( pFoundBucket ) ? pFoundBucket : Append( a_pData );
}

THashTable::Bucket* THashTable::Find( void* a_pData )
{
	TASSERT( 0 != m_iItemSize );
	TINT* pBuckets = GetHashToBucketIds();

	if ( pBuckets != TNULL )
	{
		TUINT32 uiHash  = m_ItemHashFunc( a_pData, m_iItemSize, m_iBucketSize );
		TINT    iBucket = pBuckets[ uiHash ];

		while ( iBucket != INVALID_BUCKET_ID )
		{
			Bucket& item = m_pBuckets[ iBucket ];

			if ( m_ItemCompareFunc( a_pData, m_pItems + m_iItemSize * item.iItemIndex, m_iItemSize ) )
				return &item;

			iBucket = item.iNextBucketId;
		}
	}

	return TNULL;
}

THashTable::Bucket* THashTable::Append( void* a_pData )
{
	TASSERT( 0 != m_iItemSize );

	if ( m_pHashToBucketId != TNULL )
	{
		TASSERT( !( ( m_iHashNodeCount == m_iHashNodeCountTotal ) || ( m_iItemCount == m_iItemCountTotal ) ) );

		if ( !( ( m_iHashNodeCount == m_iHashNodeCountTotal ) || ( m_iItemCount == m_iItemCountTotal ) ) )
		{
			TUINT32 uiHash = m_ItemHashFunc( a_pData, m_iItemSize, m_iBucketSize );

			m_pBuckets[ m_iHashNodeCountTotal ].iNextBucketId = m_pHashToBucketId[ uiHash ];
			m_pBuckets[ m_iHashNodeCountTotal ].iItemIndex    = m_iItemCount;

			TUtil::MemCopy( m_pItems + m_iItemCount * m_iItemSize, a_pData, m_iItemSize );

			m_pHashToBucketId[ uiHash ] = m_iHashNodeCountTotal;

			m_iItemCount++;
			m_iHashNodeCountTotal++;

			return &m_pBuckets[ m_iHashNodeCountTotal ];
		}
	}

	return TNULL;
}

TBOOL THashTable::Create( TINT a_iItemCountTotal, TINT a_iItemSize, TINT a_iBucketSize, TINT a_iHashNodeCount )
{
	if ( m_pHashToBucketId == TNULL )
	{
		if ( a_iHashNodeCount < a_iItemCountTotal )
		{
			a_iHashNodeCount = a_iItemCountTotal;
		}

		m_iBucketSize     = a_iBucketSize;
		m_iItemSize       = a_iItemSize;
		m_iItemCount      = 0;
		m_iItemCountTotal = a_iItemCountTotal;
		m_iHashNodeCount  = a_iHashNodeCount;
		m_pHashToBucketId = new TINT[ m_iBucketSize ];
		m_pBuckets        = new Bucket[ m_iHashNodeCount ];
		m_pItems          = TSTATICCAST( TBYTE, TMemalign( m_iItemSize * m_iItemCountTotal, 16 ) );

		SetItemHashFunction( DefaultItemHashFunc );
		SetItemCompareFunction( DefaultItemCompareFunc );

		for ( TINT i = 0; i < m_iBucketSize; i++ )
		{
			m_pHashToBucketId[ i ] = INVALID_BUCKET_ID;
		}

		return TTRUE;
	}

	return TFALSE;
}

TOSHI_NAMESPACE_END
