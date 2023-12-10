#pragma once
#include "Toshi/Core/TNodeList.h"

namespace Toshi {

	class TMemoryLegacy :
		public TSingleton<TMemoryLegacy>
	{
	public:
		static constexpr TUINT NUM_BLOCK_SLOTS = 128;
		static constexpr TUINT NUM_FREE_LISTS = 9;

		struct MemBlock;
		struct MemBlockSlot;

		struct Hole
		{
			Hole* m_Unk1;
			TUINT m_uiSize;
			union {
				Hole* m_pNextHole;
				MemBlock* m_pMemBlock;
			};
			union {
				Hole* m_pPrevHole;
			};
		};

		struct MemBlock
		{
			MemBlockSlot* m_pSlot;
			TUINT m_uiTotalSize1;
			MemBlock* m_pNextBlock;
			TUINT m_uiTotalSize2;
			Hole* m_pHoles[NUM_FREE_LISTS];
			Hole* m_pFirstHole;
			char m_szSignature[8];
			char m_szName[52];
			Hole m_RootHole;
		};

		struct MemBlockFooter
		{
			TUINT m_Unk1;            // -0x10
			TUINT m_Unk2;            // -0x0C
			MemBlock* m_pBlockOwner; // -0x08
			TUINT m_Unk4;            // -0x04
		};

		struct MemBlockSlot :
			public TNodeList<MemBlockSlot>::TNode
		{
			MemBlock* m_pPtr = TNULL;
		};

	public:
		TMemoryLegacy();

		void* Alloc(TUINT a_uiSize, TINT a_uiAlignment, MemBlock* a_pMemBlock, char* a_szUnused1, TINT a_iUnused2);
		TBOOL Free(void* a_pMem);

		MemBlock* CreateHeapInPlace(void* a_pMem, TUINT a_uiSize, const char* a_szName);

		MemBlock* GetMainMemBlock() const
		{
			return m_pMainBlock;
		}

	private:
		void DumpMemInfo();
		void PrintDebug(const char* a_szFormat, ...);

	public:
		static TBOOL Initialise(TUINT a_uiHeapSize, TUINT a_uiReservedSize);
		static TUINT MapSizeToFreeList(TUINT a_uiSize);

	private:
		inline static class TMutex* ms_pGlobalMutex;

	private:
		TUINT m_TotalAllocatedSize;
		TUINT m_ReservedSize;
		TUINT m_MainBlockSize;
		TNodeList<MemBlockSlot> m_UsedBlocks;
		TNodeList<MemBlockSlot> m_FreeBlocks;
		MemBlockSlot m_aBlockSlots[NUM_BLOCK_SLOTS];
		void* m_pMainBlockMemory;
		MemBlock* m_pMainBlock;
		TUINT m_Unknown1;
		TUINT m_Unknown2;
		TBOOL m_bFlag1;
		TBOOL m_bFlag2;
		TBOOL m_bFlag3;
		TBOOL m_bFlag4;
		char m_Unknown4[104];
	};

}
