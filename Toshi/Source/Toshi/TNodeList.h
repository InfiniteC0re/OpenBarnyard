#pragma once

TOSHI_NAMESPACE_START

template <class T>
class TNodeList
{
public:
	class TNode
	{
	protected:
		friend TNodeList;

	public:
		constexpr TNode()
		    : m_pList( TNULL ), m_pNext( TSTATICCAST( T, this ) ), m_pPrev( TSTATICCAST( T, this ) )
		{}

		constexpr TNode( const TNode& a_rcNode )
		    : m_pList( a_rcNode.m_pList ), m_pNext( a_rcNode.m_pNext ), m_pPrev( a_rcNode.m_pPrev )
		{}

		TNode( TNode&& a_rNode )
		    : m_pList( a_rNode.m_pList ), m_pNext( a_rNode.m_pNext ), m_pPrev( a_rNode.m_pPrev )
		{
			a_rNode.m_pList = TNULL;
			a_rNode.m_pNext = TNULL;
			a_rNode.m_pPrev = TNULL;
		}

		T* Next() const { return m_pNext; }
		T* Prev() const { return m_pPrev; }

		void Remove() { GetList()->Remove( TSTATICCAST( T, this ) ); }

		void       SetList( TNodeList* list ) { m_pList = list; }
		TNodeList* GetList() const { return m_pList; }

		TBOOL IsLinked() const { return m_pList != TNULL; }

	protected:
		TNodeList* m_pList;
		T*         m_pNext;
		T*         m_pPrev;
	};

	class Iterator
	{
	public:
		constexpr Iterator()
		    : m_pPtr( TNULL )
		{}

		constexpr Iterator( T* a_pPtr )
		    : m_pPtr( a_pPtr )
		{}

		constexpr Iterator( const Iterator& other )
		    : m_pPtr( other.m_pPtr )
		{}

		T* Get() const
		{
			return m_pPtr;
		}

		void operator=( const Iterator& other )
		{
			m_pPtr = other.m_pPtr;
		}

		void operator=( T* pPtr )
		{
			m_pPtr = pPtr;
		}

		T* operator->() const
		{
			TASSERT( m_pPtr != TNULL );
			return m_pPtr;
		}

		operator T*() const
		{
			TASSERT( m_pPtr != TNULL );
			return m_pPtr;
		}

		TBOOL operator==( const Iterator& a_rcIterator ) const
		{
			return m_pPtr == a_rcIterator.m_pPtr;
		}

		// TNodeList<AUnitManager::CharacterUnitNode> 0040f7f0
		// $Barnyard: FUNCTION 0040f7f0
		Iterator operator++( TINT )
		{
			TASSERT( m_pPtr != TNULL );
			Iterator old = m_pPtr;
			m_pPtr       = m_pPtr->Next();
			return old;
		}

		Iterator operator--( TINT )
		{
			TASSERT( m_pPtr != TNULL );
			Iterator old = m_pPtr;
			m_pPtr       = m_pPtr->Prev();
			return old;
		}

		Iterator operator++()
		{
			TASSERT( m_pPtr != TNULL );
			m_pPtr = m_pPtr->Next();
			return Iterator{ m_pPtr };
		}

		Iterator operator--()
		{
			TASSERT( m_pPtr != TNULL );
			m_pPtr = m_pPtr->Prev();
			return Iterator{ m_pPtr };
		}

	protected:
		T* m_pPtr;
	};

public:
	// TNodeList<AUnitProxy> 0040d9f0, folded for the types with the node at offset 0 (AUnit, AUnitHash::CellObject...) 0040ea80
	// $Barnyard: FUNCTION 0040d9f0
	// $Barnyard: FUNCTION 0040ea80
	TNodeList()
	{
		m_iCount = 0;
	}

	// $Barnyard: FUNCTION 00678c70
	// $Barnyard: FUNCTION 006797c0
	// $Barnyard: FUNCTION 0063aa00
	// $Barnyard: FUNCTION 006b55e0
	// $Barnyard: FUNCTION 006d9600
	// $Barnyard: FUNCTION 006c7f50
	// 005a39b0: tail jump to the DeleteAll instance 005a3230 (TNodeList<AInteractionSequence>, AUnitManager)
	// $Barnyard: FUNCTION 005a39b0
	// TNodeList<AUnitProxy> 0040dca0, TNodeList<AUnitManager::CharacterUnitNode> 00410a90
	// $Barnyard: FUNCTION 0040dca0
	// $Barnyard: FUNCTION 00410a90
	~TNodeList()
	{
		DeleteAll();
	}

	void InsertAfter( T* insertAfter, T* newNode )
	{
		TASSERT( !newNode->IsLinked() );
		newNode->SetList( this );
		newNode->m_pNext              = insertAfter->m_pNext;
		newNode->m_pPrev              = insertAfter;
		insertAfter->m_pNext->m_pPrev = newNode;
		insertAfter->m_pNext          = newNode;
		m_iCount++;
	}

	void InsertBefore( T* insertBefore, T* newNode )
	{
		TASSERT( !newNode->IsLinked() );
		newNode->SetList( this );
		newNode->m_pNext               = insertBefore;
		newNode->m_pPrev               = insertBefore->m_pPrev;
		insertBefore->m_pPrev->m_pNext = newNode;
		insertBefore->m_pPrev          = newNode;
		m_iCount++;
	}

	// Instantiations: 005a66b0 (used by 005c8720, 005c8bb0)
	// $Barnyard: FUNCTION 005a66b0
	T* Remove( T* pNode )
	{
		TASSERT( pNode->GetList() == this );
		pNode->SetList( TNULL );
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		pNode->m_pNext          = pNode;
		pNode->m_pPrev          = pNode;
		m_iCount--;
		TASSERT( pNode->IsLinked() == TFALSE );
		return pNode;
	}

	// $Barnyard: FUNCTION 0061f490
	// $Barnyard: FUNCTION 005c8350
	// TNodeList<AUnitProxy> 0040da20
	// $Barnyard: FUNCTION 0040da20
	void RemoveAll()
	{
		while ( !IsEmpty() )
		{
			Remove( m_oRoot.Next() );
		}
	}

	// $Barnyard: FUNCTION 006b4e30
	Iterator RemoveHead()
	{
		if ( !IsEmpty() )
		{
			return Remove( m_oRoot.Next() );
		}

		return TNULL;
	}

	Iterator RemoveTail()
	{
		if ( !IsEmpty() )
		{
			return Remove( m_oRoot.Prev() );
		}

		return TNULL;
	}

	void DeleteHead() { Delete( Head() ); }
	void DeleteTail() { Delete( Tail() ); }

	// $Barnyard: FUNCTION 005a0190
	// $Barnyard: FUNCTION 00678290
	// $Barnyard: FUNCTION 00679770
	// $Barnyard: FUNCTION 005a3230
	// $Barnyard: FUNCTION 0063a180
	// $Barnyard: FUNCTION 0060a830
	// $Barnyard: FUNCTION 0060a880
	// $Barnyard: FUNCTION 0060a8d0
	// $Barnyard: FUNCTION 0060a8e0
	// $Barnyard: FUNCTION 005f7530
	// $Barnyard: FUNCTION 005f4690
	// TNodeList<AUnitProxy> 0040dac0, TNodeList<AUnitType> 0040e320 and TNodeList<AUnitManager::CharacterUnitNode> 004108d0 (their destructors aren't virtual)
	// $Barnyard: FUNCTION 0040dac0
	// $Barnyard: FUNCTION 0040e320
	// $Barnyard: FUNCTION 004108d0
	void DeleteAll()
	{
		while ( !IsEmpty() )
		{
			DeleteHead();
		}
	}

	void Delete( T* pNode )
	{
		Remove( pNode );
		delete pNode;
	}

	// Moves every node of a_rList to the end of this list
	// $Barnyard: FUNCTION 00678c80
	void InsertListTail( TNodeList& a_rList )
	{
		if ( a_rList.IsEmpty() )
			return;

		T* pFirst = a_rList.m_oRoot.m_pNext;
		T* pLast  = a_rList.m_oRoot.m_pPrev;
		T* pTail  = m_oRoot.m_pPrev;

		for ( T* pNode = pFirst; pNode != TSTATICCAST( T, &a_rList.m_oRoot ); pNode = pNode->m_pNext )
			pNode->m_pList = this;

		m_iCount += a_rList.m_iCount;

		pTail->m_pNext  = pFirst;
		pFirst->m_pPrev = pTail;
		pLast->m_pNext  = TSTATICCAST( T, &m_oRoot );
		m_oRoot.m_pPrev = pLast;

		a_rList.m_iCount        = 0;
		a_rList.m_oRoot.m_pPrev = TSTATICCAST( T, &a_rList.m_oRoot );
		a_rList.m_oRoot.m_pNext = TSTATICCAST( T, &a_rList.m_oRoot );
	}

	void InsertHead( T* a_pNode ) { InsertAfter( End(), a_pNode ); }
	void InsertTail( T* a_pNode ) { InsertBefore( End(), a_pNode ); }

	// Instantiations: 005a66e0 (used by 005c8720, 005c8bb0)
	// $Barnyard: FUNCTION 005a66e0
	TBOOL IsEmpty() const { return Begin() == End(); }
	TBOOL IsValid( const T* a_pNode ) const { return a_pNode != TNULL && a_pNode->m_pList == this; }

	TSIZE Count() const { return m_iCount; }

	Iterator Head() const { return m_oRoot.m_pNext; }
	Iterator Tail() const { return m_oRoot.m_pPrev; }

	// Folded for every type: 0040ea70
	// $Barnyard: FUNCTION 0040ea70
	Iterator Begin() const { return m_oRoot.m_pNext; }
	// $Barnyard: FUNCTION 00639c30
	// TNodeList<AUnitProxy> 0040d880
	// $Barnyard: FUNCTION 0040d880
	Iterator End() const { return TSTATICCAST( T, &m_oRoot ); }

	Iterator RBegin() const { return m_oRoot.m_pPrev; }
	Iterator REnd() const { return TSTATICCAST( T, &m_oRoot ); }

protected:
	mutable TNode m_oRoot;
	TINT          m_iCount;
};

TOSHI_NAMESPACE_END
