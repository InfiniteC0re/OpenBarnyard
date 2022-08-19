#pragma once
#include "Toshi/Utils/TMutexLock.h"

#define TOSHI_MEM_BLOCK_COUNT 128
typedef void* HANDLE;

namespace Toshi
{
	struct TMemoryBlockInfo
	{
		TMemoryBlockInfo* Container;
		TMemoryBlockInfo* Next;
		TMemoryBlockInfo* Prev;
		size_t Count;
	};

	struct TMemoryBlockRegion
	{
		TMemoryBlockInfo* Block;
		size_t Size;
		char Name[32];
		struct TMemoryChunk* UsedChunks;
		struct TMemoryChunk* FreeChunks;
	};

	struct TMemoryChunk
	{
		TMemoryBlockRegion* Region;
		struct TMemoryChunk* Prev;
		struct TMemoryChunk* Next;
		size_t Size;
		void* Data;
	};

	class TMemory
	{
	public:
		TMemory();
		TMemory(const TMemory&) = delete;
		~TMemory();

		TMemoryBlockRegion* AllocMem(void* memory, size_t size, const char* name);

		TMemoryBlockInfo* TakeoutAvailableBlock();

		inline TMemoryBlockRegion* GlobalBlock() const { return m_GlobalBlock; }

		static void Initialize(size_t memoryLimit = 0, size_t reservedSize = 0);
		
		inline static TMemory& Instance() { return s_Instance; }
		inline static size_t GetFreePhysicalMemory() { return (64 * 1024 * 1024); }

	private:
		static TMemory s_Instance;
		static TMutex s_Mutex;

		size_t m_Size;
		size_t m_ReservedSize;
		size_t m_FreeSize;

		TMemoryBlockInfo m_BlocksContainer;
		TMemoryBlockInfo m_UsedBlocksContainer;
		TMemoryBlockInfo m_Blocks[TOSHI_MEM_BLOCK_COUNT];
		TMemoryBlockRegion* m_GlobalBlock;
		void* m_AllocatedMem;
	};

	void* tmalloc(uint32_t size, TMemoryBlockRegion* block = nullptr);
	bool tfree(void* ptr);
}