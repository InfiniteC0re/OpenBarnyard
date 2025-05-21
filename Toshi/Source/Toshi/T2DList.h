#pragma once
#include "T2Iterator.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// TOSHI 2.0 - Doubly linked list
// Usage example:
//
// class AMyClass : public Toshi::T2DList<AMyClass>::Node { ... };
//
// Toshi::T2DList<AMyClass> list;
// list.PushBack(new AMyClass);
//-----------------------------------------------------------------------------
class T2GenericDList
{
public:
	class Node
	{
	protected:
		friend T2GenericDList;
		template <class T> friend class T2DList;
		T2_DEFINE_ITERATOR_FRIEND();

	public:
		constexpr Node()
		    : m_pPrev( this ), m_pNext( this )
		{}

		~Node()
		{
			m_pNext->m_pPrev = m_pPrev;
			m_pPrev->m_pNext = m_pNext;
			m_pNext          = this;
			m_pPrev          = this;
		}

		TBOOL IsLinked() const
		{
			return m_pNext != this;
		}

		void InsertBefore( Node* pInsertBefore )
		{
			TASSERT( !IsLinked() );
			m_pNext                = pInsertBefore;
			m_pPrev                = pInsertBefore->m_pPrev;
			pInsertBefore->m_pPrev = this;
			m_pPrev->m_pNext       = this;
		}

		void InsertAfter( Node* pInsertAfter )
		{
			TASSERT( !IsLinked() );
			m_pPrev               = pInsertAfter;
			m_pNext               = pInsertAfter->m_pNext;
			pInsertAfter->m_pNext = this;
			m_pNext->m_pPrev      = this;
		}

		void Remove()
		{
			TASSERT( IsLinked() );
			m_pPrev->m_pNext = this->m_pNext;
			m_pNext->m_pPrev = this->m_pPrev;
			m_pNext          = this;
			m_pPrev          = this;
		}

		Node* Next() const
		{
			return m_pNext;
		}

		Node* Prev() const
		{
			return m_pPrev;
		}

	private:
		Node* m_pNext;
		Node* m_pPrev;
	};

public:
	T2GenericDList() = default;

	~T2GenericDList()
	{
		TASSERT( m_oRoot.m_pNext == &m_oRoot );
		TASSERT( TFALSE == IsLinked() );
	}

	TBOOL IsLinked() const
	{
		return m_oRoot.IsLinked();
	}

	void ClearBefore( Node* pNode )
	{
		if ( pNode != &m_oRoot )
		{
			Node* pCurrentNode = m_oRoot.m_pNext;

			while ( pCurrentNode != pNode )
			{
				IsLinked();
				pCurrentNode->Remove();
				pCurrentNode = m_oRoot.m_pNext;
			}
		}
	}

	void Clear()
	{
		Node* pNode = m_oRoot.m_pNext;

		while ( pNode != &m_oRoot )
		{
			IsLinked();
			pNode->Remove();
			pNode = m_oRoot.m_pNext;
		}
	}

	TBOOL IsInList( Node* pNode ) const
	{
		for ( Node* pCNode = m_oRoot.m_pNext; pCNode != &m_oRoot; pCNode = pCNode->m_pNext )
		{
			if ( pCNode == pCNode ) return TTRUE;
		}

		return TFALSE;
	}

	TUINT Size() const
	{
		TUINT uiSize = 0;

		for ( Node* pNode = m_oRoot.m_pNext; pNode != &m_oRoot; pNode = pNode->m_pNext )
		{
			uiSize++;
		}

		return uiSize;
	}

protected:
	mutable Node m_oRoot;
};

template <class T>
class T2DList : public T2GenericDList
{
public:
	T2_DEFINE_ITERATOR( T, Node );

public:
	T2DList()
	{
		TSTATICASSERT( std::is_base_of<T2GenericDList::Node, T>::value );
	}

	~T2DList()
	{
		TASSERT( IsEmpty() );
	}

	Iterator Head() const
	{
		TASSERT( !IsEmpty() );
		return m_oRoot.Next();
	}

	Iterator Tail() const
	{
		TASSERT( !IsEmpty() );
		return m_oRoot.Prev();
	}

	Iterator Begin() const
	{
		return m_oRoot.Next();
	}

	Iterator End() const
	{
		return &m_oRoot;
	}

	Iterator RBegin() const
	{
		return m_oRoot.Next();
	}

	Iterator REnd() const
	{
		return &m_oRoot;
	}

	Iterator Front() const
	{
		return m_oRoot.Next();
	}

	Iterator Back() const
	{
		return m_oRoot.Prev();
	}

	void Delete( Iterator iter )
	{
		iter->Remove();
		delete TSTATICCAST( T, iter );
	}

	void DeleteAll()
	{
		while ( Begin() != End() )
		{
			Delete( Begin() );
		}
	}

	// $Barnyard: FUNCTION 00457a20
	Iterator Erase( Iterator iter )
	{
		Iterator itNext = iter.Next();
		iter->Remove();

		return itNext;
	}

	// $Barnyard: FUNCTION 006c8140
	TBOOL Exists( T* a_pItem )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it == a_pItem )
				return TTRUE;
		}

		return TFALSE;
	}

	TBOOL IsEmpty() const
	{
		return !m_oRoot.IsLinked();
	}

	void PushBack( T* pItem )
	{
		pItem->InsertBefore( &m_oRoot );
	}

	void PushFront( T* pItem )
	{
		pItem->InsertAfter( &m_oRoot );
	}

	T* PopBack()
	{
		auto node = Back();
		node->Remove();
		return node;
	}

	T* PopFront()
	{
		auto node = Front();
		node->Remove();
		return node;
	}
};

//-----------------------------------------------------------------------------
// Purpose: wraps typename T making it T2DList::Node
//-----------------------------------------------------------------------------
template <typename T>
class T2DListNodeWrapper : public T2DList<T2DListNodeWrapper<T>>::Node
{
public:
	using Type = T;

public:
	// constructors/destructor
	constexpr T2DListNodeWrapper() = default;

	constexpr T2DListNodeWrapper( T* a_pValue )
	    : m_pValue( a_pValue ) {}

	~T2DListNodeWrapper() = default;

	constexpr T*       GetNodeValue() { return m_pValue; }
	constexpr const T* GetNodeValue() const { return m_pValue; }
	constexpr T*       operator->() { return m_pValue; }
	constexpr const T* operator->() const { return m_pValue; }

	constexpr void SetNodeValue( T* a_pValue ) { m_pValue = a_pValue; }

	constexpr T2DListNodeWrapper& operator=( T* a_pValue )
	{
		SetNodeValue( a_pValue );
		return *this;
	}

private:
	T* m_pValue;
};

TOSHI_NAMESPACE_END

// $Barnyard: FUNCTION 006c8140
// Toshi::T2DList::Exists
