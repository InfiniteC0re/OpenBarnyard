#pragma once
#include "Toshi/Typedefs.h"

TOSHI_NAMESPACE_START

class TOSHI_API T2GenericSList
{
public:
	class Node
	{
	public:
		friend class T2GenericSList;

	public:
		Node()
		{
			Reset();
		}

		void Reset()
		{
			m_pNext = this;
		}

		Node* Next() const
		{
			return m_pNext;
		}

		TBOOL IsLinked() const
		{
			return m_pNext != this;
		}

		void InsertAfter( Node* a_pNode )
		{
			TASSERT( !IsLinked() );
			m_pNext          = a_pNode->m_pNext;
			a_pNode->m_pNext = this;
		}

	private:
		Node* m_pNext;
	};

public:
	T2GenericSList()  = default;
	~T2GenericSList() = default;

	// Calling this won't clear the list so the items will still be linked to each other!
	// Use this only if you're sure the nodes are freed
	void Reset()
	{
		m_oRoot.Reset();
	}

	Node* Begin() const
	{
		return m_oRoot.Next();
	}

	Node* RBegin() const
	{
		return m_oRoot.Next();
	}

	Node* End() const
	{
		return &m_oRoot;
	}

	Node* REnd() const
	{
		return &m_oRoot;
	}

	TBOOL IsLinked() const
	{
		return m_oRoot.m_pNext != &m_oRoot;
	}

	Node* Back() const
	{
		Node* pBack = End();

		for ( auto it = Begin(); it != End(); it = it->Next() )
		{
			pBack = it;
		}

		return pBack;
	}

	Node* FindNodeBefore( Node* a_pNode )
	{
		Node* pResult = End();

		for ( auto it = Begin(); it != a_pNode; it = it->Next() )
		{
			TASSERT( it != End() );
			pResult = it;
		}

		return pResult;
	}

	void PushBack( Node* a_pNode )
	{
		a_pNode->InsertAfter( Back() );
	}

	void PushFront( Node* a_pNode )
	{
		a_pNode->InsertAfter( &m_oRoot );
	}

	Node* PopBack()
	{
		Node* pBack       = Back();
		Node* pNodeBefore = FindNodeBefore( pBack );

		pNodeBefore->m_pNext = &m_oRoot;
		pBack->m_pNext       = pBack;

		return pBack;
	}

	Node* PopFront()
	{
		Node* pNode     = Begin();
		m_oRoot.m_pNext = pNode->m_pNext;
		pNode->m_pNext  = pNode;
		return pNode;
	}

	Node* Erase( Node* a_pFrom, Node* a_pTo )
	{
		Node* pNodeBefore = FindNodeBefore( a_pFrom );

		while ( a_pFrom != a_pTo )
		{
			Node* pNext      = a_pFrom->m_pNext;
			a_pFrom->m_pNext = a_pFrom;
			a_pFrom          = pNext;
		}

		pNodeBefore->m_pNext = a_pTo;
		return a_pTo;
	}

	void Clear()
	{
		Node* pNode = Begin();

		while ( pNode != End() )
		{
			Node* pNext    = pNode->Next();
			pNode->m_pNext = pNode;
			pNode          = pNext;
		}

		m_oRoot.m_pNext = &m_oRoot;
	}

	TUINT Size() const
	{
		TUINT uiSize = 0;

		for ( auto it = Begin(); it != End(); it = it->Next() )
		{
			uiSize++;
		}

		return uiSize;
	}

	Node* Transfer( Node* a_pNode, T2GenericSList& a_rList )
	{
		Node* pNodeBefore    = FindNodeBefore( a_pNode );
		pNodeBefore->m_pNext = a_pNode->m_pNext;
		a_pNode->m_pNext     = a_pNode;
		a_rList.PushBack( a_pNode );

		return pNodeBefore;
	}

private:
	mutable Node m_oRoot;
};

//-----------------------------------------------------------------------------
// TOSHI 2.0 - Singly linked list
// Usage example:
//
// class AMyClass : public Toshi::T2SList<AMyClass>::Node { ... };
//
// Toshi::T2SList<AMyClass> list;
// list.PushBack(new AMyClass);
//-----------------------------------------------------------------------------
template <class T, TINT Unknown = 0>
class T2SList : public T2GenericSList
{
public:
	class Node : public T2GenericSList::Node
	{
	public:
		T* Next() const
		{
			return TSTATICCAST( T, T2GenericSList::Node::Next() );
		}

		T* Prev() const
		{
			TASSERT( !"Nah, you can't do this" );
			return TNULL;
		}
	};

	T2_DEFINE_ITERATOR_FRIEND();
	T2_DEFINE_ITERATOR( T, T2GenericSList::Node );

public:
	T2SList() {}
	~T2SList() { Clear(); }

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

	Iterator Begin() const { return T2GenericSList::Begin(); }
	Iterator RBegin() const { return T2GenericSList::RBegin(); }

	Iterator End() const { return T2GenericSList::End(); }
	Iterator REnd() const { return T2GenericSList::REnd(); }

	Iterator Head() const { return T2GenericSList::Begin(); }
	Iterator Back() const { return T2GenericSList::Back(); }

	Iterator FindNodeBefore( Iterator a_Node ) { return T2GenericSList::FindNodeBefore( a_Node ); }

	Iterator PopBack() { return T2GenericSList::PopBack(); }
	Iterator PopFront() { return T2GenericSList::PopFront(); }

	Iterator Erase( Iterator a_Iterator ) { return T2GenericSList::Erase( a_Iterator, a_Iterator.Next() ); }
	Iterator Erase( Iterator a_From, Iterator a_To ) { return T2GenericSList::Erase( a_From, a_To ); }
	Iterator Transfer( Node* a_pNode, T2GenericSList& a_rList ) { return T2GenericSList::Transfer( a_pNode, a_rList ); }
};

//-----------------------------------------------------------------------------
// Purpose: wraps typename T making it T2SList::Node
//-----------------------------------------------------------------------------
template <typename T>
class T2SListNodeWrapper : public T2SList<T2SListNodeWrapper<T>>::Node
{
public:
	using Type = T;

public:
	// constructors/destructor
	constexpr T2SListNodeWrapper() = default;

	constexpr T2SListNodeWrapper( T* a_pValue )
	    : m_pValue( a_pValue ) {}

	~T2SListNodeWrapper() = default;

	constexpr T*       GetNodeValue() { return m_pValue; }
	constexpr const T* GetNodeValue() const { return m_pValue; }
	constexpr T*       operator->() { return m_pValue; }
	constexpr const T* operator->() const { return m_pValue; }

	constexpr void SetNodeValue( T* a_pValue ) { m_pValue = a_pValue; }

	constexpr T2SListNodeWrapper& operator=( T* a_pValue )
	{
		SetNodeValue( a_pValue );
		return *this;
	}

private:
	T* m_pValue;
};

TOSHI_NAMESPACE_END
