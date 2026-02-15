#pragma once

TOSHI_NAMESPACE_START

template <typename T, TINT MaxSize>
class T2Vector
{
public:
	class Iterator
	{
	public:
		friend class T2Vector;

	public:
		constexpr Iterator()
		    : m_iIndex( 0 ), m_pVector( TNULL ) {}
		constexpr Iterator( T2Vector* a_pVector )
		    : m_iIndex( 0 ), m_pVector( a_pVector ) {}
		constexpr Iterator( TINT a_iIndex, T2Vector* a_pVector )
		    : m_iIndex( a_iIndex ), m_pVector( a_pVector ) {}
		constexpr Iterator( const Iterator& a_rOther )
		    : m_iIndex( a_rOther.m_iIndex ), m_pVector( a_rOther.m_pVector ) {}

		constexpr TINT Index() const noexcept
		{
			return m_iIndex;
		}

		T* Get() noexcept
		{
			return &m_pVector->At( m_iIndex );
		}

		const T* Get() const noexcept
		{
			return &m_pVector->At( m_iIndex );
		}

		T& Value() noexcept
		{
			return m_pVector->At( m_iIndex );
		}

		const T& Value() const noexcept
		{
			return m_pVector->At( m_iIndex );
		}

		Iterator Next() const
		{
			Iterator temp = *this;
			temp.m_iIndex += 1;
			return temp;
		}

		Iterator Prev() const
		{
			Iterator temp = *this;
			temp.m_iIndex -= 1;
			return temp;
		}

		operator T*()
		{
			return &Value();
		}

		operator const T*() const
		{
			return &Value();
		}

		T& operator*() noexcept
		{
			return Value();
		}

		const T& operator*() const noexcept
		{
			return Value();
		}

		T* operator->() noexcept
		{
			return &Value();
		}

		const T* operator->() const noexcept
		{
			return &Value();
		}

		TBOOL operator==( const Iterator& a_rOther ) const noexcept
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex == m_iIndex;
		}

		TBOOL operator>( const Iterator& a_rOther ) const noexcept
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex > m_iIndex;
		}

		TBOOL operator>=( const Iterator& a_rOther ) const noexcept
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex >= m_iIndex;
		}

		TBOOL operator<( const Iterator& a_rOther ) const noexcept
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex < m_iIndex;
		}

		TBOOL operator<=( const Iterator& a_rOther ) const noexcept
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex <= m_iIndex;
		}

		constexpr Iterator operator++( TINT ) noexcept
		{
			Iterator temp = *this;
			m_iIndex++;
			return temp;
		}

		constexpr Iterator operator--( TINT ) noexcept
		{
			Iterator temp = *this;
			m_iIndex--;
			return temp;
		}

		constexpr Iterator& operator++() noexcept
		{
			m_iIndex++;
			return *this;
		}

		constexpr Iterator& operator--() noexcept
		{
			m_iIndex--;
			return *this;
		}

		constexpr Iterator& operator=( const Iterator& a_rOther ) noexcept
		{
			m_iIndex  = a_rOther.m_iIndex;
			m_pVector = a_rOther.m_pVector;
			return *this;
		}

		Iterator operator+( TINT a_uiValue ) const noexcept
		{
			TASSERT( m_iIndex + a_uiValue < m_pVector->Size() );
			return Iterator( m_iIndex + a_uiValue, m_pVector );
		}

		Iterator operator-( TINT a_uiValue ) const noexcept
		{
			TASSERT( m_iIndex - a_uiValue < m_pVector->Size() );
			return Iterator( m_iIndex - a_uiValue, m_pVector );
		}

		Iterator& operator+=( TINT a_uiValue ) noexcept
		{
			TASSERT( m_iIndex + a_uiValue < m_pVector->Size() );
			m_iIndex += a_uiValue;
			return *this;
		}

		Iterator& operator-=( TINT a_uiValue ) noexcept
		{
			TASSERT( m_iIndex - a_uiValue < m_pVector->Size() );
			m_iIndex -= a_uiValue;
			return *this;
		}

	private:
		TINT      m_iIndex;
		T2Vector* m_pVector;
	};

public:
	constexpr static TINT CAPACITY = MaxSize;

public:
	T2Vector()
	{
		m_iNumElements = 0;
	}

	~T2Vector()
	{
		Clear();
	}

	Iterator InsertBefore( Iterator a_itInsertBefore, const T& a_rcItem = T() )
	{
		InsertGap( a_itInsertBefore.Index(), 1 );
		TConstruct<T>( &AtUnsafe( a_itInsertBefore.Index() ), a_rcItem );

		return Iterator( a_itInsertBefore.Index(), this );
	}

	Iterator InsertAfter( Iterator a_itInsertAfter, const T& a_rcItem = T() )
	{
		InsertGap( a_itInsertAfter.Index() + 1, 1 );
		TConstruct<T>( &AtUnsafe( a_itInsertAfter.Index() + 1 ), a_rcItem );

		return Iterator( a_itInsertAfter.Index() + 1, this );
	}

	template <class... Args>
	Iterator EmplaceBack( Args&&... args )
	{
		TASSERT( m_iNumElements < CAPACITY );
		TConstruct<T>( &AtUnsafe( m_iNumElements++ ), std::forward<Args>( args )... );

		return Iterator( m_iNumElements - 1, this );
	}

	Iterator PushBack( const T& item = T() )
	{
		TASSERT( m_iNumElements < CAPACITY );
		TConstruct<T>( &AtUnsafe( m_iNumElements++ ), item );

		return Iterator( m_iNumElements - 1, this );
	}

	void PopBack()
	{
		TASSERT( m_iNumElements > 0 );
		AtUnsafe( --m_iNumElements ).~T();
	}

	void PopFront()
	{
		TASSERT( m_iNumElements > 0 );
		Erase( Begin() );
	}

	void Clear()
	{
		for ( TINT i = 0; i < Size(); i++ )
			AtUnsafe( i ).~T();

		m_iNumElements = 0;
	}

	void Copy( const T2Vector<T, MaxSize>& a_rcCopyFrom )
	{
		TINT i;
		for ( i = 0; i < a_rcCopyFrom.Size(); i++ )
		{
			TConstruct<T>( &AtUnsafe( i ), a_rcCopyFrom.At( i ) );
		}

		if ( Size() > a_rcCopyFrom.Size() )
		{
			for ( ; i < Size(); i++ )
			{
				AtUnsafe( i ).~T();
			}
		}

		m_iNumElements = a_rcCopyFrom.Size();
	}

	Iterator Find( const T& a_rValue )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it.Value() == a_rValue )
				return it;
		}

		return End();
	}

	// Erases element preserving order
	void Erase( const Iterator& a_rIterator )
	{
		TINT uiItemIndex = a_rIterator.Index();
		TASSERT( uiItemIndex < m_iNumElements );

		AtUnsafe( uiItemIndex ).~T();

		if ( uiItemIndex + 1 < Size() )
		{
			for ( TINT i = uiItemIndex + 1; i < Size(); i++ )
			{
				TConstruct<T>( &AtUnsafe( i - 1 ), std::move( AtUnsafe( i ) ) );
				AtUnsafe( i ).~T();
			}
		}

		m_iNumElements--;
	}

	// Finds and erases element preserving order
	void FindAndErase( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			Erase( it );
	}

	// Erases element ignoring order but with a faster algorithm
	void EraseFast( Iterator& a_rIterator )
	{
		TINT uiItemIndex = a_rIterator.Index();
		TASSERT( uiItemIndex < m_iNumElements );

		a_rIterator.Value() = Back().Value();
		PopBack();
	}

	// Finds and erases element ignoring order but with a faster algorithm
	void FindAndEraseFast( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			EraseFast( it );
	}

	TINT Size() const
	{
		return m_iNumElements;
	}

	constexpr TINT Capacity() const
	{
		return CAPACITY;
	}

	TBOOL IsEmpty() const
	{
		return Size() == 0;
	}

	Iterator Front()
	{
		TASSERT( m_iNumElements > 0 );
		return Iterator( 0, this );
	}

	Iterator Back()
	{
		TASSERT( m_iNumElements > 0 );
		return Iterator( m_iNumElements - 1, this );
	}

	constexpr Iterator Begin()
	{
		return Iterator( this );
	}

	constexpr Iterator End()
	{
		return Iterator( m_iNumElements, this );
	}

	T& At( TINT a_iIndex )
	{
		TASSERT( a_iIndex < m_iNumElements );
		return *(T*)( m_aBuffer + a_iIndex * sizeof( T ) );
	}

	const T& At( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumElements );
		return *(const T*)( m_aBuffer + a_iIndex * sizeof( T ) );
	}

	T& operator[]( TINT a_iIndex )
	{
		return At( a_iIndex );
	}

	const T& operator[]( TINT a_iIndex ) const
	{
		return At( a_iIndex );
	}

private:
	TFORCEINLINE constexpr T& AtUnsafe( TINT a_iIndex )
	{
		return *( (T*)( m_aBuffer ) + a_iIndex );
	}

	TFORCEINLINE constexpr const T& AtUnsafe( TINT a_iIndex ) const
	{
		return *( (T*)( m_aBuffer ) + a_iIndex );
	}

	void InsertGap( TINT a_iGapAt, TINT a_iGapSize, TBOOL a_bUseMemMove = TTRUE )
	{
		TASSERT( m_iNumElements + a_iGapSize <= CAPACITY );

		if ( m_iNumElements > a_iGapAt )
		{
			if ( a_bUseMemMove )
			{
				// Unsafe way but suitable with simple types
				TUtil::MemMove( &AtUnsafe( a_iGapAt + a_iGapSize ), &AtUnsafe( a_iGapAt ), ( m_iNumElements - a_iGapAt ) * sizeof( T ) );
			}
			else
			{
				// This is much safer
				for ( TINT i = m_iNumElements - 1, k = i + a_iGapSize; i >= a_iGapAt; i--, k-- )
				{
					TConstruct<T>( &AtUnsafe( k ), std::move( AtUnsafe( i ) ) );
					AtUnsafe( i ).~T();
				}
			}
		}

		m_iNumElements += a_iGapSize;
	}

private:
	TINT  m_iNumElements;
	TBYTE m_aBuffer[ MaxSize * sizeof( T ) ];
};

TOSHI_NAMESPACE_END
