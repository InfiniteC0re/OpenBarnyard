#pragma once

TOSHI_NAMESPACE_START

template < typename T, TSIZE MaxSize >
class T2Vector
{
public:
    class Iterator
    {
    public:
        friend class T2Vector;

    public:
        constexpr Iterator( T2Vector* a_pVector ) :
            m_uiIndex( 0 ), m_pVector( a_pVector ) {}
        constexpr Iterator( TSIZE a_uiIndex, T2Vector* a_pVector ) :
            m_uiIndex( a_uiIndex ), m_pVector( a_pVector ) {}
        constexpr Iterator( const Iterator& a_rOther ) :
            m_uiIndex( a_rOther.m_uiIndex ), m_pVector( a_rOther.m_pVector ) {}

        constexpr TSIZE Index() const noexcept
        {
            return m_uiIndex;
        }

        T* Pointer() noexcept
        {
            return &m_pVector->At( m_uiIndex );
        }

        const T* Pointer() const noexcept
        {
            return &m_pVector->At( m_uiIndex );
        }

        T& Value() noexcept
        {
            return m_pVector->At( m_uiIndex );
        }

        const T& Value() const noexcept
        {
            return m_pVector->At( m_uiIndex );
        }

        T& operator*() noexcept
        {
            return m_pVector->At( m_uiIndex );
        }

        const T& operator*() const noexcept
        {
            return m_pVector->At( m_uiIndex );
        }

        T* operator->() noexcept
        {
            return &m_pVector->At( m_uiIndex );
        }

        const T* operator->() const noexcept
        {
            return &m_pVector->At( m_uiIndex );
        }

        TBOOL operator==( const Iterator& a_rOther ) const noexcept
        {
            return a_rOther.m_pVector == m_pVector && a_rOther.m_uiIndex == m_uiIndex;
        }

        TBOOL operator>( const Iterator& a_rOther ) const noexcept
        {
            return a_rOther.m_pVector == m_pVector && a_rOther.m_uiIndex > m_uiIndex;
        }

        TBOOL operator>=( const Iterator& a_rOther ) const noexcept
        {
            return a_rOther.m_pVector == m_pVector && a_rOther.m_uiIndex >= m_uiIndex;
        }

        TBOOL operator<( const Iterator& a_rOther ) const noexcept
        {
            return a_rOther.m_pVector == m_pVector && a_rOther.m_uiIndex < m_uiIndex;
        }

        TBOOL operator<=( const Iterator& a_rOther ) const noexcept
        {
            return a_rOther.m_pVector == m_pVector && a_rOther.m_uiIndex <= m_uiIndex;
        }

        constexpr Iterator operator++( TINT ) noexcept
        {
            Iterator temp = *this;
            m_uiIndex++;
            return temp;
        }

        constexpr Iterator operator--( TINT ) noexcept
        {
            Iterator temp = *this;
            m_uiIndex--;
            return temp;
        }

        constexpr Iterator& operator++() noexcept
        {
            m_uiIndex++;
            return *this;
        }

        constexpr Iterator& operator--() noexcept
        {
            m_uiIndex--;
            return *this;
        }

        constexpr Iterator& operator=( const Iterator& a_rOther ) noexcept
        {
            m_uiIndex = a_rOther.m_uiIndex;
            m_pVector = a_rOther.m_pVector;
        }

        Iterator operator+( TSIZE a_uiValue ) const noexcept
        {
            TASSERT( m_uiIndex + a_uiValue < m_pVector->Size() );
            return Iterator( m_uiIndex + a_uiValue, m_pVector );
        }

        Iterator operator-( TSIZE a_uiValue ) const noexcept
        {
            TASSERT( m_uiIndex - a_uiValue < m_pVector->Size() );
            return Iterator( m_uiIndex - a_uiValue, m_pVector );
        }

        Iterator& operator+=( TSIZE a_uiValue ) const noexcept
        {
            TASSERT( m_uiIndex + a_uiValue < m_pVector->Size() );
            m_uiIndex += a_uiValue;
            return *this;
        }

        Iterator& operator-=( TSIZE a_uiValue ) const noexcept
        {
            TASSERT( m_uiIndex - a_uiValue < m_pVector->Size() );
            m_uiIndex -= a_uiValue;
            return *this;
        }

    private:
        TSIZE     m_uiIndex;
        T2Vector* m_pVector;
    };

public:
    constexpr static TSIZE CAPACITY = MaxSize;

public:
    T2Vector()
    {
        m_iNumElements = 0;
    }

    ~T2Vector()
    {
        Clear();
    }

    template < class... Args >
    T* EmplaceBack( Args&&... args )
    {
        TASSERT( m_iNumElements < CAPACITY );
        return TConstruct< T >( &AtUnsafe( m_iNumElements++ ), std::forward< Args >( args )... );
    }

    void PushBack( const T& item = T() )
    {
        TASSERT( m_iNumElements < CAPACITY );
        AtUnsafe( m_iNumElements++ ) = item;
    }

    void PopBack()
    {
        TASSERT( m_iNumElements > 0 );
        AtUnsafe( --m_iNumElements ).~T();
    }

    void Clear()
    {
        for ( TSIZE i = 0; i < Size(); i++ )
            AtUnsafe( i ).~T();

        m_iNumElements = 0;
    }

    void Copy( const T2Vector< T, MaxSize >& a_rcCopyFrom )
    {
        TSIZE i;
        for ( i = 0; i < a_rcCopyFrom.Size(); i++ )
        {
            AtUnsafe( i ) = a_rcCopyFrom.At( i );
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

    void Erase( const Iterator& a_rIterator )
    {
        TSIZE uiItemIndex = a_rIterator.Index();
        TASSERT( uiItemIndex < m_iNumElements );

        if ( uiItemIndex + 1 < Size() )
        {
            for ( TSIZE i = uiItemIndex + 1; i < Size(); i++ )
                AtUnsafe( i - 1 ) = std::move( At( i ) );
        }
        else
        {
            AtUnsafe( uiItemIndex ).~T();
        }

        m_iNumElements--;
    }

    TSIZE Size() const
    {
        return m_iNumElements;
    }

    constexpr TSIZE Capacity() const
    {
        return CAPACITY;
    }

    TBOOL IsEmpty() const
    {
        return Size() == 0;
    }

    T& Front()
    {
        TASSERT( m_iNumElements > 0 );
        return AtUnsafe( 0 );
    }

    const T& Front() const
    {
        TASSERT( m_iNumElements > 0 );
        return AtUnsafe( 0 );
    }

    T& Back()
    {
        TASSERT( m_iNumElements > 0 );
        return AtUnsafe( m_iNumElements - 1 );
    }

    const T& Back() const
    {
        TASSERT( m_iNumElements > 0 );
        return AtUnsafe( m_iNumElements - 1 );
    }

    constexpr Iterator Begin()
    {
        return Iterator( this );
    }

    constexpr Iterator End()
    {
        return Iterator( m_iNumElements, this );
    }

    T& At( TSIZE a_uiIndex )
    {
        TASSERT( a_uiIndex < m_iNumElements );
        return *(T*)( m_aBuffer + a_uiIndex * sizeof( T ) );
    }

    const T& At( TSIZE a_uiIndex ) const
    {
        TASSERT( a_uiIndex < m_iNumElements );
        return *(const T*)( m_aBuffer + a_uiIndex * sizeof( T ) );
    }

    T& operator[]( TSIZE a_uiIndex )
    {
        return At( a_uiIndex );
    }

    const T& operator[]( TSIZE a_uiIndex ) const
    {
        return At( a_uiIndex );
    }

private:
    TFORCEINLINE constexpr T& AtUnsafe( TSIZE a_uiIndex )
    {
        return *( (T*)( m_aBuffer ) + a_uiIndex );
    }

    TFORCEINLINE constexpr const T& AtUnsafe( TSIZE a_uiIndex ) const
    {
        return *( (T*)( m_aBuffer ) + a_uiIndex );
    }

private:
    TSIZE m_iNumElements;
    TBYTE m_aBuffer[ MaxSize * sizeof( T ) ];
};

TOSHI_NAMESPACE_END