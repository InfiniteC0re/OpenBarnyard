#include "ToshiPCH.h"
#include "TFreeList.h"

namespace Toshi {

	TFreeList::TFreeList(TUINT a_uiItemSize, TINT a_iInitialSize, TINT a_iGrowSize)
	{
		m_uiItemSize = a_uiItemSize;
		m_iCapacity = 0;
		m_pMemoryHeap = TNULL;
		TASSERT(m_iGrowSize >= 0);
		TASSERT(a_iInitialSize >= 0);
		SetGrowSize(a_iGrowSize);

		m_pPrevList = ms_pLastList;
		ms_pLastList = this;
	}

	TFreeList::Node* TFreeList::Allocate(TINT a_iNumber, TINT a_iSize)
	{
		TASSERT(a_iNumber > 0);
		m_iCapacity += a_iNumber;

		Node* pNewNode = (m_pMemoryHeap != TNULL) ?
			TREINTERPRETCAST(Node*, m_pMemoryHeap->Malloc(a_iNumber * a_iSize + sizeof(Node))) :
			TREINTERPRETCAST(Node*, TMalloc(a_iNumber * a_iSize + sizeof(Node)));

		pNewNode->pNext = m_RootNode.pNext;
		m_RootNode.pNext = pNewNode;

		auto pData = pNewNode + 1;
		Node* pNext = TNULL;

		for (TINT i = a_iNumber - 1; i != 0; i--)
		{
			pData->pNext = pNext;
			pNext = pData;

			pData = TREINTERPRETCAST(Node*, TREINTERPRETCAST(uintptr_t, pData) + a_iSize);
		}

		m_LastNode.pNext = pNext;
		return pData;
	}

	void TFreeList::SetCapacity(TINT a_iNewCapacity)
	{
		if (m_iCapacity < a_iNewCapacity)
		{
			auto pNode = Allocate(a_iNewCapacity - m_iCapacity, m_uiItemSize);

			pNode->pNext = m_LastNode.pNext;
			m_LastNode.pNext = pNode;
		}
	}

	void* TFreeList::New(TUINT a_uiSize)
	{
		if (a_uiSize != m_uiItemSize)
		{
			return TMalloc(a_uiSize);
		}

		auto pNode = m_LastNode.pNext;

		if (pNode != TNULL)
		{
			m_LastNode.pNext = pNode->pNext;
			return pNode;
		}
		else
		{
			return Allocate(m_iGrowSize, a_uiSize);
		}
	}

	void TFreeList::Delete(void* a_Ptr)
	{
		Node* pNode = TSTATICCAST(Node*, a_Ptr);

		if (m_LastNode.pNext != TNULL)
		{
			pNode->pNext = m_LastNode.pNext;
			m_LastNode.pNext = pNode;
		}
		else
		{
			m_LastNode.pNext = pNode;
			pNode->pNext = TNULL;
		}
	}

}
