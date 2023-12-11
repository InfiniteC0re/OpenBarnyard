#pragma once
#include "Toshi/Core/TNodeList.h"
#include "Toshi/Utils/TSingleton.h"

namespace Toshi {

	class TMemory :
		public TSingleton<TMemory>
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
			Hole* m_pPrevHole;
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

		struct MemInfo
		{
			TUINT m_uiTotalSize;
			TUINT m_uiLogicTotalSize;
			TUINT m_uiTotalFree;
			TUINT m_uiLogicTotalFree;
			TUINT m_uiTotalUsed;
			TUINT m_uiLogicTotalUsed;
			TINT m_iNumHoles;
			TINT m_iNumProcesses;
			TUINT m_uiLargestHole;
			TUINT m_uiLargestProcess;
			TUINT m_uiSmallestHole;
			TUINT m_uiSmallestProcess;
			TUINT m_uiUnk3;
			TUINT m_uiUnk4;
		};

		struct HALMemInfo
		{
			HALMemInfo();

			TUINT m_Unknown[26];
		};

	public:
		TMemory();

		void* Alloc(TUINT a_uiSize, TINT a_uiAlignment, MemBlock* a_pMemBlock, const char* a_szUnused1, TINT a_iUnused2);
		
		TBOOL Free(void* a_pMem);

		MemBlock* CreateMemBlock(TUINT a_uiSize, const char* a_szName, MemBlock* a_pOwnerBlock);
		MemBlock* CreateMemBlockInPlace(void* a_pMem, TUINT a_uiSize, const char* a_szName);
		void DestroyMemBlock(MemBlock* a_pMemBlock);

		MemBlock* GetGlobalBlock() const
		{
			return m_pGlobalBlock;
		}

		MemBlock* SetGlobalBlock(MemBlock* a_pMemBlock)
		{
			auto pOldMemBlock = m_pGlobalBlock;
			m_pGlobalBlock = a_pMemBlock;
			return pOldMemBlock;
		}

		void DumpMemInfo();
		void GetHALMemInfo(HALMemInfo& a_rHALMemInfo);
		void PrintDebug(const char* a_szFormat, ...);

	private:
		TBOOL FreeMemBlock(MemBlock* a_pMemBlock);
		void SetMemBlockUnused(MemBlock* a_pMemBlock);

	public:
		static void GetMemInfo(MemInfo& a_rMemInfo, MemBlock* a_pMemBlock);
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
		MemBlock* m_pGlobalBlock;
		TUINT m_Unknown1;
		TUINT m_Unknown2;
		TBOOL m_bFlag1;
		TBOOL m_bFlag2;
		HALMemInfo m_HALMemInfo;
	};

}

void* TMalloc(TUINT a_uiSize);
void* TMalloc(TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock, const char* a_szUnused1 = TNULL, TINT a_iUnused2 = -1);
void* TMemalign(TUINT a_uiSize, TINT a_iAlignment);
void* TMemalign(TINT a_iAlignment, TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock);
void TFree(void* a_pMem);

inline void* __CRTDECL operator new(size_t size, Toshi::TMemory::MemBlock* block)
{
	return TMalloc(size, block, TNULL, -1);
}

inline void* __CRTDECL operator new[](size_t size, Toshi::TMemory::MemBlock* block)
{
	return TMalloc(size, block, TNULL, -1);
}

inline void __CRTDECL operator delete(void* ptr, Toshi::TMemory::MemBlock* block) noexcept
{
	TFree(ptr);
}

inline void __CRTDECL operator delete[](void* ptr, Toshi::TMemory::MemBlock* block) noexcept
{
	TFree(ptr);
}
