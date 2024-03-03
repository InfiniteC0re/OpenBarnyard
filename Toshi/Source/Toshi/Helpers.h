#pragma once
#include <limits>

template <typename T>
TFORCEINLINE void TSwapValues(T& a, T& b)
{
	T temp = std::move(a);
	a = std::move(b);
	b = std::move(temp);
}

TFORCEINLINE constexpr TUINT32 TMakeFour(const TCHAR str[4])
{
	return ((str[3] << 24) | (str[2] << 16) | (str[1] << 8) | str[0]);
}

TFORCEINLINE TBOOL TIsAlignedAddress(const void* a_pPointer, TUINT a_uiAlignment)
{
	return ISZERO(uintptr_t(a_pPointer) & (a_uiAlignment - 1));
}

template <class T>
constexpr T* TAlignPointer(T* ptr)
{
	return TREINTERPRETCAST(
		T*,
		(uintptr_t(ptr) + (sizeof(T*) - 1)) & (std::numeric_limits<uintptr_t>::max() - (sizeof(T*) - 1))
	);
}

template <class T>
constexpr T TAlignNumDown(T a_iValue, TINT a_iAlignment = 4)
{
	return a_iValue & (std::numeric_limits<T>::max() - (a_iAlignment - 1));
}

template <class T>
constexpr T TAlignNumUp(T a_iValue, TINT a_iAlignment = 4)
{
	return (a_iValue + (a_iAlignment - 1)) & (std::numeric_limits<T>::max() - (a_iAlignment - 1));
}