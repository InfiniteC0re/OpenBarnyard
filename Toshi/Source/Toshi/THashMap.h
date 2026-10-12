#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Hash map with a fixed number of buckets of singly linked nodes.
// The bucket of a key is a_rcKey % number of buckets.
// NOTE: JPOG exports Toshi::THashMap<K, V, THash<K>, TEqual<K>>; Barnyard only
// uses THashMap<TINT, TManagedModel*> (AGateManager::Create), so the hash and
// equality functors are folded into the key's operator% and operator==.
//-----------------------------------------------------------------------------
template <class KEY, class VALUE>
class THashMap
{
public:
	struct Node
	{
		Node( const KEY& a_rcKey, const VALUE& a_rcValue )
		    : oKey( a_rcKey ), oValue( a_rcValue ), pNext( TNULL )
		{}

		// $Barnyard: FUNCTION 005dd9a0
		~Node()
		{
			if ( pNext )
				delete pNext;
		}

		KEY   oKey;   // 0x00
		VALUE oValue; // 0x04
		Node* pNext;  // 0x08
	};

public:
	THashMap( TINT a_iNumBuckets )
	    : m_iNumItems( 0 ), m_iNumBuckets( 0 ), m_ppBuckets( TNULL )
	{
		Create( a_iNumBuckets );
	}

	~THashMap()
	{
		Clear();
		delete[] m_ppBuckets;
	}

	// Deletes every node and allocates a_iNumBuckets empty buckets
	void Create( TINT a_iNumBuckets )
	{
		Clear();
		delete[] m_ppBuckets;

		m_ppBuckets = new Node*[ a_iNumBuckets ];

		for ( TINT i = 0; i < a_iNumBuckets; i++ )
			m_ppBuckets[ i ] = TNULL;

		m_iNumBuckets = a_iNumBuckets;
		m_iNumItems   = 0;
	}

	// Deletes every node (the buckets are kept)
	// $Barnyard: FUNCTION 005ddb00
	void Clear()
	{
		if ( m_ppBuckets )
		{
			for ( TINT i = 0; i < m_iNumBuckets; i++ )
			{
				if ( m_ppBuckets[ i ] )
				{
					delete m_ppBuckets[ i ];
					m_ppBuckets[ i ] = TNULL;
				}
			}
		}

		m_iNumItems = 0;
	}

	// Sets the value of a_rcKey, adds a node at the end of its bucket if there's none
	// $Barnyard: FUNCTION 005de4e0
	Node* Insert( const KEY& a_rcKey, const VALUE& a_rcValue )
	{
		Node** ppBucket = &m_ppBuckets[ a_rcKey % m_iNumBuckets ];
		Node*  pPrev    = TNULL;

		for ( Node* pNode = *ppBucket; pNode; pNode = pNode->pNext )
		{
			if ( pNode->oKey == a_rcKey )
			{
				pNode->oValue = a_rcValue;
				return pNode;
			}

			pPrev = pNode;
		}

		Node* pNode = new Node( a_rcKey, a_rcValue );

		if ( pPrev )
			pPrev->pNext = pNode;
		else
			*ppBucket = pNode;

		m_iNumItems++;
		return pNode;
	}

	// Returns the node of a_rcKey, TNULL if there's none
	Node* Find( const KEY& a_rcKey ) const
	{
		for ( Node* pNode = m_ppBuckets[ a_rcKey % m_iNumBuckets ]; pNode; pNode = pNode->pNext )
		{
			if ( pNode->oKey == a_rcKey )
				return pNode;
		}

		return TNULL;
	}

	TINT GetNumItems() const { return m_iNumItems; }

private:
	TINT   m_iNumItems;   // 0x00
	TINT   m_iNumBuckets; // 0x04
	Node** m_ppBuckets;   // 0x08
};

TOSHI_NAMESPACE_END
