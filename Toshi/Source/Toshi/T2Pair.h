#pragma once
#include "Toshi/TComparator.h"

TOSHI_NAMESPACE_START

template <class First, class Second, class Comparator = TComparator<First>>
struct T2Pair
{
	First  first;
	Second second;

	constexpr T2Pair( const First& a_rcFirst, const Second& a_rcSecond )
	    : first( a_rcFirst )
	    , second( a_rcSecond )
	{}

	constexpr T2Pair( const First& a_rcFirst )
	    : first( a_rcFirst )
		, second( Second() )
	{ }

	constexpr T2Pair()
	    : first( First() )
	    , second( Second() )
	{ }

	constexpr First& GetFirst() { return first; }

	constexpr Second& GetSecond() { return second; }

	constexpr T2Pair& operator=( const T2Pair<First, Second, Comparator>& a_rcOther )
	{
		if ( this != &a_rcOther )
		{
			first  = a_rcOther.first;
			second = a_rcOther.second;
		}

		return *this;
	}

	constexpr TBOOL operator==( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsEqual( first, other.first );
	}

	constexpr TBOOL operator!=( const T2Pair<First, Second, Comparator>& other ) const
	{
		return !Comparator::IsEqual( first, other.first );
	}

	constexpr TBOOL operator>( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsGreater( first, other.first );
	}

	constexpr TBOOL operator>=( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsGreaterOrEqual( first, other.first );
	}

	constexpr TBOOL operator<( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsLess( first, other.first );
	}

	constexpr TBOOL operator<=( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsLessOrEqual( first, other.first );
	}
};

TOSHI_NAMESPACE_END
