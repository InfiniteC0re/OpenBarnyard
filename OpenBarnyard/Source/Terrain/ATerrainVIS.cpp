#include "pch.h"
#include "ATerrainVIS.h"
#include "ATerrainInterface.h"
#include "Assets/AAssetStreaming.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// $Barnyard: FUNCTION 005ed550
void ATerrainVIS::LoadSkeleton()
{
	auto pTerrain         = ATerrainInterface::GetSingleton();
	auto pPersistantBlock = pTerrain->m_pTerrainVIS->m_pPersistantTerrainBlock;

	m_pLocatorList = new ( pPersistantBlock->GetMemBlock() ) ATerrainLocatorList();
	auto pTRB      = new ( pPersistantBlock->GetMemBlock() ) Toshi::TTRB();

	pPersistantBlock->SetupTRB( pTRB, pPersistantBlock );

	auto pTRBJob = pTerrain->GetFreeTRBLoaderJob();
	pTRBJob->InitJob( pTRB, m_szSkelFilePath );

	auto pSkeletonJob = pTerrain->GetFreeSkeletonLoaderJob();
	pSkeletonJob->InitJob( this, pTRBJob );

	AAssetStreaming::GetSingleton()->AddMainThreadJob( pTRBJob );
	AAssetStreaming::GetSingleton()->AddMainThreadJob( pSkeletonJob );
}
