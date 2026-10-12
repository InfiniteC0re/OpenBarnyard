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

		// $Barnyard: FUNCTION 0051bce0
		TBOOL operator==( const Iterator& a_rOther ) const noexcept
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex == m_iIndex;
		}

		// Shared (identical code folding) by the instantiations: 00513e50
		// $Barnyard: FUNCTION 00513e50
		TBOOL operator!=( const Iterator& a_rOther ) const noexcept
		{
			return m_iIndex != a_rOther.m_iIndex || m_pVector != a_rOther.m_pVector;
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

		// Shared (identical code folding) by the instantiations: 00479c20
		// $Barnyard: FUNCTION 00479c20
		constexpr Iterator& operator=( const Iterator& a_rOther ) noexcept
		{
			if ( this != &a_rOther )
			{
				m_pVector = a_rOther.m_pVector;
				m_iIndex  = a_rOther.m_iIndex;
			}

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

	// $Barnyard: FUNCTION 005ddb60
	Iterator InsertBefore( Iterator a_itInsertBefore, const T& a_rcItem = T(), TBOOL a_bUseMemMove = TTRUE )
	{
		InsertGap( a_itInsertBefore.Index(), 1, a_bUseMemMove );
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

	// $Barnyard: FUNCTION 00573510
	Iterator PushBack( const T& item = T() )
	{
		TASSERT( m_iNumElements < CAPACITY );
		TConstruct<T>( &AtUnsafe( m_iNumElements++ ), item );

		return Iterator( m_iNumElements - 1, this );
	}

	// $Barnyard: FUNCTION 00517bb0
	// $Barnyard: FUNCTION 0057cf00
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

	// $Barnyard: FUNCTION 00517c20
	// $Barnyard: FUNCTION 0057cf70
	// $Barnyard: FUNCTION 0057eca0
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

	// $Barnyard: FUNCTION 0062fba0
	// T2Vector<TVector4, 6>: 004a5430
	// $Barnyard: FUNCTION 004a5430
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
	// $Barnyard: FUNCTION 005141e0
	// $Barnyard: FUNCTION 0051c230
	// $Barnyard: FUNCTION 0062fb60
	// $Barnyard: FUNCTION 005ddba0
	Iterator Erase( const Iterator& a_rIterator )
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
		return Iterator( uiItemIndex, this );
	}

	// Returns the item if it's stored in the vector or TNULL otherwise (for pointer types)
	// $Barnyard: FUNCTION 00573600
	T FindItem( T a_Item )
	{
		auto it = Find( a_Item );

		if ( it == End() )
			return TNULL;

		return it.Value();
	}

	// Finds and erases element preserving order
	// $Barnyard: FUNCTION 005735a0
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

	// T2Vector<AGameState*, 10> (calls the folded Begin 004ab160): 00429680
	// $Barnyard: FUNCTION 00429680
	constexpr Iterator Begin()
	{
		return Iterator( this );
	}

	// Returns an iterator to the element at a_iIndex
	// NOTE: Retail folds it with the same helper of the other index based containers and
	// calls it e.g. from T2SortedVector::FindInsertionPoint and ACollisionModel::CollideRay (inlined in our code)
	// $Barnyard: FUNCTION 0061a720
	Iterator GetIterator( TINT a_iIndex )
	{
		return Iterator( a_iIndex, this );
	}

	// $Barnyard: FUNCTION 00575090
	// T2Vector<AGameState*, 10> (calls the folded End 00575090): 004296a0
	// $Barnyard: FUNCTION 004296a0
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

	// $Barnyard: FUNCTION 00639ba0
	// Instantiations: T2Vector<AGateManager::OpeningGate, 8> 005dd9d0
	// $Barnyard: FUNCTION 005dd9d0
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
