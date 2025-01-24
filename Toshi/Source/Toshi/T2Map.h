#pragma once
#include "T2Pair.h"
#include "T2RedBlackTree.h"
#include "Toshi/TComparator.h"

TOSHI_NAMESPACE_START

template <class KeyType, class ValueType, class Comparator = TComparator<KeyType>>
class T2Map
{
public:
	using Pair      = T2Pair<KeyType, ValueType, Comparator>;
	using Iterator  = T2RedBlackTree<Pair>::Iterator;
	using CIterator = T2RedBlackTree<Pair>::CIterator;
	using Node      = T2RedBlackTree<Pair>::Node;

public:
	T2Map( T2Allocator* a_pAllocator = GetGlobalAllocator() )
	    : m_RedBlackTree( a_pAllocator )
	{
	}

	~T2Map()
	{
		Clear();
	}

	void Clear()
	{
		m_RedBlackTree.DeleteAll();
	}

	TSIZE Size() const
	{
		return m_RedBlackTree.Size();
	}

	ValueType* Insert( const KeyType& key, const ValueType& value )
	{
		Iterator result = m_RedBlackTree.Insert( { key, value } );
		return &result.GetValue()->GetSecond();
	}

	void Remove( const KeyType& key )
	{
		Iterator result = m_RedBlackTree.Find( { key } );

		TASSERT( result != End() );
		m_RedBlackTree.Delete( result.GetNode() );
	}

	void Remove( Iterator& it )
	{
		TASSERT( it != End() );
		m_RedBlackTree.Delete( it.GetNode() );
	}

	Iterator FindByValue( const ValueType& value )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it.GetValue()->GetSecond() == value )
			{
				return it;
			}
		}

		return End();
	}

	Iterator Find( const KeyType& key )
	{
		return m_RedBlackTree.Find( { key } );
	}

	Iterator FindNext( Iterator a_oIterator, const KeyType& a_rKey )
	{
		return m_RedBlackTree.FindNext( a_oIterator.GetNode(), { a_rKey } );
	}

	TBOOL IsValid( Iterator a_oIterator ) const
	{
		return a_oIterator != End();
	}

	Iterator Begin()
	{
		return m_RedBlackTree.Begin();
	}

	Iterator End()
	{
		return m_RedBlackTree.End();
	}

	const CIterator Begin() const
	{
		return m_RedBlackTree.Begin();
	}

	const CIterator End() const
	{
		return m_RedBlackTree.End();
	}

	TBOOL IsEmpty() const
	{
		return Begin() == End();
	}

	Iterator operator[]( const KeyType& key )
	{
		return m_RedBlackTree.Find( { key } );
	}

private:
	T2RedBlackTree<Pair> m_RedBlackTree;
};

TOSHI_NAMESPACE_END
