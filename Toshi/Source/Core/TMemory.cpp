#include "ToshiPCH.h"
#include "TMemory.h"
#include "Thread/TMutex.h"
#include "Thread/TMutexLock.h"

#ifdef TMEMORY_DEBUG
#include "Core/TMemoryDebug.h"
#include "Toshi/T2Allocator.h"
#include "Toshi/T2DList.h"
#include "Toshi/T2NamedPipeServer.h"
#endif // TMEMORY_DEBUG

#define MEM_TO_HOLE(PTR) (((Hole*)(((TUINT)PTR) + 4)) - 1)

void* __CRTDECL operator new(size_t size)
{
#ifdef TMEMORY_DEBUG
	return TMalloc(size, TNULL, TMemory__FILE__, TMemory__LINE__);
#else
	return TMalloc(size, TNULL, TNULL, -1);
#endif // TMEMORY_DEBUG
}

void* __CRTDECL operator new(size_t size, ::std::nothrow_t const&) noexcept
{
#ifdef TMEMORY_DEBUG
	return TMalloc(size, TNULL, TMemory__FILE__, TMemory__LINE__);
#else
	return TMalloc(size, TNULL, TNULL, -1);
#endif // TMEMORY_DEBUG
}

void* __CRTDECL operator new[](size_t size)
{
#ifdef TMEMORY_DEBUG
	return TMalloc(size, TNULL, TMemory__FILE__, TMemory__LINE__);
#else
	return TMalloc(size, TNULL, TNULL, -1);
#endif // TMEMORY_DEBUG
}

void* __CRTDECL operator new[](size_t size, ::std::nothrow_t const&) noexcept
{
#ifdef TMEMORY_DEBUG
	return TMalloc(size, TNULL, TMemory__FILE__, TMemory__LINE__);
#else
	return TMalloc(size, TNULL, TNULL, -1);
#endif // TMEMORY_DEBUG
}

void __CRTDECL operator delete(void* ptr) noexcept
{
	TFree(ptr);
}

void __CRTDECL operator delete(void* ptr, ::std::nothrow_t const&) noexcept
{
	TFree(ptr);
}

void __CRTDECL operator delete[](void* ptr) noexcept
{
	TFree(ptr);
}

void __CRTDECL operator delete[](void* ptr, ::std::nothrow_t const&) noexcept
{
	TFree(ptr);
}

void __CRTDECL operator delete[](void* ptr, size_t _Size) noexcept
{
	TFree(ptr);
}

namespace Toshi {

#ifdef TMEMORY_DEBUG

	class TMemoryAllocationDebugInfo : public T2DList<TMemoryAllocationDebugInfo>::Node
	{
	public:
		TMemoryAllocationDebugInfo()
		{
			SetMemory(TNULL);
			SetFileName(TNULL);
			SetLineNum(-1);
		}

		void* GetMemory() const { return m_pMemory; }
		void SetMemory(void* a_pMemory) { m_pMemory = a_pMemory; }

		const TCHAR* GetFileName() const { return m_szFileName; }
		void SetFileName(const TCHAR* a_szFileName) { m_szFileName = a_szFileName; }

		TINT GetLineNum() const { return m_iLineNum; }
		void SetLineNum(TINT a_iLineNum) { m_iLineNum = a_iLineNum; }

	private:
		void* m_pMemory;
		const TCHAR* m_szFileName;
		TINT m_iLineNum;
	};

	static TMemoryAllocationDebugInfo g_MemoryDebugInfoSlots[65535];
	static T2DList<TMemoryAllocationDebugInfo> g_FreeDebugInfoSlots;
	static T2DList<TMemoryAllocationDebugInfo> g_UsedDebugInfoSlots;
	static TUINT g_uiDebugNumTrackedAllocations = 0;
	static T2NamedPipeServer g_MemoryDebugPipeServer;

#endif // TMEMORY_DEBUG

	TMemory::TMemory()
	{
		m_TotalAllocatedSize = 0;

		for (TINT i = 0; i < NUM_BLOCK_SLOTS; i++)
		{
			m_FreeBlocks.InsertTail(m_aBlockSlots[i]);
		}

		m_bFlag1 = TFALSE;
		m_bFlag2 = TTRUE;

#ifdef TMEMORY_DEBUG
		T2_FOREACH_ARRAY(g_MemoryDebugInfoSlots, i)
		{
			g_FreeDebugInfoSlots.PushBack(&g_MemoryDebugInfoSlots[i]);
		}
#endif // TMEMORY_DEBUG
	}

	TMemory::~TMemory()
	{
		
	}

	void* TMemory::Alloc(TUINT a_uiSize, TINT a_uiAlignment, MemBlock* a_pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum)
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
			TASSERT(TFALSE);
			return TNULL;
		}

		if (!a_pMemBlock)
		{
			a_pMemBlock = m_pGlobalBlock;
		}

		if (a_uiSize == 0)
		{
			PrintDebug("Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName);
			TASSERT(TFALSE);
			return TNULL;
		}

		a_uiSize = TMath::AlignNumUp(a_uiSize);
		TUINT uiFreeListId = MapSizeToFreeList(a_uiSize);

		TUINT iDataStart = 0;
		TUINT iDataEnd;
		TUINT iDataSize;
		Hole* pFreeList = TNULL;

#ifdef TMEMORY_DEBUG
		#include "TMemoryDebugOff.h"

		auto SaveDebugInfo = [a_szFileName, a_iLineNum](void* pMem) {
			if (pMem && a_szFileName)
			{
				TMemory__FILE__ = TNULL;
				TMemory__LINE__ = -1;

				TASSERT(!g_FreeDebugInfoSlots.IsEmpty(), "TMemory Debug: No empty slots left to save allocation info");

				auto pDebugInfo = g_FreeDebugInfoSlots.PopFront();
				pDebugInfo->SetMemory(pMem);
				pDebugInfo->SetFileName(a_szFileName);
				pDebugInfo->SetLineNum(a_iLineNum);

				g_UsedDebugInfoSlots.PushBack(pDebugInfo);
				g_uiDebugNumTrackedAllocations++;
			}
		};
#endif // TMEMORY_DEBUG

		for (TUINT i = uiFreeListId; i < NUM_FREE_LISTS; i++)
		{
			pFreeList = a_pMemBlock->m_pHoles[i];

			while (pFreeList != TNULL)
			{
				iDataEnd = TREINTERPRETCAST(TUINT, &pFreeList->m_pPrevHole) + TMath::AlignNum(pFreeList->m_uiSize);
				iDataStart = TREINTERPRETCAST(TUINT, &pFreeList->m_pNextHole) + a_uiAlignment + 3 & ~(a_uiAlignment - 1);
				iDataSize = iDataEnd - iDataStart;

				if (iDataStart < iDataEnd && a_uiSize <= iDataSize)
				{
					auto pHole = MEM_TO_HOLE(iDataStart);
					auto unk1 = pFreeList->m_Unk1;

					if (unk1 == TNULL)
					{
						if (pHole != pFreeList)
						{
							a_pMemBlock->m_pFirstHole = pHole;
						}
					}
					else
					{
						unk1->m_uiSize = (TUINT)(iDataStart + (-0x18 - (TUINT)unk1)) | unk1->m_uiSize & 3;
					}

					if (a_uiSize + sizeof(Hole) < iDataSize)
					{
						if (pFreeList->m_pPrevHole == TNULL)
						{
							a_pMemBlock->m_pHoles[i] = pFreeList->m_pNextHole;
						}
						else
						{
							pFreeList->m_pPrevHole->m_pNextHole = pFreeList->m_pNextHole;
						}

						if (pFreeList->m_pNextHole != TNULL)
						{
							pFreeList->m_pNextHole->m_pPrevHole = pFreeList->m_pPrevHole;
						}

						if (pFreeList != pHole)
						{
							auto uiSize = pFreeList->m_uiSize;
							pHole->m_Unk1 = pFreeList->m_Unk1;
							*(Hole**)((TUINT)&pFreeList->m_pPrevHole + TMath::AlignNum(uiSize)) = pHole;
						}

						pHole->m_uiSize = GetSingleton()->m_Unknown1 | iDataSize | 1;
						pHole->m_pMemBlock = a_pMemBlock;

						auto pNewHole = (Hole*)(iDataStart + a_uiSize);
						pNewHole->m_uiSize = ((TMath::AlignNum(pHole->m_uiSize) - iDataStart) - a_uiSize) + (TUINT)pHole;

						pHole->m_pMemBlock = a_pMemBlock;
						pHole->m_uiSize = (TINT)pNewHole + (-0xc - (TINT)pHole) | ms_pSingleton->m_Unknown1 | 1;
						pNewHole->m_Unk1 = pHole;

						TUINT uiNewFreeListId = MapSizeToFreeList(pNewHole->m_uiSize);

						auto pOldHole = a_pMemBlock->m_pHoles[uiNewFreeListId];
						pNewHole->m_pNextHole = pOldHole;

						if (pOldHole != TNULL)
						{
							pOldHole->m_pPrevHole = pNewHole;
						}

						pNewHole->m_pPrevHole = TNULL;
						a_pMemBlock->m_pHoles[uiNewFreeListId] = pNewHole;
						*(Hole**)((TUINT)&pNewHole->m_pPrevHole + TMath::AlignNum(pNewHole->m_uiSize)) = pNewHole;

#ifdef TMEMORY_DEBUG
						SaveDebugInfo((void*)iDataStart);
#endif // TMEMORY_DEBUG
						return (void*)iDataStart;
					}
					else
					{
						if (pFreeList->m_pPrevHole == TNULL)
						{
							a_pMemBlock->m_pHoles[i] = pFreeList->m_pNextHole;
						}
						else
						{
							pFreeList->m_pPrevHole->m_pNextHole = pFreeList->m_pNextHole;
						}

						if (pFreeList->m_pNextHole != TNULL)
						{
							pFreeList->m_pNextHole->m_pPrevHole = pFreeList->m_pPrevHole;
						}

						if (pFreeList != pHole)
						{
							auto uiSize = pFreeList->m_uiSize;
							pHole->m_Unk1 = pFreeList->m_Unk1;
							*(Hole**)((TUINT)&pFreeList->m_pPrevHole + TMath::AlignNum(uiSize)) = pHole;
						}

						pHole->m_uiSize = ms_pSingleton->m_Unknown1 | iDataSize | 1;
						pHole->m_pMemBlock = a_pMemBlock;

#ifdef TMEMORY_DEBUG
						SaveDebugInfo((void*)iDataStart);
#endif // TMEMORY_DEBUG

						return (void*)iDataStart;
					}
				}

				pFreeList = pFreeList->m_pNextHole;
			}

			pFreeList = TNULL;
		}

		if (pFreeList)
		{
#ifdef TMEMORY_DEBUG
			SaveDebugInfo((void*)iDataStart);
#endif // TMEMORY_DEBUG

			return (void*)iDataStart;
		}

		PrintDebug("Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName);
		PrintDebug("Requested memory block size: %d\n", a_uiSize);
		DumpMemInfo();
		TASSERT(TFALSE);

		return TNULL;
	}

	TBOOL TMemory::Free(void* a_pMem)
	{
		TMutexLock lock(ms_pGlobalMutex);

		TUINT uiMem = TREINTERPRETCAST(TUINT, a_pMem);

		if ((uiMem & 3) == 0 && a_pMem != TNULL)
		{
			auto pRootHole = MEM_TO_HOLE(a_pMem);
			auto pMemBlock = pRootHole->m_pMemBlock;
			auto uiSize = TMath::AlignNum(pRootHole->m_uiSize);
			auto ppHole = &pRootHole->m_pNextHole;
			auto pNextHole = (Hole*)(uiMem + uiSize);

			auto pHole = pRootHole->m_Unk1;
			pRootHole->m_uiSize = uiSize;

			if (pHole && (pHole->m_uiSize & 1) == 0)
			{
				auto uiHoleSize = pHole->m_uiSize;
				pHole->m_uiSize = uiSize + 0xc + TMath::AlignNum(uiHoleSize) | uiHoleSize & 3;
				pNextHole->m_Unk1 = pHole;
				pRootHole = pHole;
				ppHole = &pHole->m_pNextHole;

				if (pHole->m_pPrevHole == TNULL)
				{
					auto uiHoleId = MapSizeToFreeList(TMath::AlignNum(uiHoleSize));
					pMemBlock->m_pHoles[uiHoleId] = *ppHole;
				}
				else
				{
					pHole->m_pPrevHole->m_pNextHole = *ppHole;
				}


				if (*ppHole)
				{
					(*ppHole)->m_pPrevHole = pHole->m_pPrevHole;
				}
			}

			if ((pNextHole->m_uiSize & 1) == 0)
			{
				if (pNextHole->m_pPrevHole == TNULL)
				{
					auto uiHoleId = MapSizeToFreeList(TMath::AlignNum(pNextHole->m_uiSize));
					pMemBlock->m_pHoles[uiHoleId] = pNextHole->m_pNextHole;
				}
				else
				{
					pNextHole->m_pPrevHole->m_pNextHole = pNextHole->m_pNextHole;
				}

				if (pNextHole->m_pNextHole != TNULL)
				{
					pNextHole->m_pNextHole->m_pPrevHole = pNextHole->m_pPrevHole;
				}

				pRootHole->m_uiSize = TMath::AlignNum(pNextHole->m_uiSize) + 0xc + TMath::AlignNum(pRootHole->m_uiSize) | pRootHole->m_uiSize & 3;
				*(Hole**)((TUINT)&pNextHole->m_pPrevHole + TMath::AlignNum(pNextHole->m_uiSize)) = pRootHole;
			}

			auto uiHoleId = MapSizeToFreeList(TMath::AlignNum(pRootHole->m_uiSize));
			pRootHole->m_pPrevHole = TNULL;

			auto pOldHole = pMemBlock->m_pHoles[uiHoleId];
			*ppHole = pOldHole;

			if (pOldHole)
			{
				pOldHole->m_pPrevHole = pRootHole;
			}

			pMemBlock->m_pHoles[uiHoleId] = pRootHole;

#ifdef TMEMORY_DEBUG

			if (!g_UsedDebugInfoSlots.IsEmpty())
			{
				T2_FOREACH_BACK(g_UsedDebugInfoSlots, slot)
				{
					if (slot->GetMemory() == a_pMem)
					{
						slot->Remove();
						g_FreeDebugInfoSlots.PushBack(slot);
						g_uiDebugNumTrackedAllocations--;
						break;
					}
				}
			}

#endif // TMEMORY_DEBUG

			return TTRUE;
		}

		return TFALSE;
	}

	TMemory::MemBlock* TMemory::CreateMemBlock(TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock)
	{
		void* pMem = Alloc(a_uiSize, 16, a_pOwnerBlock, TNULL, -1);
		return CreateMemBlockInPlace(pMem, a_uiSize, a_szName);
	}

	TMemory::MemBlock* TMemory::CreateMemBlockInPlace(void* a_pMem, TUINT a_uiSize, const TCHAR* a_szName)
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
				pHole->m_uiSize = uiBlockTotalSize - CHUNK_RESERVED_SIZE;
				pHole->m_pNextHole = TNULL;
				pHole->m_pPrevHole = TNULL;
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

	void TMemory::DestroyMemBlock(MemBlock* a_pMemBlock)
	{
		FreeMemBlock(a_pMemBlock);
		Free(a_pMemBlock);
	}

	TBOOL TMemory::FreeMemBlock(MemBlock* a_pMemBlock)
	{
		TMutexLock lock(ms_pGlobalMutex);

		SetMemBlockUnused(a_pMemBlock);
		a_pMemBlock->m_pSlot->Remove();
		m_FreeBlocks.InsertHead(*a_pMemBlock->m_pSlot);

		return TTRUE;
	}

	void TMemory::SetMemBlockUnused(MemBlock* a_pMemBlock)
	{
		a_pMemBlock->m_pSlot->Remove();
		m_FreeBlocks.InsertHead(*a_pMemBlock->m_pSlot);

		CreateMemBlockInPlace(a_pMemBlock, a_pMemBlock->m_uiTotalSize1, "_unused");
		TStringManager::String8Copy(a_pMemBlock->m_szSignature, "xxxxxxx");
	}

	void TMemory::PrintDebug(const TCHAR* a_szFormat, ...)
	{
		va_list args;
		va_start(args, a_szFormat);
		
		auto pBuffer = TStringManager::GetTempString8();
		vsprintf(pBuffer, a_szFormat, args);
		OutputDebugStringA(pBuffer);
		TTRACE(pBuffer);

		va_end(args);
	}

	TBOOL TMemory::Initialise(TUINT a_uiHeapSize, TUINT a_uiReservedSize)
	{
		auto tmemory = TSTATICCAST(TMemory*, calloc(sizeof(TMemory), 1));
		new (tmemory) TMemory();

		tmemory->m_pMainBlockMemory = TNULL;
		tmemory->m_pGlobalBlock = TNULL;
		tmemory->m_Unknown1 = 0;
		tmemory->m_Unknown2 = 0;

		ms_pGlobalMutex = TSTATICCAST(TMutex*, malloc(sizeof(TMutex)));
		new (ms_pGlobalMutex) TMutex();

		tmemory->m_ReservedSize = a_uiReservedSize;
		tmemory->m_TotalAllocatedSize = (a_uiHeapSize == 0) ? 128 * 1024 * 1024 : a_uiHeapSize;
		tmemory->m_MainBlockSize = tmemory->m_TotalAllocatedSize - a_uiReservedSize;
		tmemory->m_pMainBlockMemory = malloc(tmemory->m_TotalAllocatedSize);
		tmemory->m_pGlobalBlock = tmemory->CreateMemBlockInPlace(
			tmemory->m_pMainBlockMemory,
			tmemory->m_TotalAllocatedSize,
			"Toshi"
		);

		return TTRUE;
	}

	void TMemory::Deinitialise()
	{
		TASSERT(TTRUE == IsSingletonCreated());
		auto pMemManager = TMemory::GetSingleton();
		auto pMainBlockMemory = pMemManager->m_pMainBlockMemory;

		pMemManager->m_UsedBlocks.RemoveAll();
		pMemManager->m_FreeBlocks.RemoveAll();
		pMemManager->~TMemory();

		free(pMemManager);
		free(pMainBlockMemory);
	}

	TUINT TMemory::MapSizeToFreeList(TUINT a_uiSize)
	{
		TFLOAT fSize = TFLOAT(TMath::AlignNumUp(a_uiSize) - 1);
		TUINT uiExponentSign = (*(TINT*)&fSize) >> 23;
		TUINT uiResult = uiExponentSign - 127;

		if (uiResult & 1)
			uiResult = uiExponentSign - 126;

		uiResult = uiResult >> 1;

		if (uiResult != 0)
			uiResult = uiResult - 1;

		if (uiResult >= NUM_FREE_LISTS)
			uiResult = NUM_FREE_LISTS - 1;

		return uiResult;
	}

	TBOOL TMemory::StartDebugPipe()
	{
#ifdef TMEMORY_DEBUG
		static TUINT8 s_DebugInfoPipeBuffer[4 + 262 * TARRAYSIZE(g_MemoryDebugInfoSlots)];

		g_MemoryDebugPipeServer.SetMemoryStreamUpdateCallback(
			[](void*& a_rMemoryStream, TUINT& a_rDataSize) {
				TUINT32* pNumAllocInfos = TREINTERPRETCAST(TUINT32*, s_DebugInfoPipeBuffer);
				*pNumAllocInfos = g_uiDebugNumTrackedAllocations;

				TUINT8* pAllocInfoCursor = TREINTERPRETCAST(TUINT8*, pNumAllocInfos + 1);

				T2_FOREACH(g_UsedDebugInfoSlots, slot)
				{
					*(void**)pAllocInfoCursor = slot->GetMemory();
					pAllocInfoCursor += 4;

					*(TUINT16*)pAllocInfoCursor = (TUINT16)slot->GetLineNum();
					pAllocInfoCursor += 2;

					auto uiFileNameLength = TMath::Min(TUINT8(TStringManager::String8Length(slot->GetFileName())), TUINT8(255));

					*(TUINT8*)pAllocInfoCursor = uiFileNameLength;
					pAllocInfoCursor += 1;

					TStringManager::String8Copy((TCHAR*)pAllocInfoCursor, slot->GetFileName(), uiFileNameLength);
					pAllocInfoCursor += uiFileNameLength;
				}

				a_rMemoryStream = s_DebugInfoPipeBuffer;
				a_rDataSize = TREINTERPRETCAST(TUINT8*, pAllocInfoCursor) - s_DebugInfoPipeBuffer;
			}
		);

		return g_MemoryDebugPipeServer.Start("TOSHI-TMemory", 1000, 1, sizeof(s_DebugInfoPipeBuffer));
#else
		return TFALSE;
#endif // TMEMORY_DEBUG

	}

	void TMemory::DumpMemInfo()
	{
		MemInfo memInfo;

		for (auto it = m_UsedBlocks.Tail(); it != m_UsedBlocks.End(); it--)
		{
			GetMemInfo(memInfo, it->m_pPtr);

			PrintDebug("Pool: \'%s\'\n", it->m_pPtr->m_szName);
			PrintDebug("\tLargest Hole    : %d\n", memInfo.m_uiLargestHole);
			PrintDebug("\tSmallest Hole   : %d\n", memInfo.m_uiSmallestHole);
			PrintDebug("\tLargest Process : %d\n", memInfo.m_uiLargestProcess);
			PrintDebug("\tSmallest Process: %d\n", memInfo.m_uiSmallestProcess);
			PrintDebug("\tTotal Free      : %d\n", memInfo.m_uiTotalFree);
			PrintDebug("\tTotal Used      : %d\n", memInfo.m_uiTotalUsed);
			PrintDebug("\tTotal Size      : %d\n", memInfo.m_uiTotalSize);
			PrintDebug("\tLogic Total Free: %d\n", memInfo.m_uiLogicTotalFree);
			PrintDebug("\tLogic Total Used: %d\n", memInfo.m_uiLogicTotalUsed);
			PrintDebug("\tLogic Total Size: %d\n", memInfo.m_uiLogicTotalSize);

			TFLOAT fLogicTotalUsed = TMath::Abs(TFLOAT(memInfo.m_uiLogicTotalUsed));
			TFLOAT fLogicTotalSize = TMath::Abs(TFLOAT(memInfo.m_uiLogicTotalSize));
			PrintDebug("\t%%Logical Used   : %f\n", (fLogicTotalUsed / fLogicTotalSize) * 100.0);

			TFLOAT fTotalUsed = TMath::Abs(TFLOAT(memInfo.m_uiTotalUsed));
			TFLOAT fTotalSize = TMath::Abs(TFLOAT(memInfo.m_uiTotalSize));
			PrintDebug("\t%%Used\t          : %f\n", (fTotalUsed / fTotalSize) * 100.0);
			PrintDebug("------\n\n");
		}
	}

	void TMemory::GetMemInfo(MemInfo& a_rMemInfo, MemBlock* a_pMemBlock)
	{
		TMutexLock lock(ms_pGlobalMutex);

		if (!a_pMemBlock)
		{
			a_pMemBlock = GetSingleton()->m_pGlobalBlock;
		}

		a_rMemInfo.m_uiUnk3 = 28;
		a_rMemInfo.m_uiUnk4 = 28;
		a_rMemInfo.m_uiTotalSize = 0;
		a_rMemInfo.m_uiLogicTotalSize = 0;
		a_rMemInfo.m_uiLargestHole = 0;
		a_rMemInfo.m_uiLargestProcess = 0;
		a_rMemInfo.m_uiSmallestHole = 0;
		a_rMemInfo.m_uiSmallestProcess = 0;
		a_rMemInfo.m_uiTotalFree = 0;
		a_rMemInfo.m_uiLogicTotalFree = 0;
		a_rMemInfo.m_uiTotalUsed = 0;
		a_rMemInfo.m_uiLogicTotalUsed = 0;
		a_rMemInfo.m_iNumHoles = 0;
		a_rMemInfo.m_iNumProcesses = 0;
		a_rMemInfo.m_uiTotalSize = a_pMemBlock->m_uiTotalSize1;
		a_rMemInfo.m_uiLogicTotalSize = a_pMemBlock->m_uiTotalSize1;
		a_rMemInfo.m_uiSmallestProcess = -1;
		a_rMemInfo.m_uiSmallestHole = -1;
		a_rMemInfo.m_uiLargestProcess = 0;
		a_rMemInfo.m_uiLargestHole = 0;

		auto uiUnk = (TUINT)a_pMemBlock->m_pFirstHole + (-88 - (TUINT)a_pMemBlock);
		a_rMemInfo.m_uiUnk3 = uiUnk;
		a_rMemInfo.m_uiUnk4 = uiUnk;

		auto pHole = a_pMemBlock->m_pFirstHole;
		TUINT uiHoleSize = pHole->m_uiSize;

		while (TMath::AlignNum(uiHoleSize) != 0)
		{
			a_rMemInfo.m_uiUnk3 += 12;
			uiHoleSize = TMath::AlignNum(pHole->m_uiSize);

			if ((pHole->m_uiSize & 1) == 0)
			{
				a_rMemInfo.m_iNumHoles += 1;
				a_rMemInfo.m_uiTotalFree += uiHoleSize;

				if (a_rMemInfo.m_uiLargestHole <= uiHoleSize && uiHoleSize != a_rMemInfo.m_uiLargestHole)
				{
					a_rMemInfo.m_uiLargestHole = uiHoleSize;
				}

				if (uiHoleSize < a_rMemInfo.m_uiSmallestHole)
				{
					a_rMemInfo.m_uiSmallestHole = uiHoleSize;
				}
			}
			else
			{
				a_rMemInfo.m_iNumProcesses += 1;
				a_rMemInfo.m_uiTotalUsed += uiHoleSize;

				if (a_rMemInfo.m_uiLargestProcess <= uiHoleSize && uiHoleSize != a_rMemInfo.m_uiLargestProcess)
				{
					a_rMemInfo.m_uiLargestProcess = uiHoleSize;
				}

				if (uiHoleSize < a_rMemInfo.m_uiSmallestProcess)
				{
					a_rMemInfo.m_uiSmallestProcess = uiHoleSize;
				}
			}

			pHole = (Hole*)((TUINT)&pHole->m_pPrevHole + uiHoleSize);
			uiHoleSize = pHole->m_uiSize;
		}

		a_rMemInfo.m_uiLogicTotalFree = a_rMemInfo.m_uiTotalFree;
		a_rMemInfo.m_uiLogicTotalUsed = a_rMemInfo.m_uiTotalUsed + uiUnk;

		if (a_rMemInfo.m_uiSmallestHole == -1)
		{
			a_rMemInfo.m_uiSmallestHole = 0;
		}
	}

	void TMemory::GetHALMemInfo(HALMemInfo& a_rHALMemInfo)
	{
		TUtil::MemClear(&a_rHALMemInfo, sizeof(a_rHALMemInfo));
	}

	TMemory::HALMemInfo::HALMemInfo()
	{
		TUtil::MemClear(this, sizeof(*this));
	}
}

void* TMalloc(TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum)
{
	auto pMemManager = Toshi::TMemory::GetSingleton();

	if (!a_pMemBlock)
	{
		a_pMemBlock = pMemManager->GetGlobalBlock();
	}

	auto pMem = pMemManager->Alloc(a_uiSize, 16, a_pMemBlock, a_szFileName, a_iLineNum);

	if (!pMem)
	{
		pMemManager->PrintDebug("Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName);
	}

	return pMem;
}

void* TMalloc(TUINT a_uiSize, const TCHAR* a_szFileName, TINT a_iLineNum)
{
	auto pMemManager = Toshi::TMemory::GetSingleton();
	auto pMemBlock = pMemManager->GetGlobalBlock();

	auto pMem = pMemManager->Alloc(a_uiSize, 16, pMemBlock, a_szFileName, a_iLineNum);

	if (!pMem)
	{
		pMemManager->PrintDebug("Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName);
	}

	return pMem;
}

void* TMalloc(TUINT a_uiSize)
{
	auto pMemManager = Toshi::TMemory::GetSingleton();
	auto pMemBlock = pMemManager->GetGlobalBlock();

	auto pMem = pMemManager->Alloc(a_uiSize, 16, pMemBlock, TNULL, -1);

	if (!pMem)
	{
		pMemManager->PrintDebug("Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName);
	}

	return pMem;
}

void* TMemalign(TINT a_iAlignment, TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock)
{
	auto pMemManager = Toshi::TMemory::GetSingleton();

	if (!a_pMemBlock)
	{
		a_pMemBlock = pMemManager->GetGlobalBlock();
	}

	auto pMem = pMemManager->Alloc(a_uiSize, a_iAlignment, a_pMemBlock, TNULL, -1);

	if (!pMem)
	{
		pMemManager->PrintDebug("Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName);
	}

	return pMem;
}

void* TMemalign(TUINT a_uiSize, TINT a_iAlignment)
{
	auto pMemManager = Toshi::TMemory::GetSingleton();
	auto pMemBlock = pMemManager->GetGlobalBlock();

	auto pMem = pMemManager->Alloc(a_uiSize, a_iAlignment, pMemBlock, TNULL, -1);

	if (!pMem)
	{
		pMemManager->PrintDebug("Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName);
	}

	return pMem;
}

void TFree(void* a_pMem)
{
	Toshi::TMemory::GetSingleton()->Free(a_pMem);
}
