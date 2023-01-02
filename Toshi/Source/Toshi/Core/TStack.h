#pragma once
namespace Toshi
{
	template <typename T, int C>
	class TStack
	{
	public:
		inline void Push(T& a_item)
		{
			m_iTop++;
			if (IsFull()) return;
			m_pStack[m_iTop] = a_item;
		}

		inline void PushNull()
		{
			m_iTop++;
		}

		inline T& Pop()
		{
			T& item = m_pStack[m_iTop];
			m_iTop--;
			if (IsEmpty()) m_iTop = 0;
			return item;
		}

		inline bool IsFull()
		{
			return m_iTop == (C - 1);
		}

		inline T& Top()
		{
			return m_pStack[m_iTop];
		}

		inline bool IsEmpty()
		{
			return m_iTop == -1;
		}

		inline void Reset()
		{
			m_iTop = 0;
		}

	private:
		T m_pStack[C];
		int m_iTop = -1;
	};
}