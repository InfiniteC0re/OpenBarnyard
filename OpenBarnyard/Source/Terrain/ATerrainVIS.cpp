#include "pch.h"
#include "ATerrainVIS.h"
#include "ATerrainInterface.h"
#include "Assets/AAssetStreaming.h"
#include "Assets/AMaterialLibraryManager.h"

#include <Render/TRenderInterface.h>

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

// $Barnyard: FUNCTION 005edb70
void ATerrainVIS::Destroy()
{
	auto pTerrain = ATerrainInterface::GetSingleton();

	for ( TINT i = 0; i < m_iNumSections; i++ )
	{
		auto pSection = &m_pSections[ i ];

		if ( pSection->IsLODLoaded( ATerrainLODType_High ) )
		{
			pSection->DestroyLOD( ATerrainLODType_High );
		}

		if ( pSection->IsLODLoaded( ATerrainLODType_Low ) )
		{
			pSection->DestroyLOD( ATerrainLODType_Low );
		}

		if ( pSection->m_pCollisionModelData )
		{
			pTerrain->DestroyModelData( pSection->m_pCollisionModelData );
			pSection->m_pCollisionModelData = TNULL;
		}
	}

	if ( m_pLocatorList )
	{
		delete m_pLocatorList;
		m_pLocatorList = TNULL;
	}

	if ( m_pKeyframeLibrary )
	{
		Toshi::TRenderInterface::GetSingleton()->GetKeyframeLibraryManager().UnloadLibrary( m_pKeyframeLibrary );
		m_pKeyframeLibrary = TNULL;
	}

	if ( m_pMaterialLibrary )
	{
		AMaterialLibraryManager::GetSingleton()->UnloadTexturesOfLibrary( m_pMaterialLibrary );
		AMaterialLibraryManager::List::GetSingleton()->DestroyLibrary( m_pMaterialLibrary );
		m_pMaterialLibrary = TNULL;

		if ( m_pMaterialLibraryTRB )
		{
			delete m_pMaterialLibraryTRB;
			m_pMaterialLibraryTRB = TNULL;
		}
	}

	for ( TINT i = 0; i < m_iNumHighBlocks; i++ )
	{
		if ( m_ppHighBlocks[ i ] )
		{
			delete m_ppHighBlocks[ i ];
		}
	}

	delete[] m_ppHighBlocks;
	m_ppHighBlocks = TNULL;

	for ( TINT i = 0; i < m_iNumLowBlocks; i++ )
	{
		if ( m_ppLowBlocks[ i ] )
		{
			delete m_ppLowBlocks[ i ];
		}
	}

	delete[] m_ppLowBlocks;
	m_ppLowBlocks = TNULL;

	if ( m_pPersistantTerrainBlock )
	{
		delete m_pPersistantTerrainBlock;
		m_pPersistantTerrainBlock = TNULL;
	}
}
