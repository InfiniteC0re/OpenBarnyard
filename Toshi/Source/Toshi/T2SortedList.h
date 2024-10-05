#pragma once
#include "T2DList.h"
#include "T2Iterator.h"

TOSHI_NAMESPACE_START

// -1 => push right here before a_rcVal2
//  0 => they are equal, add new element after it to preserve original ordering
//  1 => should add somewhere after a_rcVal2, so go to the next node
template <class T>
struct T2SortedListDefaultSortResults
{
	TINT operator()( const T& a_rcVal1, const T& a_rcVal2 ) const
	{
		if ( a_rcVal1 < a_rcVal2 )
			return 1;

		return -1;
	}
};

template <typename T, typename Container = T2DList<T>, typename SortResults = T2SortedListDefaultSortResults<T>>
class T2SortedList : private Container
{
public:
	using Iterator = Container::Iterator;
	using Node     = Container::Node;

public:
	T2SortedList()  = default;
	~T2SortedList() = default;

	Iterator FindInsertionPoint( const T& a_rcValue )
	{
		T2_FOREACH( *this, it )
		{
			if ( SortResults()( a_rcValue, *it.Get() ) < 0 )
				return it;
		}

		return End();
	}

	Iterator Push( T& a_rValue )
	{
		a_rValue.InsertBefore( FindInsertionPoint( a_rValue ) );
		return &a_rValue;
	}

	Iterator Push( T* a_pValue ) { return Push( *a_pValue ); }

	T* PopBack() { return Container::PopBack(); }
	T* PopFront() { return Container::PopFront(); }

	// Removes element from the list and adds it again to make sure the order is right
	Iterator ReInsert( T& a_rValue )
	{
		if ( a_rValue.IsLinked() )
			Erase( &a_rValue );

		return Push( a_rValue );
	}

	// Removes element from the list and adds it again to make sure the order is right
	Iterator ReInsert( T* a_pValue )
	{
		return ReInsert( *a_pValue );
	}

	void Delete( Iterator a_It ) { Container::Delete( a_It ); }
	void DeleteAll() { Container::DeleteAll(); }

	Iterator Erase( Iterator a_It ) { return Container::Erase( a_It ); }

	TBOOL IsEmpty() const { return Container::IsEmpty(); }
	TUINT Size() const { return Container::Size(); }

	Iterator Head() const { return Container::Head(); }
	Iterator Tail() const { return Container::Tail(); }
	Iterator Begin() const { return Container::Begin(); }
	Iterator RBegin() const { return Container::RBegin(); }
	Iterator End() const { return Container::End(); }
	Iterator REnd() const { return Container::REnd(); }
	Iterator Front() const { return Container::Front(); }
	Iterator Back() const { return Container::Back(); }

	Container* AccessContainer() { return TSTATICCAST( Container, this ); }
	Container* operator->() { return TSTATICCAST( Container, this ); }
};

TOSHI_NAMESPACE_END
