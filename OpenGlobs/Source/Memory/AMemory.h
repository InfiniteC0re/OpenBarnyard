#pragma once
#include "Toshi/Memory/TMemory.h"

class AMemory
{
public:
	typedef uint32_t Pool;
	enum POOL_ : Pool
	{
		POOL_FrequentAllocations,
		POOL_XUI,
		POOL_NumOf
	};

	struct PoolDefinitionInfo
	{
		const char* Name;
		Pool Type;
		uint32_t Size;
	};

public:
	AMemory() = delete;

	static void CreatePools();

	static void CreatePool(Pool type);
	static void DestroyPool(Pool type);

	static bool IsPoolCreated(Pool type)
	{
		TASSERT(type < POOL_NumOf);
		return ms_apMemoryBlocks[type] != TNULL;
	}

	static Toshi::TMemoryHeap* GetPool(Pool type)
	{
		return ms_apMemoryBlocks[type];
	}

public:
	static PoolDefinitionInfo ms_aPoolDefinitionInfo[POOL_NumOf];
	static Toshi::TMemoryHeap* ms_apMemoryBlocks[POOL_NumOf];
	static Toshi::T2Allocator* ms_aMemoryAllocators;
};