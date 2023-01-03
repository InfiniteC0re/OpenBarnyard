#pragma once
#include <cstdint>
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
		uint32_t Index;
		uint32_t Size;
		Toshi::TMemory::Flags Flags;
	};

public:
	AMemory() = delete;

	static void CreatePool(Pool type);
	static void DestroyPool(Pool type);

public:
	static PoolDefinitionInfo ms_aPoolDefinitionInfo[POOL_NumOf];
	static Toshi::TMemoryHeap* ms_apMemoryBlocks[POOL_NumOf];
};