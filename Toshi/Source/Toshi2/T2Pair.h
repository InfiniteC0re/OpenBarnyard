#pragma once
#include "Toshi/Core/TComparator.h"

namespace Toshi
{
	template <class First, class Second, class Comparator = TComparator<First>>
	class T2Pair
	{
	public:
		T2Pair(const First& first, const Second& second)
		{
			m_First = first;
			m_Second = second;
		}

		T2Pair(const First& first)
		{
			m_First = first;
		}

		First& GetFirst()
		{
			return m_First;
		}

		Second& GetSecond()
		{
			return m_Second;
		}

		bool operator==(const T2Pair<First, Second, Comparator>& other) const
		{
			return Comparator::IsEqual(m_First, other.m_First);
		}

		bool operator!=(const T2Pair<First, Second, Comparator>& other) const
		{
			return !Comparator::IsEqual(m_First, other.m_First);
		}

		bool operator>(const T2Pair<First, Second, Comparator>& other) const
		{
			return Comparator::IsGreater(m_First, other.m_First);
		}

		bool operator>=(const T2Pair<First, Second, Comparator>& other) const
		{
			return Comparator::IsGreaterOrEqual(m_First, other.m_First);
		}

		bool operator<(const T2Pair<First, Second, Comparator>& other) const
		{
			return Comparator::IsLess(m_First, other.m_First);
		}

		bool operator<=(const T2Pair<First, Second, Comparator>& other) const
		{
			return Comparator::IsLessOrEqual(m_First, other.m_First);
		}

	private:
		First m_First;
		Second m_Second;
	};
}