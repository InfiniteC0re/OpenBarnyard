#include "pch.h"
#include "TMemory.h"

#include <cassert>

namespace Toshi
{
	bool TMemory::s_Initialized = false;
	TMutex TMemory::s_Mutex;
	TMemory* TMemory::s_Instance;

	void TMemory::Initialize(size_t memoryLimit, size_t reservedSize)
	{
		TASSERT(!s_Initialized, "Trying to initialize TMemManager twice!");
		
		s_Initialized = true;
		s_Instance = new TMemory();

		s_Mutex.Create();

		if (memoryLimit == 0)
		{
			s_Instance->m_TotalSize = GetFreePhysicalMemory();
		}
		else
		{
			s_Instance->m_TotalSize = memoryLimit;
		}

		s_Instance->m_ReservedSize = reservedSize;
		s_Instance->m_Size = s_Instance->m_TotalSize - s_Instance->m_ReservedSize;

		// todo: it's better to add some kind of an auto extension of the allocated memory
		// so we won't allocate a lot of memory at the application startup
		s_Instance->m_ToshiRegion = new char[s_Instance->m_TotalSize];

		// add the global toshi block
		s_Instance->m_GlobalBlock = s_Instance->AllocMem(s_Instance->m_ToshiRegion, s_Instance->m_TotalSize, "Toshi");
	}

	TMemory::TMemory() : m_BlocksContainer(), m_UsedBlocksContainer(), m_Blocks()
	{
		m_BlocksContainer.Next = &m_BlocksContainer;
		m_BlocksContainer.Prev = &m_BlocksContainer;
		m_BlocksContainer.Container = nullptr;
		m_BlocksContainer.Count = 0;

		m_UsedBlocksContainer.Next = &m_UsedBlocksContainer;
		m_UsedBlocksContainer.Prev = &m_UsedBlocksContainer;
		m_UsedBlocksContainer.Container = nullptr;
		m_UsedBlocksContainer.Count = 0;

		for (int i = 0; i < TOSHI_MEM_BLOCK_COUNT; i++)
		{
			auto& block = m_Blocks[i];

			// pointer to the main block
			block.Container = &m_BlocksContainer;

			// it will be updated by next block so don't care
			block.Next = &m_BlocksContainer;

			// not used?
			block.Count = 0;

			// update previous
			block.Prev = m_BlocksContainer.Prev;
			m_BlocksContainer.Prev->Next = &block;
			m_BlocksContainer.Prev = &block;

			m_BlocksContainer.Count++;
		}
	}

	TMemory::~TMemory()
	{
		// free the allocated memory
		delete[] m_ToshiRegion;
	}

	TMemoryBlockRegion* TMemory::AllocMem(void* memory, size_t size, const char* name)
	{
		TMemoryBlockRegion* result = nullptr;

		// align address to the pointer size
		void* alignedMemory = TMath::AlignPointer(memory);
		size_t alignedSize = TMath::AlignNum((size_t)memory + size) - (size_t)alignedMemory;

		// this mutex will unlock automatically at return
		TMutexLock mutexLock(s_Mutex);

		if (size > 0 && alignedMemory != nullptr && memory != nullptr)
		{
			TMemoryBlockInfo* block = nullptr;
			TMemoryBlockRegion* blockRegion = (TMemoryBlockRegion*)alignedMemory;

			if (alignedMemory != 0)
			{
				block = TakeoutAvailableBlock();
				blockRegion->Block = block;
				blockRegion->Block->Count = (size_t)alignedMemory;

				// place taken block to the UsedBlocks container
				block->Container = &m_UsedBlocksContainer;
				block->Prev = &m_UsedBlocksContainer;
				block->Next = m_UsedBlocksContainer.Next;
				m_UsedBlocksContainer.Next->Prev = block;
				m_UsedBlocksContainer.Next = block;

				m_UsedBlocksContainer.Count++;
			}

			if (size != 0)
			{
				strncpy_s(blockRegion->Name, name, sizeof(blockRegion->Name));
				blockRegion->Size = alignedSize - sizeof(TMemoryBlockRegion);

				// place the first empty chunk right after blockRegion
				TMemoryChunk* chunk = (TMemoryChunk*)((char*)blockRegion + sizeof(TMemoryBlockRegion));
				chunk->Region = blockRegion;
				chunk->Next = nullptr;
				chunk->Prev = nullptr;
				chunk->Size = blockRegion->Size - sizeof(TMemoryChunk);
				chunk->Data = (char*)chunk + sizeof(TMemoryChunk);

				blockRegion->UsedChunks = nullptr;
				blockRegion->FreeChunks = chunk;

				result = blockRegion;
			}
		}

		return result;
	}

	TMemoryBlockInfo* TMemory::TakeoutAvailableBlock()
	{
		assert(m_BlocksContainer.Count > 0 && "Ran out of Toshi memory blocks");
		auto block = m_BlocksContainer.Next;

		if (block != &m_BlocksContainer)
		{
			/*
			*  Since we take the first block after the main one
			*  block->Prev is always equal to m_BlocksContainer
			*/

			block->Container = 0;
			block->Prev->Next = block->Next;
			block->Next->Prev = block->Prev;

			block->Next = block;
			block->Prev = block;

			m_BlocksContainer.Count--;

			return block;
		}

		return nullptr;
	}

	// TNodeGenericList?

	static void AddChunkToList(TMemoryChunk** list, TMemoryChunk* chunk)
	{
		if (*list == nullptr)
		{
			*list = chunk;
		}
		else
		{
			chunk->Next = *list;
			(*list)->Prev = chunk;
			*list = chunk;
		}
	}

	static void DeleteChunkFromList(TMemoryChunk** list, TMemoryChunk* chunk)
	{
		if (*list == chunk)
		{
			*list = chunk->Next;
		}
		else
		{
			chunk->Prev->Next = chunk->Next;
			if (chunk->Next) { chunk->Next->Prev = chunk->Prev; }
		}

		chunk->Next = nullptr;
		chunk->Prev = nullptr;
	}

	static void DefragmentChunks(TMemoryChunk** list)
	{
		auto chunk1 = *list;
		auto chunk2 = *list;

		while (chunk1 != nullptr)
		{
			TMemoryChunk* mergeableChunk = (TMemoryChunk*)((char*)chunk1 + sizeof(TMemoryChunk) + chunk1->Size);

			// look for a mergeable chunk in the list
			while (chunk2 != nullptr)
			{
				if (chunk2 == mergeableChunk)
				{
					chunk1->Size += chunk2->Size;
					DeleteChunkFromList(list, chunk2);
					break;
				}

				chunk2 = chunk2->Next;
			}

			chunk1 = chunk1->Next;
		}
	}

	void* tmalloc(uint32_t size, TMemoryBlockRegion* block)
	{
		auto& memoryManager = TMemory::Instance();

		if (block == nullptr) { block = memoryManager.GlobalBlock(); }
		auto chunk = block->FreeChunks;

		// set minimum size to 4
		if (size < 4) size = 4;
		size = TMath::AlignNum(size + 3);

		while (chunk != nullptr)
		{
			if (chunk->Size >= size)
			{
				size_t oldSize = chunk->Size;
				size_t newSize = oldSize - size;
				chunk->Size = size;

				auto newChunk = (TMemoryChunk*)((char*)chunk->Data + chunk->Size);
				newChunk->Region = chunk->Region;
				newChunk->Prev = nullptr;
				newChunk->Next = nullptr;
				newChunk->Size = newSize - sizeof(TMemoryChunk);
				newChunk->Data = (char*)newChunk + sizeof(TMemoryChunk);

				DeleteChunkFromList(&newChunk->Region->FreeChunks, chunk);

				AddChunkToList(&newChunk->Region->FreeChunks, newChunk);
				AddChunkToList(&newChunk->Region->UsedChunks, chunk);

				return chunk->Data;
			}
			else
			{
				chunk = chunk->Next;
			}
		}

		assert(false && "Out of Toshi Memory on block {0}", block->Name);
		return nullptr;
	}

	bool tfree(void* ptr)
	{
		auto chunk = (TMemoryChunk*)((char*)ptr - sizeof(TMemoryChunk));

		if (ptr == chunk->Data)
		{
			auto block = chunk->Region;

			DeleteChunkFromList(&block->UsedChunks, chunk);
			AddChunkToList(&block->FreeChunks, chunk);
			DefragmentChunks(&block->FreeChunks);

			return true;
		}
		else
		{
			TOSHI_CORE_ERROR("Cannot free memory at 0x{0:X}", (size_t)ptr);
			TBREAK();

			return false;
		}
	}
}