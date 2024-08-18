#pragma once
#include "Toshi/TComparator.h"

TOSHI_NAMESPACE_START

template <class First, class Second, class Comparator = TComparator<First>>
class T2Pair
{
public:
	T2Pair( const First& first, const Second& second )
	{
		m_First  = first;
		m_Second = second;
	}

	T2Pair( const First& first )
	{
		m_First = first;
	}

	T2Pair()
	{
	}

	First& GetFirst()
	{
		return m_First;
	}

	Second& GetSecond()
	{
		return m_Second;
	}

	void operator=( const T2Pair<First, Second, Comparator>& other )
	{
		m_First  = other.m_First;
		m_Second = other.m_Second;
	}

	TBOOL operator==( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsEqual( m_First, other.m_First );
	}

	TBOOL operator!=( const T2Pair<First, Second, Comparator>& other ) const
	{
		return !Comparator::IsEqual( m_First, other.m_First );
	}

	TBOOL operator>( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsGreater( m_First, other.m_First );
	}

	TBOOL operator>=( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsGreaterOrEqual( m_First, other.m_First );
	}

	TBOOL operator<( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsLess( m_First, other.m_First );
	}

	TBOOL operator<=( const T2Pair<First, Second, Comparator>& other ) const
	{
		return Comparator::IsLessOrEqual( m_First, other.m_First );
	}

public:
	First  m_First;
	Second m_Second;
};

TOSHI_NAMESPACE_END
