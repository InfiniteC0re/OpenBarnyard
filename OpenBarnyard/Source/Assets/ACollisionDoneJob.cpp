#include "pch.h"
#include "ACollisionDoneJob.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void ACollisionDoneJob::BeginJob()
{
}

// $Barnyard: FUNCTION 005ed0d0
TBOOL ACollisionDoneJob::RunJob()
{
	ATerrainInterface* pTerrainInterface = ATerrainInterface::GetSingleton();

	m_pVISGroup->m_eFlags &= ~ATerrainSection::FLAGS_COLLISION_LOADING;

	ATerrainLODBlock* pLODBlock = ( pTerrainInterface->IsCollisionStreamed() ) ?
	    m_pVISGroup->m_ppHighLODBlocks[ m_pVISGroup->m_iCollisionMemBlockID ] :
	    pTerrainInterface->GetVIS()->GetPersistantBlock();

	if ( !m_bNoModelInstance )
	{
		// Swap memory block
		TMemory::MemBlock* pOldMemBlock = g_pMemory->SetGlobalBlock( pLODBlock->GetMemBlock() );

		// Create model instance
		pTerrainInterface->CreateModelInstance( m_pVISGroup->m_pCollisionModelData, "", "collision" );

		// Set previous memory block back
		g_pMemory->SetGlobalBlock( pOldMemBlock );

		// Run the callback
		if ( pTerrainInterface->GetOnCollisionModelReadyCallback() )
			pTerrainInterface->GetOnCollisionModelReadyCallback()( m_pVISGroup->m_pCollisionModelData );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005ed170
TBOOL ACollisionDoneJob::CancelJob()
{
	if ( ATerrainInterface::GetSingleton()->IsCollisionStreamed() )
		m_bNoModelInstance = TTRUE;

	return TFALSE;
}

void ACollisionDoneJob::InitJob( ATerrainSection* a_pVISGroup, TBOOL a_bFlag )
{
	m_pVISGroup        = a_pVISGroup;
	m_bNoModelInstance = a_bFlag;
}
