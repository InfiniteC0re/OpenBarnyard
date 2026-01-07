#pragma once
#include "Toshi/TComparator.h"

TOSHI_NAMESPACE_START

template <class First, class Second, class C = TComparator<First>>
struct T2Pair
{
	First  first;
	Second second;

	struct Comparator
	{
		TINT operator()( const T2Pair& a, const T2Pair& b )
		{
			return C()( a.first, b.first );
		}
	};

	constexpr T2Pair()
	    : first( First() )
	    , second( Second() )
	{}

	constexpr T2Pair( const First& a_rcFirst, const Second& a_rcSecond )
	    : first( a_rcFirst )
	    , second( a_rcSecond )
	{}

	constexpr T2Pair( const First& a_rcFirst )
	    : first( a_rcFirst )
	    , second( Second() )
	{}

	constexpr T2Pair( First&& a_rFirst )
	    : first( a_rFirst )
	    , second( Second() )
	{}

	constexpr T2Pair( const First& a_rcFirst, Second&& a_rSecond )
	    : first( a_rcFirst )
	    , second( a_rSecond )
	{}

	constexpr T2Pair( First&& a_rFirst, Second&& a_rSecond )
	    : first( a_rFirst )
	    , second( a_rSecond )
	{}

	constexpr First&  GetFirst() { return first; }
	constexpr Second& GetSecond() { return second; }

	constexpr T2Pair& operator=( const T2Pair<First, Second, C>& a_rcOther )
	{
		if ( this != &a_rcOther )
		{
			first  = a_rcOther.first;
			second = a_rcOther.second;
		}

		return *this;
	}
};

TOSHI_NAMESPACE_END
