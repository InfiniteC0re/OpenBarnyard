#include "ToshiPCH.h"
#include "TFreeList.h"

namespace Toshi {

	TFreeList::TFreeList(int size, int a_iInitialSize, int a_iGrowSize)
	{
		m_iItemSize = size;
		m_iCapacity = 0;
		m_pLastNode = TNULL;
		m_pRootBlock = TNULL;
		m_pMemoryHeap = TNULL;
		TASSERT(m_iGrowSize >= 0);
		TASSERT(a_iInitialSize >= 0);
		SetGrowSize(a_iGrowSize);
		m_pNextBlock = ms_pRootFreeList;
		ms_pRootFreeList = this;
	}

	void* TFreeList::Allocate(int a_iNumber, int size)
	{
		TASSERT(a_iNumber > 0);
		m_iCapacity += a_iNumber;

		TFreeList* newList = TNULL;

		if (m_pMemoryHeap != TNULL)
		{
			newList = reinterpret_cast<TFreeList*>(m_pMemoryHeap->Malloc(a_iNumber * size + 4));
		}
		else
		{
			newList = reinterpret_cast<TFreeList*>(TMalloc(a_iNumber * size + 4));
		}

		a_iNumber--;

		newList->m_pNextBlock = m_pRootBlock;
		m_pRootBlock = newList;

		TFreeList* newList2 = TNULL;

		for (size_t i = a_iNumber; i > 0; i--)
		{
			newList2 = newList;
			newList2->m_pNextBlock = newList;
			newList = m_pNextBlock + size;
			newList = newList2;
		}
		m_pLastNode = newList2;
		return newList;
	}

	void* TFreeList::New(uint32_t size)
	{
		if (size != m_iItemSize)
		{
			return TMalloc(size);
		}

		TFreeList* lastNode = m_pLastNode;

		if (lastNode == TNULL)
		{
			TASSERT((0 < m_iGrowSize), "Tried to grow TFreeList with 0 grow size\n");
			return Allocate(m_iGrowSize, size);
		}

		m_pLastNode = m_pLastNode->m_pNextBlock;
		return lastNode;
	}

	void TFreeList::Delete(void* a_pData)
	{
		if (m_pLastNode)
		{
			a_pData = m_pLastNode;
			m_pLastNode = TSTATICCAST(TFreeList*, a_pData);
		}
		else
		{
			m_pLastNode = TSTATICCAST(TFreeList*, a_pData);
			a_pData = TNULL;
		}
	}

}
