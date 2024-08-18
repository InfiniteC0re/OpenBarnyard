#include "pch.h"
#include "ATerrainVIS.h"
#include "ATerrain.h"
#include "Assets/AAssetStreaming.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

void ATerrainVIS::LoadSkeleton()
{
	auto pTerrain         = ATerrainInterface::GetSingleton();
	auto pPersistantBlock = pTerrain->m_pTerrainVIS->m_pPersistantTerrainBlock;

	m_pLocatorManager = new ( pPersistantBlock->GetMemBlock() ) ATerrainLocatorManager();
	auto pTRB         = new ( pPersistantBlock->GetMemBlock() ) Toshi::TTRB();

	pPersistantBlock->SetupTRB( pTRB, pPersistantBlock );

	auto pTRBJob = pTerrain->GetFreeTRBLoaderJob();
	pTRBJob->InitJob( pTRB, m_szSkelFilePath );

	auto pSkeletonJob = pTerrain->GetFreeSkeletonLoaderJob();
	pSkeletonJob->InitJob( this, pTRBJob );

	AAssetStreaming::GetSingleton()->AddMainThreadJob( pTRBJob );
	AAssetStreaming::GetSingleton()->AddMainThreadJob( pSkeletonJob );
}
