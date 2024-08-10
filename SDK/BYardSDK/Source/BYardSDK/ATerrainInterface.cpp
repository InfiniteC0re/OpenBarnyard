#include "pch.h"
#include "ATerrainInterface.h"

TOSHI_NAMESPACE_USING

void ATerrainInterface::Update()
{
	
}

TBOOL ATerrainInterface::IsLoaded() const
{
	return TTRUE;
}

void ATerrainInterface::LoadFromFile(const char* a_szFilePath, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision)
{
	
}

void ATerrainInterface::WaitUntilLoaded()
{
	
}

void ATerrainInterface::DestroyModelData(ATerrainSection::ModelNode* a_pModelData)
{
	
}

void ATerrainInterface::UpdateUsedBlocks(ATerrainLODType a_eLODType)
{
	
}

void ATerrainInterface::QueueStreamingAssets()
{
	
}

void ATerrainInterface::UnqueueStreamingAssets()
{
	
}

TBOOL ATerrainInterface::HasAnyLODsQueued()
{
	return TFALSE;
}

ATerrainLODBlock* ATerrainInterface::AllocateLODBlock(ATerrainLODType a_eLODType, ATerrainSection* a_pVISGroup)
{
	return TNULL;
}

void ATerrainInterface::FlushJobs()
{
	
}

void ATerrainInterface::MoveAllFinishedJobs(Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs)
{
	
}

void ATerrainInterface::CancelUnrequiredJobs()
{
	
}

void ATerrainInterface::UpdateNightMaterials()
{
	TIMPLEMENT();
}

TINT ATerrainInterface::GetCurrentVISGroupIndex()
{
	TIMPLEMENT();
	return 0;
}

TINT ATerrainInterface::GetPersistantVISGroupIndex()
{
	return ms_iPersistantVISGroupIndex;
}

void ABYardTerrainManager::SetTerrain(TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup)
{
	
}

void ABYardTerrainManager::StartLoading()
{
	
}
