#pragma once



template <class T>
class TArray
{
private:
	class TGenericArray
	{
		void* GetArrayAt(int a_iIndex = 0) const
		{
			TASSERT(a_iIndex >= 0);
			TASSERT(a_iIndex < m_iNumElements);
			return m_pData[a_iIndex];
		}

	private:

		int m_iGrowSize;         // 0x0
		int m_iNumElements;      // 0x4
		int m_iNumAllocElements; // 0x8
		T* m_pData;              // 0xC

	};
public:
	int IncrementIndex()
	{
		m_iIndex++;
		TASSERT(m_poArray);
		if (m_poArray->m_iNumElements <= m_iIndex || m_iIndex == 0) m_iIndex = -1;
		return m_iIndex;
	}

	T* GetArrayAt() const
	{
		TASSERT(m_iIndex >= 0);
		TASSERT(m_poArray);
		TASSERT(m_iIndex < m_poArray->m_iNumElements);
		return m_poArray->m_pData[m_iIndex];
	}
private:
	int m_iIndex;             // 0x0
	TGenericArray* m_poArray; // 0x4
};