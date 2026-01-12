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

	auto operator()( ArgType a, ArgType b ) const { return a - b; }
};

//-----------------------------------------------------------------------------
// Purpose: allows you doing comparisons of any type using result of a
// comparator function with named methods to make code more readable.
//-----------------------------------------------------------------------------
struct TComparatorHelper
{
	template<typename T> static TBOOL IsEqual( T a_iResult ) { TSTATICASSERT( std::is_arithmetic_v<T> ); return a_iResult == 0; }
	template<typename T> static TBOOL IsGreater( T a_iResult ) { TSTATICASSERT( std::is_arithmetic_v<T> ); return a_iResult > 0; }
	template<typename T> static TBOOL IsLess( T a_iResult ) { TSTATICASSERT( std::is_arithmetic_v<T> ); return a_iResult < 0; }
	template<typename T> static TBOOL IsLessOrEqual( T a_iResult ) { TSTATICASSERT( std::is_arithmetic_v<T> ); return a_iResult <= 0; }
	template<typename T> static TBOOL IsGreaterOrEqual( T a_iResult ) { TSTATICASSERT( std::is_arithmetic_v<T> ); return a_iResult >= 0; }
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
