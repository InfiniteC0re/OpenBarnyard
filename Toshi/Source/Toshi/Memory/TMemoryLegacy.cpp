#include "ToshiPCH.h"
#include "TMemoryLegacy.h"
#include "Toshi/Thread/TMutex.h"
#include "Toshi/Thread/TMutexLock.h"

namespace Toshi {

	TMemoryLegacy::TMemoryLegacy()
	{
		m_TotalAllocatedSize = 0;

		for (TINT i = 0; i < NUM_BLOCK_SLOTS; i++)
		{
			m_FreeBlocks.InsertTail(m_aBlockSlots[i]);
		}

		TUtil::MemClear(m_Unknown4, sizeof(m_Unknown4));
		m_bFlag1 = TFALSE;
		m_bFlag2 = TTRUE;
	}

	TMemoryLegacy::MemBlock* TMemoryLegacy::CreateHeapInPlace(void* a_pMem, TUINT a_uiSize, const char* a_szName)
	{
		TMutexLock lock(ms_pGlobalMutex);

		if (a_pMem && a_uiSize != 0 && !m_FreeBlocks.IsEmpty())
		{
			auto pBlock = TREINTERPRETCAST(MemBlock*, TMath::AlignNum(TREINTERPRETCAST(TUINT32, a_pMem)));
			auto uiBlockTotalSize = (TREINTERPRETCAST(TUINT32, a_pMem) + TMath::AlignNum(a_uiSize)) - TREINTERPRETCAST(TUINT32, pBlock);
			
			if (pBlock)
			{
				auto pSlot = m_FreeBlocks.RemoveHead();
				m_UsedBlocks.InsertHead(*pSlot);

				pBlock->m_pSlot = pSlot;
				pBlock->m_pSlot->m_pPtr = pBlock;
			}

			if (uiBlockTotalSize != 0)
			{
				constexpr TUINT CHUNK_RESERVED_SIZE = (sizeof(MemBlock) + 12);

				pBlock->m_uiTotalSize1 = uiBlockTotalSize;
				pBlock->m_pFreeLists[0] = TNULL;
				pBlock->m_pFreeLists[1] = TNULL;
				pBlock->m_pFreeLists[2] = TNULL;
				pBlock->m_pFreeLists[3] = TNULL;
				pBlock->m_pFreeLists[4] = TNULL;
				pBlock->m_pFreeLists[5] = TNULL;
				pBlock->m_pFreeLists[6] = TNULL;
				pBlock->m_pFreeLists[7] = TNULL;
				pBlock->m_pFreeLists[8] = TNULL;
				pBlock->m_pFirstChunk = &pBlock->m_RootChunk;

				auto pChunk = &pBlock->m_RootChunk;
				pChunk->m_uiDataSize = uiBlockTotalSize - CHUNK_RESERVED_SIZE;
				pChunk->m_Unk2 = 0;
				pChunk->m_Unk3 = 0;
				pChunk->m_Unk1 = 0;

				auto uiFreeListId = MapSizeToFreeList(uiBlockTotalSize - CHUNK_RESERVED_SIZE);
				pBlock->m_pFreeLists[uiFreeListId] = pChunk;

				auto pBlockFooter = TREINTERPRETCAST(MemBlockFooter*, TREINTERPRETCAST(TUINT32, pBlock) + uiBlockTotalSize) - 1;
				pBlockFooter->m_pBlockOwner = TNULL;
				pBlockFooter->m_Unk4 = 0;
				pBlockFooter->m_Unk1 = 0;
				pBlockFooter->m_Unk2 = GetSingleton()->m_Unknown1 | 1;
				pBlockFooter->m_pBlockOwner = pBlock;

				pBlock->m_pNextBlock = pBlock;
				pBlock->m_uiTotalSize2 = uiBlockTotalSize;
				TStringManager::String8Copy(pBlock->m_szSignature, "#MB_uID");
				TStringManager::String8Copy(pBlock->m_szName, a_szName);

				return pBlock;
			}
		}

		return TNULL;
	}

	TBOOL TMemoryLegacy::Initialise(TUINT a_uiHeapSize, TUINT a_uiReservedSize)
	{
		auto tmemory = TSTATICCAST(TMemoryLegacy*, calloc(sizeof(TMemoryLegacy), 1));
		new (tmemory) TMemoryLegacy();

		tmemory->m_pMainBlockMemory = TNULL;
		tmemory->m_pMainBlock = TNULL;
		tmemory->m_Unknown1 = 0;
		tmemory->m_Unknown2 = 0;

		ms_pGlobalMutex = TSTATICCAST(TMutex*, malloc(sizeof(TMutex)));
		new (ms_pGlobalMutex) TMutex();

		tmemory->m_ReservedSize = a_uiReservedSize;
		tmemory->m_TotalAllocatedSize = (a_uiHeapSize == 0) ? 128 * 1024 * 1024 : a_uiHeapSize;
		tmemory->m_MainBlockSize = tmemory->m_TotalAllocatedSize - a_uiReservedSize;
		tmemory->m_pMainBlockMemory = malloc(tmemory->m_TotalAllocatedSize);
		tmemory->m_pMainBlock = tmemory->CreateHeapInPlace(
			tmemory->m_pMainBlockMemory,
			tmemory->m_TotalAllocatedSize,
			"Toshi"
		);

		return TTRUE;
	}

	TUINT TMemoryLegacy::MapSizeToFreeList(TUINT a_uiSize)
	{
		TFLOAT fSize = TFLOAT(TMath::AlignNumUp(a_uiSize) - 1);
		TUINT uiExponentSign = (*(TINT*)&fSize) >> 23;
		TUINT uiResult = uiExponentSign - 0x7F;

		if (uiResult & 1)
			uiResult = uiExponentSign - 0x7e;

		uiResult = uiResult >> 1;

		if (uiResult != 0)
			uiResult = uiResult - 1;

		if (8 < uiResult)
			uiResult = 8;

		return uiResult;
	}

}
