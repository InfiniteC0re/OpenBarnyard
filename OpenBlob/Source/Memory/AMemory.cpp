#include "pch.h"
#include "AMemory.h"

AMemory::PoolDefinitionInfo AMemory::ms_aPoolDefinitionInfo[POOL_NumOf] = {
	{ "POOL_FrequentAllocations", 0, 0x40000, Toshi::TMemory::Flags_Standard },
	{ "POOL_FrequentAllocators", 0, 0x40000, Toshi::TMemory::Flags_Standard },
};

Toshi::TMemoryHeap* AMemory::ms_apMemoryBlocks[POOL_NumOf];

void AMemory::CreatePool(Pool pool)
{
	if (pool - 1 < 2)
	{

	}
	TIMPLEMENT();
}

void AMemory::DestroyPool(Pool pool)
{
	TIMPLEMENT();
}
