#pragma once
#include "AMemoryPoolAllocator.h"

#include <cstdint>

class AMemoryPool
{
public:
	constexpr AMemoryPool(const char* a_szName, int a_iHeapIndex, size_t a_uiSize, int a_iUnk1, int a_iUnk2) :
		m_szName(a_szName),
		m_iHeapIndex(a_iHeapIndex),
		m_uiSize(a_uiSize),
		m_iUnk1(a_iUnk1),
		m_iUnk2(a_iUnk2)
	{

	}

	const char* GetName() const { return m_szName; }
	int GetHeapIndex() const { return m_iHeapIndex; }
	size_t GetSize() const { return m_uiSize; }

private:
	const char* m_szName;
	int m_iHeapIndex;
	size_t m_uiSize;
	int m_iUnk1;
	int m_iUnk2;
};

class AMemory
{
public:
	using POOL = uint32_t;
	enum POOL_ : POOL
	{
		POOL_None = 0,
		POOL_StringPool,
		POOL_Misc,
		POOL_Collision,
		POOL_Sound,
		POOL_Debug,
		POOL_MemTrace,
		POOL_Viewport,
		POOL_NUMOF = 8
	};

	struct AllocatorList
	{
		void* Unknown;
		AMemoryPoolAllocator aAllocators[POOL_NUMOF];
	};

	inline static Toshi::TMemory::MemBlock* s_aMemBlocks[POOL_NUMOF];
	inline static AllocatorList s_AllocatorList;

	inline static constexpr AMemoryPool s_aPools[POOL_NUMOF] =
	{
#define DEFINE_MEMORY_POOL(NAME, SIZE, UNK1, UNK2) AMemoryPool(#NAME, NAME, SIZE, UNK1, UNK2)
		DEFINE_MEMORY_POOL(POOL_None, 0, 0, 0),
		DEFINE_MEMORY_POOL(POOL_StringPool, 0x00100000, 0x00200000, 0x00200000),
		DEFINE_MEMORY_POOL(POOL_Misc, 0x00080000, 0x001c2000, 0x00096000),
		DEFINE_MEMORY_POOL(POOL_Collision, 0x00008000, 0x00008000, 0),
		DEFINE_MEMORY_POOL(POOL_Sound, 0x00040000, 0x00040000, 0x00010000),
		DEFINE_MEMORY_POOL(POOL_Debug, 0x00fa0000, 0x00fa0000, 0),
		DEFINE_MEMORY_POOL(POOL_MemTrace, 0x00000400, 0x00003000, 0),
		DEFINE_MEMORY_POOL(POOL_Viewport, 0x00004000, 0x00004000, 0),
#undef DEFINE_MEMORY_POOL
	};

public:
	static void CreatePool(POOL a_ePool);
	static Toshi::T2Allocator* GetAllocator(POOL a_ePool);
	static Toshi::TMemory::MemBlock* GetMemBlock(POOL a_ePool);
};
