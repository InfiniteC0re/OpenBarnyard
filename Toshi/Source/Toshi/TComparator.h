#pragma once
#include "Toshi/Defines.h"

#include <type_traits>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: struct that allows you to create custom comparators or to use basic
// comparison functions with types that support substraction.
//-----------------------------------------------------------------------------
template <class T>
struct TComparator
{
	using Type = T;

	// Optimize comparison by passing values instead of references (pointers) when applicable
	using ArgType = std::conditional_t<sizeof( Type ) == sizeof( void* ), Type, const Type&>;

	TINT operator()( ArgType a, ArgType b ) const { return a - b; }
};

//-----------------------------------------------------------------------------
// Purpose: allows you doing comparisons of any type using result of a
// comparator function with named methods to make code more readable.
//-----------------------------------------------------------------------------
struct TComparatorHelper
{
	static TBOOL IsEqual( TINT a_iResult ) { return a_iResult == 0; }
	static TBOOL IsGreater( TINT a_iResult ) { return a_iResult > 0; }
	static TBOOL IsLess( TINT a_iResult ) { return a_iResult < 0; }
	static TBOOL IsLessOrEqual( TINT a_iResult ) { return a_iResult <= 0; }
	static TBOOL IsGreaterOrEqual( TINT a_iResult ) { return a_iResult >= 0; }
};

//-----------------------------------------------------------------------------
// Purpose: allows you doing comparisons of type T using provided comparator
// with named methods to make code more readable.
//-----------------------------------------------------------------------------
template <class T, class C = TComparator<T>>
struct TComparatorHelperT
{
	using Type = T;
	using Comparator = C;

	// Optimize comparison by passing values instead of references (pointers) when applicable
	using ArgType = std::conditional_t<sizeof( Type ) == sizeof( void* ), Type, const Type&>;

	static TBOOL IsEqual( ArgType a, ArgType b ) { return TComparatorHelper::IsEqual( Comparator()( a, b ) ); }
	static TBOOL IsGreater( ArgType a, ArgType b ) { return TComparatorHelper::IsGreater( Comparator()( a, b ) ); }
	static TBOOL IsLess( ArgType a, ArgType b ) { return TComparatorHelper::IsLess( Comparator()( a, b ) ); }
	static TBOOL IsLessOrEqual( ArgType a, ArgType b ) { return TComparatorHelper::IsLessOrEqual( Comparator()( a, b ) ); }
	static TBOOL IsGreaterOrEqual( ArgType a, ArgType b ) { return TComparatorHelper::IsGreaterOrEqual( Comparator()( a, b ) ); }
};

TOSHI_NAMESPACE_END
