#pragma once
#include <type_traits>

#define T2_FOREACH( vecName, iteratorName ) \
	for ( auto iteratorName = ( vecName ).Begin(); iteratorName != ( vecName ).End(); ++iteratorName )

#define T2_FOREACH_BACK( vecName, iteratorName ) \
	for ( auto iteratorName = ( vecName ).Tail(); iteratorName != ( vecName ).End(); --iteratorName )

#define T2_FOREACH_ARRAY( arrName, iteratorName ) \
	for ( TINT iteratorName = 0; iteratorName < TARRAYSIZE( arrName ); ++iteratorName )

#define T2_FOREACH_ARRAY_BACK( arrName, iteratorName ) \
	for ( TINT iteratorName = TARRAYSIZE( arrName ) - 1; iteratorName >= 0; --iteratorName )

#define T2_DEFINE_ITERATOR_FRIEND() \
	template <class K, class Node> friend class Toshi::T2Iterator

#define T2_DEFINE_ITERATOR( TYPE, NODE_TYPE ) \
	using Iterator = Toshi::T2Iterator<TYPE, NODE_TYPE>

TOSHI_NAMESPACE_START

template <class K, class Node>
class T2Iterator
{
public:
	TSTATICASSERT( std::is_base_of<Node, K>::value );

	T2Iterator()
	{
		m_pPtr = TNULL;
	}

	T2Iterator( Node* pPtr )
	{
		m_pPtr = TSTATICCAST( K, pPtr );
	}

	T2Iterator( K* pPtr )
	{
		m_pPtr = pPtr;
	}

	T2Iterator( const T2Iterator& other )
	{
		m_pPtr = other.m_pPtr;
	}

	K* Get()
	{
		return m_pPtr;
	}

	const K* Get() const
	{
		return m_pPtr;
	}

	K& operator*() noexcept
	{
		return *m_pPtr;
	}

	const K& operator*() const noexcept
	{
		return *m_pPtr;
	}

	T2Iterator Next() const
	{
		TASSERT( m_pPtr != TNULL );
		return T2Iterator{ TSTATICCAST( Node, m_pPtr )->Next() };
	}

	T2Iterator Prev() const
	{
		TASSERT( m_pPtr != TNULL );
		return T2Iterator{ TSTATICCAST( Node, m_pPtr )->Prev() };
	}

	void operator=( const T2Iterator& other )
	{
		m_pPtr = other.m_pPtr;
	}

	void operator=( K* pPtr )
	{
		m_pPtr = pPtr;
	}

	K* operator->() const
	{
		TASSERT( m_pPtr != TNULL );
		return m_pPtr;
	}

	operator K*() const
	{
		TASSERT( m_pPtr != TNULL );
		return TSTATICCAST( K, m_pPtr );
	}

	T2Iterator operator++( TINT )
	{
		TASSERT( m_pPtr != TNULL );
		T2Iterator old = m_pPtr;
		m_pPtr         = TSTATICCAST( K, TSTATICCAST( Node, m_pPtr )->Next() );
		return old;
	}

	T2Iterator operator--( TINT )
	{
		TASSERT( m_pPtr != TNULL );
		T2Iterator old = m_pPtr;
		m_pPtr         = TSTATICCAST( K, TSTATICCAST( Node, m_pPtr )->Prev() );
		return old;
	}

	T2Iterator operator++()
	{
		TASSERT( m_pPtr != TNULL );
		m_pPtr = TSTATICCAST( K, TSTATICCAST( Node, m_pPtr )->Next() );
		return T2Iterator{ m_pPtr };
	}

	T2Iterator operator--()
	{
		TASSERT( m_pPtr != TNULL );
		m_pPtr = TSTATICCAST( K, TSTATICCAST( Node, m_pPtr )->Prev() );
		return T2Iterator{ m_pPtr };
	}

private:
	K* m_pPtr;
};

TOSHI_NAMESPACE_END
