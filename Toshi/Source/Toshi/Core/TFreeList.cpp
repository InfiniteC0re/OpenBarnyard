#include "ToshiPCH.h"
#include "TFreeList.h"

namespace Toshi {

	TFreeList::TFreeList(TUINT a_uiItemSize, TINT a_iInitialSize, TINT a_iGrowSize)
	{
		m_uiItemSize = a_uiItemSize;
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

	void* TFreeList::Allocate(TINT a_iNumber, TINT a_iSize)
	{
		TASSERT(a_iNumber > 0);
		m_iCapacity += a_iNumber;

		TFreeList* newList = TNULL;

		if (m_pMemoryHeap != TNULL)
		{
			newList = TREINTERPRETCAST(TFreeList*, m_pMemoryHeap->Malloc(a_iNumber * a_iSize + 4));
		}
		else
		{
			newList = TREINTERPRETCAST(TFreeList*, TMalloc(a_iNumber * a_iSize + 4));
		}

		a_iNumber--;

		newList->m_pNextBlock = m_pRootBlock;
		m_pRootBlock = newList;

		TFreeList* newList2 = TNULL;

		for (size_t i = a_iNumber; i > 0; i--)
		{
			newList2 = newList;
			newList2->m_pNextBlock = newList;
			newList = m_pNextBlock + a_iSize;
			newList = newList2;
		}
		m_pLastNode = newList2;
		return newList;
	}

	void TFreeList::SetCapacity(TINT a_iNewCapacity)
	{
		if (m_iCapacity < a_iNewCapacity)
		{
			auto pBlock = TREINTERPRETCAST(TFreeList*, Allocate(a_iNewCapacity - m_iCapacity, m_uiItemSize));
			pBlock->m_pNextBlock = m_pLastNode;
			m_pLastNode = pBlock;
		}
	}

	void* TFreeList::New(TUINT a_uiSize)
	{
		if (a_uiSize != m_uiItemSize)
		{
			return TMalloc(a_uiSize);
		}

		TFreeList* lastNode = m_pLastNode;

		if (lastNode == TNULL)
		{
			TASSERT((0 < m_iGrowSize), "Tried to grow TFreeList with 0 grow size");
			return Allocate(m_iGrowSize, a_uiSize);
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
