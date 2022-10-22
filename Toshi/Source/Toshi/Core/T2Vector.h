#pragma once
namespace Toshi2
{
	template<typename T, int Cap>
	class T2Vector
	{
		int m_iNumElements = 0;

		T m_array[Cap];

		template<typename I>
		void push_back(I item)
		{
			m_array[m_iNumElements++] = item;
		}

		T pop_back()
		{
			T value = m_array[m_iNumElements];
			Toshi::tfree(m_array[m_iNumElements]);
			return value;
		}

		inline int Size() const { return m_iNumElements; }
		inline int Capacity() const { return Cap; }
	};
}

