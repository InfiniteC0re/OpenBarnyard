#pragma once
#include "Math/TMathInline.h"
#include <limits>

// Swaps values of two variables with a help of moving semantic
template <typename T>
TFORCEINLINE void TSwapValues( T& a, T& b )
{
	T temp = std::move( a );
	a      = std::move( b );
	b      = std::move( temp );
}

// Returns TUINT32 value that consists of the specified 4 characters
TFORCEINLINE constexpr TUINT32 TFourCC( const TCHAR str[ 4 ] )
{
	return ( ( str[ 3 ] << 24 ) | ( str[ 2 ] << 16 ) | ( str[ 1 ] << 8 ) | str[ 0 ] );
}

// Returns TTRUE if the pointer is aligned to the specified alignment value
// For example, TTRUE  is returned if pointer is 0x8 and alignment is 4
//              TFALSE is returned if pointer is 0x8 and alignment is 7
TFORCEINLINE TBOOL TIsPointerAligned( const void* a_pPointer, TSIZE a_uiAlignment = sizeof( void* ) )
{
	return 0 == ( TUINTPTR( a_pPointer ) % a_uiAlignment );
}

template <class T>
TFORCEINLINE constexpr T* TAlignPointerUp( T* a_pMem, TSIZE a_uiAlignment = sizeof( T* ) )
{
	return TREINTERPRETCAST(
	    T*,
	    ( TUINTPTR( a_pMem ) + ( a_uiAlignment - 1 ) ) & ( ~( a_uiAlignment - 1U ) ) );
}

template <class T>
TFORCEINLINE constexpr T* TAlignPointerDown( T* a_pMem, TSIZE a_uiAlignment = sizeof( T* ) )
{
	return TREINTERPRETCAST(
	    T*,
	    ( TUINTPTR( a_pMem ) ) & ( ~( a_uiAlignment - 1U ) ) );
}

template <class T>
TFORCEINLINE constexpr T TAlignNumDown( T a_iValue, TSIZE a_uiAlignment = 4 )
{
	TSTATICASSERT( std::is_arithmetic<T>::value );
	return a_iValue & (T)( ~( a_uiAlignment - 1U ) );
}

template <class T>
TFORCEINLINE constexpr T TAlignNumUp( T a_iValue, TSIZE a_uiAlignment = 4 )
{
	TSTATICASSERT( std::is_arithmetic<T>::value );
	return ( a_iValue + ( a_uiAlignment - 1U ) ) & (T)( ~( a_uiAlignment - 1U ) );
}
