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

	void* TMemoryLegacy::Alloc(TUINT a_uiSize, TINT a_uiAlignment, MemBlock* a_pMemBlock, char* a_szUnused1, TINT a_iUnused2)
	{
		TMutexLock lock(ms_pGlobalMutex);

		if (a_uiSize < 4)
		{
			a_uiSize = 4;
		}

		if (a_uiAlignment < 16)
		{
			a_uiAlignment = 16;
		}
		else if (a_uiAlignment < 4)
		{
			PrintDebug("Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName);
			return TNULL;
		}

		a_uiSize = TMath::AlignNumUp(a_uiSize);
		TUINT uiFreeListId = MapSizeToFreeList(a_uiSize);

		TUINT iDataStart = 0;
		TUINT iDataEnd;
		TUINT iChunkSize;
		Hole* pFreeList = TNULL;

		if (uiFreeListId < NUM_FREE_LISTS)
		{
			for (TUINT i = uiFreeListId; i < NUM_FREE_LISTS; i++)
			{
				pFreeList = a_pMemBlock->m_pHoles[i];

				while (pFreeList != TNULL)
				{
					iDataEnd = TREINTERPRETCAST(TUINT, &pFreeList->m_pPrev) + TMath::AlignNum(pFreeList->m_uiDataSize);
					iDataStart = (TREINTERPRETCAST(TUINT, &pFreeList->m_pNext) + 3 + a_uiAlignment) & ~(a_uiAlignment - 1);
					iChunkSize = iDataEnd - iDataStart;

					if (iDataStart < iDataEnd && a_uiSize <= iChunkSize)
					{
						auto pDataHeader = ((DataHeader*)iDataStart) - 1;
						auto unk1 = pFreeList->m_Unk1;

						if (unk1 == 0)
						{
							if (pDataHeader->m_pChunk != pFreeList)
							{
								a_pMemBlock->m_pFirstHole = pDataHeader->m_pChunk;
							}
						}
						else
						{
							*(TUINT*)(unk1 + 4) = iDataStart + (-0x18 - unk1) | *(TUINT*)(unk1 + 4) & 3;
						}

						if (a_uiSize + 16 < iChunkSize)
						{
							if (pFreeList->m_pPrev == TNULL)
							{
								a_pMemBlock->m_pHoles[i] = pFreeList->m_pNext;
							}
							else
							{
								pFreeList->m_pPrev->m_pNext = pFreeList->m_pNext;
							}

							if (pFreeList->m_pNext != TNULL)
							{
								pFreeList->m_pNext->m_pPrev = pFreeList->m_pPrev;
							}
							
							Hole* unk2 = (Hole*)(iDataStart + -0xc);

							if (pFreeList != unk2)
							{
								unk1 = pFreeList->m_uiDataSize;
								unk2->m_Unk1 = pFreeList->m_Unk1;
								*(Hole**)((int)&pFreeList->m_pPrev + unk1) = unk2;
							}

							pDataHeader->m_uiSize = ms_pSingleton->m_Unknown1 | iChunkSize | 1;
							pDataHeader->m_pMemBlock = a_pMemBlock;

							auto pNewHole = (Hole*)(iDataStart + a_uiSize);
							TVALIDPTR(pNewHole);

							pNewHole->m_uiDataSize = ((TMath::AlignNum(pDataHeader->m_uiSize) - iDataStart) - a_uiSize) + (TUINT)unk2;

							pDataHeader->m_pMemBlock = a_pMemBlock;
							pDataHeader->m_uiSize = (TUINT)pNewHole + (-0xc - (TUINT)unk2) | a_uiSize | 1;
							pNewHole->m_Unk1 = (TUINT)unk2;

							TUINT uiNewFreeListId = MapSizeToFreeList(pNewHole->m_uiDataSize);
							auto pOldChunk = a_pMemBlock->m_pHoles[uiNewFreeListId];
							pNewHole->m_pNext = pOldChunk;
							
							if (pOldChunk != TNULL)
							{
								pOldChunk->m_pPrev = pNewHole;
							}

							pNewHole->m_pPrev = TNULL;
							a_pMemBlock->m_pHoles[uiNewFreeListId] = pNewHole;
							*(Hole**)((TUINT)&pNewHole->m_pPrev + TMath::AlignNum(pNewHole->m_uiDataSize)) = pNewHole;

							return (void*)iDataStart;
						}
						else
						{
							if (pFreeList->m_pPrev == TNULL)
							{
								a_pMemBlock->m_pHoles[i] = pFreeList->m_pNext;
							}
							else
							{
								pFreeList->m_pPrev->m_pNext = pFreeList->m_pNext;
							}

							if (pFreeList->m_pNext != TNULL)
							{
								pFreeList->m_pNext->m_pPrev = pFreeList->m_pPrev;
							}

							auto pChunk = (Hole*)(iDataStart - 0xc);

							if (pFreeList != pChunk)
							{
								unk1 = pFreeList->m_uiDataSize;
								pChunk->m_Unk1 = pFreeList->m_Unk1;
								*(Hole**)((int)&pFreeList->m_pPrev + TMath::AlignNum(unk1)) = pChunk;
							}

							pDataHeader->m_uiSize = ms_pSingleton->m_Unknown1 | iChunkSize | 1;
							pDataHeader->m_pMemBlock = a_pMemBlock;

							return (void*)iDataStart;
						}
					}	
				}
			}

			if (pFreeList)
			{
				return (void*)iDataStart;
			}
		}

		PrintDebug("Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName);
		PrintDebug("Requested memory block size: %d\n", a_uiSize);
		DumpMemInfo();

		return TNULL;
	}

	void TMemoryLegacy::Free(void* a_pMem)
	{
		TMutexLock lock(ms_pGlobalMutex);
		TUINT uiMem = TREINTERPRETCAST(TUINT, a_pMem);

		if ((uiMem & 3) == 0 && a_pMem != TNULL)
		{

		}
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
				TUtil::MemClear(pBlock->m_pHoles, sizeof(pBlock->m_pHoles));
				pBlock->m_pFirstHole = &pBlock->m_RootHole;

				auto pHole = &pBlock->m_RootHole;
				pHole->m_uiDataSize = uiBlockTotalSize - CHUNK_RESERVED_SIZE;
				pHole->m_pNext = TNULL;
				pHole->m_pPrev = TNULL;
				pHole->m_Unk1 = 0;

				auto uiFreeListId = MapSizeToFreeList(uiBlockTotalSize - CHUNK_RESERVED_SIZE);
				pBlock->m_pHoles[uiFreeListId] = pHole;

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

	void TMemoryLegacy::PrintDebug(const char* a_szFormat, ...)
	{
		va_list args;
		va_start(args, a_szFormat);
		auto pBuffer = TStringManager::GetTempString8();
		int iLen = vsprintf(pBuffer, a_szFormat, args);
		OutputDebugStringA(pBuffer);
		va_end(args);

#ifdef TOSHI_DEBUG
		pBuffer[iLen - 1] = '\0';
		TOSHI_CRITICAL(pBuffer);
		TASSERT(TFALSE);
#endif
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
		TUINT uiResult = uiExponentSign - 127;

		if (uiResult & 1)
			uiResult = uiExponentSign - 126;

		uiResult = uiResult >> 1;

		if (uiResult != 0)
			uiResult = uiResult - 1;

		if (uiResult > NUM_FREE_LISTS - 1)
			uiResult = NUM_FREE_LISTS - 1;

		return uiResult;
	}

	void TMemoryLegacy::DumpMemInfo()
	{
		TIMPLEMENT();
	}

}
