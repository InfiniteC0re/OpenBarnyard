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

class MODAPI T2GenericRedBlackTreeNode
{
public:
	friend class T2GenericRedBlackTree;

	template <class T, class C>
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

class MODAPI T2GenericRedBlackTree
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
	template <class T, class C>
	friend class T2RedBlackTree;

public:
	// ANamedAnimationSet map (T2Map<TPString8, ANamedAnimation*>) 00581330, ANamedAnimationTransitionSet map (T2Map<TPString8, ANamedAnimationTransition>) 00581d40
	// $Barnyard: FUNCTION 00581330
	// $Barnyard: FUNCTION 00581d40
	// AInputMapManager maps (<AInputCommand, Command> 00597520, <Doodad, TPString8> 00597580)
	// $Barnyard: FUNCTION 00597520
	// $Barnyard: FUNCTION 00597580
	// $Barnyard: FUNCTION 00606d70
	// Instantiations: T2Map<TPString8, TINT/TKeyframeLibrary*> 00606d70 (AKeyFrameLibraryManager, AMaterialLibraryManager),
	// T2Map<TPString8, TINT/TSIZE> 005a3b70 (AInteractionManager, ASoundManager), T2Map<TPString8, TUINT> 005d1a50 (AUnitFlagSetsManager), T2Map<TPString8, AWaveBank*> 005dc7c0 (ASoundManager::ms_WaveBanks)
	// $Barnyard: FUNCTION 005a3b70
	// $Barnyard: FUNCTION 005d1a50
	// $Barnyard: FUNCTION 005dc7c0
	// Instantiations: T2Map<TPString8, TINT/TKeyframeLibrary*> 00606d70 (AKeyFrameLibraryManager, AMaterialLibraryManager)
	// T2Map<TPString8, AModel*> 006128b0 (AModelRepos), T2Map<TPString8, ...> 006147f0 (AMaterialLibraryManager)
	// $Barnyard: FUNCTION 006128b0
	// $Barnyard: FUNCTION 006147f0
	// Instantiations: T2Map<TPString8, TINT/TKeyframeLibrary*> 00606d70 (AKeyFrameLibraryManager, AMaterialLibraryManager)
	// T2Map<TPString8, AInteriorNetwork::LookupList>: 00553bc0
	// $Barnyard: FUNCTION 00553bc0
	T2RedBlackTreeNode( const T& value )
	    : m_Value( value )
	{
	}

	T2RedBlackTreeNode( T&& value )
	    : m_Value( std::move( value ) )
	{
	}

	// Instantiations: T2Map<TPString8, TINT/TKeyframeLibrary*> 00606d00 (AKeyFrameLibraryManager), T2Map<TPString8, AMaterialLibrary*> 00614180 (AMaterialLibraryManager)
	// ANamedAnimationTransitionSet map (T2Map<TPString8, ANamedAnimationTransition>) 00581c90
	// $Barnyard: FUNCTION 00581c90
	// $Barnyard: FUNCTION 00606d00
	// $Barnyard: FUNCTION 00614180
	// AInputMapManager maps (<Doodad, TPString8> 005974b0)
	// $Barnyard: FUNCTION 005974b0
	// $Barnyard: FUNCTION 005a3b00
	// $Barnyard: FUNCTION 005d19e0
	// $Barnyard: FUNCTION 005dc750
	// T2Map<TPString8, AModel*> 00612840 (AModelRepos)
	// $Barnyard: FUNCTION 00612840
	~T2RedBlackTreeNode()
	{
	}

	T* GetValue()
	{
		return &m_Value;
	}

private:
	T m_Value;
};

template <class T, class C = TComparator<T>>
class T2RedBlackTree : public T2GenericRedBlackTree
{
public:
	using Type             = T;
	using Comparator       = C;
	using ComparatorHelper = TComparatorHelperT<Type, Comparator>;
	using Node             = T2RedBlackTreeNode<Type>;

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

		TFORCEINLINE Type* GetValue()
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE TBOOL operator==( const Iterator& other ) const
		{
			return m_pNode == other.m_pNode;
		}

		TFORCEINLINE Type& operator*() const
		{
			return m_pNode->m_Value;
		}

		TFORCEINLINE Type* operator->() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE operator Type*() const
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
	// Instantiations: unreferenced instance next to AWaveBank 005dc290
	// $Barnyard: FUNCTION 005dc290
	T2RedBlackTree( T2Allocator* pAllocator = GetGlobalAllocator() )
	    : T2GenericRedBlackTree( pAllocator )
	{
	}

	TBOOL IsRoot( Node* pNode )
	{
		return pNode == &m_oRoot;
	}

	// NOTE: every instantiation is folded into one function
	// $Barnyard: FUNCTION 005a22e0
	Iterator Begin()
	{
		return Iterator( TSTATICCAST( Node, GetFirstNode() ) );
	}

	// NOTE: every instantiation is folded into one function
	// $Barnyard: FUNCTION 00596830
	Iterator End()
	{
		return Iterator( TSTATICCAST( Node, &m_oRoot ) );
	}

	const CIterator Begin() const
	{
		return CIterator( TSTATICCAST( const Node, GetFirstNode() ) );
	}

	const CIterator End() const
	{
		return CIterator( TSTATICCAST( const Node, &m_oRoot ) );
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

	// Instantiations: T2Map<TPString8, TINT/TKeyframeLibrary*> 00606f80 (AKeyFrameLibraryManager), T2Map<TPString8, AMaterialLibrary*> 006141f0 (AMaterialLibraryManager)
	// $Barnyard: FUNCTION 00606f80
	// $Barnyard: FUNCTION 006141f0
	// ANamedAnimationTransitionSet map (T2Map<TPString8, ANamedAnimationTransition>) 00581fa0
	// $Barnyard: FUNCTION 00581fa0
	// AInputMap (T2Map<Doodad, CommandBinding>) 00596b20
	// $Barnyard: FUNCTION 00596b20
	// $Barnyard: FUNCTION 005a55f0
	// $Barnyard: FUNCTION 005d1cf0
	// T2Map<TPString8, AModel*> 00612ac0 (AModelRepos)
	// $Barnyard: FUNCTION 00612ac0
	void Erase( Node* a_pFrom, Node* a_pTo )
	{
		Node* pNode = a_pFrom;

		while ( pNode != a_pTo )
		{
			Node* pNext = TSTATICCAST( Node, GetSuccessorOf( pNode ) );
			Delete( pNode );
			pNode = pNext;
		}
	}

	// NOTE: retail takes the value by copy and destroys it after constructing the node
	// Instantiations: T2Map<TPString8, TINT/TKeyframeLibrary*> 00606dc0 (AKeyFrameLibraryManager, AMaterialLibraryManager),
	// T2Map<TINT, ASound*/ASoundLegacy*> 005d7110 (ASoundManager)
	// $Barnyard: FUNCTION 00606dc0
	// $Barnyard: FUNCTION 005d7110
	// ANamedAnimationSet map (T2Map<TPString8, ANamedAnimation*>) 00581420, ANamedAnimationTransitionSet map (T2Map<TPString8, ANamedAnimationTransition>) 00581e20
	// $Barnyard: FUNCTION 00581420
	// $Barnyard: FUNCTION 00581e20
	// AInputMap (T2Map<Doodad, CommandBinding>) 00596840, AInputMapManager maps (<Doodad, ActionId>) 00596e80
	// $Barnyard: FUNCTION 00596840
	// $Barnyard: FUNCTION 00596e80
	// $Barnyard: FUNCTION 005a4480
	// $Barnyard: FUNCTION 005d1b30
	// $Barnyard: FUNCTION 005dcbd0
	// T2Map<TPString8, AModel*> 00612900 (AModelRepos), T2Map<TPString8, ...> 00614840 (AMaterialLibraryManager)
	// $Barnyard: FUNCTION 00612900
	// $Barnyard: FUNCTION 00614840
	Node* CreateNode( const T& a_rValue )
	{
		return m_pAllocator->New<Node>( a_rValue );
	}

	Iterator Insert( const T& value )
	{
		Node* pNode     = CreateNode( value );
		pNode->m_pLeft  = &ms_oNil;
		pNode->m_pRight = &ms_oNil;

		return InsertImpl( pNode );
	}

	// Instantiations: T2Map<TPString8, TINT/TKeyframeLibrary*> 00606e40 (AKeyFrameLibraryManager, AMaterialLibraryManager)
	// $Barnyard: FUNCTION 00606e40
	// ANamedAnimationSet map (T2Map<TPString8, ANamedAnimation*>) 005814a0, ANamedAnimationTransitionSet map (T2Map<TPString8, ANamedAnimationTransition>) 00581e90
	// $Barnyard: FUNCTION 005814a0
	// $Barnyard: FUNCTION 00581e90
	// AInputMap (T2Map<Doodad, CommandBinding>) 00596990, AInputMapManager maps (<Doodad, ActionId>) 00597080
	// $Barnyard: FUNCTION 00596990
	// $Barnyard: FUNCTION 00597080
	// $Barnyard: FUNCTION 006bc250
	// $Barnyard: FUNCTION 005a5210
	// $Barnyard: FUNCTION 005d1bb0
	// $Barnyard: FUNCTION 005dcc50
	// T2Map<TINT, ASound*/ASoundLegacy*> 005d7af0 (ASoundManager)
	// $Barnyard: FUNCTION 005d7af0
	// T2Map<TPString8, AModel*> 00612980 (AModelRepos), T2Map<TPString8, ...> 006148c0 (AMaterialLibraryManager)
	// $Barnyard: FUNCTION 00612980
	// $Barnyard: FUNCTION 006148c0
	Iterator Insert( T&& value )
	{
		Node* pNode     = CreateNode( value );
		pNode->m_pLeft  = &ms_oNil;
		pNode->m_pRight = &ms_oNil;

		return InsertImpl( pNode );
	}

	// AInputMapManager maps (folded int-keyed instantiations) 00596f80
	// $Barnyard: FUNCTION 00596f80
	// Instantiations: TPString8-keyed maps (folded, used by ASoundManager, AInteractionManager, AInteriorNetwork, ...) 005dc2b0
	// $Barnyard: FUNCTION 005dc2b0
	Iterator Find( const T& value ) const
	{
		Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );

		while ( pCurrentNode != &ms_oNil )
		{
			auto iCmpResult = Comparator()( *pCurrentNode->GetValue(), value );

			if ( TComparatorHelper::IsEqual( iCmpResult ) )
				return pCurrentNode;

			pCurrentNode = TSTATICCAST( Node, ( TComparatorHelper::IsLessOrEqual( iCmpResult ) ? pCurrentNode->m_pRight : pCurrentNode->m_pLeft ) );
		}

		return TSTATICCAST( Node, &m_oRoot );
	}

	// AInputMapManager maps (folded int-keyed instantiations) 00596fd0
	// $Barnyard: FUNCTION 00596fd0
	// $Barnyard: FUNCTION 006bc1d0
	Iterator FindLast( const T& value ) const
	{
		Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );

		while ( pCurrentNode != &ms_oNil )
		{
			auto iCmpResult = Comparator()( *pCurrentNode->GetValue(), value );

			if ( TComparatorHelper::IsEqual( iCmpResult ) )
			{
				// Find the last one
				Node* pLeft = TSTATICCAST( Node, pCurrentNode->m_pLeft );
				while ( pCurrentNode->m_pLeft != &ms_oNil && ComparatorHelper::IsEqual( *pLeft->GetValue(), value ) )
				{
					pCurrentNode = pLeft;
					pLeft        = TSTATICCAST( Node, pCurrentNode->m_pLeft );
				}

				return pCurrentNode;
			}

			pCurrentNode = TSTATICCAST( Node, ( TComparatorHelper::IsLessOrEqual( iCmpResult ) ? pCurrentNode->m_pRight : pCurrentNode->m_pLeft ) );
		}

		return TSTATICCAST( Node, &m_oRoot );
	}

	// AInputMapManager maps (folded int-keyed instantiations) 00597040
	// $Barnyard: FUNCTION 00597040
	Iterator FindNext( const Node* a_pNextAfter, const T& value ) const
	{
		Node* pNode = TSTATICCAST( Node, GetSuccessorOf( a_pNextAfter ) );

		if ( ComparatorHelper::IsEqual( *pNode->GetValue(), value ) )
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
			pInsertTo    = pCurrentNode;
			pCurrentNode = TSTATICCAST( Node, ( ComparatorHelper::IsLessOrEqual( *pCurrentNode->GetValue(), *pNode->GetValue() ) ? pCurrentNode->m_pRight : pCurrentNode->m_pLeft ) );
		}

		pNode->m_pParent = pInsertTo;
		if ( pInsertTo == &m_oRoot || ComparatorHelper::IsGreater( *pInsertTo->GetValue(), *pNode->GetValue() ) )
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
