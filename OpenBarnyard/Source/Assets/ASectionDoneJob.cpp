#include "pch.h"
#include "ASectionDoneJob.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void ASectionDoneJob::BeginJob()
{
}

// $Barnyard: FUNCTION 005ed750
TBOOL ASectionDoneJob::RunJob()
{
	if ( !m_bIsCanceled )
	{
		// Loading is done and it is not canceled
		TINT iNumLODs = m_pSection->GetLODCount( m_eLODType );

		ATerrainLODBlock** ppLODBlocks;
		TUINT16            uiNumMemBlocks;
		m_pSection->GetLODBlocks( m_eLODType, ppLODBlocks, uiNumMemBlocks );

		// Set memory block of the LOD
		auto pOldMemBlock = g_pMemory->SetGlobalBlock( ppLODBlocks[ uiNumMemBlocks - 1 ]->GetMemBlock() );

		for ( TINT i = 0; i < iNumLODs; i++ )
		{
			auto pModelNode = m_pSection->m_ppLODModelsData[ m_eLODType ][ i ];
			ATerrainInterface::GetSingleton()->CreateModelInstance( pModelNode, "", TNULL );

			if ( !ISZERO( m_pSection->m_aLODFlags[ m_eLODType ][ i ] & 1 ) )
			{
				pModelNode->SetUseLighting( TTRUE );
				pModelNode->SetGlow( TTRUE );
			}
		}

		// Set new state of LOD
		m_pSection->SetLODLoaded( m_eLODType, TTRUE );
		m_pSection->SetLODLoading( m_eLODType, TFALSE );
		m_pSection->SetLODEmpty( m_eLODType, TTRUE );

		// Restore memory block
		g_pMemory->SetGlobalBlock( pOldMemBlock );

		// Execute model node ready event if it is assigned
		auto cbModelNodeReady = ATerrainInterface::GetSingleton()->GetOnModelNodeReadyCallback();

		if ( TNULL != cbModelNodeReady )
		{
			for ( TINT i = 0; i < iNumLODs; i++ )
			{
				cbModelNodeReady( m_pSection->m_ppLODModelsData[ m_eLODType ][ i ] );
			}
		}

		return TTRUE;
	}

	// Loading was canceled before it was done
	m_pSection->SetLODLoading( m_eLODType, TFALSE );
	m_pSection->SetLODQueued( m_eLODType, TFALSE );
	m_pSection->SetLODEmpty( m_eLODType, TFALSE );
	m_pSection->DestroyLOD( m_eLODType );

	ATerrainInterface::GetSingleton()->QueueStreamingAssets();
	return TTRUE;
}

// $Barnyard: FUNCTION 005ecf50
TBOOL ASectionDoneJob::CancelJob()
{
	m_bIsCanceled = TTRUE;
	return TFALSE;
}

void ASectionDoneJob::InitJob( ATerrainSection* a_pVISGroup, ATerrainLODType a_eLODType )
{
	m_eLODType    = a_eLODType;
	m_pSection    = a_pVISGroup;
	m_bIsCanceled = TFALSE;
}
