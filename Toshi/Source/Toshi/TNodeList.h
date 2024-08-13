#pragma once

TOSHI_NAMESPACE_START

class TOSHI_API TGenericNodeList
{
protected:
	class TOSHI_API TNode
	{
	public:
		friend TGenericNodeList;

	public:
		constexpr TNode() :
			m_pNext( this ),
			m_pPrev( this ),
			m_pList( TNULL )
		{}

		constexpr TNode( const TNode& a_rcNode ) :
			m_pNext( a_rcNode.m_pNext ),
			m_pPrev( a_rcNode.m_pPrev ),
			m_pList( a_rcNode.m_pList )
		{}

		TNode( TNode&& a_rNode ) :
			m_pNext( a_rNode.m_pNext ),
			m_pPrev( a_rNode.m_pPrev ),
			m_pList( a_rNode.m_pList )
		{
			a_rNode.m_pList = TNULL;
			a_rNode.m_pNext = TNULL;
			a_rNode.m_pPrev = TNULL;
		}

		TNode* Next() const { return m_pNext; }
		TNode* Prev() const { return m_pPrev; }

		void Remove() { GetList()->Remove( this ); }

		void			  SetList( TGenericNodeList* list ) { m_pList = list; }
		TGenericNodeList* GetList() const { return m_pList; }

		TBOOL IsLinked() const { return m_pList != TNULL; }

	protected:
		TGenericNodeList* m_pList;
		TNode*			  m_pNext;
		TNode*			  m_pPrev;
	};

	class TOSHI_API Iterator
	{
	public:
		constexpr Iterator() :
			m_pPtr( TNULL )
		{}

		constexpr Iterator( TNode* a_pPtr ) :
			m_pPtr( a_pPtr )
		{}

		constexpr Iterator( const Iterator& other ) :
			m_pPtr( other.m_pPtr )
		{}

		TNode* Get() const
		{
			return m_pPtr;
		}

		void operator=( const Iterator& other )
		{
			m_pPtr = other.m_pPtr;
		}

		void operator=( TNode* pPtr )
		{
			m_pPtr = pPtr;
		}

		TNode* operator->() const
		{
			TASSERT( m_pPtr != TNULL );
			return m_pPtr;
		}

		operator TNode*() const
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
			m_pPtr		 = m_pPtr->Next();
			return old;
		}

		Iterator operator--( TINT )
		{
			TASSERT( m_pPtr != TNULL );
			Iterator old = m_pPtr;
			m_pPtr		 = m_pPtr->Prev();
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

	private:
		TNode* m_pPtr;
	};

public:
	TGenericNodeList()
	{
		m_iCount = 0;
	}

	~TGenericNodeList()
	{
		DeleteAll();
	}

	void InsertAfter( TNode* insertAfter, TNode* newNode )
	{
		TASSERT( !newNode->IsLinked() );
		newNode->SetList( this );
		newNode->m_pNext			  = insertAfter->m_pNext;
		newNode->m_pPrev			  = insertAfter;
		insertAfter->m_pNext->m_pPrev = newNode;
		insertAfter->m_pNext		  = newNode;
		m_iCount++;
	}

	void InsertBefore( TNode* insertBefore, TNode* newNode )
	{
		TASSERT( !newNode->IsLinked() );
		newNode->SetList( this );
		newNode->m_pNext			   = insertBefore;
		newNode->m_pPrev			   = insertBefore->m_pPrev;
		insertBefore->m_pPrev->m_pNext = newNode;
		insertBefore->m_pPrev		   = newNode;
		m_iCount++;
	}

	TNode* Remove( TNode* pNode )
	{
		TASSERT( pNode->GetList() == this );
		pNode->SetList( TNULL );
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		pNode->m_pNext			= pNode;
		pNode->m_pPrev			= pNode;
		m_iCount--;
		TASSERT( pNode->IsLinked() == TFALSE );
		return pNode;
	}

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

	void DeleteAll()
	{
		while ( !IsEmpty() )
		{
			DeleteHead();
		}
	}

	void Delete( TNode* pNode )
	{
		Remove( pNode );
		delete pNode;
	}

	void InsertHead( TNode* a_pNode ) { InsertAfter( &m_oRoot, a_pNode ); }
	void InsertTail( TNode* a_pNode ) { InsertBefore( &m_oRoot, a_pNode ); }

	TBOOL IsEmpty() const { return Begin() == End(); }
	TBOOL IsValid( const TNode* a_pNode ) const { return a_pNode != TNULL && a_pNode->m_pList == this; }

	TSIZE Count() const { return m_iCount; }

	Iterator Head() const { return m_oRoot.m_pNext; }
	Iterator Tail() const { return m_oRoot.m_pPrev; }

	Iterator Begin() const { return m_oRoot.m_pNext; }
	Iterator End() const { return &m_oRoot; }

	Iterator RBegin() const { return m_oRoot.m_pPrev; }
	Iterator REnd() const { return &m_oRoot; }

protected:
	mutable TNode m_oRoot;
	TINT		  m_iCount;
};

template <class T>
class TNodeList : public TGenericNodeList
{
public:
	class TNode : public TGenericNodeList::TNode
	{
	public:
		T* Next() const { return TSTATICCAST( T, m_pNext ); }
		T* Prev() const { return TSTATICCAST( T, m_pPrev ); }
	};

	class Iterator : public TGenericNodeList::Iterator
	{
	public:
		constexpr Iterator() {}

		Iterator( T* a_pPtr ) :
			TGenericNodeList::Iterator( a_pPtr )
		{}

		Iterator( TNode* a_pPtr ) :
			TGenericNodeList::Iterator( a_pPtr )
		{}

		Iterator( const TGenericNodeList::Iterator& other ) :
			TGenericNodeList::Iterator( other )
		{}

		T* Get()
		{
			return TSTATICCAST( T, TGenericNodeList::Iterator::Get() );
		}

		T* operator->() const
		{
			return TSTATICCAST( T, TGenericNodeList::Iterator::Get() );
		}

		operator T*() const
		{
			return TSTATICCAST( T, TGenericNodeList::Iterator::Get() );
		}

		Iterator operator++( TINT dummy )
		{
			return TGenericNodeList::Iterator::operator++( dummy );
		}

		Iterator operator--( TINT dummy )
		{
			return TGenericNodeList::Iterator::operator--( dummy );
		}

		Iterator operator++()
		{
			return TGenericNodeList::Iterator::operator++();
		}

		Iterator operator--()
		{
			return TGenericNodeList::Iterator::operator--();
		}
	};

public:
	T* Remove( T& a_rNode )
	{
		return TSTATICCAST( T, TGenericNodeList::Remove( &a_rNode ) );
	}

	Iterator RemoveHead()
	{
		return TGenericNodeList::RemoveHead();
	}

	Iterator RemoveTail()
	{
		return TGenericNodeList::RemoveTail();
	}

	Iterator Head() const { return TGenericNodeList::Head(); }
	Iterator Tail() const { return TGenericNodeList::Tail(); }

	Iterator Begin() const { return TGenericNodeList::Begin(); }
	Iterator End() const { return TGenericNodeList::End(); }

	Iterator RBegin() const { return TGenericNodeList::RBegin(); }
	Iterator REnd() const { return TGenericNodeList::REnd(); }
};

TOSHI_NAMESPACE_END
