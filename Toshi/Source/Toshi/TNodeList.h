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
	TNodeList()
	{
		m_iCount = 0;
	}

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
	void RemoveAll()
	{
		while ( !IsEmpty() )
		{
			Remove( m_oRoot.Next() );
		}
	}

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

	void InsertHead( T* a_pNode ) { InsertAfter( End(), a_pNode ); }
	void InsertTail( T* a_pNode ) { InsertBefore( End(), a_pNode ); }

	TBOOL IsEmpty() const { return Begin() == End(); }
	TBOOL IsValid( const T* a_pNode ) const { return a_pNode != TNULL && a_pNode->m_pList == this; }

	TSIZE Count() const { return m_iCount; }

	Iterator Head() const { return m_oRoot.m_pNext; }
	Iterator Tail() const { return m_oRoot.m_pPrev; }

	Iterator Begin() const { return m_oRoot.m_pNext; }
	Iterator End() const { return TSTATICCAST( T, &m_oRoot ); }

	Iterator RBegin() const { return m_oRoot.m_pPrev; }
	Iterator REnd() const { return TSTATICCAST( T, &m_oRoot ); }

protected:
	mutable TNode m_oRoot;
	TINT          m_iCount;
};

TOSHI_NAMESPACE_END
