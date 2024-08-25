#include "pch.h"
#include "ATerrainInterface.h"
#include "Tasks/ARootTask.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AAssetStreaming.h"
#include "ALoadScreen.h"

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

	TTODO( "Call FUN_00619040() and initialise some other values" );
	m_iCurrentSection  = a_iStartVISGroup;
	m_iPreviousSection = -1;
	m_bUnused4         = TFALSE;
	// ...
	m_fUnused3                = 0.0f;
	m_cbOnCollsionModelLoaded = TNULL;
	m_cbOnModelLoaded         = TNULL;
	m_cbOnVISGroupChanged     = TNULL;
	m_fnGetCurrentVISGroup    = GetCurrentSectionID;
}

ATerrainInterface::~ATerrainInterface()
{
	TIMPLEMENT();
	m_bIsLoaded = TFALSE;

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

void ATerrainInterface::Update()
{
	if ( !m_pTerrainVIS ) return;

	if ( m_fnGetCurrentVISGroup )
	{
		TINT iCurrentGroup = m_fnGetCurrentVISGroup();

		if ( iCurrentGroup != -1 )
		{
			m_iCurrentSection = iCurrentGroup;
		}
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
							else if ( pGroups[ i ].IsLODQueued( ATerrainLODType_Low ) )
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

		auto pTerrainVIS = m_pTerrainVIS;

		if ( TNULL != pTerrainVIS )
		{
			// Reset current section if it is out of range
			if ( pTerrainVIS->m_iNumSections <= m_iCurrentSection || m_iCurrentSection < 0 )
			{
				m_iCurrentSection = 0;
			}

			auto pSections       = pTerrainVIS->m_pSections;
			auto iCurrentSection = m_iCurrentSection;
			auto pCurrentSection = &pSections[ iCurrentSection ];

			if ( TNULL == m_pOrderDVIS )
			{
				pCurrentSection->Draw( ATerrainLODType_High );
			}

			for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
			{
				ATerrainLODType eSectionVisibility = pCurrentSection->m_pVisibility[ i ];

				if ( eSectionVisibility != ATerrainLODType_None )
				{
					pSections[ i ].Draw( eSectionVisibility );
				}
			}
		}
	}
}

TBOOL ATerrainInterface::IsLoaded() const
{
	if ( m_pTerrainVIS == TNULL )
	{
		return TTRUE;
	}

	if ( m_pTerrainVIS->GetLocatorManager() && m_pTerrainVIS->GetLocatorManager()->GetTRB() )
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
		    "Terrain_Persistant" );
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
			    szBlockName );
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
			    szBlockName );
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
		    pPersitantBlock->GetMemBlock() );

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
			ThreadSleep( 20 );
		}

		WaitUntilLoaded();
	}

	m_bIsLoaded = TTRUE;
}

void ATerrainInterface::WaitUntilLoaded()
{
	TPROFILER_SCOPE();

	while ( TTRUE )
	{
		Update();

		if ( IsLoaded() ) return;
		TRenderInterface::GetSingleton()->Update( 1.0f / 1000.0f );
		AAssetStreaming::GetSingleton()->Update();
		ThreadSleep( 20 );
	}
}

void ATerrainInterface::DestroyModelData( ATerrainSection::ModelNode* a_pModelData )
{
	TVALIDPTR( a_pModelData );

	if ( a_pModelData->IsLinked() )
	{
		m_ModelDatas.Erase( a_pModelData );
	}

	if ( a_pModelData )
	{
		delete a_pModelData;
	}
}

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
	         ATerrainSection::FLAGS_HIGH_LOD_LOADED | ATerrainSection::FLAGS_HIGH_LOD_LOADING ) )
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

	TFLOAT fMinAccessTime  = TMath::MAXFLOAT;
	TINT   iPrevFoundIndex = -1;
	TINT   iFoundIndex;

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
				if ( pBlock->m_pVISGroup = TNULL ) break;

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

	return TNULL;
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

void ATerrainInterface::MoveAllFinishedJobs( Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs )
{
	for ( auto it = a_rUsedJobs.Begin(); it != a_rUsedJobs.End(); )
	{
		auto pNext = it.Next();

		if ( !it->GetNodeValue()->IsRunning() )
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
	TTODO( "Calculate real lighting colour" );

	TVector4 colour;
	colour.x = 0.3f;
	colour.y = 0.3f;
	colour.z = 0.1952941f;
	colour.w = 1.0f;

	TDYNAMICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() )->SetColours( colour, colour );
	a_pMeshSphere->m_pCellMesh->pMesh->Render();
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
			    pRenderContext->GetProjectionParams() );

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

ATerrainSection::ModelNode* ATerrainInterface::CreateModelInstance( ATerrainSection::ModelNode* a_pModelNode, const char* a_szModelName, const char* a_szType )
{
	// Make sure model is created
	auto pModel = a_pModelNode->m_ModelRef.GetModel();
	if ( !pModel || !pModel->IsCreated() )
	{
		a_pModelNode->m_ModelRef.Create(
		    a_szModelName,
		    AAssetLoader::GetAssetTRB( AAssetType_AssetPack ) );
	}

	auto pInstance          = a_pModelNode->m_ModelRef.CreateInstance();
	auto pInstanceTransform = &pInstance->GetTransform();

	pInstanceTransform->SetEuler( TVector3( 1.570796f, 0.0f, 0.0f ) );
	pInstanceTransform->SetTranslate( TVector3::VEC_ZERO );

	TMatrix44 collisionLocalMatrix;
	pInstanceTransform->GetLocalMatrixImp( collisionLocalMatrix );
	pInstanceTransform->SetMatrix( collisionLocalMatrix );

	a_pModelNode->m_pModelInstance = pInstance;

	pInstance->GetSomeVector1().x = 1.0f;
	pInstance->GetSomeVector1().y = 1.0f;
	pInstance->GetSomeVector2().x = 10000000.0f;
	pInstance->GetSomeVector2().y = 10000001.0f;
	pInstance->EnableSkeletonUpdate();
	pInstance->EnableUnknown1();

	TTODO( "Create collision model set" );

	auto pModelInstance = pInstance->GetInstance();
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

	m_ModelDatas.PushBack( a_pModelNode );
	a_pModelNode->m_bCreated = TTRUE;
	return a_pModelNode;
}

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
				if ( pGroup->m_pVisibility[ i ] == ( ( pGroup->m_eFlags >> 2 ) - 1 ) % ATerrainLODType_NUMOF ) return;
				if ( i == m_iCurrentSection ) return;

				AAssetStreaming::GetSingleton()->CancelAllJobs();
				return;
			}
		}
	}
}

void ATerrainInterface::UpdateNightMaterials()
{
	TIMPLEMENT();
}

TINT ATerrainInterface::GetCurrentSectionID()
{
	TIMPLEMENT();
	return 0;
}

TINT ATerrainInterface::GetPersistantSectionID()
{
	return ms_iPersistantVISGroupIndex;
}
