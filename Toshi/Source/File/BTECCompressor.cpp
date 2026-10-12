#include "ToshiPCH.h"
#include "BTECCompressor.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

BTECCompressor::BTECCompressor()
{
	this->m_Buffer1     = TNULL;
	this->m_DataEnd     = TNULL;
	this->m_Buffer2     = TNULL;
	this->m_Buffer3     = TNULL;
	this->m_pNodes      = TNULL;
	this->m_Buckets     = TNULL;
	this->m_BucketCount = 0;
}

BTECCompressor::~BTECCompressor()
{
	TFree( m_pNodes );

	for ( TSIZE i = 0; i < m_BucketCount; i++ )
	{
		for ( TSIZE k = 0; k < 256; k++ )
		{
			TFree( m_Buckets[ i ][ k ] );
		}

		TFree( m_Buckets[ i ] );
	}

	TFree( m_Buckets );
}

void BTECCompressor::Initialize( TBYTE* buffer, TSIZE bufferSize, TINT maxoffset, TINT unk )
{
	// 0068ac60 +++
	m_DataEnd     = buffer + bufferSize;
	m_Buffer1     = buffer;
	m_Buffer2     = buffer;
	m_Buffer3     = buffer;
	m_MaxOffset   = maxoffset;
	m_BucketCount = ( unk < 1 ) ? 0 : unk - 1;

	m_Root1.Reset();
	m_Root2.Reset();

	TSIZE arraySize1 = m_BucketCount * maxoffset;
	m_pNodes         = (Node*)TMemalign( arraySize1 * sizeof( Node ), 16 );

	for ( TSIZE i = 0; i < arraySize1; i++ )
	{
		m_pNodes[ i ].m_pNext          = &m_Root1;
		m_pNodes[ i ].m_pPrev          = m_Root1.m_pPrev;
		m_pNodes[ i ].m_pNext->m_pPrev = &m_pNodes[ i ];
		m_pNodes[ i ].m_pPrev->m_pNext = &m_pNodes[ i ];
	}

	TUtil::MemClear( m_Offsets, sizeof( m_Offsets ) );

	m_Buckets = (Bucket*)TMemalign( m_BucketCount * sizeof( Bucket ), 16 );

	for ( TSIZE i = 0; i < m_BucketCount; i++ )
	{
		m_Buckets[ i ] = (Bucket)TMemalign( 256 * sizeof( Node* ), 16 );

		for ( TSIZE k = 0; k < 256; k++ )
		{
			Node* someNode        = new Node();
			someNode->m_pHashNext = someNode;
			someNode->m_pHashPrev = someNode;
			m_Buckets[ i ][ k ]   = someNode;
		}
	}
}

void BTECCompressor::AdvanceWindow( TSIZE dataSize )
{
	// 0068ae40
	auto oldBuffer3 = m_Buffer3;
	auto newBuffer3 = m_Buffer3 + dataSize;

	if ( m_DataEnd <= newBuffer3 && newBuffer3 != m_DataEnd )
	{
		newBuffer3 = m_DataEnd;
	}

	m_Buffer3 = newBuffer3;

	if ( m_MaxOffset < static_cast<uintptr_t>( m_Buffer3 - m_Buffer2 ) )
	{
		m_Buffer2 = m_Buffer3 - m_MaxOffset;

		for ( TSIZE i = 0; i < 256; i++ )
		{
			if ( m_Offsets[ i ] < m_Buffer2 )
			{
				m_Offsets[ i ] = TNULL;
			}
		}

		auto node = m_Root2.m_pNext;
		while ( node != &m_Root2 )
		{
			if ( m_Buffer2 <= node->m_pData ) break;
			auto nextNode = node->m_pNext;

			node->m_pHashNext->m_pHashPrev = node->m_pHashPrev;
			node->m_pHashPrev->m_pHashNext = node->m_pHashNext;
			node->m_pHashNext              = node;
			node->m_pHashPrev              = node;
			node->m_pNext->m_pPrev         = node->m_pPrev;
			node->m_pPrev->m_pNext         = node->m_pNext;
			node->m_pNext                  = &m_Root1;
			node->m_pPrev                  = m_Root1.m_pPrev;
			m_Root1.m_pPrev                = node;
			node->m_pPrev->m_pNext         = node;

			node = nextNode;
		}
	}

	while ( oldBuffer3 < m_Buffer3 )
	{
		AllocSubstring( oldBuffer3 );
		oldBuffer3++;
	}
}

TBOOL BTECCompressor::FindMatch( TBYTE* buffer, TSIZE bufferSize, TBYTE*& offset, TSIZE& dataSize )
{
	// +++
	offset   = TNULL;
	dataSize = 0;

	if ( bufferSize < 1 || m_Offsets[ (TBYTE)*buffer ] == TNULL )
	{
		return TFALSE;
	}

	if ( m_BucketCount > 0 )
	{
		TSIZE  size   = 0;
		Node*  node1  = TNULL;
		Node*  node2  = TNULL;
		Bucket bucket = TNULL;

		for ( TSIZE i = 0; i < m_BucketCount; i++ )
		{
			TSIZE localSize = 2 << ( i % 32 );
			if ( bufferSize < localSize ) break;

			Node* localNode1;
			Node* localNode2;
			TBOOL bRes = FindNodeInBucket( buffer, m_Buckets[ i ], localSize, localNode1, localNode2 );
			if ( bRes == TFALSE ) break;

			size   = localSize;
			node1  = localNode1;
			node2  = localNode2;
			bucket = m_Buckets[ i ];
		}

		if ( node1 != TNULL )
		{
			TSIZE someSize;

			if ( bucket == m_Buckets[ m_BucketCount - 1 ] )
			{
				someSize = TMath::Min( bufferSize, static_cast<TSIZE>( m_DataEnd - node1->m_pData ) );
			}
			else
			{
				someSize = TMath::Min( bufferSize, size * 2 - 1 );
			}

			if ( node1 == node2 )
			{
				return TTRUE;
			}

			while ( TTRUE )
			{
				size = 0;

				while ( node1->m_pData[ size ] == buffer[ size ] )
				{
					if ( someSize <= size ) break;
					size++;
				}

				if ( dataSize < size )
				{
					dataSize = size;
					offset   = node1->m_pData;

					if ( someSize <= size )
					{
						return TTRUE;
					}
				}

				node1 = node1->m_pHashNext;

				if ( node1 == node2 )
				{
					return TTRUE;
				}
			}
		}
	}

	offset   = m_Offsets[ (TBYTE)*buffer ];
	dataSize = 1;

	return TTRUE;
}

void BTECCompressor::AllocSubstring( TBYTE* buffer )
{
	// 0068b180 +++
	m_Offsets[ (TBYTE)*buffer ] = buffer;

	for ( TSIZE i = 0; i < m_BucketCount; i++ )
	{
		TSIZE unknown = 2 << ( i % 32 );

		if ( static_cast<uintptr_t>( m_DataEnd - buffer ) < unknown )
		{
			return;
		}

		auto node = m_Root1.m_pNext;
		if ( node == &m_Root1 )
		{
			TERROR( "SlidingWindow::AllocSubstring: No free substrings!\n" );
			TBREAK();
			return;
		}

		node->m_pNext->m_pPrev = node->m_pPrev;
		node->m_pPrev->m_pNext = node->m_pNext;
		node->m_pNext          = &m_Root2;
		node->m_pPrev          = m_Root2.m_pPrev;
		m_Root2.m_pPrev        = node;
		node->m_pPrev->m_pNext = node;
		node->m_Unk4           = (void*)( m_DataEnd - buffer );
		node->m_pData          = buffer;

		TUINT32 hash = HashData( buffer, unknown );

		auto hashedNode                = m_Buckets[ i ][ hash % 256 ];
		node->m_pHashNext              = hashedNode;
		node->m_pHashPrev              = hashedNode->m_pHashPrev;
		hashedNode->m_pHashPrev        = node;
		node->m_pHashPrev->m_pHashNext = node;
	}
}

TBOOL BTECCompressor::FindNodeInBucket( TBYTE* buffer, Bucket nodeBucket, TSIZE bufferSize, Node*& out1, Node*& out2 )
{
	auto hash       = HashData( buffer, bufferSize );
	auto hashedNode = nodeBucket[ hash % 256 ];

	out2       = hashedNode;
	hashedNode = hashedNode->m_pHashNext;
	out1       = hashedNode;

	while ( hashedNode != out2 )
	{
		TINT difference = Toshi::TUtil::MemCompare( out1->m_pData, buffer, bufferSize );
		if ( difference == 0 ) break;

		hashedNode = hashedNode->m_pHashNext;
		out1       = hashedNode;
	}

	return out1 != out2;
}

TOSHI_NAMESPACE_END
