#include "ToshiPCH.h"
#include "TMemory.h"

namespace Toshi
{
	TMutex TMemory::s_Mutex;

	void TMemory::Initialize(size_t memoryLimit, size_t reservedSize)
	{
		static bool s_Initialized = false;
		TASSERT(!s_Initialized, "TMemory is already initialized");

		auto& instance = Instance();
		instance.m_Size = memoryLimit <= 0 ? GetFreePhysicalMemory() : memoryLimit;

		s_Mutex.Create();
		instance.m_ReservedSize = reservedSize;
		instance.m_FreeSize = instance.m_Size - instance.m_ReservedSize;

		// todo: it's better to add some kind of an auto extension of the allocated memory
		// so we won't allocate a lot of memory at the application startup
		instance.m_AllocatedMem = new char[instance.m_Size];

		// add the global toshi block
		instance.m_GlobalBlock = instance.AllocMem(instance.m_AllocatedMem, instance.m_Size, "Toshi");
		s_Initialized = true;
	}

	TMemory::TMemory() : m_Blocks()
	{
		m_AllocatedMem = nullptr;
		m_GlobalBlock = nullptr;
		m_Size = 0;
		m_ReservedSize = 0;
		m_FreeSize = 0;

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
		delete[] m_AllocatedMem;
	}

	TMemoryBlockRegion* TMemory::AllocMem(void* memory, size_t size, const char* name)
	{
		TMemoryBlockRegion* result = nullptr;

		// align address to the pointer size
		uintptr_t alignedMemory = TMath::AlignPointer(memory);
		size_t alignedSize = TMath::AlignNum((size_t)memory + size) - alignedMemory;

		// this mutex will unlock automatically at return
		TMutexLock mutexLock(s_Mutex);

		if (size > 0 && alignedMemory != 0 && memory != nullptr)
		{
			TMemoryBlockInfo* block = nullptr;

			uintptr_t blockRegionPtr = alignedMemory;
			TMemoryBlockRegion* blockRegion = reinterpret_cast<TMemoryBlockRegion*>(blockRegionPtr);

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

				uintptr_t chunkPtr = blockRegionPtr + sizeof(TMemoryBlockRegion);
				TMemoryChunk* chunk = reinterpret_cast<TMemoryChunk*>(chunkPtr);
				
				// place the first empty chunk right after blockRegion
				chunk->Region = blockRegion;
				chunk->Next = nullptr;
				chunk->Prev = nullptr;
				chunk->Size = blockRegion->Size - sizeof(TMemoryChunk);
				chunk->Data = reinterpret_cast<void*>(chunkPtr + sizeof(TMemoryChunk));

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
		if (*list != nullptr)
		{
			// there is at least one element in the list
			chunk->Next = *list;
			chunk->Next->Prev = chunk;
		}
		else
		{
			// there is no other elements in the list
			chunk->Next = nullptr;
		}

		chunk->Prev = nullptr;
		*list = chunk;
	}

	static void DeleteChunkFromList(TMemoryChunk** list, TMemoryChunk* chunk)
	{
		if (*list == chunk)
		{
			// deleting the first chunk in the list
			*list = chunk->Next;
		}
		else
		{
			// deleting the first chunk in the list
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

	void* tmalloc(size_t size, TMemoryBlockRegion* block)
	{
		auto& memoryManager = TMemory::Instance();

		if (block == nullptr) { block = memoryManager.GlobalBlock(); }
		auto chunk = block->FreeChunks;

		// set minimum size to 4
		if (size < 4) size = 4;
		size = TMath::AlignNumUp(size);

		size_t totalSize = size + sizeof(TMemoryChunk);

		while (chunk != nullptr)
		{
			if (chunk->Size >= totalSize)
			{
				uintptr_t newChunkPtr = (uintptr_t)chunk->Data + size;
				auto newChunk = reinterpret_cast<TMemoryChunk*>(newChunkPtr);
				
				newChunk->Region = chunk->Region;
				newChunk->Size = chunk->Size - totalSize;
				newChunk->Data = reinterpret_cast<void*>(newChunkPtr + sizeof(TMemoryChunk));
				chunk->Size = size;

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

		TASSERT(false, "Out of Toshi Memory on block {0}", block->Name);
		return nullptr;
	}

	bool tfree(void* ptr)
	{
		auto chunk = reinterpret_cast<TMemoryChunk*>((uintptr_t)ptr - sizeof(TMemoryChunk));

		if (ptr == chunk->Data)
		{
			auto block = chunk->Region;

#ifdef TOSHI_DEBUG
			// check if the chunk isn't already freed
			auto usedChunk = block->UsedChunks;
			bool isValid = false;

			while (usedChunk != nullptr)
			{
				if (usedChunk == chunk)
				{
					isValid = true;
					break;
				}

				usedChunk = usedChunk->Next;
			}

			TASSERT(isValid, "Trying to tfree memory at 0x{0:X} the second time", (size_t)ptr);
#endif

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