#pragma once
#include "T2Pair.h"
#include "T2RedBlackTree.h"
#include "Toshi/TComparator.h"

TOSHI_NAMESPACE_START

template <class KeyType, class ValueType, class Comparator = TComparator<KeyType>>
class T2Map
{
public:
	using Pair     = T2Pair<KeyType, ValueType, Comparator>;
	using Iterator = T2RedBlackTree<Pair>::Iterator;
	using Node     = T2RedBlackTree<Pair>::Node;

public:
	T2Map( T2Allocator* a_pAllocator = GetGlobalAllocator() ) :
	    m_RedBlackTree( a_pAllocator )
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

	void RemoveNode( Node* a_pNode )
	{
		TASSERT( TNULL != a_pNode && a_pNode != End() );
		m_RedBlackTree.Delete( a_pNode );
	}

	ValueType* Find( const KeyType& key )
	{
		Iterator result = m_RedBlackTree.Find( { key } );
		return &result.GetValue()->GetSecond();
	}

	Node* FindNode( const KeyType& key )
	{
		Iterator result = m_RedBlackTree.Find( { key } );
		return result.GetNode();
	}

	Node* FindNext( Node* a_pNode, const KeyType& a_rKey )
	{
		Iterator result = m_RedBlackTree.FindNext( a_pNode, { a_rKey } );
		return result.GetValue();
	}

	Node* FindNextNode( Node* a_pNode, const KeyType& a_rKey )
	{
		Iterator result = m_RedBlackTree.FindNext( a_pNode, { a_rKey } );
		return result.GetNode();
	}

	Iterator Begin()
	{
		return m_RedBlackTree.Begin();
	}

	Iterator End()
	{
		return m_RedBlackTree.End();
	}

private:
	T2RedBlackTree<Pair> m_RedBlackTree;
};

TOSHI_NAMESPACE_END
