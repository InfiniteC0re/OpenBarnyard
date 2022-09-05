#pragma once
#include "Toshi/Utils/TSingleton.h"
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

	void* tmalloc(size_t size, TMemoryBlockRegion* block = nullptr);
	bool tfree(void* ptr);

	template<typename T>
	inline T* tmalloc() { return static_cast<T*>(tmalloc(sizeof(T), nullptr)); }

	template<class T, class... Args>
	inline T* tnew(Args&& ...args)
	{
		return new(tmalloc<T>()) T(std::forward<Args>(args)...);
	}

	template<class T>
	inline void tdelete(T* ptr)
	{
		if (ptr)
		{
			ptr->~T();
			tfree(ptr);
		}
	}

	class TMemory : public TSingleton<TMemory>
	{
	public:
		TMemory();
		TMemory(const TMemory&) = delete;
		~TMemory();

		static void Initialize(size_t memoryLimit = 0, size_t reservedSize = 0);

		TMemoryBlockRegion* AllocMem(void* memory, size_t size, const char* name);
		TMemoryBlockInfo* TakeoutAvailableBlock();
		
		inline TMemoryBlockRegion* GlobalBlock() const { return m_GlobalBlock; }
		inline static TMutex& Mutex() { return s_Mutex; }
		inline static size_t GetFreePhysicalMemory() { return (64 * 1024 * 1024); }

	private:
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

	template <class Type>
	class TAllocator
	{
	public:
		using value_type = Type;

		using pointer = Type*;
		using const_pointer = const Type*;

		using void_pointer = void*;
		using const_void_pointer = const void*;

		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

		TAllocator() = default;
		~TAllocator() = default;

		template<class U>
		TAllocator(const TAllocator<U>& other) { }

		template<class... Args>
		inline pointer create(Args&& ...args)
		{
			pointer ptr = allocate(1);
			construct(ptr, std::forward<Args>(args)...);
			return ptr;
		}

		inline pointer allocate(size_type count)
		{
			return static_cast<pointer>(tmalloc(sizeof(value_type) * count));
		}

		inline pointer allocate(size_type count, const_void_pointer hint)
		{
			return allocate(count);
		}

		inline void deallocate(pointer ptr, size_type count)
		{
			tfree(ptr);
		}

		inline size_type max_size() const
		{
			return std::numeric_limits<size_type>::max();
		}

		template<class U, class... Args>
		inline void construct(U* ptr, Args&& ...args)
		{
			new(ptr) U(std::forward<Args>(args)...);
		}

		template<class U>
		inline void destroy(U* ptr)
		{
			ptr->~U();
		}
	};
}