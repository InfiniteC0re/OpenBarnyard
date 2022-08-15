#include "pch.h"
#include "TMemory.h"
#include <cassert>

namespace Toshi
{
	namespace TMemory
	{
		HANDLE TMemManager::s_Mutex;
		TMemManager* TMemManager::s_Class;
		size_t TMemManager::s_GlobalBlockSize;

		void TMemManager::Initialize(size_t globalBlockSize)
		{
			s_GlobalBlockSize = globalBlockSize;
			s_Class = new TMemManager;
		}

		TMemManager::TMemManager(size_t memoryLimit, size_t reservedSize) : m_BlocksContainer(), m_UsedBlocksContainer(), m_Blocks()
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

			s_Mutex = CreateMutexA(NULL, FALSE, NULL);

			if (memoryLimit == 0) { m_TotalSize = s_GlobalBlockSize; }
			else { m_TotalSize = memoryLimit; }

			m_ReservedSize = reservedSize;
			m_Size = m_TotalSize - m_ReservedSize;

			// todo: it's better to add some kind of an auto extension of the allocated memory
			// so we won't allocate a lot of memory at the application startup
			m_ToshiRegion = new char[m_TotalSize];

			// add the global toshi block
			m_GlobalBlock = AddNamedBlock(m_ToshiRegion, m_TotalSize, "Toshi");
		}

		TMemManager::~TMemManager()
		{
			// free the allocated memory
			delete[] m_ToshiRegion;
		}

		MemoryBlockRegion* TMemManager::AddNamedBlock(void* memory, size_t size, const char* name)
		{
			// this will be returned
			MemoryBlockRegion* result = nullptr;

			// calculate the value to align addresses
			size_t maxVal = -1;
			size_t alignValue = maxVal - (sizeof(void*) - 1);

			// align address to the pointer size
			void* alignedMemory = (void*)((size_t)memory & alignValue);
			size_t alignedSize = ((size_t)memory + size & alignValue) - (size_t)alignedMemory;

			WaitForSingleObject(s_Mutex, 0xFFFFFFFF);

			if (size > 0 && alignedMemory != nullptr && memory != nullptr)
			{
				MemoryBlockInfo* block = nullptr;
				MemoryBlockRegion* blockRegion = (MemoryBlockRegion*)alignedMemory;

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
					strncpy_s(blockRegion->Name, name, 52);
					blockRegion->Size = alignedSize - sizeof(MemoryBlockRegion);

					// place the first empty chunk right after blockRegion
					MemoryChunk* chunk = (MemoryChunk*)((char*)blockRegion + sizeof(MemoryBlockRegion));
					chunk->Region = blockRegion;
					chunk->Next = nullptr;
					chunk->Prev = nullptr;
					chunk->Size = blockRegion->Size - sizeof(MemoryChunk);
					chunk->Data = (char*)chunk + sizeof(MemoryChunk);

					blockRegion->UsedChunks = nullptr;
					blockRegion->FreeChunks = chunk;

					result = blockRegion;
				}

				ReleaseMutex(s_Mutex);
			}

			ReleaseMutex(s_Mutex);
			return result;
		}

		MemoryBlockInfo* TMemManager::TakeoutAvailableBlock()
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

		static void AddChunkToList(MemoryChunk** list, MemoryChunk* chunk)
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

		static void DeleteChunkFromList(MemoryChunk** list, MemoryChunk* chunk)
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

		static void DefragmentChunks(MemoryChunk** list)
		{
			auto chunk1 = *list;
			auto chunk2 = *list;

			while (chunk1 != nullptr)
			{
				MemoryChunk* mergeableChunk = (MemoryChunk*)((char*)chunk1 + sizeof(MemoryChunk) + chunk1->Size);

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
	}

	void* tmalloc(size_t size, TMemory::MemoryBlockRegion* block)
	{
		auto& memoryManager = TMemory::TMemManager::Instance();

		if (block == nullptr) { block = memoryManager.GlobalBlock(); }
		auto chunk = block->FreeChunks;

		while (chunk != nullptr)
		{
			if (chunk->Size >= size)
			{
				size_t oldSize = chunk->Size;
				size_t newSize = oldSize - size;
				chunk->Size = size;

				auto newChunk = (TMemory::MemoryChunk*)((char*)chunk->Data + chunk->Size);
				newChunk->Region = chunk->Region;
				newChunk->Prev = nullptr;
				newChunk->Next = nullptr;
				newChunk->Size = newSize - sizeof(TMemory::MemoryChunk);
				newChunk->Data = (char*)newChunk + sizeof(TMemory::MemoryChunk);

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

		assert(false && "Out of memory");
		return nullptr;
	}

	bool tfree(void* ptr, TMemory::MemoryBlockRegion* block)
	{
		auto& memoryManager = TMemory::TMemManager::Instance();

		if (block == nullptr) { block = memoryManager.GlobalBlock(); }
		auto chunk = block->UsedChunks;

		while (chunk != nullptr)
		{
			if (chunk->Data == ptr)
			{
				DeleteChunkFromList(&block->UsedChunks, chunk);
				AddChunkToList(&block->FreeChunks, chunk);
				DefragmentChunks(&block->FreeChunks);

				return true;
			}

			chunk = chunk->Next;
		}

		return false;
	}
}