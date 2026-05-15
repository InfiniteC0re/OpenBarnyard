#pragma once

#define TARRAY_FOREACH( vecName, iteratorName ) \
	for ( auto iteratorName = vecName.Begin(); !iteratorName.IsOver(); iteratorName++ )

#define TARRAY_FOREACH_BACK( vecName, iteratorName ) \
	for ( auto iteratorName = vecName.Tail(); !iteratorName.IsOver(); iteratorName-- )

TOSHI_NAMESPACE_START

template <class T>
class TGenericArray
{
public:
	class Iterator
	{
	public:
		friend TGenericArray;
		using Storage = TGenericArray;

	public:
		Iterator()
		{
			m_iIndex  = 0;
			m_poArray = TNULL;
		}

		Iterator( const Iterator& other )
		{
			m_poArray = other.m_poArray;
			m_iIndex  = 0;
		}

		Iterator( Storage& a_poArray )
		{
			m_iIndex  = 0;
			m_poArray = &a_poArray;
		}

		Iterator( TINT a_iIndex, Storage& a_poArray )
		{
			m_iIndex  = a_iIndex;
			m_poArray = &a_poArray;
		}

		TINT Index() const
		{
			return m_iIndex;
		}

		void Push( const T& element )
		{
			TASSERT( m_poArray );
			m_poArray->Push( element );
		}

		T* operator->() const
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_poArray );
			TASSERT( m_iIndex < m_poArray->m_iNumElements );
			return &m_poArray->m_pData[ m_iIndex ];
		}

		T& Get()
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_poArray );
			TASSERT( m_iIndex < m_poArray->m_iNumElements );
			return m_poArray->m_pData[ m_iIndex ];
		}

		bool IsOver() const
		{
			return ( m_iIndex >= m_poArray->Size() || m_iIndex == -1 );
		}

		const T& Get() const
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_poArray );
			TASSERT( m_iIndex < m_poArray->m_iNumElements );
			return m_poArray->m_pData[ m_iIndex ];
		}

		TBOOL operator==( const Iterator& a_rIt ) const
		{
			return ( m_iIndex == a_rIt.m_iIndex ) && ( m_poArray == a_rIt.m_poArray );
		}

		Iterator& operator++()
		{
			m_iIndex++;
			return *this;
		}

		Iterator operator++( TINT )
		{
			Iterator temp = *this;
			m_iIndex++;
			return temp;
		}

		Iterator& operator--()
		{
			m_iIndex--;
			return *this;
		}

		Iterator operator--( TINT )
		{
			Iterator temp = *this;
			m_iIndex--;
			return temp;
		}

		Iterator operator+( TINT a_iValue ) const
		{
			return Iterator( m_iIndex + a_iValue, m_poArray );
		}

		Iterator operator-( TINT a_iValue ) const
		{
			return Iterator( m_iIndex - a_iValue, m_poArray );
		}

		// T2SortedVector compatibility
		Iterator( TINT a_iIndex, Storage* a_poArray )
		{
			m_iIndex  = a_iIndex;
			m_poArray = a_poArray;
		}

		T& Value()
		{
			return Get();
		}

		const T& Value() const
		{
			return Get();
		}

		TBOOL operator!=( const Iterator& a_rIt ) const
		{
			return !( *this == a_rIt );
		}

		T& operator*()
		{
			return Get();
		}

		const T& operator*() const
		{
			return Get();
		}

	private:
		TINT     m_iIndex;  // 0x0
		Storage* m_poArray; // 0x4
	};

public:
	TGenericArray( TINT a_iGrowSize, TINT a_iSize )
	{
		m_iGrowSize         = a_iGrowSize;
		m_iNumAllocElements = a_iSize;
		m_iNumElements      = 0;

		if ( m_iNumAllocElements > 0 )
		{
			m_pData = TSTATICCAST( T, TMemalign( m_iNumAllocElements * sizeof( T ), alignof( T ) ) );
		}
		else
		{
			TASSERT( m_iGrowSize != 0 );
			m_pData = TNULL;
		}
	}

	~TGenericArray()
	{
		if ( m_pData )
		{
			TFree( m_pData );
		}
	}

	void Resize( TINT a_iNewSize )
	{
		if ( a_iNewSize != 0 )
		{
			T*     pNewBuffer = TSTATICCAST( T, TMemalign( a_iNewSize * sizeof( T ), alignof( T ) ) );
			size_t uiCopySize = TMath::Min( m_iNumElements, a_iNewSize );

			TUtil::MemCopy( pNewBuffer, m_pData, sizeof( T ) * uiCopySize );

			m_iNumAllocElements = a_iNewSize;
			TASSERT( m_iNumElements <= m_iNumAllocElements );

			if ( m_pData ) TFree( m_pData );
			m_pData = pNewBuffer;
		}
		else
		{
			if ( m_pData ) TFree( m_pData );
			m_pData             = TNULL;
			m_iNumAllocElements = 0;
			m_iNumElements      = 0;
		}
	}

	void Clear()
	{
		if ( m_iNumAllocElements < 0 )
		{
			if ( m_pData )
			{
				TFree( m_pData );
				m_pData = TNULL;
			}

			m_iNumAllocElements = 0;
		}

		m_iNumElements = 0;
	}

	Iterator Begin()
	{
		return Iterator( ( m_iNumElements == 0 ) ? -1 : 0, *this );
	}

	Iterator Tail()
	{
		return Iterator( ( m_iNumElements == 0 ) ? -1 : m_iNumElements - 1, *this );
	}

	Iterator End()
	{
		return Iterator( -1, *this );
	}

	TINT Size() const
	{
		return m_iNumElements;
	}

	void SetSize( TINT a_iSize )
	{
		m_iNumElements = a_iSize;
	}

	TINT SizeAllocated() const
	{
		return m_iNumAllocElements;
	}

	TINT GetGrowSize() const
	{
		return m_iGrowSize;
	}

	void SetGrowSize( TINT a_iGrowSize )
	{
		m_iGrowSize = a_iGrowSize;
	}

	T* Push( const T& element = T() )
	{
		GrowBy( 1 );
		return TConstruct( &m_pData[ m_iNumElements++ ], element );
	}

	template <class... Args>
	T* EmplaceBack( Args&&... args )
	{
		GrowBy( 1 );
		return TConstruct( &m_pData[ m_iNumElements++ ], std::forward<Args>( args )... );
	}

	T& Pop()
	{
		TASSERT( m_iNumElements >= 1 );
		return m_pData[ --m_iNumElements ];
	}

	T& operator[]( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < m_iNumElements );
		return m_pData[ a_iIndex ];
	}

	const T& operator[]( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < m_iNumElements );
		return m_pData[ a_iIndex ];
	}

	// T2SortedVector compatibility
	Iterator Front()
	{
		TASSERT( m_iNumElements > 0 );
		return Iterator( 0, *this );
	}

	Iterator Back()
	{
		TASSERT( m_iNumElements > 0 );
		return Iterator( m_iNumElements - 1, *this );
	}

	TINT Capacity() const
	{
		return m_iNumAllocElements;
	}

	TBOOL IsEmpty() const
	{
		return Size() == 0;
	}

	Iterator InsertBefore( Iterator a_itInsertBefore, const T& a_rcItem = T() )
	{
		TINT iIndex = a_itInsertBefore.Index();
		TASSERT( iIndex >= 0 );
		TASSERT( iIndex <= m_iNumElements );

		InsertGap( iIndex, 1 );
		TConstruct( &m_pData[ iIndex ], a_rcItem );

		return Iterator( iIndex, *this );
	}

	Iterator InsertAfter( Iterator a_itInsertAfter, const T& a_rcItem = T() )
	{
		return InsertBefore( Iterator( a_itInsertAfter.Index() + 1, *this ), a_rcItem );
	}

	void PopBack()
	{
		TASSERT( m_iNumElements >= 1 );
		m_pData[ --m_iNumElements ].~T();
	}

	void PopFront()
	{
		TASSERT( m_iNumElements >= 1 );
		Erase( Begin() );
	}

	Iterator Find( const T& a_rcValue )
	{
		for ( auto it = Begin(); !it.IsOver(); it++ )
		{
			if ( it.Value() == a_rcValue )
				return it;
		}

		return End();
	}

	void Erase( const Iterator& a_rIterator )
	{
		TINT iIndex = a_rIterator.Index();
		TASSERT( iIndex >= 0 );
		TASSERT( iIndex < m_iNumElements );

		m_pData[ iIndex ].~T();

		for ( TINT i = iIndex + 1; i < m_iNumElements; i++ )
		{
			TConstruct( &m_pData[ i - 1 ], std::move( m_pData[ i ] ) );
			m_pData[ i ].~T();
		}

		m_iNumElements--;
	}

	void FindAndErase( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			Erase( it );
	}

	void EraseFast( const Iterator& a_rIterator )
	{
		TINT iIndex = a_rIterator.Index();
		TASSERT( iIndex >= 0 );
		TASSERT( iIndex < m_iNumElements );

		m_pData[ iIndex ].~T();

		if ( iIndex != m_iNumElements - 1 )
		{
			TConstruct( &m_pData[ iIndex ], std::move( m_pData[ m_iNumElements - 1 ] ) );
			m_pData[ m_iNumElements - 1 ].~T();
		}

		m_iNumElements--;
	}

	void FindAndEraseFast( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			EraseFast( it );
	}

	T& At( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < m_iNumElements );
		return m_pData[ a_iIndex ];
	}

	const T& At( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < m_iNumElements );
		return m_pData[ a_iIndex ];
	}

private:
	// T2SortedVector compatibility
	void InsertGap( TINT a_iIndex, TINT a_iCount )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex <= m_iNumElements );
		TASSERT( a_iCount >= 0 );

		if ( a_iCount == 0 )
			return;

		GrowBy( a_iCount );

		for ( TINT i = m_iNumElements - 1; i >= a_iIndex; i-- )
		{
			TConstruct( &m_pData[ i + a_iCount ], std::move( m_pData[ i ] ) );
			m_pData[ i ].~T();
		}

		m_iNumElements += a_iCount;
	}

	void GrowBy( TINT a_iGrowBy )
	{
		if ( m_iNumAllocElements < m_iNumElements + a_iGrowBy )
		{
			TASSERT( m_iGrowSize != 0 );

			auto iNewSize = TMath::Max( m_iNumAllocElements + m_iGrowSize, m_iNumElements + a_iGrowBy );
			Resize( iNewSize );
		}
	}

private:
	TINT m_iGrowSize;         // 0x0
	TINT m_iNumElements;      // 0x4
	TINT m_iNumAllocElements; // 0x8
	T*   m_pData;             // 0xC
};

template <class T>
class TArray : public TGenericArray<T>
{
public:
	TArray( TINT a_iGrowSize = 10, TINT a_iSize = 0 )
	    : TGenericArray<T>( a_iGrowSize, a_iSize )
	{
	}
};

TOSHI_NAMESPACE_END
