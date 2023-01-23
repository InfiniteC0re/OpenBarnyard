#pragma once

namespace Toshi
{
	template <class First, class Second, class Comparator>
	class T2Pair
	{
	public:
		First& GetFirst()
		{
			return m_First;
		}

		Second& GetSecond()
		{
			return m_Second;
		}

	private:
		First m_First;
		Second m_Second;
	};
}