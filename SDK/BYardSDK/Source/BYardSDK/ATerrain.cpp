#include "pch.h"
#include "ATerrain.h"

TOSHI_NAMESPACE_USING

void ATerrain::Update()
{
	
}

TBOOL ATerrain::IsLoaded() const
{
	return TTRUE;
}

void ATerrain::LoadFromFile(const char* a_szFilePath, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision)
{
	
}

void ATerrain::WaitUntilLoaded()
{
	
}

void ATerrain::DestroyModelData(ATerrainSection::ModelNode* a_pModelData)
{
	
}

void ATerrain::UpdateUsedBlocks(ATerrainLODType a_eLODType)
{
	
}

void ATerrain::QueueStreamingAssets()
{
	
}

void ATerrain::UnqueueStreamingAssets()
{
	
}

TBOOL ATerrain::HasAnyLODsQueued()
{
	return TFALSE;
}

ATerrainLODBlock* ATerrain::AllocateLODBlock(ATerrainLODType a_eLODType, ATerrainSection* a_pVISGroup)
{
	return TNULL;
}

void ATerrain::FlushJobs()
{
	
}

void ATerrain::MoveAllFinishedJobs(Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs)
{
	
}

void ATerrain::CancelUnrequiredJobs()
{
	
}

void ATerrain::UpdateNightMaterials()
{
	TIMPLEMENT();
}

TINT ATerrain::GetCurrentVISGroupIndex()
{
	TIMPLEMENT();
	return 0;
}

TINT ATerrain::GetPersistantVISGroupIndex()
{
	return ms_iPersistantVISGroupIndex;
}

void ATerrainManager::SetTerrain(TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup)
{
	
}

void ATerrainManager::StartLoading()
{
	
}
