
template <typename T, int C>
class AStack
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
		if (IsEmpty()) Reset();
		return item;
	}

	TBOOL IsFull()
	{
		return m_iTop == (C - 1);
	}

	T& Top()
	{
		return m_pStack[m_iTop];
	}

	TBOOL IsEmpty() const
	{
		return m_iTop == -1;
	}

	void Reset()
	{
		m_iTop = -1;
	}

	int Size() const
	{
		return m_iTop + 1;
	}

	T& operator[] (int index) const
	{
		return &m_pStack[index];
	}

	T& operator[] (int index)
	{
		return m_pStack[index];
	}

private:
	int m_iTop = -1;
	T m_pStack[C];
};