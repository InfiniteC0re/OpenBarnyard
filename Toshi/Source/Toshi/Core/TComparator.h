#pragma once

namespace Toshi
{
	template <class T>
	class TComparator
	{
	public:
		static bool IsEqual(const T& a, const T& b)
		{
			return a == b;
		}

		static bool IsGreater(const T& a, const T& b)
		{
			return a > b;
		}

		static bool IsLess(const T& a, const T& b)
		{
			return a < b;
		}

		static bool IsLessOrEqual(const T& a, const T& b)
		{
			return a <= b;
		}

		static bool IsGreaterOrEqual(const T& a, const T& b)
		{
			return a >= b;
		}
	};
}