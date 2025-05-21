#include "pch.h"
#include "ATerrainSection.h"
#include "ATerrainInterface.h"
#include "Assets/AAssetStreaming.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Render/AWorldShader/AWorldMaterial.h"

#include <Toshi/TScheduler.h>

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/AWorldShader/AWorldMaterial_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#endif // TOSHI_SKU_WINDOWS

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void ATerrainSection::Draw( ATerrainLODType a_eLODType )
{
	TASSERT( a_eLODType != ATerrainLODType_None );

	if ( a_eLODType == ATerrainLODType_High && IsLODLoaded( ATerrainLODType_High ) )
	{
		// Draw models of the high LOD
		for ( TINT i = 0; i < m_iNumHighModelFiles; i++ )
		{
			m_ppLODModelsData[ ATerrainLODType_High ][ i ]->Render();
		}
	}
	else
	{
		// High LOD is still loading so let's check if we can render low LOD instead
		if ( IsLODLoaded( ATerrainLODType_Low ) )
		{
			// Draw models of the low LOD
			for ( TINT i = 0; i < m_iNumLowModelFiles; i++ )
			{
				m_ppLODModelsData[ ATerrainLODType_Low ][ i ]->Render();
			}
		}
	}
}

// $Barnyard: FUNCTION 005ed310
void ATerrainSection::LoadCollision()
{
	auto pTerrain = ATerrainInterface::GetSingleton();

	if ( m_szCollisionFilename[ 0 ] != '\0' )
	{
		auto pBlock = ( !pTerrain->IsCollisionStreamed() ) ?
		    pTerrain->GetVIS()->m_pPersistantTerrainBlock :
		    m_ppHighLODBlocks[ m_iCollisionMemBlockID ];

		m_pCollisionModelData = new ( pBlock->GetMemBlock() ) ModelNode();

		auto pTRB = new ( pBlock->GetMemBlock() ) TTRB();
		pBlock->SetupTRB( pTRB, pBlock );

		auto pModelLoaderJob = pTerrain->GetFreeModelLoaderJob();
		pModelLoaderJob->InitJob(
		    &m_pCollisionModelData->m_ModelRef,
		    pTRB,
		    m_szCollisionFilename,
		    pTerrain->IsCollisionStreamed()
		);

		auto pCollisionJob = pTerrain->GetFreeCollisionLoaderJob();
		pCollisionJob->InitJob( this, TFALSE );

		AAssetStreaming::GetSingleton()->AddMainThreadJob( pModelLoaderJob );
		AAssetStreaming::GetSingleton()->AddMainThreadJob( pCollisionJob );
		m_eFlags |= FLAGS_COLLISION_LOADING;
	}
}

// $Barnyard: FUNCTION 005ed8f0
void ATerrainSection::LoadModels( ATerrainLODType a_eLODType )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	auto pTerrain = ATerrainInterface::GetSingleton();

	if ( !IsLODLoading( a_eLODType ) && !IsLODLoaded( a_eLODType ) )
	{
		const TCHAR** ppLODNames;
		TINT          iNumLODs;
		GetLODNames( a_eLODType, ppLODNames, iNumLODs );

		if ( iNumLODs == 0 && m_szCollisionFilename[ 0 ] == '\0' )
		{
			SetLODQueued( a_eLODType, TFALSE );
			SetLODEmpty( a_eLODType, TTRUE );
			SetLODLoaded( a_eLODType, TTRUE );
		}
		else
		{
			ATerrainLODBlock** ppLODBlocks;
			TUINT16            uiNumMemBlocks;
			GetLODBlocks( a_eLODType, ppLODBlocks, uiNumMemBlocks );

			TUINT8* pLODToBlock = ( a_eLODType == ATerrainLODType_High ) ? m_pHighLODToBlock : m_pLowLODToBlock;

			for ( TINT i = 0; i < uiNumMemBlocks; i++ )
			{
				if ( ppLODBlocks[ i ] == TNULL )
				{
					ppLODBlocks[ i ] = pTerrain->AllocateLODBlock( a_eLODType, this );

					if ( ppLODBlocks[ i ] == TNULL )
					{
						TASSERT( TFALSE, "Couldn't allocated new LOD block!" );
						return;
					}

					ppLODBlocks[ i ]->m_bIsUnused = TFALSE;
				}
			}

			if ( a_eLODType == ATerrainLODType_High && pTerrain->IsCollisionStreamed() )
			{
				LoadCollision();
			}

			LoadMatlib( a_eLODType );

			for ( TINT i = 0; i < iNumLODs; i++ )
			{
				if ( m_ppLODModelsData[ a_eLODType ][ i ] == TNULL )
				{
					auto pBlock     = ppLODBlocks[ pLODToBlock[ i ] ];
					auto pModelData = new ( pBlock->GetMemBlock() ) ModelNode();

					auto pTRB = new ( pBlock->GetMemBlock() ) TTRB();
					pBlock->SetupTRB( pTRB, pBlock );
					m_ppLODModelsData[ a_eLODType ][ i ] = pModelData;

					auto pModelLoaderJob = pTerrain->GetFreeModelLoaderJob();
					pModelLoaderJob->InitJob(
					    &pModelData->m_ModelRef,
					    pTRB,
					    ppLODNames[ i ],
					    TTRUE
					);

					AAssetStreaming::GetSingleton()->AddMainThreadJob( pModelLoaderJob );
				}
			}

			auto pSectionJob = pTerrain->GetFreeSectionLoaderJob();
			pSectionJob->InitJob( this, a_eLODType );

			AAssetStreaming::GetSingleton()->AddMainThreadJob( pSectionJob );

			SetLODQueued( a_eLODType, TFALSE );
			SetLODLoading( a_eLODType, TTRUE );
		}
	}
}

void ATerrainSection::LoadMatlib( ATerrainLODType a_eLODType )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	TUINT8             uiAllocAtBlock;
	ATerrainLODBlock** ppBlocks;

	if ( a_eLODType == ATerrainLODType_High )
	{
		if ( m_szHighMatLibFilename[ 0 ] == '\0' ) return;
		uiAllocAtBlock = m_iHighMatLibMemBlockID;
		ppBlocks       = m_ppHighLODBlocks;
	}
	else
	{
		if ( m_szLowMatLibFilename[ 0 ] == '\0' ) return;
		uiAllocAtBlock = m_iLowMatLibMemBlockID;
		ppBlocks       = m_ppLowLODBlocks;
	}

	auto pBlock     = ppBlocks[ uiAllocAtBlock ];
	auto pMatlibJob = ATerrainInterface::GetSingleton()->GetFreeMatlibLoaderJob();

	if ( a_eLODType == ATerrainLODType_High )
	{
		m_pMatLibHighTRB = new ( pBlock->GetMemBlock() ) TTRB();
		pBlock->SetupTRB( m_pMatLibHighTRB, pBlock );

		pMatlibJob->InitJob( m_szHighMatLibFilename, m_pMatLibHighTRB, m_pMatLibHigh, pBlock->GetMemBlock() );
	}
	else
	{
		m_pMatLibLowTRB = new ( pBlock->GetMemBlock() ) TTRB();
		pBlock->SetupTRB( m_pMatLibLowTRB, pBlock );

		pMatlibJob->InitJob( m_szLowMatLibFilename, m_pMatLibLowTRB, m_pMatLibLow, pBlock->GetMemBlock() );
	}

	AAssetStreaming::GetSingleton()->AddMainThreadJob( pMatlibJob );
}

void ATerrainSection::UnloadMatlib( ATerrainLODType a_eLODType )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_eLODType == ATerrainLODType_High )
	{
		if ( m_pMatLibHigh )
		{
			AMaterialLibraryManager::GetSingleton()->UnloadTexturesOfLibrary( m_pMatLibHigh );
			m_pMatLibHigh->Destroy();
			m_pMatLibHigh = TNULL;
		}

		if ( m_pMatLibHighTRB )
		{
			delete m_pMatLibHighTRB;
			m_pMatLibHighTRB = TNULL;
		}
	}
	else
	{
		if ( m_pMatLibLow )
		{
			AMaterialLibraryManager::GetSingleton()->UnloadTexturesOfLibrary( m_pMatLibLow );
			m_pMatLibLow->Destroy();
			m_pMatLibLow = TNULL;
		}

		if ( m_pMatLibLowTRB )
		{
			delete m_pMatLibLowTRB;
			m_pMatLibLowTRB = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 005ed650
void ATerrainSection::DestroyLOD( ATerrainLODType a_eLODType )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	auto pTerrain = ATerrainInterface::GetSingleton();
	TINT iNumLODs = GetLODCount( a_eLODType );

	for ( TINT i = 0; i < iNumLODs; i++ )
	{
		if ( m_ppLODModelsData[ a_eLODType ][ i ] )
		{
			pTerrain->DestroyModelData( m_ppLODModelsData[ a_eLODType ][ i ] );
			m_ppLODModelsData[ a_eLODType ][ i ] = TNULL;
		}
	}

	UnloadMatlib( a_eLODType );

	TUINT32 eNewFlags;

	if ( a_eLODType == ATerrainLODType_High )
	{
		if ( m_pCollisionModelData && pTerrain->IsCollisionStreamed() )
		{
			pTerrain->DestroyModelData( m_pCollisionModelData );
			m_pCollisionModelData = TNULL;
		}

		for ( TUINT i = 0; i < m_iNumHighMemBlocksUsed; i++ )
		{
			auto pBlock = m_ppHighLODBlocks[ i ];

			if ( pBlock )
			{
				m_ppHighLODBlocks[ i ] = TNULL;
				pBlock->Assign( TNULL, ATerrainLODType_None );
			}
		}

		eNewFlags = m_eFlags & ~( FLAGS_HIGH_LOD_LOADED | FLAGS_HIGH_LOD_LOADING | FLAGS_HIGH_LOD_QUEUED );
	}
	else
	{
		for ( TUINT i = 0; i < m_iNumLowMemBlocksUsed; i++ )
		{
			auto pBlock = m_ppLowLODBlocks[ i ];

			if ( pBlock )
			{
				m_ppLowLODBlocks[ i ] = TNULL;
				pBlock->Assign( TNULL, ATerrainLODType_None );
			}
		}

		eNewFlags = m_eFlags & ~( FLAGS_LOW_LOD_LOADED | FLAGS_LOW_LOD_LOADING | FLAGS_LOW_LOD_QUEUED );
	}

	m_eFlags = eNewFlags;
	SetLODEmpty( a_eLODType, TFALSE );
}

// $Barnyard: FUNCTION 005ecea0
void ATerrainSection::RemoveFromStreamingQueue()
{
	for ( TINT i = 0; i < ATerrainLODType_NUMOF; i++ )
	{
		if ( IsLODQueued( i ) )
		{
			ATerrainLODBlock** ppLODBlocks;
			TUINT16            uiNumLODBlocks;
			GetLODBlocks( i, ppLODBlocks, uiNumLODBlocks );

			for ( TINT k = 0; k < uiNumLODBlocks; k++ )
			{
				ppLODBlocks[ k ]->Assign( TNULL, ATerrainLODType_None );
			}

			SetLODQueued( i, TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 005ecf20
TBOOL ATerrainSection::IsMatLibLoaded( ATerrainLODType a_eLODType ) const
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_eLODType == ATerrainLODType_High )
	{
		return m_pMatLibHigh != TNULL;
	}
	else
	{
		return m_pMatLibLow != TNULL;
	}
}

void ATerrainSection::SetLODQueued( ATerrainLODType a_eLODType, TBOOL a_bQueued )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_bQueued )
	{
		m_eFlags |= ( 16 << a_eLODType );
	}
	else
	{
		m_eFlags &= ~( 16 << a_eLODType );
	}
}

void ATerrainSection::SetLODLoaded( ATerrainLODType a_eLODType, TBOOL a_bLoaded )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_bLoaded )
	{
		m_eFlags |= ( 1 << ( a_eLODType ) );
	}
	else
	{
		m_eFlags &= ~( 1 << ( a_eLODType ) );
	}
}

void ATerrainSection::SetLODLoading( ATerrainLODType a_eLODType, TBOOL a_bLoading )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_bLoading )
	{
		m_eFlags |= ( 1 << ( a_eLODType + ATerrainLODType_NUMOF ) );
	}
	else
	{
		m_eFlags &= ~( 1 << ( a_eLODType + ATerrainLODType_NUMOF ) );
	}
}

// $Barnyard: FUNCTION 005ece70
void ATerrainSection::SetLODEmpty( ATerrainLODType a_eLODType, TBOOL a_bEmpty )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_bEmpty )
	{
		m_eFlags |= ( 64 << a_eLODType );
	}
	else
	{
		m_eFlags &= ~( 64 << a_eLODType );
	}
}

// $Barnyard: FUNCTION 005ecf60
ATerrainSection::ModelNode::ModelNode()
    : m_eFlags( MNF_NONE ), m_bCreated( TFALSE ), m_pAnimatedMaterial( TNULL )
{
	m_szType[ 0 ] = '\0';
}

// $Barnyard: FUNCTION 005e8240
ATerrainSection::ModelNode::~ModelNode()
{
	TIMPLEMENT_D( "Destroy collision data" );

	if ( m_pSceneObject )
	{
		m_pSceneObject->Delete();
	}
}

// $Barnyard: FUNCTION 005e9380
void ATerrainSection::ModelNode::Render()
{
	if ( m_bCreated )
	{
		AWorldMaterialHAL* pAnimatedMaterial = TDYNAMICCAST( AWorldMaterialHAL, m_pAnimatedMaterial );

		if ( pAnimatedMaterial )
		{
			TScheduler* pScheduler = g_oSystemManager.GetScheduler();

			pAnimatedMaterial->AddUVOffsetY( 1, pScheduler->GetCurrentDeltaTime() * 0.02f );

			if ( 1.0 <= pAnimatedMaterial->GetUVOffsetY( 1 ) )
				pAnimatedMaterial->AddUVOffsetY( 1, -1.0f );
		}

		ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
		AWorldShaderHAL*   pShader  = TSTATICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );

		if ( m_eFlags & MNF_USE_LIGHTING )
			pShader->SetColours( pTerrain->GetLitShadowColor(), pTerrain->GetLitAmbientColor() );

		m_pSceneObject->EnableSkeletonUpdate();
		m_pSceneObject->RenderIfVisible();

		if ( m_eFlags & MNF_USE_LIGHTING )
			pShader->SetColours( pTerrain->GetDefaultShadowColor(), pTerrain->GetDefaultAmbientColor() );
	}
}

// $Barnyard: FUNCTION 005e82c0
void ATerrainSection::ModelNode::SetUseLighting( TBOOL a_bUseLighting )
{
	if ( a_bUseLighting )
		m_eFlags |= MNF_USE_LIGHTING;
	else
		m_eFlags &= ~MNF_USE_LIGHTING;
}

// $Barnyard: FUNCTION 005e9430
void ATerrainSection::ModelNode::SetGlow( TBOOL a_bIsGlow )
{
	if ( a_bIsGlow )
		m_eFlags |= MNF_GLOW;
	else
		m_eFlags &= ~MNF_GLOW;

	if ( TNULL != m_pSceneObject )
	{
		auto pModel = m_pSceneObject->GetModelRef()->GetModel();

		for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
		{
			auto& rLOD = pModel->GetLOD( i );

			for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
			{
				auto pMesh     = rLOD.ppMeshes[ k ];
				auto pMaterial = TSTATICCAST( AWorldMaterial, pMesh->GetMaterial() );

				pMesh->SetFlags( TMesh::State_Glow, TTRUE );
				pMaterial->SetFlags( TMaterial::FLAGS_GLOW, a_bIsGlow );
				pMaterial->SetBlendMode( 3 );
			}
		}
	}
}
