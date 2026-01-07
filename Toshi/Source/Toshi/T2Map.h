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
	using Tree      = T2RedBlackTree<Pair, typename Pair::Comparator>;
	using Iterator  = Tree::Iterator;
	using CIterator = Tree::CIterator;
	using Node      = Tree::Node;

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

	template <class... Args>
	ValueType* Emplace( const KeyType& key, Args&&... args )
	{
		Pair     pair{ key, ValueType( std::forward<Args>( args )... ) };
		Iterator result = m_RedBlackTree.Insert( std::move( pair ) );
		return &result.GetValue()->GetSecond();
	}

	ValueType* Insert( const KeyType& key, const ValueType& value )
	{
		Pair     pair{ key, value };
		Iterator result = m_RedBlackTree.Insert( std::move( pair ) );
		return &result.GetValue()->GetSecond();
	}

	ValueType* Insert( const KeyType& key, ValueType&& value )
	{
		Pair     pair{ key, std::move( value ) };
		Iterator result = m_RedBlackTree.Insert( std::move( pair ) );
		return &result.GetValue()->GetSecond();
	}

	void FindAndRemove( const KeyType& key )
	{
		Iterator result = m_RedBlackTree.Find( { key } );

		TASSERT( result != End() );
		m_RedBlackTree.Delete( result.GetNode() );
	}

	void FindLastAndRemove( const KeyType& key )
	{
		Iterator result = m_RedBlackTree.FindLast( { key } );

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

	Iterator FindLast( const KeyType& key )
	{
		return m_RedBlackTree.FindLast( { key } );
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

	T2Allocator* GetAllocator() const
	{
		return m_RedBlackTree.GetAllocator();
	}

private:
	Tree m_RedBlackTree;
};

TOSHI_NAMESPACE_END
