#pragma once

namespace Toshi {

	class TFreeList
	{
	public:
		struct Node
		{
			Node* pNext = TNULL;
		};

	public:
		TFreeList(TUINT a_uiItemSize, TINT a_iInitialSize, TINT a_iGrowSize);

		Node* Allocate(TINT a_iNumber, TINT a_iSize);
		void SetCapacity(TINT a_iNewCapacity);

		TINT GetCapacity() const { return m_iCapacity; }
		TINT GetGrowSize() const { return m_iGrowSize; }

		void SetGrowSize(TINT a_iGrowSize) { a_iGrowSize < 0 ? m_iGrowSize = 8 : m_iGrowSize = a_iGrowSize; }
		void* New(TUINT a_uiSize);
		
		void Delete(void* a_Ptr);

	private:
		inline static TFreeList* ms_pLastList = TNULL;

	private:
		TFreeList* m_pPrevList;
		TINT m_iCapacity;
		TUINT m_uiItemSize;
		Node m_LastNode;
		Node m_RootNode;
		TINT m_iGrowSize;
		TMemoryHeap* m_pMemoryHeap;
	};
}
