/**
 * @file T2RedBlackTree.h
 * Red-black tree implementation for the Toshi engine
 * 
 * This file provides a red-black tree implementation that includes:
 * - Balanced binary tree operations
 * - Tree node management
 * - Tree traversal and searching
 * - Tree balancing and maintenance
 */

#pragma once
#include "Toshi/TComparator.h"
#include "T2Allocator.h"

TOSHI_NAMESPACE_START

class T2GenericRedBlackTreeNode
{
public:
	friend class T2GenericRedBlackTree;

	template <class T>
	friend class T2RedBlackTree;

protected:
	constexpr T2GenericRedBlackTreeNode()
	{
		red       = 0;
		m_pLeft   = this;
		m_pRight  = this;
		m_pParent = this;
	}

	T2GenericRedBlackTreeNode( T2GenericRedBlackTreeNode* pNil )
	{
		red       = 0;
		m_pLeft   = pNil;
		m_pRight  = pNil;
		m_pParent = pNil;
	}

protected:
	TINT                       red;
	T2GenericRedBlackTreeNode* m_pLeft;
	T2GenericRedBlackTreeNode* m_pRight;
	T2GenericRedBlackTreeNode* m_pParent;
};

class T2GenericRedBlackTree
{
protected:
	// $Barnyard: FUNCTION 006cefc0
	T2GenericRedBlackTree( T2Allocator* pAllocator )
	    : m_oRoot( ms_oNil )
	{
		m_pAllocator   = pAllocator;
		m_iNumElements = 0;
	}

	~T2GenericRedBlackTree()
	{
		TASSERT( m_iNumElements == 0 );
	}

	T2Allocator* GetAllocator() const
	{
		return m_pAllocator;
	}

	size_t GetNumElements() const
	{
		return m_iNumElements;
	}

	T2GenericRedBlackTreeNode* GetFirstNode() const;
	T2GenericRedBlackTreeNode* Insert( T2GenericRedBlackTreeNode* pNode );
	void                       DeleteFixUp( T2GenericRedBlackTreeNode* pNode );
	T2GenericRedBlackTreeNode* DeleteNode( T2GenericRedBlackTreeNode* pNode );

	void                              LeftRotate( T2GenericRedBlackTreeNode* pNode );
	void                              RightRotate( T2GenericRedBlackTreeNode* pNode );
	static T2GenericRedBlackTreeNode* GetSuccessorOf( const T2GenericRedBlackTreeNode* pNode );
	static T2GenericRedBlackTreeNode* GetPredecessorOf( const T2GenericRedBlackTreeNode* pNode );

private:
	void CheckValid()
	{
		TASSERT( ms_oNil.red == 0 );
		TASSERT( m_oRoot.red == 0 );
	}

protected:
	static constinit T2GenericRedBlackTreeNode ms_oNil;

protected:
	T2Allocator*                      m_pAllocator;
	mutable T2GenericRedBlackTreeNode m_oRoot;
	size_t                            m_iNumElements;
};

template <class T>
class T2RedBlackTreeNode : public T2GenericRedBlackTreeNode
{
public:
	template <class T>
	friend class T2RedBlackTree;

public:
	T2RedBlackTreeNode( const T& value )
	    : m_Value( value )
	{
	}

	T2RedBlackTreeNode( T&& value )
	    : m_Value( std::move( value ) )
	{
	}

	T* GetValue()
	{
		return &m_Value;
	}

	TBOOL IsLeftNodeNext( const T& value ) const
	{
		// Insert same values to the right side
		if ( TComparator<T>::IsEqual( m_Value, value ) )
			return TFALSE;

		return TComparator<T>::IsLess( m_Value, value );
	}

	TBOOL operator==( const T& other ) const
	{
		return TComparator<T>::IsEqual( m_Value, other );
	}

	TBOOL operator==( const T2RedBlackTreeNode<T>& other ) const
	{
		return TComparator<T>::IsEqual( m_Value, other.m_Value );
	}

	TBOOL operator!=( const T& other ) const
	{
		return !TComparator<T>::IsEqual( m_Value, other );
	}

	TBOOL operator!=( const T2RedBlackTreeNode<T>& other ) const
	{
		return !TComparator<T>::IsEqual( m_Value, other.m_Value );
	}

	TBOOL operator>( const T2RedBlackTreeNode<T>& other ) const
	{
		return TComparator<T>::IsGreater( m_Value, other.m_Value );
	}

	TBOOL operator<( const T2RedBlackTreeNode<T>& other ) const
	{
		return TComparator<T>::IsLess( m_Value, other.m_Value );
	}

	TBOOL operator<=( const T2RedBlackTreeNode<T>& other ) const
	{
		return TComparator<T>::IsLessOrEqual( m_Value, other.m_Value );
	}

	TBOOL operator>=( const T2RedBlackTreeNode<T>& other ) const
	{
		return TComparator<T>::IsGreaterOrEqual( m_Value, other.m_Value );
	}

private:
	T m_Value;
};

template <class T>
class T2RedBlackTree : public T2GenericRedBlackTree
{
public:
	using Node = T2RedBlackTreeNode<T>;

	class Iterator
	{
	public:
		TFORCEINLINE Iterator( Node* a_pNode )
		    : m_pNode( a_pNode ) {}

		TFORCEINLINE Iterator Next()
		{
			return TSTATICCAST( Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
		}

		TFORCEINLINE Iterator Prev()
		{
			return TSTATICCAST( Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
		}

		TFORCEINLINE Node* GetNode() const
		{
			return m_pNode;
		}

		TFORCEINLINE T* GetValue()
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE TBOOL operator==( const Iterator& other ) const
		{
			return m_pNode == other.m_pNode;
		}

		TFORCEINLINE T& operator*() const
		{
			return m_pNode->m_Value;
		}

		TFORCEINLINE T* operator->() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE operator T*() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE Iterator operator++( TINT )
		{
			Iterator curr = *this;
			m_pNode       = TSTATICCAST( Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
			return curr;
		}

		TFORCEINLINE Iterator& operator++()
		{
			m_pNode = TSTATICCAST( Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
			return *this;
		}

		TFORCEINLINE Iterator operator--( TINT )
		{
			Iterator curr = *this;
			m_pNode       = TSTATICCAST( Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
			return curr;
		}

		TFORCEINLINE Iterator& operator--()
		{
			m_pNode = TSTATICCAST( Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
			return *this;
		}

	private:
		Node* m_pNode;

		friend class CIterator;
	};

	class CIterator
	{
	public:
		TFORCEINLINE CIterator( const Node* a_pNode )
		    : m_pNode( a_pNode ) {}

		TFORCEINLINE CIterator( const Iterator& a_rcIterator )
		    : m_pNode( a_rcIterator.m_pNode ) {}

		TFORCEINLINE CIterator Next()
		{
			CIterator next = *this;
			m_pNode        = TSTATICCAST( const Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
			return next;
		}

		TFORCEINLINE CIterator Prev()
		{
			CIterator prev = *this;
			m_pNode        = TSTATICCAST( const Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
			return prev;
		}

		TFORCEINLINE const Node* GetNode() const
		{
			return m_pNode;
		}

		TFORCEINLINE const T* GetValue() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE TBOOL operator==( const Iterator& other ) const
		{
			return m_pNode == other.GetNode();
		}

		TFORCEINLINE TBOOL operator==( const CIterator& other ) const
		{
			return m_pNode == other.m_pNode;
		}

		TFORCEINLINE const T& operator*() const
		{
			return m_pNode->m_Value;
		}

		TFORCEINLINE const T* operator->() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE operator const T*() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE CIterator operator++( TINT )
		{
			return Next();
		}

		TFORCEINLINE CIterator& operator++()
		{
			m_pNode = TSTATICCAST( const Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
			return *this;
		}

		TFORCEINLINE CIterator operator--( TINT )
		{
			return Prev();
		}

		TFORCEINLINE CIterator& operator--()
		{
			m_pNode = TSTATICCAST( const Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
			return *this;
		}

	private:
		const Node* m_pNode;

		friend class Iterator;
	};

public:
	T2RedBlackTree( T2Allocator* pAllocator = GetGlobalAllocator() )
	    : T2GenericRedBlackTree( pAllocator )
	{
	}

	TBOOL IsRoot( Node* pNode )
	{
		return pNode == &m_oRoot;
	}

	Iterator Begin()
	{
		return Iterator( TREINTERPRETCAST( Node*, GetFirstNode() ) );
	}

	Iterator End()
	{
		return Iterator( TREINTERPRETCAST( Node*, &m_oRoot ) );
	}

	const CIterator Begin() const
	{
		return CIterator( TREINTERPRETCAST( const Node*, GetFirstNode() ) );
	}

	const CIterator End() const
	{
		return CIterator( TREINTERPRETCAST( const Node*, &m_oRoot ) );
	}

	void DeleteAll()
	{
		while ( GetNumElements() > 0 )
		{
			Delete( TSTATICCAST( Node, GetFirstNode() ) );
		}
	}

	void Delete( Node* a_pNode )
	{
		m_pAllocator->Delete( TSTATICCAST( Node, DeleteNode( a_pNode ) ) );
	}

	void Erase( Node* a_pFrom, Node* a_pTo )
	{
		Node* pNode = a_pFrom;

		while ( pNode != a_pTo )
		{
			auto pNext = GetSuccessorOf( pNode );
			Delete( pNode );
			pNode = pNext;
		}
	}

	Iterator Insert( const T& value )
	{
		Node* pNode     = m_pAllocator->New<Node>( value );
		pNode->m_pLeft  = &ms_oNil;
		pNode->m_pRight = &ms_oNil;

		return InsertImpl( pNode );
	}

	Iterator Insert( T&& value )
	{
		Node* pNode     = m_pAllocator->New<Node>( value );
		pNode->m_pLeft  = &ms_oNil;
		pNode->m_pRight = &ms_oNil;

		return InsertImpl( pNode );
	}

	Iterator Find( const T& value ) const
	{
		Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );

		while ( pCurrentNode != &ms_oNil )
		{
			if ( pCurrentNode->operator==( value ) )
			{
				return pCurrentNode;
			}

			if ( pCurrentNode->IsLeftNodeNext( value ) )
			{
				pCurrentNode = TSTATICCAST( Node, pCurrentNode->m_pLeft );
			}
			else
			{
				pCurrentNode = TSTATICCAST( Node, pCurrentNode->m_pRight );
			}
		}

		return TSTATICCAST( Node, &m_oRoot );
	}

	Iterator FindNext( const Node* a_pNextAfter, const T& value ) const
	{
		Node* pNode = TSTATICCAST( Node, GetSuccessorOf( a_pNextAfter ) );

		if ( pNode->operator==( value ) )
		{
			return pNode;
		}

		return TSTATICCAST( Node, &m_oRoot );
	}

	TSIZE Size() const
	{
		return GetNumElements();
	}

	T2Allocator* GetAllocator() const
	{
		return m_pAllocator;
	}

private:
	Iterator InsertImpl( Node* pNode )
	{
		Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );
		Node* pInsertTo    = TSTATICCAST( Node, &m_oRoot );

		while ( pCurrentNode != &ms_oNil )
		{
			pInsertTo = pCurrentNode;

			if ( pCurrentNode->IsLeftNodeNext( *pNode->GetValue() ) )
			{
				pCurrentNode = TSTATICCAST( Node, pCurrentNode->m_pLeft );
			}
			else
			{
				pCurrentNode = TSTATICCAST( Node, pCurrentNode->m_pRight );
			}
		}

		pNode->m_pParent = pInsertTo;

		if ( pInsertTo == &m_oRoot || pInsertTo->IsLeftNodeNext( *pNode->GetValue() ) )
		{
			pInsertTo->m_pLeft = pNode;
		}
		else
		{
			pInsertTo->m_pRight = pNode;
		}

		TASSERT( ms_oNil.red == 0, "ms_oNil not red in T2GenericRedBlackTree::TreeInsertHelp" ); // TreeInsertHelp????
		T2GenericRedBlackTree::Insert( pNode );

		return pNode;
	}
};

TOSHI_NAMESPACE_END
