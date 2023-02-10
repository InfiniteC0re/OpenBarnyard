#pragma once
namespace Toshi
{
	template <typename T, int C>
	class TStack
	{
	public:
		void Push(T& a_item)
		{
			m_iTop++;
			if (IsFull()) return;
			m_pStack[m_iTop] = a_item;
		}

		void PushNull()
		{
			m_iTop++;
		}

		T& Pop()
		{
			T& item = m_pStack[m_iTop];
			m_iTop--;
			if (IsEmpty()) m_iTop = 0;
			return item;
		}

		bool IsFull()
		{
			return m_iTop == (C - 1);
		}

		T& Top()
		{
			return m_pStack[m_iTop];
		}

		bool IsEmpty()
		{
			return m_iTop == -1;
		}

		void Reset()
		{
			m_iTop = 0;
		}

	private:
		T m_pStack[C];
		int m_iTop = -1;
	};
}