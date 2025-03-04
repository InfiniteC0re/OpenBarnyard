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
	this->m_SomeArray1  = TNULL;
	this->m_Buckets     = TNULL;
	this->m_BucketCount = 0;
}

BTECCompressor::~BTECCompressor()
{
	TFree( m_SomeArray1 );

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
	m_SomeArray1     = (Node*)TMemalign( arraySize1 * sizeof( Node ), 16 );

	for ( TSIZE i = 0; i < arraySize1; i++ )
	{
		m_SomeArray1[ i ].m_Ptr1         = &m_Root1;
		m_SomeArray1[ i ].m_Ptr2         = m_Root1.m_Ptr2;
		m_SomeArray1[ i ].m_Ptr1->m_Ptr2 = &m_SomeArray1[ i ];
		m_SomeArray1[ i ].m_Ptr2->m_Ptr1 = &m_SomeArray1[ i ];
	}

	TUtil::MemClear( m_Offsets, sizeof( m_Offsets ) );

	m_Buckets = (Bucket*)TMemalign( m_BucketCount * sizeof( Bucket ), 16 );

	for ( TSIZE i = 0; i < m_BucketCount; i++ )
	{
		m_Buckets[ i ] = (Bucket)TMemalign( 256 * sizeof( Node* ), 16 );

		for ( TSIZE k = 0; k < 256; k++ )
		{
			Node* someNode      = new Node();
			someNode->m_Unk1    = someNode;
			someNode->m_Unk2    = someNode;
			m_Buckets[ i ][ k ] = someNode;
		}
	}
}

void BTECCompressor::FUN_0068ae40( TSIZE dataSize )
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

		auto node = m_Root2.m_Ptr1;
		while ( node != &m_Root2 )
		{
			if ( m_Buffer2 <= node->m_Unk3 ) break;
			auto nextNode = node->m_Ptr1;

			node->m_Unk1->m_Unk2 = node->m_Unk2;
			node->m_Unk2->m_Unk1 = node->m_Unk1;
			node->m_Unk1         = node;
			node->m_Unk2         = node;
			node->m_Ptr1->m_Ptr2 = node->m_Ptr2;
			node->m_Ptr2->m_Ptr1 = node->m_Ptr1;
			node->m_Ptr1         = &m_Root1;
			node->m_Ptr2         = m_Root1.m_Ptr2;
			m_Root1.m_Ptr2       = node;
			node->m_Ptr2->m_Ptr1 = node;

			node = nextNode;
		}
	}

	while ( oldBuffer3 < m_Buffer3 )
	{
		AllocSubstring( oldBuffer3 );
		oldBuffer3++;
	}
}

TBOOL BTECCompressor::FUN_0068af10( TBYTE* buffer, TSIZE bufferSize, TBYTE*& offset, TSIZE& dataSize )
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
			TBOOL bRes = FUN_0068b300( buffer, m_Buckets[ i ], localSize, localNode1, localNode2 );
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
				someSize = TMath::Min( bufferSize, static_cast<TSIZE>( m_DataEnd - node1->m_Unk3 ) );
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

				while ( node1->m_Unk3[ size ] == buffer[ size ] )
				{
					if ( someSize <= size ) break;
					size++;
				}

				if ( dataSize < size )
				{
					dataSize = size;
					offset   = node1->m_Unk3;

					if ( someSize <= size )
					{
						return TTRUE;
					}
				}

				node1 = node1->m_Unk1;

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

		auto node = m_Root1.m_Ptr1;
		if ( node == &m_Root1 )
		{
			TERROR( "SlidingWindow::AllocSubstring: No free substrings!\n" );
			TBREAK();
			return;
		}

		node->m_Ptr1->m_Ptr2 = node->m_Ptr2;
		node->m_Ptr2->m_Ptr1 = node->m_Ptr1;
		node->m_Ptr1         = &m_Root2;
		node->m_Ptr2         = m_Root2.m_Ptr2;
		m_Root2.m_Ptr2       = node;
		node->m_Ptr2->m_Ptr1 = node;
		node->m_Unk4         = (void*)( m_DataEnd - buffer );
		node->m_Unk3         = buffer;

		TUINT32 hash = HashData( buffer, unknown );

		auto hashedNode      = m_Buckets[ i ][ hash % 256 ];
		node->m_Unk1         = hashedNode;
		node->m_Unk2         = hashedNode->m_Unk2;
		hashedNode->m_Unk2   = node;
		node->m_Unk2->m_Unk1 = node;
	}
}

TBOOL BTECCompressor::FUN_0068b300( TBYTE* buffer, Bucket nodeBucket, TSIZE bufferSize, Node*& out1, Node*& out2 )
{
	auto hash       = HashData( buffer, bufferSize );
	auto hashedNode = nodeBucket[ hash % 256 ];

	out2       = hashedNode;
	hashedNode = hashedNode->m_Unk1;
	out1       = hashedNode;

	while ( hashedNode != out2 )
	{
		TINT difference = Toshi::TUtil::MemCompare( out1->m_Unk3, buffer, bufferSize );
		if ( difference == 0 ) break;

		hashedNode = hashedNode->m_Unk1;
		out1       = hashedNode;
	}

	return out1 != out2;
}

TOSHI_NAMESPACE_END
