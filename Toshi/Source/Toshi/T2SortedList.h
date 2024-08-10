#pragma once
#include "T2DList.h"
#include "T2Iterator.h"

TOSHI_NAMESPACE_START

// This is an example of a comparator
template <class T>
struct T2SortedListDefaultComparator
{
	TINT operator()( const T& a_rcVal1, const T& a_rcVal2 ) const
	{
		if ( a_rcVal1 < a_rcVal2 )
			return 1;

		return -1;
	}
};

template <typename T, typename Container = T2DList<T>, typename Comparator = T2SortedListDefaultComparator<T>>
class T2SortedList : private Container
{
public:
	using Iterator = Container::Iterator;
	using Node = Container::Node;

public:
	T2SortedList() = default;
	~T2SortedList() = default;

	Iterator FindInsertionPoint( const T& a_rcValue )
	{
		Comparator comparator;

		T2_FOREACH( *this, it )
		{
			if ( comparator( a_rcValue, *it.Get() ) < 0 )
				return it;
		}

		return End();
	}

	Iterator Push( T& a_rValue )
	{
		a_rValue.InsertBefore( FindInsertionPoint( a_rValue ) );
		return &a_rValue;
	}

	Iterator Push( T* a_pValue )
	{
		return Push( *a_pValue );
	}

	void Delete( const Iterator& a_rcIt ) { Container::Delete( a_rcIt ); }
	void DeleteAll() { Container::DeleteAll(); }

	void Erase( const Iterator& a_rcIt ) { Container::Erase( a_rcIt ); }

	TBOOL IsEmpty() const { return Container::IsEmpty(); }

	Iterator Head() const { return Container::Head(); }
	Iterator Tail() const { return Container::Tail(); }
	Iterator Begin() const { return Container::Begin(); }
	Iterator RBegin() const { return Container::RBegin(); }
	Iterator End() const { return Container::End(); }
	Iterator REnd() const { return Container::REnd(); }
	Iterator Front() const { return Container::Front(); }
	Iterator Back() const { return Container::Back(); }

};

TOSHI_NAMESPACE_END
