#pragma once
#include "T2Iterator.h"

TOSHI_NAMESPACE_START

class TGenericDList
{
public:
	class TNode
	{
	public:
		friend TGenericDList;
		template <class T> friend class TDList;
		T2_DEFINE_ITERATOR_FRIEND();

		enum TUninitialised
		{
			Unitiailised
		};

	public:
		template <typename T>
		T* As() { return TSTATICCAST( T, this ); }

	protected:
		TNode();
		TNode( TUninitialised );
		~TNode();

		TNode* Next() const { return m_pNext; }
		TNode* Prev() const { return m_pPrev; }

	public:
		void Remove();
		void InsertAfter( TNode* a_pNode );
		void InsertBefore( TNode* a_pNode );

		TBOOL IsLinked() const { return this != m_pNext; }

	private:
		void Reset();

	protected:
		TNode* m_pNext;
		TNode* m_pPrev;
	};

public:
	void InsertSegmentAtHead( TNode* a_pNode1, TNode* a_pNode2 );
	void InsertSegmentAtTail( TNode* a_pNode1, TNode* a_pNode2 );

	void InsertHead( TNode* a_pNode ) { a_pNode->InsertAfter( &m_oRoot ); }
	void InsertTail( TNode* a_pNode ) { a_pNode->InsertBefore( &m_oRoot ); }
	void RemoveHead()
	{
		if ( !IsEmpty() ) m_oRoot.Next()->Remove();
	}
	void RemoveTail()
	{
		if ( !IsEmpty() ) m_oRoot.Prev()->Remove();
	}
	TBOOL  IsEmpty() { return m_oRoot.Next() == &m_oRoot; }
	TNode* Head() { return m_oRoot.Next(); }
	TNode* Tail() { return m_oRoot.Prev(); }
	TNode* Begin() { return m_oRoot.Next(); }
	TNode* End() { return &m_oRoot; }

	void RemoveAll();

public:
	static void InsertSegmentAfter( TNode* a_pNode1, TNode* a_pNode2, TNode* a_pNode3 );
	static void InsertSegmentBefore( TNode* a_pNode1, TNode* a_pNode2, TNode* a_pNode3 );

protected:
	TGenericDList();
	~TGenericDList();

private:
	TNode m_oRoot;
};

class TGenericPriList
{
public:
	class TNode
	{
	public:
		friend TGenericPriList;
		template <class T> friend class TPriList;
		T2_DEFINE_ITERATOR_FRIEND();

		enum TUninitialised
		{
			Unitiailised
		};

	public:
		template <typename T>
		T* As() { return TSTATICCAST( T, this ); }

	protected:
		TNode();
		TNode( TUninitialised );
		~TNode();

		TNode* Next() const { return m_pNext; }
		TNode* Prev() const { return m_pPrev; }

	public:
		void Remove();
		void InsertAfter( TNode* a_pNode );
		void InsertBefore( TNode* a_pNode );

		TBOOL IsLinked() const { return this != m_pNext; }

		void SetPriority( TINT priority ) { m_iPriority = priority; }
		TINT GetPriority() const { return m_iPriority; }

		TNode& operator=( const TNode& a_pNode );

	private:
		void Reset();

	public:
		TNode* m_pNext;
		TNode* m_pPrev;
		TINT   m_iPriority;
	};

public:
	TBOOL  IsEmpty() { return m_pNext == End(); }
	TNode* Head() { return m_pNext; }
	TNode* Tail() { return m_pPrev; }
	TNode* Begin() { return m_pNext; }
	TNode* End() { return (TNode*)this; }

	void Insert( TNode* a_pNode, TINT iPriority );
	void Insert( TNode* a_pNode );

	void RemoveAll();

protected:
	TGenericPriList();
	~TGenericPriList();

private:
	TNode* m_pNext;
	TNode* m_pPrev;
};

template <class T>
class TPriList : public TGenericPriList
{
public:
	T2_DEFINE_ITERATOR( T, TNode );

	TPriList() {}

	Iterator Begin() { return TGenericPriList::Begin(); }
	Iterator End() { return TGenericPriList::End(); }
	TBOOL    IsLinked() { return !IsEmpty(); }
};

template <class T>
class TDList : public TGenericDList
{
public:
	T2_DEFINE_ITERATOR( T, TNode );

	TDList() {}

	T*       Head() { return TGenericDList::Head()->As<T>(); }
	T*       Tail() { return TGenericDList::Tail()->As<T>(); }
	Iterator Begin() { return TGenericDList::Begin()->As<T>(); }
	Iterator End() { return TGenericDList::End()->As<T>(); }
	TBOOL    IsEmpty() { return TGenericDList::IsEmpty(); }
	TBOOL    IsLinked() { return m_oRoot.IsLinked(); }
	void     RemoveHead() { TGenericDList::RemoveHead(); }
	void     RemoveTail() { TGenericDList::RemoveTail(); }
};

TOSHI_NAMESPACE_END
