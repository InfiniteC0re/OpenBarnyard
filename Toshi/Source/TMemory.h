#pragma once

#define TOSHI_MEM_BLOCK_COUNT 128
typedef void* HANDLE;

namespace Toshi
{
	namespace TMemory
	{
		struct MemoryBlockInfo
		{
			MemoryBlockInfo* Container;
			MemoryBlockInfo* Next;
			MemoryBlockInfo* Prev;
			size_t Count;
		};

		struct MemoryBlockRegion
		{
			MemoryBlockInfo* Block;
			size_t Size;
			char Name[32];
			struct MemoryChunk* UsedChunks;
			struct MemoryChunk* FreeChunks;
		};

		struct MemoryChunk
		{
			MemoryBlockRegion* Region;
			struct MemoryChunk* Prev;
			struct MemoryChunk* Next;
			size_t Size;
			void* Data;
		};

		class TMemManager
		{
		public:
			TMemManager(size_t memoryLimit = 0, size_t reservedSize = 0);
			TMemManager(const TMemManager&) = delete;
			~TMemManager();

			static void Initialise(size_t globalBlockSize);

			MemoryBlockRegion* AddNamedBlock(void* memory, size_t size, const char* name);

			MemoryBlockInfo* TakeoutAvailableBlock();

			inline static TMemManager& Instance() { return *s_Class; }

			inline MemoryBlockRegion* GlobalBlock() const { return m_GlobalBlock; }
		private:
			static size_t s_GlobalBlockSize;
			static TMemManager* s_Class;
			static HANDLE s_Mutex;

			size_t m_TotalSize;
			size_t m_ReservedSize;
			size_t m_Size;

			MemoryBlockInfo m_BlocksContainer;
			MemoryBlockInfo m_UsedBlocksContainer;
			MemoryBlockInfo m_Blocks[TOSHI_MEM_BLOCK_COUNT];
			void* m_ToshiRegion;
			MemoryBlockRegion* m_GlobalBlock;
		};
	}

	void* tmalloc(size_t size, TMemory::MemoryBlockRegion* block = nullptr);
	bool tfree(void* ptr, TMemory::MemoryBlockRegion* block = nullptr);
}