#include "pch.h"
#include "ATerrainInterface.h"
#include "Tasks/ARootTask.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AAssetStreaming.h"
#include "ALoadScreen.h"
#include "Physics/ACollisionModelSet.h"
#include "Physics/AOpcodeCache.h"
#include "Cameras/ACameraManager.h"

#include <Toshi/T2String.h>

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/AWorldShader/AWorldMesh_DX8.h"
#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005e94f0
ATerrainInterface::ATerrainInterface( TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup )
{
	m_bIsLoaded                = TTRUE;
	m_pTerrainVIS              = TNULL;
	m_iNumChunks               = 16;
	m_iPreloadTerrainBlockSize = a_iPreloadTerrainBlockSize;

	m_pJobs       = new JobSlot[ NUM_TOTAL_JOBS ];
	auto pJobSlot = &m_pJobs[ 0 ];

	for ( TUINT i = 0; i < MAX_NUM_MODEL_LOADER_JOBS; i++ )
	{
		pJobSlot->SetNodeValue( &m_aModelLoaderJobs[ i ] );
		m_FreeModelLoaderJobs.PushFront( pJobSlot );
		pJobSlot++;
	}

	for ( TUINT i = 0; i < MAX_NUM_KEYLIB_LOADER_JOBS; i++ )
	{
		pJobSlot->SetNodeValue( &m_aKeylibLoaderJobs[ i ] );
		m_FreeKeylibLoaderJobs.PushFront( pJobSlot );
		pJobSlot++;
	}

	for ( TUINT i = 0; i < MAX_NUM_MATLIB_LOADER_JOBS; i++ )
	{
		pJobSlot->SetNodeValue( &m_aMatlibLoaderJobs[ i ] );
		m_FreeMatlibLoaderJobs.PushFront( pJobSlot );
		pJobSlot++;
	}

	for ( TUINT i = 0; i < MAX_NUM_SECTION_LOADER_JOBS; i++ )
	{
		pJobSlot->SetNodeValue( &m_aSectionDoneJobs[ i ] );
		m_FreeSectionLoaderJobs.PushFront( pJobSlot );
		pJobSlot++;
	}

	for ( TUINT i = 0; i < MAX_NUM_TRB_LOADER_JOBS; i++ )
	{
		pJobSlot->SetNodeValue( &m_aTRBLoaderJobs[ i ] );
		m_FreeTRBLoaderJobs.PushFront( pJobSlot );
		pJobSlot++;
	}

	for ( TUINT i = 0; i < MAX_NUM_SKELETON_LOADER_JOBS; i++ )
	{
		pJobSlot->SetNodeValue( &m_aSkeletonDoneJobs[ i ] );
		m_FreeSkeletonLoaderJobs.PushFront( pJobSlot );
		pJobSlot++;
	}

	for ( TUINT i = 0; i < MAX_NUM_COLLISION_LOADER_JOBS; i++ )
	{
		pJobSlot->SetNodeValue( &m_aCollisionDoneJobs[ i ] );
		m_FreeCollisionLoaderJobs.PushFront( pJobSlot );
		pJobSlot++;
	}

	ACollisionModel::CreateStaticCache();
	m_iCurrentSection          = a_iStartVISGroup;
	m_iPreviousSection         = -1;
	m_bUnused4                 = TFALSE;
	m_bLightOverride           = TFALSE;
	m_fUnused3                 = 0.0f;
	m_cbOnCollisionModelLoaded = TNULL;
	m_cbOnModelLoaded          = TNULL;
	m_cbOnVISGroupChanged      = TNULL;
	m_fnGetCurrentVISGroup     = GetSectionPlayerIsIn;

	for ( TUINT i = 0; i < NUM_LIGHT_MAGS; i++ )
	{
		m_apLightMagMaterials[ i ] = TNULL;
		m_afLightMags[ i ]         = 0.0f;
	}
}

// $Barnyard: FUNCTION 005ec3e0
ATerrainInterface::~ATerrainInterface()
{
	TIMPLEMENT();

	m_bIsLoaded = TFALSE;
	ACollisionModel::DestroyStaticCache();

	m_UsedModelLoaderJobs.Clear();
	m_FreeModelLoaderJobs.Clear();
	m_UsedUnknownLoaderJobs.Clear();
	m_FreeUnknownLoaderJobs.Clear();
	m_UsedSectionLoaderJobs.Clear();
	m_FreeSectionLoaderJobs.Clear();
	m_UsedKeylibLoaderJobs.Clear();
	m_FreeKeylibLoaderJobs.Clear();
	m_UsedMatlibLoaderJobs.Clear();
	m_FreeMatlibLoaderJobs.Clear();
	m_UsedTRBLoaderJobs.Clear();
	m_FreeTRBLoaderJobs.Clear();
	m_UsedSkeletonLoaderJobs.Clear();
	m_FreeSkeletonLoaderJobs.Clear();
	m_UsedCollisionLoaderJobs.Clear();
	m_FreeCollisionLoaderJobs.Clear();

	delete[] m_pJobs;
}

// $Barnyard: FUNCTION 005eafa0
void ATerrainInterface::Update()
{
	if ( !m_pTerrainVIS ) return;

	if ( m_fnGetCurrentVISGroup )
	{
		TINT iCurrentGroup = m_fnGetCurrentVISGroup();

		if ( iCurrentGroup != -1 )
			m_iCurrentSection = iCurrentGroup;
	}

	if ( m_iCurrentSection >= 0 )
	{
		if ( m_iPreviousSection != m_iCurrentSection )
		{
			UpdateUsedBlocks( ATerrainLODType_High );
			UpdateUsedBlocks( ATerrainLODType_Low );

			UnqueueStreamingAssets();
			CancelUnrequiredJobs();

			if ( m_cbOnVISGroupChanged )
			{
				auto pPrevious = m_iPreviousSection >= 0 ? &m_pTerrainVIS->m_pSections[ m_iPreviousSection ] : TNULL;
				auto pCurrent  = m_iCurrentSection >= 0 ? &m_pTerrainVIS->m_pSections[ m_iCurrentSection ] : TNULL;

				m_cbOnVISGroupChanged( pPrevious, pCurrent );
			}

			m_iPreviousSection = m_iCurrentSection;
			QueueStreamingAssets();
		}

		if ( m_bIsLoaded && !IsLoaded() )
		{
			TFIXME( "Create and push ATerrainLoadingStallState if needed" );
		}

		TBOOL bSkipLoadingModels = TFALSE;

		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			if ( m_pTerrainVIS->m_pSections[ i ].IsLODLoading( ATerrainLODType_High ) ||
			     m_pTerrainVIS->m_pSections[ i ].IsLODLoading( ATerrainLODType_Low ) )
			{
				// There are already some models loading, don't create any new jobs
				bSkipLoadingModels = TTRUE;
				break;
			}
		}

		if ( !bSkipLoadingModels )
		{
			if ( HasAnyLODsQueued() )
			{
				auto pGroups = m_pTerrainVIS->m_pSections;

				if ( m_pOrderDVIS )
				{
					// Use precomputed order of queued groups to load
					auto pDVIS = &m_pOrderDVIS[ m_iCurrentSection ];

					for ( TINT i = 0; i < pDVIS->iCount; i++ )
					{
						auto uiGroupIndex = pDVIS->pInfo[ i ].uiGroupIndex;
						auto eLODType     = pDVIS->pInfo[ i ].eLODType;

						if ( pGroups[ uiGroupIndex ].IsLODQueued( eLODType ) )
						{
							pGroups[ uiGroupIndex ].LoadModels( eLODType );
							break;
						}
					}
				}
				else
				{
					// Brute force order of queued groups to load
					if ( pGroups[ m_iCurrentSection ].IsLODQueued( ATerrainLODType_High ) )
					{
						pGroups[ m_iCurrentSection ].LoadModels( ATerrainLODType_High );
					}
					else
					{
						for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
						{
							if ( pGroups[ i ].IsLODQueued( ATerrainLODType_High ) )
							{
								pGroups[ i ].LoadModels( ATerrainLODType_High );
								break;
							}
							
							if ( pGroups[ i ].IsLODQueued( ATerrainLODType_Low ) )
							{
								pGroups[ i ].LoadModels( ATerrainLODType_Low );
								break;
							}
						}
					}
				}
			}
		}

		TFIXME( "Restore value of m_fnGetCurrentVISGroup if it was saved" );

		// Wait till queued LODs are ready
		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			if ( m_pTerrainVIS->m_pSections[ i ].IsLODQueued( ATerrainLODType_High ) ||
			     m_pTerrainVIS->m_pSections[ i ].IsLODQueued( ATerrainLODType_Low ) )
			{
				FlushJobs();
				return;
			}
		}

		// Wait till all LODs are loaded
		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			if ( m_pTerrainVIS->m_pSections[ i ].IsLODLoading( ATerrainLODType_High ) ||
			     m_pTerrainVIS->m_pSections[ i ].IsLODLoading( ATerrainLODType_Low ) )
			{
				FlushJobs();
				return;
			}
		}

		if ( m_pOrderDVIS )
		{
			auto pDVIS = &m_pOrderDVIS[ m_iCurrentSection ];

			for ( TINT i = 0; i < pDVIS->iCount; i++ )
			{
				auto pInfo  = &pDVIS->pInfo[ i ];
				auto pGroup = &m_pTerrainVIS->m_pSections[ pInfo->uiGroupIndex ];

				if ( pGroup->IsLODLoaded( pInfo->eLODType ) && !pGroup->IsLODEmpty( pInfo->eLODType ) )
				{
					TRenderInterface::GetSingleton()->BeginEndSceneHAL();
					pGroup->DestroyLOD( pInfo->eLODType );
					QueueStreamingAssets();
				}
			}
		}
	}

	FlushJobs();
}

static TFLOAT s_ShadowColor2Multiplier = 0.6f;

// $Barnyard: FUNCTION 005ea8b0
void ATerrainInterface::Render()
{
	m_pTerrainVIS->m_pPersistantTerrainBlock->UpdateLastAccessTime();

	auto pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();
	auto pCameraObject  = pRenderContext->GetCameraObject();

	if ( pCameraObject )
	{
		auto pWorldShader = TDYNAMICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );

		m_DefaultShadowColor  = pWorldShader->GetShadowColour();
		m_DefaultAmbientColor = pWorldShader->GetAmbientColour();

		m_LitShadowColor.x = 0.5f;
		m_LitShadowColor.y = 0.5f;
		m_LitShadowColor.z = 0.3254902f;
		m_LitShadowColor.w = 1.0f;
		m_LitShadowColor.Multiply( s_ShadowColor2Multiplier );

		m_LitAmbientColor = m_LitShadowColor;
		m_bUnused4        = TFALSE;
		m_fUnused3        = 1.0f;
		m_bUnused4        = TTRUE;

		if ( m_pTerrainVIS )
		{
			// Reset current section if it is out of range
			if ( m_pTerrainVIS->m_iNumSections <= m_iCurrentSection || m_iCurrentSection < 0 )
				m_iCurrentSection = 0;

			auto pSections       = m_pTerrainVIS->m_pSections;
			auto iCurrentSection = m_iCurrentSection;
			auto pCurrentSection = &pSections[ iCurrentSection ];

			if ( !m_pOrderDVIS )
			{
				if ( pCurrentSection->IsLODLoaded( ATerrainLODType_High ) )
				{
					// Draw models of the high LOD
					for ( TINT i = 0; i < pCurrentSection->m_iNumHighModelFiles; i++ )
					{
						pCurrentSection->m_ppLODModelsData[ ATerrainLODType_High ][ i ]->Render();
					}
				}
				else if ( pCurrentSection->IsLODLoaded( ATerrainLODType_Low ) )
				{
					// High LOD is still loading so let's render low LOD instead
					for ( TINT i = 0; i < pCurrentSection->m_iNumLowModelFiles; i++ )
					{
						pCurrentSection->m_ppLODModelsData[ ATerrainLODType_Low ][ i ]->Render();
					}
				}
			}

			for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
			{
				ATerrainLODType eSectionVisibility = pSections[ iCurrentSection ].m_pVisibility[ i ];

				if ( eSectionVisibility != ATerrainLODType_None )
				{
					TINT iTargetNumModels = ( eSectionVisibility == ATerrainLODType_High ) ? pSections[ i ].m_iNumHighModelFiles : pSections[ i ].m_iNumLowModelFiles;

					if ( !pSections[ i ].IsLODLoaded( eSectionVisibility ) ||
					     !pSections[ i ].IsLODEmpty( eSectionVisibility ) ||
					     iTargetNumModels < 1 )
					{
						if ( pSections[ i ].IsLODLoaded( ATerrainLODType_Low )  )
						{
							// Target LOD is still loading, so let's see if we can render low LOD instead
							for ( TINT k = 0; k < pSections[ i ].m_iNumLowModelFiles; k++ )
							{
								if ( !pSections[ i ].m_ppLODModelsData[ ATerrainLODType_Low ][ k ]->IsGlow() )
									pSections[ i ].m_ppLODModelsData[ ATerrainLODType_Low ][ k ]->Render();
							}
						}
					}
					else
					{
						// Draw models of the target LOD, since it is loaded
						for ( TINT k = 0; k < iTargetNumModels; k++ )
						{
							if ( !pSections[ i ].m_ppLODModelsData[ eSectionVisibility ][ k ]->IsGlow() )
								pSections[ i ].m_ppLODModelsData[ eSectionVisibility ][ k ]->Render();
						}
					}
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 005e7d30
TBOOL ATerrainInterface::IsLoaded() const
{
	if ( m_pTerrainVIS == TNULL )
	{
		return TTRUE;
	}

	if ( m_pTerrainVIS->GetLocatorList() && m_pTerrainVIS->GetLocatorList()->GetTRB() )
	{
		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			if ( HASANYFLAG( m_pTerrainVIS->m_pSections[ i ].m_eFlags, 0xF3C ) )
			{
				return TFALSE;
			}
		}

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005ebfd0
void ATerrainInterface::LoadFromFile( const TCHAR* a_szFilePath, TBOOL a_bLoadLater, TBOOL a_bStreamCollision )
{
	TFIXME( "Do something with ATerrainSectionHudElement" );

	m_bIsLoaded = TFALSE;

	m_VISTRB.Close();
	m_VISTRB.Load( a_szFilePath, 0 );

	m_pTerrainVIS = m_VISTRB.CastSymbol<ATerrainVIS>( "terrainvis" );
	m_pOrderDVIS  = m_VISTRB.CastSymbol<ATerrainOrderDVIS>( "orderdvis" );

	if ( m_VISTRB.GetSymbolAddress( "preloadcollision" ) )
	{
		// We won't stream collision models if there's any preload collision
		a_bStreamCollision         = TFALSE;
		m_iPreloadTerrainBlockSize = m_pTerrainVIS->m_uiPersistantTerrainBlockSize;
	}

	m_VISTRB.DeleteSymbolTable();

	if ( !a_bStreamCollision )
	{
		m_pTerrainVIS->m_uiPersistantTerrainBlockSize = m_iPreloadTerrainBlockSize;
	}

	if ( m_pTerrainVIS->m_uiPersistantTerrainBlockSize > 0 )
	{
		m_pTerrainVIS->m_pPersistantTerrainBlock = new ATerrainLODBlock(
		    m_pTerrainVIS->m_uiPersistantTerrainBlockSize,
		    "Terrain_Persistant"
		);
	}

	// Create HighLOD blocks
	if ( m_pTerrainVIS->m_iNumHighBlocks > 0 )
	{
		m_pTerrainVIS->m_ppHighBlocks = new ATerrainLODBlock*[ m_pTerrainVIS->m_iNumHighBlocks ];

		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumHighBlocks; i++ )
		{
			TCHAR szBlockName[ 64 ];
			TStringManager::String8Format( szBlockName, sizeof( szBlockName ), "Terrain_HighLOD_Block%d", i );

			m_pTerrainVIS->m_ppHighBlocks[ i ] = new ATerrainLODBlock(
			    m_pTerrainVIS->m_uiHighBlockSize,
			    szBlockName
			);
		}
	}

	// Create LowLOD blocks
	if ( m_pTerrainVIS->m_iNumLowBlocks > 0 )
	{
		m_pTerrainVIS->m_ppLowBlocks = new ATerrainLODBlock*[ m_pTerrainVIS->m_iNumLowBlocks ];

		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumLowBlocks; i++ )
		{
			TCHAR szBlockName[ 64 ];
			TStringManager::String8Format( szBlockName, sizeof( szBlockName ), "Terrain_LowLOD_Block%d", i );

			m_pTerrainVIS->m_ppLowBlocks[ i ] = new ATerrainLODBlock(
			    m_pTerrainVIS->m_uiLowBlockSize,
			    szBlockName
			);
		}
	}

	m_pTerrainVIS->LoadSkeleton();

	// Load global matlib if it is specified
	if ( *m_pTerrainVIS->m_szMatLibrary != '\0' )
	{
		auto pMatlibJob      = GetFreeMatlibLoaderJob();
		auto pPersitantBlock = m_pTerrainVIS->m_pPersistantTerrainBlock;

		auto pTRB                            = new ( pPersitantBlock->GetMemBlock() ) TTRB();
		m_pTerrainVIS->m_pMaterialLibraryTRB = pTRB;
		pPersitantBlock->SetupTRB( pTRB, pPersitantBlock );

		pMatlibJob->InitJob(
		    m_pTerrainVIS->m_szMatLibrary,
		    m_pTerrainVIS->m_pMaterialLibraryTRB,
		    m_pTerrainVIS->m_pMaterialLibrary,
		    pPersitantBlock->GetMemBlock()
		);

		AAssetStreaming::GetSingleton()->AddMainThreadJob( pMatlibJob );
	}

	m_bStreamCollision = a_bStreamCollision;

	// If collision is not streamed, create jobs to load it for every section
	if ( !a_bStreamCollision )
	{
		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			m_pTerrainVIS->m_pSections[ i ].LoadCollision();
		}
	}

	// Wait until all assets are loaded
	if ( !a_bLoadLater )
	{
		while ( AAssetStreaming::GetSingleton()->HasActiveJobs() )
		{
			AAssetStreaming::GetSingleton()->Update();

#ifndef BARNYARD_COMMUNITY_PATCH
			ThreadSleep( 20 );
#endif // BARNYARD_COMMUNITY_PATCH
		}

		WaitUntilLoaded();
	}

	m_bIsLoaded = TTRUE;
}

// $Barnyard: FUNCTION 005ebec0
void ATerrainInterface::WaitUntilLoaded()
{
	TPROFILER_SCOPE();

	while ( TTRUE )
	{
		Update();

		if ( IsLoaded() ) return;
		TRenderInterface::GetSingleton()->Update( 1.0f / 1000.0f );
		AAssetStreaming::GetSingleton()->Update();

#ifndef BARNYARD_COMMUNITY_PATCH
		ThreadSleep( 20 );
#endif // BARNYARD_COMMUNITY_PATCH
	}
}

// $Barnyard: FUNCTION 005ead40
void ATerrainInterface::DestroyModelData( ATerrainSection::ModelNode* a_pModelData )
{
	TVALIDPTR( a_pModelData );

	if ( a_pModelData->IsLinked() )
	{
		m_llModelNodes.Erase( a_pModelData );
	}

	if ( a_pModelData )
	{
		delete a_pModelData;
	}
}

// $Barnyard: FUNCTION 005e8140
void ATerrainInterface::UpdateUsedBlocks( ATerrainLODType a_eLODType )
{
	TINT               iNumBlocks;
	ATerrainLODBlock** ppBlocks;

	if ( a_eLODType == ATerrainLODType_High )
	{
		iNumBlocks = m_pTerrainVIS->m_iNumHighBlocks;
		ppBlocks   = m_pTerrainVIS->m_ppHighBlocks;
	}
	else
	{
		iNumBlocks = m_pTerrainVIS->m_iNumLowBlocks;
		ppBlocks   = m_pTerrainVIS->m_ppLowBlocks;
	}

	auto pCurrentVISGroup = m_pTerrainVIS->m_pSections + m_iCurrentSection;

	for ( TINT i = 0; i < iNumBlocks; i++ )
	{
		auto pBlock = ppBlocks[ i ];
		pBlock->SetUnused();

		if ( a_eLODType == ATerrainLODType_High && pBlock->m_pVISGroup == pCurrentVISGroup )
		{
			pBlock->SetUsed();
		}
		else
		{
			auto pVIS = m_pTerrainVIS;

			for ( TINT k = 0; k < pVIS->m_iNumSections; k++ )
			{
				if ( pCurrentVISGroup->m_pVisibility[ k ] == a_eLODType &&
				     pBlock->m_pVISGroup == &pVIS->m_pSections[ k ] )
				{
					pBlock->SetUsed();
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 005e80b0
void ATerrainInterface::QueueStreamingAssets()
{
	TASSERT( m_iCurrentSection != -1 );

	auto pCurrentGroup = &m_pTerrainVIS->m_pSections[ m_iCurrentSection ];

	for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
	{
		auto eLODType = pCurrentGroup->m_pVisibility[ i ];

		if ( eLODType > ATerrainLODType_None )
		{
			auto pGroup = &m_pTerrainVIS->m_pSections[ i ];

			if ( ( pGroup->m_eFlags & ( 21 << eLODType ) ) == 0 )
			{
				pGroup->SetLODQueued( eLODType, TTRUE );
			}
		}
	}

	if ( !HASANYFLAG(
	         pCurrentGroup->m_eFlags,
	         ATerrainSection::FLAGS_HIGH_LOD_LOADED | ATerrainSection::FLAGS_HIGH_LOD_LOADING
	     ) )
	{
		pCurrentGroup->SetLODQueued( ATerrainLODType_High, TTRUE );
	}
}

void ATerrainInterface::UnqueueStreamingAssets()
{
	for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
	{
		m_pTerrainVIS->m_pSections[ i ].RemoveFromStreamingQueue();
	}
}

// $Barnyard: FUNCTION 005e8070
TBOOL ATerrainInterface::HasAnyLODsQueued()
{
	for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
	{
		auto pGroup = &m_pTerrainVIS->m_pSections[ i ];

		if ( pGroup->m_eFlags & ( ATerrainSection::FLAGS_HIGH_LOD_QUEUED | ATerrainSection::FLAGS_LOW_LOD_QUEUED ) )
		{
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005ecca0
ATerrainLODBlock* ATerrainInterface::AllocateLODBlock( ATerrainLODType a_eLODType, ATerrainSection* a_pVISGroup )
{
	TINT               iNumBlocks;
	ATerrainLODBlock** ppBlocks;

	if ( a_eLODType == ATerrainLODType_High )
	{
		iNumBlocks = m_pTerrainVIS->m_iNumHighBlocks;
		ppBlocks   = m_pTerrainVIS->m_ppHighBlocks;
	}
	else
	{
		iNumBlocks = m_pTerrainVIS->m_iNumLowBlocks;
		ppBlocks   = m_pTerrainVIS->m_ppLowBlocks;
	}

	if ( iNumBlocks > 0 )
	{
		TFLOAT fMinAccessTime  = TMath::MAXFLOAT;
		TINT   iPrevFoundIndex = -1;
		TINT   iFoundIndex     = -1;

		for ( TINT i = 0; i < iNumBlocks; i++ )
		{
			auto pBlock = ppBlocks[ i ];
			iFoundIndex = iPrevFoundIndex;

			if ( !pBlock->IsUsed() )
			{
				iFoundIndex = i;
				if ( pBlock->m_pVISGroup == TNULL ) break;

				iFoundIndex = iPrevFoundIndex;
				if ( !pBlock->m_pVISGroup->IsLODLoading( a_eLODType ) )
				{
					iFoundIndex = i;
					if ( pBlock->m_pVISGroup == TNULL ) break;

					iFoundIndex = iPrevFoundIndex;
					if ( pBlock->m_fLastAccessTime < fMinAccessTime )
					{
						fMinAccessTime = pBlock->m_fLastAccessTime;
						iFoundIndex    = i;
					}
				}
			}

			iPrevFoundIndex = iFoundIndex;
		}

		if ( iFoundIndex >= 0 )
		{
			auto pBlock    = ppBlocks[ iFoundIndex ];
			auto pVISGroup = pBlock->m_pVISGroup;

			if ( pVISGroup )
			{
				if ( pVISGroup->IsLODEmpty( pBlock->m_eLODType ) )
				{
					pVISGroup->SetLODEmpty( pBlock->m_eLODType, TFALSE );
					return TNULL;
				}

				if ( pVISGroup )
				{
					if ( ( pVISGroup->m_eFlags & ( 21 << a_eLODType ) ) != 0 )
					{
						if ( pVISGroup->IsMatLibLoaded( pBlock->m_eLODType ) )
						{
							pVISGroup->SetLODEmpty( pBlock->m_eLODType, TFALSE );

							if ( pBlock->m_pVISGroup )
							{
								pBlock->m_pVISGroup->UnloadMatlib( pBlock->m_eLODType );
								return TNULL;
							}
						}
						else if ( pBlock->m_pVISGroup )
						{
							pBlock->m_pVISGroup->DestroyLOD( pBlock->m_eLODType );
						}

						return TNULL;
					}

					if ( pVISGroup )
					{
						pVISGroup->DestroyLOD( pBlock->m_eLODType );
					}
				}
			}

			pBlock->Assign( a_pVISGroup, a_eLODType );
			return pBlock;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005e8d70
void ATerrainInterface::GetLocatorTransform( const TCHAR* a_pszLocatorName, TMatrix44& a_rOutTransform )
{
	ATerrainLocatorList* pLocatorManager = GetLocatorList();

	ATerrainLocator* pLocator = pLocatorManager->FindLocator( a_pszLocatorName );
	TASSERT( pLocator != TNULL );

	// Get transform of the locator
	TMatrix44 matLocator;
	pLocator->GetMatrix( matLocator );

	// Get locators global transform
	pLocatorManager->GetWorldTransform().GetLocalMatrixImp( a_rOutTransform );

	// Transform locator by the global transform
	a_rOutTransform.Multiply( matLocator );
}

// $Barnyard: FUNCTION 005e8b60
TVector4 ATerrainInterface::GetLocatorPos( const TCHAR* a_pszLocatorName )
{
	ATerrainLocatorList* pLocatorManager = GetLocatorList();

	ATerrainLocator* pLocator = pLocatorManager->FindLocator( a_pszLocatorName );
	TASSERT( pLocator != TNULL );

	// Get transform of the locator
	TMatrix44 matLocator;
	pLocator->GetMatrix( matLocator );

	// Get locators global transform and transform the locator
	TMatrix44 matTransformed;
	pLocatorManager->GetWorldTransform().GetLocalMatrixImp( matTransformed );
	matTransformed.Multiply( matLocator );

	return matTransformed.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $Barnyard: FUNCTION 005e8bf0
void ATerrainInterface::GetLocatorPosAndRot( const TCHAR* a_pszLocatorName, Toshi::TVector4& a_rOutPosition, Toshi::TQuaternion& a_rOutRotation )
{
	ATerrainLocatorList* pLocatorManager = GetLocatorList();

	ATerrainLocator* pLocator = pLocatorManager->FindLocator( a_pszLocatorName );
	TASSERT( pLocator != TNULL );

	// Get transform of the locator
	TMatrix44 matLocator;
	pLocator->GetMatrix( matLocator );

	// Get locators global transform and transform the locator
	TMatrix44 matTransformed;
	pLocatorManager->GetWorldTransform().GetLocalMatrixImp( matTransformed );
	matTransformed.Multiply( matLocator );

	TVector3& vecUp = matTransformed.AsBasisVector3( BASISVECTOR_UP );
	a_rOutRotation  = TQuaternion( -vecUp.x, -vecUp.y, -vecUp.z, 1.0f );
	a_rOutPosition  = matTransformed.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $Barnyard: FUNCTION 005e82f0
ATerrainLocator* ATerrainInterface::FindLocator( const TCHAR* a_pszLocatorName )
{
	if ( !m_pTerrainVIS )
		return TNULL;

	return GetLocatorList()->FindLocator( a_pszLocatorName );
}

ATRBLoaderJob* ATerrainInterface::GetFreeTRBLoaderJob()
{
	TASSERT( m_FreeTRBLoaderJobs.Size() > 0, "No free ATRBLoaderJobs left!" );

	if ( m_FreeTRBLoaderJobs.Size() > 0 )
	{
		auto pTRBJobSlot = m_FreeTRBLoaderJobs.PopFront();
		m_UsedTRBLoaderJobs.PushFront( pTRBJobSlot );

		return TSTATICCAST( ATRBLoaderJob, pTRBJobSlot->GetNodeValue() );
	}
	else
	{
		return TNULL;
	}
}

ASkeletonDoneJob* ATerrainInterface::GetFreeSkeletonLoaderJob()
{
	TASSERT( m_FreeSkeletonLoaderJobs.Size() > 0, "No free ASkeletonDoneJobs left!" );

	if ( m_FreeSkeletonLoaderJobs.Size() > 0 )
	{
		auto pTRBJobSlot = m_FreeSkeletonLoaderJobs.PopFront();
		m_UsedSkeletonLoaderJobs.PushFront( pTRBJobSlot );

		return TSTATICCAST( ASkeletonDoneJob, pTRBJobSlot->GetNodeValue() );
	}
	else
	{
		return TNULL;
	}
}

// $Barnyard: FUNCTION 005ed190
AMatLibLoaderJob* ATerrainInterface::GetFreeMatlibLoaderJob()
{
	TASSERT( m_FreeMatlibLoaderJobs.Size() > 0, "No free AMatlibLoaderJobs left!" );

	if ( m_FreeMatlibLoaderJobs.Size() > 0 )
	{
		auto pTRBJobSlot = m_FreeMatlibLoaderJobs.PopFront();
		m_UsedMatlibLoaderJobs.PushFront( pTRBJobSlot );

		return TSTATICCAST( AMatLibLoaderJob, pTRBJobSlot->GetNodeValue() );
	}
	else
	{
		return TNULL;
	}
}

ACollisionDoneJob* ATerrainInterface::GetFreeCollisionLoaderJob()
{
	TASSERT( m_FreeCollisionLoaderJobs.Size() > 0, "No free ACollisionDoneJob left!" );

	if ( m_FreeCollisionLoaderJobs.Size() > 0 )
	{
		auto pTRBJobSlot = m_FreeCollisionLoaderJobs.PopFront();
		m_UsedCollisionLoaderJobs.PushFront( pTRBJobSlot );

		return TSTATICCAST( ACollisionDoneJob, pTRBJobSlot->GetNodeValue() );
	}
	else
	{
		return TNULL;
	}
}

AModelLoaderJob* ATerrainInterface::GetFreeModelLoaderJob()
{
	TASSERT( m_FreeModelLoaderJobs.Size() > 0, "No free AModelLoaderJob left!" );

	if ( m_FreeModelLoaderJobs.Size() > 0 )
	{
		auto pTRBJobSlot = m_FreeModelLoaderJobs.PopFront();
		m_UsedModelLoaderJobs.PushFront( pTRBJobSlot );

		return TSTATICCAST( AModelLoaderJob, pTRBJobSlot->GetNodeValue() );
	}
	else
	{
		return TNULL;
	}
}

ASectionDoneJob* ATerrainInterface::GetFreeSectionLoaderJob()
{
	TASSERT( m_FreeSectionLoaderJobs.Size() > 0, "No free ASectionDoneJob left!" );

	if ( m_FreeSectionLoaderJobs.Size() > 0 )
	{
		auto pTRBJobSlot = m_FreeSectionLoaderJobs.PopFront();
		m_UsedSectionLoaderJobs.PushFront( pTRBJobSlot );

		return TSTATICCAST( ASectionDoneJob, pTRBJobSlot->GetNodeValue() );
	}
	else
	{
		return TNULL;
	}
}

void ATerrainInterface::FlushJobs()
{
	MoveAllFinishedJobs( m_FreeModelLoaderJobs, m_UsedModelLoaderJobs );
	MoveAllFinishedJobs( m_FreeUnknownLoaderJobs, m_UsedUnknownLoaderJobs );
	MoveAllFinishedJobs( m_FreeSectionLoaderJobs, m_UsedSectionLoaderJobs );
	MoveAllFinishedJobs( m_FreeKeylibLoaderJobs, m_UsedKeylibLoaderJobs );
	MoveAllFinishedJobs( m_FreeMatlibLoaderJobs, m_UsedMatlibLoaderJobs );
	MoveAllFinishedJobs( m_FreeTRBLoaderJobs, m_UsedTRBLoaderJobs );
	MoveAllFinishedJobs( m_FreeSkeletonLoaderJobs, m_UsedSkeletonLoaderJobs );
	MoveAllFinishedJobs( m_FreeCollisionLoaderJobs, m_UsedCollisionLoaderJobs );
}

// $Barnyard: FUNCTION 005eacd0
void ATerrainInterface::MoveAllFinishedJobs( Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs )
{
	for ( auto it = a_rUsedJobs.Begin(); it != a_rUsedJobs.End(); )
	{
		auto pNext = it.Next();

		if ( it->GetNodeValue()->IsFinished() )
		{
			a_rUsedJobs.Erase( it );
			a_rFreeJobs.PushFront( it );
		}

		it = pNext;
	}
}

#ifdef TOSHI_SKU_WINDOWS

static void RenderWorldWin( TModelInstance* a_pModelInstance )
{
	auto pSkeletonInstance = a_pModelInstance->GetSkeletonInstance();

	if ( TNULL != pSkeletonInstance )
	{
		pSkeletonInstance->UpdateState( TFALSE );
	}

	TRenderInterface::GetSingleton()->GetCurrentContext()->SetSkeletonInstance( pSkeletonInstance );

	auto pModel = a_pModelInstance->GetModel();
	auto pLOD   = &pModel->GetLOD( a_pModelInstance->GetLOD() );

	for ( TINT i = 0; pLOD->iNumMeshes; i++ )
	{
		TVALIDPTR( pLOD->ppMeshes[ i ] );
		pLOD->ppMeshes[ i ]->Render();
	}
}

static void RenderNothing( TModelInstance* a_pModelInstance )
{
}

static void RenderCellMeshDefault( CellMeshSphere* a_pMeshSphere, RenderData* a_pRenderData )
{
	TVALIDPTR( a_pMeshSphere );
	a_pMeshSphere->m_pCellMesh->pMesh->Render();
}

static void RenderCellMeshWin( CellMeshSphere* a_pMeshSphere, RenderData* a_pRenderData )
{
	TVALIDPTR( a_pMeshSphere );

	ATerrainInterface* pTerrainInterface = ATerrainInterface::GetSingleton();
	TMesh*             pMesh             = a_pMeshSphere->m_pCellMesh->pMesh;

	// Get light mag
	TFLOAT fLightMag = 0.6f;
	for ( TUINT i = 0; i < ATerrainInterface::NUM_LIGHT_MAGS; i++ )
	{
		if ( pTerrainInterface->m_apLightMagMaterials[ i ] == pMesh->GetMaterial() )
		{
			fLightMag = pTerrainInterface->m_afLightMags[ i ];

			if ( fLightMag <= 0.0f )
				return;

			break;
		}
	}

	TVector4 vecColour{ 0.3f, 0.3f, 0.1952941f, 1.0f };
	vecColour *= fLightMag;

	TDYNAMICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() )->SetColours( vecColour, vecColour );
	pMesh->Render();
}

static void RenderWorldVisWin( TModelInstance* a_pModelInstance, void* a_pModelNode )
{
	auto pModel     = a_pModelInstance->GetModel();
	auto pModelNode = TSTATICCAST( ATerrainSection::ModelNode, a_pModelNode );

	if ( TNULL != pModel->GetTRB() )
	{
		auto pWorldDatabase = pModel->CastSymbol<WorldDatabase>( "Database" );
		TASSERT( TNULL != pWorldDatabase );

		if ( TNULL != pWorldDatabase )
		{
			auto pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

			// Save alpha blend value to restore it after rendering
			TFLOAT fAlphaBlend = pRenderContext->GetAlphaBlend();

			TMatrix44 projection;
			pRenderContext->ComputePerspectiveProjection(
			    projection,
			    pRenderContext->GetViewportParameters(),
			    pRenderContext->GetProjectionParams()
			);

			TMatrix44 modelView = pRenderContext->GetModelViewMatrix();

			const TBOOL bIsDefaultRenderer =
			    ( pModelNode->m_WorldVis.m_pfnRenderCallback == RenderCellMeshDefault );

			// If using lighting, specify method that calculates lighting
			if ( bIsDefaultRenderer && pModelNode->IsUsingLighting() )
				pModelNode->m_WorldVis.m_pfnRenderCallback = RenderCellMeshWin;

			// Build visibility and render
			pModelNode->m_WorldVis.Reset();
			pModelNode->m_WorldVis.Build( modelView, projection, 0 );
			pModelNode->m_WorldVis.Render( modelView );

			// Set render method back to default
			if ( bIsDefaultRenderer )
				pModelNode->m_WorldVis.m_pfnRenderCallback = RenderCellMeshDefault;

			pRenderContext->SetAlphaBlend( fAlphaBlend );
		}
	}
}

#endif // TOSHI_SKU_WINDOWS

// $Barnyard: FUNCTION 005e8670
ATerrainSection::ModelNode* ATerrainInterface::CreateModelInstance( ATerrainSection::ModelNode* a_pModelNode, const char* a_szModelName, const char* a_szType )
{
	// Make sure model is created
	auto pModel = a_pModelNode->m_ModelRef.GetModel();
	if ( !pModel || !pModel->IsCreated() )
	{
		a_pModelNode->m_ModelRef.Create(
		    a_szModelName,
		    AAssetLoader::GetAssetTRB( AAssetType_AssetPack )
		);
	}

	auto pSceneObject       = a_pModelNode->m_ModelRef.CreateSceneObject();
	auto pInstanceTransform = &pSceneObject->GetTransform();

	pInstanceTransform->SetEuler( TVector3( 1.570796f, 0.0f, 0.0f ) );
	pInstanceTransform->SetTranslate( TVector3::VEC_ZERO );

	TMatrix44 collisionLocalMatrix;
	pInstanceTransform->GetLocalMatrixImp( collisionLocalMatrix );
	pInstanceTransform->SetMatrix( collisionLocalMatrix );

	a_pModelNode->m_pSceneObject = pSceneObject;

	pSceneObject->GetSomeVector1().x = 1.0f;
	pSceneObject->GetSomeVector1().y = 1.0f;
	pSceneObject->GetSomeVector2().x = 10000000.0f;
	pSceneObject->GetSomeVector2().y = 10000001.0f;
	pSceneObject->EnableSkeletonUpdate();
	pSceneObject->EnableUnknown1();

	// Update pointer to the model object
	pModel = pSceneObject->GetInstance()->GetModel();

	// Create collision model instance
	if ( pModel->GetNumCollisionMeshes() > 0 )
	{
		ACollisionModelSet* pCollisionSet  = new ACollisionModelSet();
		a_pModelNode->m_pCollisionModelSet = pCollisionSet;

		pCollisionSet->Create( pModel );
		a_pModelNode->m_oCollisionInstance.Create( pCollisionSet, pSceneObject );
	}

	auto pModelInstance = pSceneObject->GetInstance();
	pModel              = pModelInstance->GetModel();

	auto pWorldDatabase = pModel->CastSymbol<WorldDatabase>( "Database" );

	if ( TNULL == pWorldDatabase )
	{
		// Render as a simple model
		pModelInstance->SetPreRenderCallback( RenderWorldWin );
	}
	else
	{
		// Render WorldVIS
		auto& rLOD = pModel->GetLOD( 0 );

		if ( rLOD.iNumMeshes < 1 )
		{
			// Nothing to render here
			pModelInstance->SetPreRenderCallback( RenderNothing );
		}
		else
		{
			pModelInstance->SetCustomRenderMethod( RenderWorldVisWin, a_pModelNode );
			a_pModelNode->m_WorldVis.Create( pWorldDatabase->m_ppWorlds[ 0 ] );

			TBOOL bIsWorldMesh = rLOD.ppMeshes[ 0 ]->IsA( &TGetClass( AWorldMesh ) );

			if ( bIsWorldMesh )
			{
				a_pModelNode->m_WorldVis.m_pfnRenderCallback = RenderCellMeshDefault;
				TTODO( "Setup materials" );
			}
			else
			{
				a_pModelNode->m_WorldVis.m_pfnRenderCallback = RenderCellMeshDefault;
			}
		}
	}

	if ( TNULL != a_szType )
	{
		TASSERT( TStringManager::String8Length( a_szType ) <= ATerrainSection::ModelNode::TYPE_NAME_MAX_SIZE );
		TStringManager::String8Copy( a_pModelNode->m_szType, a_szType, ATerrainSection::ModelNode::TYPE_NAME_MAX_SIZE );
	}

	m_llModelNodes.PushBack( a_pModelNode );
	a_pModelNode->m_bCreated = TTRUE;
	return a_pModelNode;
}

// $Barnyard: FUNCTION 005e92f0
void ATerrainInterface::CancelUnrequiredJobs()
{
	if ( m_iCurrentSection != m_iPreviousSection && m_iPreviousSection >= 0 )
	{
		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			auto pGroup = &m_pTerrainVIS->m_pSections[ i ];

			if ( pGroup->m_eFlags & ( ATerrainSection::FLAGS_HIGH_LOD_LOADING | ATerrainSection::FLAGS_LOW_LOD_LOADING ) )
			{
				if ( i < 0 ) return;
				if ( m_pTerrainVIS->m_pSections[ m_iCurrentSection ].m_pVisibility[ i ] == ( ( pGroup->m_eFlags >> 2 ) - 1 ) % ATerrainLODType_NUMOF ) return;
				if ( i == m_iCurrentSection ) return;

				AAssetStreaming::GetSingleton()->CancelAllJobs();
				return;
			}
		}
	}
}

// $Barnyard: FUNCTION 005e8e80
void ATerrainInterface::UpdateNightMaterials()
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 005e8de0
TINT ATerrainInterface::GetSectionPlayerIsIn()
{
	TASSERT( TTRUE == IsSingletonCreated() );

	if ( ACamera* pCurrentCamera = ACameraManager::GetSingleton()->GetCurrentCamera() )
	{
		TVector4& vecCameraPos = pCurrentCamera->GetMatrix().GetTranslation();

		TINT         iCurrSection = ms_pSingleton->m_iCurrentSection;
		ATerrainVIS* pVIS         = ms_pSingleton->m_pTerrainVIS;

		// Check if we are still in that section
		if ( iCurrSection > -1 && iCurrSection < pVIS->m_iNumSections )
		{
			if ( pVIS->m_pSections[ iCurrSection ].IsPointInside( vecCameraPos ) )
			{
				return iCurrSection;
			}
		}

		// We are not in that section anymore, so look through all of them
		for ( TINT i = 0; i < pVIS->m_iNumSections; i++ )
		{
			if ( pVIS->m_pSections[ i ].IsPointInside( vecCameraPos ) )
			{
				return i;
			}
		}
	}

	return -1;
}

TINT ATerrainInterface::GetPersistantSectionID()
{
	return ms_iPersistantVISGroupIndex;
}
