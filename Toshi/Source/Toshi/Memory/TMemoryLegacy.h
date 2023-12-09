#pragma once
#include "Toshi/Core/TNodeList.h"

namespace Toshi {

	class TMemoryLegacy :
		public TSingleton<TMemoryLegacy>
	{
	public:
		struct MemBlockSlot;

		struct Chunk
		{
			TUINT m_Unk1;
			TUINT m_uiDataSize;
			TUINT m_Unk2;
			TUINT m_Unk3;
		};

		struct MemBlock
		{
			MemBlockSlot* m_pSlot;
			TUINT m_uiTotalSize1;
			MemBlock* m_pNextBlock;
			TUINT m_uiTotalSize2;
			Chunk* m_pFreeLists[9];
			Chunk* m_pFirstChunk;
			char m_szSignature[8];
			char m_szName[52];
			Chunk m_RootChunk;
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

		static constexpr TUINT NUM_BLOCK_SLOTS = 128;

	public:
		TMemoryLegacy();

		MemBlock* CreateHeapInPlace(void* a_pMem, TUINT a_uiSize, const char* a_szName);

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
