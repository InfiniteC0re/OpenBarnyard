#pragma once
#include "Toshi/T2Allocator.h"

TOSHI_NAMESPACE_START

class TOSHI_API T2GenericDynamicVector
{
protected:
	T2GenericDynamicVector( T2Allocator* a_pAllocator, TINT a_iInitialSize, TINT a_iGrowSize, TINT a_iElementSize )
	{
		m_pAllocator   = a_pAllocator;
		m_iGrowSize    = a_iGrowSize;
		m_iNumElements = 0;
		m_iAllocSize   = a_iInitialSize;
		m_poElements   = ( a_iInitialSize > 0 ) ? a_pAllocator->Malloc( a_iInitialSize * a_iElementSize ) : TNULL;
	}

	T2GenericDynamicVector( void* a_poElements, TINT a_iInitialSize, TINT a_iNumElements )
	{
		m_pAllocator   = TNULL;
		m_iGrowSize    = 0;
		m_iNumElements = a_iNumElements;
		m_iAllocSize   = a_iInitialSize;
		m_poElements   = a_poElements;
	}

	void Reallocate( TINT a_iNewSize, TINT a_iElementSize );
	void Grow( TINT a_iNumElements, TINT a_iElementSize );

protected:
	T2Allocator* m_pAllocator;   // 0x0
	TINT         m_iGrowSize;    // 0x4
	TINT         m_iNumElements; // 0x8
	TINT         m_iAllocSize;   // 0xC
	void*        m_poElements;   // 0x10
};

template <class T>
class T2DynamicVector : public T2GenericDynamicVector
{
public:
	class Iterator
	{
	public:
		friend T2DynamicVector;

	public:
		constexpr Iterator()
		    : m_iIndex( 0 ), m_pVector( TNULL ) {}
		constexpr Iterator( T2DynamicVector* a_pVector )
		    : m_iIndex( 0 ), m_pVector( a_pVector ) {}
		constexpr Iterator( T2DynamicVector& a_rVector )
		    : m_iIndex( 0 ), m_pVector( &a_rVector ) {}
		constexpr Iterator( TINT a_iIndex, T2DynamicVector* a_pVector )
		    : m_iIndex( a_iIndex ), m_pVector( a_pVector ) {}
		constexpr Iterator( const Iterator& a_rOther )
		    : m_iIndex( a_rOther.m_iIndex ), m_pVector( a_rOther.m_pVector ) {}

		constexpr TINT Index() const
		{
			return m_iIndex;
		}

		TBOOL IsValid() const
		{
			return *this != m_pVector->End();
		}

		T* Get() noexcept
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_pVector );
			return &m_pVector->At( m_iIndex );
		}

		const T* Get() const noexcept
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_pVector );
			return &m_pVector->At( m_iIndex );
		}

		T& Value()
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_pVector );
			return m_pVector->At( m_iIndex );
		}

		const T& Value() const
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_pVector );
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

		T* operator->()
		{
			return &Value();
		}

		const T* operator->() const
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

		constexpr Iterator& operator=( const Iterator& a_rOther ) noexcept
		{
			m_iIndex  = a_rOther.m_iIndex;
			m_pVector = a_rOther.m_pVector;
			return *this;
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
			return Iterator( m_iIndex + a_iValue, m_pVector );
		}

		Iterator operator-( TINT a_iValue ) const
		{
			return Iterator( m_iIndex - a_iValue, m_pVector );
		}

	private:
		TINT             m_iIndex;  // 0x0
		T2DynamicVector* m_pVector; // 0x4
	};

public:
	constexpr T2DynamicVector( T2Allocator* a_pAllocator = GetGlobalAllocator(), TINT a_iInitialSize = 0, TINT a_iGrowSize = -1 )
	    : T2GenericDynamicVector( a_pAllocator, a_iInitialSize, a_iGrowSize, sizeof( T ) )
	{}

	constexpr T2DynamicVector( T* a_poElements, TINT a_iInitialSize, TINT a_iNumElements )
	    : T2GenericDynamicVector( a_poElements, a_iInitialSize, a_iNumElements )
	{}

	~T2DynamicVector()
	{
		FreeMemory();
	}

	Iterator InsertBefore( Iterator a_itInsertBefore, const T& a_rcItem = T() )
	{
		InsertGap( a_itInsertBefore.Index(), 1, sizeof( T ) );
		TConstruct<T>( &AtUnsafe( a_itInsertBefore.Index() ), a_rcItem );

		return Iterator( a_itInsertBefore.Index(), this );
	}

	Iterator InsertAfter( Iterator a_itInsertAfter, const T& a_rcItem = T() )
	{
		InsertGap( a_itInsertAfter.Index() + 1, 1, sizeof( T ) );
		TConstruct<T>( &AtUnsafe( a_itInsertAfter.Index() + 1 ), a_rcItem );

		return Iterator( a_itInsertAfter.Index() + 1, this );
	}

	void Clear()
	{
		for ( TINT i = 0; i < m_iNumElements; i++ )
		{
			( (T*)m_poElements + i )->~T();
		}

		m_iNumElements = 0;
	}

	void FreeMemory()
	{
		Clear();

		if ( m_pAllocator )
			Reallocate( 0, sizeof( T ) );
		else
			m_poElements = TNULL;
	}

	void Reserve( TINT a_iSize )
	{
		if ( a_iSize > m_iAllocSize )
		{
			Reallocate( a_iSize, sizeof( T ) );
		}
	}

	template <class... Args>
	T* EmplaceBack( Args&&... args )
	{
		Grow( 1, sizeof( T ) );
		return TConstruct<T>( &AtUnsafe( m_iNumElements++ ), std::forward<Args>( args )... );
	}

	void PushBack( const T& item = T() )
	{
		Grow( 1, sizeof( T ) );
		TConstruct<T>( &AtUnsafe( m_iNumElements++ ), item );
	}

	void PopBack()
	{
		TASSERT( m_iNumElements > 0 );
		AtUnsafe( --m_iNumElements ).~T();
	}

	Iterator Find( const T& a_rcValue )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it.Value() == a_rcValue )
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
	void EraseFast( const Iterator& a_rIterator )
	{
		TINT uiItemIndex = a_rIterator.Index();
		TASSERT( uiItemIndex < m_iNumElements );

		AtUnsafe( uiItemIndex ) = Back().Value();
		PopBack();
	}

	// Finds and erases element ignoring order but with a faster algorithm
	void FindAndEraseFast( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			EraseFast( it );
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

	Iterator Begin()
	{
		return Iterator( 0, this );
	}

	Iterator End()
	{
		return Iterator( m_iNumElements, this );
	}

	Iterator MakeIterator( TINT a_iIndex )
	{
		return Iterator( a_iIndex, this );
	}

	TINT Size() const
	{
		return m_iNumElements;
	}

	void SetSize( TINT a_iNewSize )
	{
		Reserve( a_iNewSize );
		m_iNumElements = a_iNewSize;
	}

	TINT Capacity() const
	{
		return m_iAllocSize;
	}

	TBOOL IsEmpty() const
	{
		return Size() == 0;
	}

	T& At( TINT a_iIndex )
	{
		TASSERT( a_iIndex < m_iNumElements );
		return *( TREINTERPRETCAST( T*, m_poElements ) + a_iIndex );
	}

	const T& At( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumElements );
		return *( TREINTERPRETCAST( T*, m_poElements ) + a_iIndex );
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
		return *( TREINTERPRETCAST( T*, m_poElements ) + a_iIndex );
	}

	TFORCEINLINE constexpr const T& AtUnsafe( TINT a_iIndex ) const
	{
		return *( TREINTERPRETCAST( T*, m_poElements ) + a_iIndex );
	}

	void InsertGap( TINT a_iGapAt, TINT a_iGapSize, TINT a_iElementSize, TBOOL a_bUseMemMove = TFALSE )
	{
		if ( m_iAllocSize < m_iNumElements + a_iGapSize )
		{
			TINT iNewSize = m_iAllocSize + a_iGapSize;

			if ( m_iGrowSize == -1 )
				iNewSize = TMath::Max( iNewSize, m_iAllocSize * 2 );
			else
				iNewSize = TMath::Max( iNewSize, m_iAllocSize + m_iGrowSize );

			TCHAR* pNewBuffer = (TCHAR*)m_pAllocator->Malloc( iNewSize * a_iElementSize );

			if ( m_poElements )
			{
				TINT iNumElementsBeforeGap = a_iGapAt;
				TINT iNumElementsAfterGap  = m_iNumElements - a_iGapAt;

				if ( a_bUseMemMove )
				{
					// Unsafe way but suitable with simple types

					// Copy elements before the gap
					if ( iNumElementsBeforeGap > 0 )
						TUtil::MemCopy( pNewBuffer, m_poElements, iNumElementsBeforeGap * a_iElementSize );

					// Copy elements after the gap
					if ( iNumElementsAfterGap > 0 )
						TUtil::MemCopy( pNewBuffer + ( a_iGapAt + a_iGapSize ) * a_iElementSize, TREINTERPRETCAST( TCHAR*, m_poElements ) + a_iGapAt * a_iElementSize, iNumElementsAfterGap * a_iElementSize );
				}
				else
				{
					// This is much safer

					// Copy elements before the gap
					for ( TINT i = 0; i < iNumElementsBeforeGap; i++ )
					{
						TConstruct<T>( TREINTERPRETCAST( T*, pNewBuffer + ( i * a_iElementSize ) ), std::move( AtUnsafe( i ) ) );
						AtUnsafe( i ).~T();
					}

					// Copy elements after the gap
					for ( TINT i = a_iGapAt + a_iGapSize, k = a_iGapAt; k < m_iNumElements; i++, k++ )
					{
						TConstruct<T>( TREINTERPRETCAST( T*, pNewBuffer + ( i * a_iElementSize ) ), std::move( AtUnsafe( k ) ) );
						AtUnsafe( k ).~T();
					}
				}

				// Free the old buffer
				m_pAllocator->Free( m_poElements );
			}

			m_poElements   = pNewBuffer;
			m_iAllocSize   = iNewSize;
			m_iNumElements = m_iNumElements + a_iGapSize;

			return;
		}
		else if ( m_iNumElements > a_iGapAt )
		{
			if ( a_bUseMemMove )
			{
				// Unsafe way but suitable with simple types
				// Note: Seems that in Barnyard there's no other way of adding a gap but in newer Toshi branches there is so I'm adding it here too anyways
				TUtil::MemMove( &AtUnsafe( a_iGapAt + a_iGapSize ), &AtUnsafe( a_iGapAt ), ( m_iNumElements - a_iGapAt ) * a_iElementSize );
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
};

TOSHI_NAMESPACE_END
