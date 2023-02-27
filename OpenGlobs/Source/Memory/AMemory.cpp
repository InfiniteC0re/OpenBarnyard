#include "pch.h"
#include "AMemory.h"

AMemory::PoolDefinitionInfo AMemory::ms_aPoolDefinitionInfo[POOL_NumOf] = {
	{ "POOL_FrequentAllocations", POOL_FrequentAllocations, 0x300000 },
	{ "POOL_XUI", POOL_XUI, 0x1400000 },
};

Toshi::TMemoryHeap* AMemory::ms_apMemoryBlocks[POOL_NumOf];

void AMemory::CreatePools()
{
	for (size_t i = 0; i < POOL_NumOf; i++)
	{
		CreatePool(ms_aPoolDefinitionInfo[i].Type);
	}
}

void AMemory::CreatePool(Pool type)
{
	TASSERT(type < POOL_NumOf);
	auto& poolDefinition = ms_aPoolDefinitionInfo[type];
	TASSERT(IsPoolCreated(poolDefinition.Type) == TFALSE);

	void* pMem = TMalloc(poolDefinition.Size);
	ms_apMemoryBlocks[poolDefinition.Type] = Toshi::TMemory::CreateHeapInPlace(pMem, poolDefinition.Size, Toshi::TMemoryHeapFlags_UseMutex, poolDefinition.Name);
}

void AMemory::DestroyPool(Pool type)
{
	TASSERT(type < POOL_NumOf);
	TIMPLEMENT();
}
