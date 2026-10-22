#include "pch.h"
#include "ATerrainInterface.h"
#include "Tasks/ARootTask.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AAssetStreaming.h"
#include "ALoadScreen.h"
#include "Physics/ACollisionModelSet.h"
#include "Physics/AOpcodeCache.h"
#include "Cameras/ACameraManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ATerrainLoadingStallState.h"
#include "Locale/ALocaleManager.h"
#include "Helpers/AGameTimeManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ATerrainSectionHudElement.h"
#include "Render/AGlowViewport.h"

#include <Toshi/T2String.h>

#include "Render/ABackend.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TINT GetNoOpVISGroup()
{
	return -1;
}

TINT                                    ATerrainInterface::ms_iForcedSection               = -1;
ATerrainInterface::t_GetCurrentVISGroup ATerrainInterface::ms_fnUnforcedGetCurrentVISGroup = TNULL;

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

	m_eSectionRayIgnoreGroups = TCollisionCommon::TOSHICGROUPFLAG( 0xB3FFE );
}

// $Barnyard: FUNCTION 005ec3e0
// $Barnyard: FUNCTION 005ec7e0
ATerrainInterface::~ATerrainInterface()
{
	m_bIsLoaded = TFALSE;
	ACollisionModel::DestroyStaticCache();

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	if ( m_pTerrainVIS && !TSystemManager::Quitted() )
	{
		UpdateUsedBlocks( ATerrainLODType_High );
		UpdateUsedBlocks( ATerrainLODType_Low );

		TINT iLoadScreenCounter = 0;

		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			UpdateUsedBlocks( ATerrainLODType_High );
			UpdateUsedBlocks( ATerrainLODType_Low );

			auto pSection = &m_pTerrainVIS->m_pSections[ i ];
			pSection->SetLODQueued( ATerrainLODType_High, TFALSE );
			pSection->SetLODQueued( ATerrainLODType_Low, TFALSE );

			iLoadScreenCounter++;

			if ( iLoadScreenCounter > 5 )
			{
				g_oLoadScreen.Update( 1.0f, TTRUE );
				iLoadScreenCounter = 0;
			}
		}

		m_fnGetCurrentVISGroup = GetNoOpVISGroup;
		WaitUntilLoaded();
	}

	if ( m_pTerrainVIS )
	{
		m_pTerrainVIS->Destroy();
		m_VISTRB.Close();
		m_pTerrainVIS = TNULL;
	}

	TRenderInterface::GetSingleton()->FlushDyingResources();

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

	UpdateNightMaterials();

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

				m_cbOnVISGroupChanged( pCurrent, pPrevious );
			}

			m_iPreviousSection = m_iCurrentSection;
			QueueStreamingAssets();
		}

		if ( m_bIsLoaded && !AGameStateController::GetSingleton()->GetCurrentState()->IsExactly( &TGetClass( ATerrainLoadingStallState ) ) && !IsLoaded() )
		{
			// The camera is above a section that isn't loaded yet, stall the game until it is
			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

			if ( pCamera && m_pTerrainVIS->m_pSections[ m_iCurrentSection ].FindUnloadedSectionBelow( pCamera->GetMatrix().GetTranslation() ) )
			{
				AGameStateController::GetSingleton()->PushState( new ATerrainLoadingStallState() );
				return;
			}
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

		// Stop forcing the section once it's loaded
		if ( m_fnGetCurrentVISGroup == GetForcedSection && IsLoaded() == TTRUE )
		{
			m_fnGetCurrentVISGroup          = ms_fnUnforcedGetCurrentVISGroup;
			ms_iForcedSection               = -1;
			ms_fnUnforcedGetCurrentVISGroup = TNULL;
		}

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

				if ( pGroup->IsLODLoaded( pInfo->eLODType ) && !pGroup->IsLODReady( pInfo->eLODType ) )
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
					     !pSections[ i ].IsLODReady( eSectionVisibility ) ||
					     iTargetNumModels < 1 )
					{
						if ( pSections[ i ].IsLODLoaded( ATerrainLODType_Low ) )
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

// $Barnyard: FUNCTION 005eab00
void ATerrainInterface::RenderGlowModels()
{
	auto pWorldShader = TSTATICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );

	m_DefaultShadowColor  = pWorldShader->GetShadowColour();
	m_DefaultAmbientColor = pWorldShader->GetAmbientColour();

	if ( m_pTerrainVIS )
	{
		// Reset current section if it is out of range
		if ( m_pTerrainVIS->m_iNumSections <= m_iCurrentSection || m_iCurrentSection < 0 )
			m_iCurrentSection = 0;

		auto pSections       = m_pTerrainVIS->m_pSections;
		auto pCurrentSection = &pSections[ m_iCurrentSection ];

		if ( !m_pOrderDVIS )
		{
			if ( pCurrentSection->IsLODLoaded( ATerrainLODType_High ) )
			{
				for ( TINT i = 0; i < pCurrentSection->m_iNumHighModelFiles; i++ )
				{
					if ( pCurrentSection->m_ppLODModelsData[ ATerrainLODType_High ][ i ]->IsGlow() )
						pCurrentSection->m_ppLODModelsData[ ATerrainLODType_High ][ i ]->Render();
				}
			}
			else if ( pCurrentSection->IsLODLoaded( ATerrainLODType_Low ) )
			{
				for ( TINT i = 0; i < pCurrentSection->m_iNumLowModelFiles; i++ )
				{
					if ( pCurrentSection->m_ppLODModelsData[ ATerrainLODType_Low ][ i ]->IsGlow() )
						pCurrentSection->m_ppLODModelsData[ ATerrainLODType_Low ][ i ]->Render();
				}
			}
		}

		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			ATerrainLODType eSectionVisibility = pCurrentSection->m_pVisibility[ i ];

			if ( eSectionVisibility > ATerrainLODType_None )
			{
				ATerrainSection* pSection         = &m_pTerrainVIS->m_pSections[ i ];
				TINT             iTargetNumModels = ( eSectionVisibility == ATerrainLODType_High ) ? pSection->m_iNumHighModelFiles : pSection->m_iNumLowModelFiles;

				if ( !pSection->IsLODLoaded( eSectionVisibility ) ||
				     !pSection->IsLODReady( eSectionVisibility ) ||
				     iTargetNumModels < 1 )
				{
					// Target LOD is not ready, fall back to the low LOD
					if ( pSection->IsLODLoaded( ATerrainLODType_Low ) && pSection->IsLODReady( ATerrainLODType_Low ) )
					{
						for ( TINT k = 0; k < pSection->m_iNumLowModelFiles; k++ )
						{
							if ( pSection->m_ppLODModelsData[ ATerrainLODType_Low ][ k ]->IsGlow() )
								pSection->m_ppLODModelsData[ ATerrainLODType_Low ][ k ]->Render();
						}
					}
				}
				else
				{
					for ( TINT k = 0; k < iTargetNumModels; k++ )
					{
						if ( pSection->m_ppLODModelsData[ eSectionVisibility ][ k ]->IsGlow() )
							pSection->m_ppLODModelsData[ eSectionVisibility ][ k ]->Render();
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
			if ( HASANYFLAG( m_pTerrainVIS->m_pSections[ i ].m_eFlags, ATerrainSection::FLAGS_HIGH_LOD_LOADING | ATerrainSection::FLAGS_LOW_LOD_LOADING | ATerrainSection::FLAGS_HIGH_LOD_QUEUED | ATerrainSection::FLAGS_LOW_LOD_QUEUED | ATerrainSection::FLAGS_COLLISION_LOADING ) )
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
	// The section the HUD shows belongs to the terrain that is being replaced
	ATerrainSectionHudElement* pSectionHud = AHudManager::GetSingleton()->FindElement<ATerrainSectionHudElement>();

	if ( pSectionHud )
		pSectionHud->m_pSection = TNULL;

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
		TRenderInterface::GetSingleton()->Update( 0.01f );
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

			if ( ( pGroup->m_eFlags & ( ( ATerrainSection::FLAGS_HIGH_LOD_LOADED | ATerrainSection::FLAGS_HIGH_LOD_LOADING | ATerrainSection::FLAGS_HIGH_LOD_QUEUED ) << eLODType ) ) == 0 )
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
				if ( pVISGroup->IsLODReady( pBlock->m_eLODType ) )
				{
					pVISGroup->SetLODReady( pBlock->m_eLODType, TFALSE );
					return TNULL;
				}

				if ( pVISGroup )
				{
					if ( ( pVISGroup->m_eFlags & ( ( ATerrainSection::FLAGS_HIGH_LOD_LOADED | ATerrainSection::FLAGS_HIGH_LOD_LOADING | ATerrainSection::FLAGS_HIGH_LOD_QUEUED ) << a_eLODType ) ) != 0 )
					{
						if ( pVISGroup->IsMatLibLoaded( pBlock->m_eLODType ) )
						{
							pVISGroup->SetLODReady( pBlock->m_eLODType, TFALSE );

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

// $Barnyard: FUNCTION 005e7fa0
TBOOL ATerrainInterface::GetLocatorPosAndDir( const TCHAR* a_pszLocatorName, TVector4& a_rOutPosition, TVector4& a_rOutDirection )
{
	ATerrainLocatorList* pLocatorManager = GetLocatorList();
	ATerrainLocator*     pLocator        = pLocatorManager->FindLocator( a_pszLocatorName );

	if ( !pLocator )
		return TFALSE;

	TMatrix44 matLocator;
	pLocator->GetMatrix( matLocator );

	TMatrix44 matTransform;
	pLocatorManager->GetWorldTransform().GetLocalMatrixImp( matTransform );
	matTransform.Multiply( matLocator );

	a_rOutDirection.x = -matTransform.m_f21;
	a_rOutDirection.w = 1.0f;
	a_rOutDirection.y = -matTransform.m_f22;
	a_rOutDirection.z = -matTransform.m_f23;
	a_rOutPosition    = matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );

	return TTRUE;
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

// $Barnyard: FUNCTION 005e8cb0
TVector4& ATerrainInterface::GetLocatorPosAndAxis( const TCHAR* a_pszLocatorName, TVector4& a_rOutPosition, TVector4& a_rOutAxis, TINT a_iAxis )
{
	ATerrainLocatorList* pLocatorManager = GetLocatorList();
	ATerrainLocator*     pLocator        = pLocatorManager->FindLocator( a_pszLocatorName );

	TMatrix44 matLocator;
	pLocator->GetMatrix( matLocator );

	TMatrix44 matTransformed;
	pLocatorManager->GetWorldTransform().GetLocalMatrixImp( matTransformed );
	matTransformed.Multiply( matLocator );

	const TVector4& vecAxis = matTransformed.AsBasisVector4( BASISVECTOR( a_iAxis ) );
	a_rOutAxis.w            = 1.0f;
	a_rOutAxis.x            = -vecAxis.x;
	a_rOutAxis.y            = -vecAxis.y;
	a_rOutAxis.z            = -vecAxis.z;
	a_rOutPosition          = matTransformed.AsBasisVector4( BASISVECTOR_TRANSLATION );

	return a_rOutPosition;
}

// Names of the areas the sections belong to (ATerrainSectionHudElement)
static const struct
{
	const TCHAR* szSection;
	TINT         iLocaleId;
} s_aSectionNames[] = {
	{ "Hilltop", 256 },
	{ "Meadownorth", 257 },
	{ "Meadowsouth", 258 },
	{ "Gopher", 259 },
	{ "Pond", 260 },
	{ "Junkyard", 261 },
	{ "Golf", 262 },
	{ "Golfnorth", 262 },
	{ "Beady", 263 },
	{ "Woods", 264 },
	{ "Woodsgopher", 264 },
	{ "Woodsgolf", 264 },
	{ "Woodsbeady", 264 },
	{ "Northpathbeady", 265 },
	{ "Gulch", 266 },
	{ "Gulchbridge", 266 },
}; // 0x00778828

// $Barnyard: FUNCTION 005e8310
const TWCHAR* ATerrainInterface::GetSectionLocalisedName( const TCHAR* a_szSectionName )
{
	if ( !a_szSectionName )
		return TNULL;

	// NOTE: the original keeps the number of names (16) at 0x4cd4
	for ( TINT i = 0; i < TARRAYSIZE( s_aSectionNames ); i++ )
	{
		if ( T2String8::Compare( a_szSectionName, s_aSectionNames[ i ].szSection ) == 0 )
			return g_pLocaleManager->GetString( s_aSectionNames[ i ].iLocaleId );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005e7f60
TINT ATerrainInterface::GetSectionIndexAtPoint( const TVector4& a_rcPoint, TBOOL a_bUnused )
{
	ATerrainSection* pSection = FindSectionAtPoint( a_rcPoint, a_bUnused );

	if ( pSection )
		return GetSectionIndex( pSection );

	return -1;
}

// $Barnyard: FUNCTION 005e7e40
ATerrainSection* ATerrainInterface::FindSectionAtPoint( const TVector4& a_rcPoint, TBOOL a_bUnused )
{
	if ( !m_pTerrainVIS )
		return TNULL;

	TVector4 vecUp;
	vecUp.Multiply( TVector4::VEC_NEGY, 40.0f );

	TVector4 vecOrigin;
	vecOrigin.x = a_rcPoint.x + vecUp.x;
	vecOrigin.y = a_rcPoint.y + vecUp.y;
	vecOrigin.z = a_rcPoint.z + vecUp.z;
	vecOrigin.w = a_rcPoint.w;

	for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
	{
		ATerrainSection::ModelNode* pCollision = m_pTerrainVIS->m_pSections[ i ].m_pCollisionModelData;

		if ( pCollision && pCollision->m_pCollisionModelSet )
		{
			TFLOAT fDistance = 80.0f;

			if ( pCollision->m_oCollisionInstance.CollideRay( vecOrigin, TVector4::VEC_POSY, fDistance, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) )
				return &m_pTerrainVIS->m_pSections[ i ];
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005ebf30
void ATerrainInterface::ReloadSkeleton( const TCHAR* a_szSkelFilePath )
{
	if ( m_pTerrainVIS->m_pLocatorList )
		delete m_pTerrainVIS->m_pLocatorList;

	m_pTerrainVIS->m_pLocatorList   = TNULL;
	m_bIsLoaded                     = TFALSE;
	m_pTerrainVIS->m_szSkelFilePath = a_szSkelFilePath;
	m_pTerrainVIS->LoadSkeleton();

	while ( AAssetStreaming::GetSingleton()->HasActiveJobs() )
	{
		AAssetStreaming::GetSingleton()->Update();
		ThreadSleep( 20 );
	}

	WaitUntilLoaded();
	m_bIsLoaded = TTRUE;
}

// $Barnyard: FUNCTION 005e9370
void ATerrainInterface::CancelStreaming()
{
	AAssetStreaming::GetSingleton()->CancelAllJobs();
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

// $Barnyard: FUNCTION 005e7cb0
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

	for ( TINT i = 0; i < pLOD->iNumMeshes; i++ )
	{
		TVALIDPTR( pLOD->ppMeshes[ i ] );
		pLOD->ppMeshes[ i ]->Render();
	}
}

static void RenderNothing( TModelInstance* a_pModelInstance )
{
}

#ifdef BARNYARD_COMMUNITY_PATCH
// Not in the original: the lights that reach a cell, for the shaders of mods (the game's ignore them)
static void BeginCellLighting( CellMeshSphere* a_pMeshSphere )
{
	// The cells are exported rotated
	const TVector4& rcSphere = a_pMeshSphere->m_BoundingSphere.AsVector4();
	const TSphere   oBounds( rcSphere.x, -rcSphere.z, rcSphere.y, a_pMeshSphere->m_BoundingSphere.GetRadius() );

	TLightIDList oLightIDs;
	AGlowViewport::GetSingleton()->GetInfluencingLightIDs( oBounds, oLightIDs );

	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();
	pRenderContext->ClearLightIDs();

	for ( TINT i = 0; i < TLightIDList::MAX_NUM_LIGHTS; i++ )
	{
		if ( oLightIDs[ i ] >= 0 )
			pRenderContext->AddLight( oLightIDs[ i ] );
	}

	pRenderContext->SetLightingBounds( oBounds );
}

static void EndCellLighting()
{
	TRenderInterface::GetSingleton()->GetCurrentContext()->ClearLightIDs();
}
#endif // BARNYARD_COMMUNITY_PATCH

// $Barnyard: FUNCTION 005e7d10
static void RenderCellMeshDefault( CellMeshSphere* a_pMeshSphere, RenderData* a_pRenderData )
{
	TVALIDPTR( a_pMeshSphere );

#ifdef BARNYARD_COMMUNITY_PATCH
	BeginCellLighting( a_pMeshSphere );
	a_pMeshSphere->m_pCellMesh->pMesh->Render();
	EndCellLighting();
#else
	a_pMeshSphere->m_pCellMesh->pMesh->Render();
#endif // BARNYARD_COMMUNITY_PATCH
}

// $Barnyard: FUNCTION 005e83e0
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

#ifdef BARNYARD_COMMUNITY_PATCH
	BeginCellLighting( a_pMeshSphere );
	pMesh->Render();
	EndCellLighting();
#else
	pMesh->Render();
#endif // BARNYARD_COMMUNITY_PATCH
}

// $Barnyard: FUNCTION 005e8490
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

	pSceneObject->GetFadeDistances().x = 1.0f;
	pSceneObject->GetFadeDistances().y = 1.0f;
	pSceneObject->GetFadeParams().x    = 10000000.0f;
	pSceneObject->GetFadeParams().y    = 10000001.0f;
	pSceneObject->EnableSkeletonUpdate();
	pSceneObject->EnableOpacity();

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

				// Setup materials of all the LODs
				for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
				{
					TModelLOD& rMeshLOD = pModel->GetLOD( i );

					for ( TINT k = 0; k < rMeshLOD.iNumMeshes; k++ )
					{
						TMesh*             pMesh     = rMeshLOD.ppMeshes[ k ];
						AWorldMaterialHAL* pMaterial = TSTATICCAST( AWorldMaterialHAL, pMesh->GetMaterial() );
						const TCHAR*       pchName   = pMaterial->GetName();

						const TBOOL      bWaterRings   = TStringManager::String8CompareNoCase( "ws_water_rings", pchName, 14 ) == 0;
						const TBOOL      bWaterfall    = TStringManager::String8CompareNoCase( "ws_waterfall", pchName, 12 ) == 0;
						const TBOOL      bWaterEnv2    = TStringManager::String8CompareNoCase( "ws_water_env2", pchName, 13 ) == 0;
						const TBOOL      bLappingWater = TStringManager::String8CompareNoCase( "ws_LappingWater", pchName, 15 ) == 0;
						AWorldShaderHAL* pShaderHAL    = TSTATICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );

						if ( bWaterRings || bWaterfall )
							pMaterial->SetOrderTable( pShaderHAL->GetOrderTable( 3 ) );

						if ( bWaterRings || bWaterfall || bWaterEnv2 || bLappingWater )
						{
							pMesh->SetFlags( TMesh::State_IsWater, TTRUE );
						}
						else if ( TStringManager::String8CompareNoCase( "ws_water_env", pchName, 12 ) == 0 )
						{
							pMesh->SetFlags( TMesh::State_IsWater, TTRUE );

							// The second texture scrolls (see ATerrainSection::ModelNode::Render)
							if ( pMaterial->GetTextureNum() < 2 )
							{
								pMaterial->SetTexture( 1, pMaterial->GetTexture( 0 ) );
								pMaterial->SetTextureNum( 2 );
								pMaterial->SetHasUVOffset( 1, TTRUE );
								a_pModelNode->m_pAnimatedMaterial = pMaterial;
							}
						}
						else if ( TStringManager::String8CompareNoCase( "ws_light", pchName, 8 ) == 0 )
						{
							// The light meshes are faded in/out by UpdateNightMaterials
							if ( TStringManager::String8CompareNoCase( "cone", pchName + 8, 4 ) == 0 )
							{
								if ( TStringManager::String8CompareNoCase( "inner", pchName + 13, 5 ) == 0 )
								{
									m_apLightMagMaterials[ 6 ] = pMaterial;
									m_apLightMagMaterials[ 7 ] = pMaterial;
								}
								else
								{
									m_apLightMagMaterials[ 8 ] = pMaterial;
									m_apLightMagMaterials[ 9 ] = pMaterial;
								}
							}
							else if ( TStringManager::String8CompareNoCase( "farmhouse", pchName + 10, 9 ) == 0 )
							{
								m_apLightMagMaterials[ 0 ] = pMaterial;
							}
							else if ( TStringManager::String8CompareNoCase( "neighb", pchName + 10, 6 ) == 0 )
							{
								m_apLightMagMaterials[ 1 ] = pMaterial;
							}
							else if ( TStringManager::String8CompareNoCase( "lodge", pchName + 10, 5 ) == 0 )
							{
								m_apLightMagMaterials[ 2 ] = pMaterial;
							}
							else if ( TStringManager::String8CompareNoCase( "beady", pchName + 10, 5 ) == 0 )
							{
								m_apLightMagMaterials[ 3 ] = pMaterial;
							}
							else if ( TStringManager::String8CompareNoCase( "barn", pchName + 10, 4 ) == 0 )
							{
								m_apLightMagMaterials[ 4 ] = pMaterial;
								m_apLightMagMaterials[ 5 ] = pMaterial;
							}
						}
						else if ( TStringManager::String8CompareNoCase( "ws_nightlight", pchName, 13 ) == 0 )
						{
							m_apLightMagMaterials[ 10 ] = pMaterial;
							m_apLightMagMaterials[ 11 ] = pMaterial;
						}
					}
				}
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
	// The hours (of the day, the end can be after midnight) the light materials are on
	static constexpr TFLOAT s_aLightTimes[ NUM_LIGHT_MAGS ][ 2 ] = {
		{ 20.71f, 21.72f },
		{ 20.0f, 22.0f },
		{ 20.0f, 23.0f },
		{ 20.0f, 24.0f },
		{ 21.71f, 30.0f },
		{ -1.0f, 6.0f },
		{ 20.5f, 29.5f },
		{ -1.0f, 5.5f },
		{ 20.5f, 29.5f },
		{ -1.0f, 5.5f },
		{ 20.5f, 29.5f },
		{ -1.0f, 5.5f },
	};

	// Time it takes to fade a light in/out (in hours)
	static constexpr TFLOAT s_fLightFadeTime = 0.2f; // 0x007788a8

	if ( m_bLightOverride == TTRUE )
		return;

	AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();
	TFLOAT            fHour            = pGameTimeManager->GetCurrentMinute() * ( 1.0f / 60.0f ) + pGameTimeManager->GetCurrentHour();

	for ( TINT i = 0; i < NUM_LIGHT_MAGS; i++ )
	{
		if ( !m_apLightMagMaterials[ i ] )
			continue;

		const TFLOAT fStart = s_aLightTimes[ i ][ 0 ];
		const TFLOAT fEnd   = s_aLightTimes[ i ][ 1 ];

		TFLOAT fTime = fHour;
		if ( fTime < fStart )
			fTime += 24.0f;

		TFLOAT fMagnitude = 1.0f;

		if ( fTime < fStart || fTime > fEnd )
			fMagnitude = 0.0f;
		else if ( fTime < fStart + s_fLightFadeTime )
			fMagnitude = ( fTime - fStart ) / s_fLightFadeTime;
		else if ( fTime >= fEnd - s_fLightFadeTime )
			fMagnitude = ( fEnd - fTime ) / s_fLightFadeTime;

		m_afLightMags[ i ] = fMagnitude * 0.6f;
	}
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

// $Barnyard: FUNCTION 005e85d0
TINT ATerrainInterface::GetCollisionModelSectionIndex( ATerrainSection::ModelNode* a_pCollisionModel ) const
{
	ATerrainSection* pSection = TNULL;

	if ( m_pTerrainVIS )
	{
		for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
		{
			if ( m_pTerrainVIS->m_pSections[ i ].m_pCollisionModelData == a_pCollisionModel )
			{
				pSection = &m_pTerrainVIS->m_pSections[ i ];
				break;
			}
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// The original doesn't check this and dereferences/subtracts from a null VIS
	if ( !pSection ) return -1;
#endif // BARNYARD_COMMUNITY_PATCH

	return TINT( pSection - m_pTerrainVIS->m_pSections );
}

// $Barnyard: FUNCTION 005e98a0
TBOOL ATerrainInterface::CollideRay( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, TFLOAT& a_rfMaxDistance, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bCulling )
{
	TBOOL bCollided = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet && it->m_oCollisionInstance.CollideRay( a_rcOrigin, a_rcDirection, a_rfMaxDistance, a_eIgnoreGroups, a_bCulling ) )
			bCollided = TTRUE;
	}

	return bCollided;
}

// $Barnyard: FUNCTION 005e9930
TBOOL ATerrainInterface::CollideRay( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, TFLOAT& a_rfMaxDistance, ACollisionModelRayHitList* a_pHitList, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bCulling )
{
	TBOOL bCollided = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet && it->m_oCollisionInstance.CollideRay( a_rcOrigin, a_rcDirection, a_rfMaxDistance, a_pHitList, a_eIgnoreGroups, a_bCulling ) )
			bCollided = TTRUE;
	}

	return bCollided;
}

// $Barnyard: FUNCTION 005e9ad0
TBOOL ATerrainInterface::CollideSphere( const TVector4& a_rcSphere, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet && it->m_oCollisionInstance.CollideSphere( a_rcSphere, a_pHitList, a_iCollGroup, a_eIgnoreGroups ) )
			bCollided = TTRUE;
	}

	return bCollided;
}

// $Barnyard: FUNCTION 005e99c0
TBOOL ATerrainInterface::CollideBox( const TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet && it->m_oCollisionInstance.CollideBox( a_rcBox, a_pHitList, a_iCollGroup, a_eIgnoreGroups ) )
			bCollided = TTRUE;
	}

	return bCollided;
}

// $Barnyard: FUNCTION 005e9a40
TBOOL ATerrainInterface::CollideSweptBox( const TOBBox& a_rcPrevBox, const TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet && it->m_oCollisionInstance.CollideSweptBox( a_rcPrevBox, a_rcBox, a_pHitList, a_iCollGroup, a_eIgnoreGroups ) )
			bCollided = TTRUE;
	}

	return bCollided;
}

// $Barnyard: FUNCTION 005e9b50
TBOOL ATerrainInterface::CollideSphere( const TVector4& a_rcSphere, TFLOAT& a_rfDistance, TVector4& a_rNormal, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet )
		{
			TFLOAT fDistance = a_rfDistance;

			if ( it->m_oCollisionInstance.CollideSphere( a_rcSphere, fDistance, a_rNormal, a_iCollGroup, a_eIgnoreGroups ) )
			{
				bCollided    = TTRUE;
				a_rfDistance = TMath::Min( a_rfDistance, fDistance );
			}
		}
	}

	return bCollided;
}

// $Barnyard: FUNCTION 005e9c00
TBOOL ATerrainInterface::PushOutSphere( const TVector4& a_rcSphere, TVector4& a_rOutPosition, TINT a_iCollGroup, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bMoved = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet )
		{
			TVector4 vecPosition = a_rOutPosition;

			if ( it->m_oCollisionInstance.PushOutSphere( a_rcSphere, vecPosition, a_iCollGroup, a_eIgnoreGroups ) )
			{
				a_rOutPosition = vecPosition;
				bMoved         = TTRUE;
			}
		}
	}

	return bMoved;
}

// $Barnyard: FUNCTION 005e9d70
TBOOL ATerrainInterface::CollideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, TINT a_iUnused, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet && it->m_oCollisionInstance.CollideSweptSphere( a_rcCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, a_rfPenetration, a_eIgnoreGroups ) )
		{
			bCollided = TTRUE;

			// Can't move at all
			if ( a_rfDistance <= 0.0f )
				return TTRUE;
		}
	}

	return bCollided;
}

// $Barnyard: FUNCTION 005ea7f0
TFLOAT ATerrainInterface::GetGroundHeight( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TVector4 vecOrigin( a_fX, a_fY, a_fZ, 0.0f );
	TFLOAT   fDistance = 40.0f;
	TBOOL    bHit      = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet && it->m_oCollisionInstance.CollideRay( vecOrigin, TVector4::VEC_POSY, fDistance, a_eIgnoreGroups, TFALSE ) )
			bHit = TTRUE;
	}

	return bHit ? a_fY + fDistance : 0.0f;
}

// $Barnyard: FUNCTION 005eaf30
TFLOAT ATerrainInterface::GetGroundHeight( const TVector4& a_rcPosition, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	return GetGroundHeight( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, a_eIgnoreGroups );
}

// $Barnyard: FUNCTION 005ead80
// $Barnyard: FUNCTION 005eaf50
TFLOAT ATerrainInterface::GetGroundHeightTop( TFLOAT a_fX, TFLOAT a_fZ, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TVector4 vecOrigin( a_fX, -40.0f, a_fZ, 0.0f );
	TFLOAT   fDistance = 80.0f;

	if ( CollideRay( vecOrigin, TVector4::VEC_POSY, fDistance, a_eIgnoreGroups, TTRUE ) )
		return vecOrigin.y + fDistance;

	return 0.0f;
}

// $Barnyard: FUNCTION 005eaf60
void ATerrainInterface::SetToGroundHeightTop( TVector4& a_rPosition, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	a_rPosition.y = GetGroundHeightTop( a_rPosition.x, a_rPosition.z, a_eIgnoreGroups );
}

// $Barnyard: FUNCTION 005eaf80
void ATerrainInterface::SetToGroundHeightBottom( TVector4& a_rPosition, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	a_rPosition.y = GetGroundHeightBottom( a_rPosition.x, a_rPosition.z, a_eIgnoreGroups );
}

// $Barnyard: FUNCTION 005eaea0
TFLOAT ATerrainInterface::GetRayHitDistance( TVector4 a_vOrigin, TVector4 a_vDirection, TFLOAT a_fMaxDistance, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bCulling )
{
	TFLOAT fDistance = a_fMaxDistance;

	if ( CollideRay( a_vOrigin, a_vDirection, fDistance, a_eIgnoreGroups, a_bCulling ) )
		return fDistance;

	return a_fMaxDistance;
}

// $Barnyard: FUNCTION 005eae10
TFLOAT ATerrainInterface::GetGroundHeightBottom( TFLOAT a_fX, TFLOAT a_fZ, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TVector4 vecOrigin( a_fX, 40.0f, a_fZ, 0.0f );
	TFLOAT   fDistance = 80.0f;

	if ( CollideRay( vecOrigin, TVector4::VEC_NEGY, fDistance, a_eIgnoreGroups, TFALSE ) )
		return vecOrigin.y - fDistance;

	return 0.0f;
}

// $Barnyard: FUNCTION 005eb440
TBOOL ATerrainInterface::GetGroundShadowInfo( const TVector4& a_rcPosition, TFLOAT a_fHeightOffset, TVector4& a_rOutPosition, TINT& a_rOutAlpha, TVector3& a_rOutRotation, TVector4& a_rOutNormal, TFLOAT a_fRadius, TFLOAT a_fMaxDistance, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TVector4 vecStart( a_rcPosition.x, a_rcPosition.y - 0.2f, a_rcPosition.z, a_rcPosition.w );
	TVector4 vecDirection = TVector4::VEC_POSY;
	TFLOAT   fDistance    = a_fMaxDistance;

	if ( !CollideRay( vecStart, vecDirection, fDistance, a_eIgnoreGroups, TFALSE ) )
		fDistance = a_fMaxDistance;

	if ( fDistance >= a_fMaxDistance )
		return TFALSE;

	TVector4 vecGround;
	vecGround.Multiply( TVector4::VEC_POSY, fDistance );

	a_rOutPosition.x = vecGround.x + vecStart.x;
	a_rOutPosition.y = vecGround.y + vecStart.y + a_fHeightOffset;
	a_rOutPosition.z = vecGround.z + vecStart.z;
	a_rOutPosition.w = vecStart.w;

	// NOTE: retail rounds with fistp
	TINT iAlpha = TINT( lrintf( ( 1.0f - fDistance / a_fMaxDistance ) * 255.0f ) );
	TMath::Clip( iAlpha, 0, 255 );
	a_rOutAlpha = iAlpha;

	// The triangles around the ground position
	ACollisionModelRayHitList oHitList;
	TVector4                  vecSphere( vecStart.x, fDistance + vecStart.y, vecStart.z, a_fRadius );
	ATerrainInterface::GetSingleton()->CollideSphere( vecSphere, &oHitList, -1, a_eIgnoreGroups );

	TVector4 vecNormal = TVector4::VEC_ZERO;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		TVector4 vecHitNormal = oHitList.aHits[ i ].GetNormal();

		// Only the ones facing up
		if ( TVector4::DotProduct3( vecHitNormal, TVector4::VEC_NEGY ) > 0.4f )
		{
			vecNormal.x += vecHitNormal.x;
			vecNormal.y += vecHitNormal.y;
			vecNormal.z += vecHitNormal.z;
		}
	}

	if ( oHitList.iNumHits > 0 )
	{
		vecNormal.Divide( TFLOAT( oHitList.iNumHits ) );
		vecNormal.Normalise();

		TVector4 vecUp = vecNormal;
		vecUp.w        = 0.0f;

		TVector4 vecRef = TVector4::VEC_NEGY;

		if ( TMath::Abs( TVector4::DotProduct3( vecUp, vecRef ) ) > FLT_MIN )
			vecRef = TVector4::VEC_POSZ;

		TVector4 vecSide;
		vecSide.CrossProduct( vecUp, vecRef );
		vecSide.Normalise();

		vecRef.CrossProduct( vecUp, vecSide );
		vecRef.Normalise();

		TMatrix44 oBasis;
		oBasis.AsBasisVector4( BASISVECTOR_RIGHT ).Set( vecSide.x, vecSide.y, vecSide.z, 0.0f );
		oBasis.AsBasisVector4( BASISVECTOR_UP ).Set( vecRef.x, vecRef.y, vecRef.z, 0.0f );
		oBasis.AsBasisVector4( BASISVECTOR_FORWARD ).Set( vecUp.x, vecUp.y, vecUp.z, 0.0f );

		TVector3 vecRotation;
		oBasis.GetEulerXYZ( vecRotation );

		a_rOutRotation = vecRotation;
		a_rOutNormal   = vecNormal;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005e7df0
TINT ATerrainInterface::GetSectionIndex( const ATerrainSection* a_pSection ) const
{
	return TINT( a_pSection - m_pTerrainVIS->m_pSections );
}

// $Barnyard: FUNCTION 005e7d80
ATerrainSection* ATerrainInterface::FindSection( const TCHAR* a_szName ) const
{
	if ( !m_pTerrainVIS )
		return TNULL;

	for ( TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++ )
	{
		if ( TStringManager::String8CompareNoCase( a_szName, m_pTerrainVIS->m_pSections[ i ].GetName() ) == 0 )
			return &m_pTerrainVIS->m_pSections[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005e8630
TINT ATerrainInterface::GetSectionIndex( const TCHAR* a_szName ) const
{
	ATerrainSection* pSection = FindSection( a_szName );

	if ( pSection )
		return GetSectionIndex( pSection );

	return -1;
}

// $Barnyard: FUNCTION 005ec3a0
void ATerrainInterface::ForceSection( TINT a_iSection, TBOOL a_bWait )
{
	ms_fnUnforcedGetCurrentVISGroup = m_fnGetCurrentVISGroup;
	ms_iForcedSection               = a_iSection;
	m_fnGetCurrentVISGroup          = GetForcedSection;

	if ( a_bWait == TTRUE )
	{
		TRenderInterface::GetSingleton()->BeginEndSceneHAL();
		WaitUntilLoaded();
	}
}

// $Barnyard: FUNCTION 005e80a0
TINT ATerrainInterface::GetForcedSection()
{
	return ms_iForcedSection;
}

// $Barnyard: FUNCTION 005eb940
TBOOL ATerrainInterface::FindGroundPlacement( const TVector4& a_rcPosition, TFLOAT a_fHeightOffset, TVector4& a_rOutPosition, TINT& a_rOutAlpha, TVector3& a_rOutRotation, TVector4& a_rOutNormal, TFLOAT a_fRadius, TFLOAT a_fMaxDistance )
{
	TVector4 vecOrigin = a_rcPosition;
	vecOrigin.y -= 0.2f;

	TVector4 vecDirection = TVector4::VEC_POSY;
	TFLOAT   fDistance    = a_fMaxDistance;

	if ( !CollideRay( vecOrigin, vecDirection, fDistance, 0, TFALSE ) )
		fDistance = a_fMaxDistance;

	if ( fDistance >= a_fMaxDistance )
		return TFALSE;

	TVector4 vecGround;
	vecGround.Multiply( TVector4::VEC_POSY, fDistance );

	a_rOutPosition.x = vecGround.x + vecOrigin.x;
	a_rOutPosition.y = vecGround.y + vecOrigin.y + a_fHeightOffset;
	a_rOutPosition.z = vecGround.z + vecOrigin.z;
	a_rOutPosition.w = vecOrigin.w;

	TINT iAlpha = TINT( ( 1.0f - fDistance / a_fMaxDistance ) * 255.0f );
	TMath::Clip( iAlpha, 0, 255 );
	a_rOutAlpha = iAlpha;

	// Average the normals of the triangles around the ground point
	TVector4 vecSphere( vecOrigin.x, fDistance + vecOrigin.y, vecOrigin.z, a_fRadius );

	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideSphere( vecSphere, &oHitList, -1, 0 );

	TVector4 vecNormal  = TVector4::VEC_ZERO;
	TFLOAT   fLastSlope = -100.0f;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		TVector4 vecHitNormal = oHitList.aHits[ i ].GetNormal();
		TFLOAT   fSlope       = TVector4::DotProduct3( vecHitNormal, TVector4::VEC_NEGY );

		if ( fSlope <= 0.4f )
			continue;

		if ( fLastSlope != -100.0f )
		{
			if ( fSlope < fLastSlope - 0.2f || fSlope > fLastSlope + 0.2f )
			{
				// The ground is uneven, just use the up vector
				vecNormal = TVector4::VEC_NEGY;
				break;
			}

			fSlope = ( fSlope + fLastSlope ) * 0.5f;
		}

		fLastSlope = fSlope;
		vecNormal.x += vecHitNormal.x;
		vecNormal.y += vecHitNormal.y;
		vecNormal.z += vecHitNormal.z;
	}

	if ( oHitList.iNumHits > 0 )
	{
		vecNormal.Divide( TFLOAT( oHitList.iNumHits ) );
		vecNormal.Normalise();

		TVector4 vecUp        = vecNormal;
		TVector4 vecReference = TVector4::VEC_NEGY;

		if ( TMath::Abs( TVector4::DotProduct3( vecUp, vecReference ) ) > TMath::MINFLOAT )
			vecReference = TVector4::VEC_POSZ;

		TVector4 vecRight;
		vecRight.CrossProduct( vecUp, vecReference );
		vecRight.Normalise();

		vecReference.CrossProduct( vecUp, vecRight );
		vecReference.Normalise();

		TMatrix44 matRotation;
		matRotation.AsBasisVector4( BASISVECTOR_RIGHT ).Set( vecRight.x, vecRight.y, vecRight.z, 0.0f );
		matRotation.AsBasisVector4( BASISVECTOR_UP ).Set( vecReference.x, vecReference.y, vecReference.z, 0.0f );
		matRotation.AsBasisVector4( BASISVECTOR_FORWARD ).Set( vecUp.x, vecUp.y, vecUp.z, 0.0f );
		matRotation.GetEulerXYZ( a_rOutRotation );

		a_rOutNormal = vecNormal;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005e9cc0
TBOOL ATerrainInterface::CollideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bCollided = TFALSE;

	T2_FOREACH( m_llModelNodes, it )
	{
		if ( it->m_pCollisionModelSet && it->m_oCollisionInstance.CollideSweptSphere( a_rcCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, a_rfPenetration, a_eIgnoreGroups ) )
		{
			bCollided = TTRUE;

			// Can't move at all
			if ( a_rfDistance <= 0.0f )
				return TTRUE;
		}
	}

	return bCollided;
}

// $Barnyard: FUNCTION 005e9e20
TBOOL ATerrainInterface::SlideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, TVector4& a_rOutPosition, TVector4& a_rOutDirection, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope )
{
	// Move the sphere out of the triangles first
	TVector4 vecStart = a_rcCenter;
	TVector4 vecSphere( a_rcCenter.x, a_rcCenter.y, a_rcCenter.z, a_fRadius );
	PushOutSphere( vecSphere, vecStart, -1, TCollisionCommon::TOSHICGROUPFLAG_NONE );

	TFLOAT fTotalDistance = a_rfDistance;
	a_rfDistance          = a_rfDistance + 0.1f;

	TVector4 vecNormal;
	TFLOAT   fPenetration = 0.0f;
	TBOOL    bHit         = CollideSweptSphere( vecStart, a_fRadius, a_rcDirection, a_rfDistance, vecNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );

	if ( bHit == TTRUE && a_rfDistance > 0.0f && a_rfDistance - 0.1f < 0.0f )
	{
		a_rfDistance = 0.0f;
	}
	else
	{
		a_rfDistance = a_rfDistance - 0.1f;

		if ( !bHit )
		{
			TVector4 vecOffset;
			vecOffset.Multiply( a_rcDirection, a_rfDistance );

			a_rOutPosition.x = vecOffset.x + vecStart.x;
			a_rOutPosition.y = vecOffset.y + vecStart.y;
			a_rOutPosition.z = vecOffset.z + vecStart.z;
			a_rOutPosition.w = vecStart.w;
			return TFALSE;
		}

		if ( a_rfDistance < 0.0f )
		{
			// Inside of something, try to get out backwards
			TVector4 vecBackDirection( -a_rcDirection.x, -a_rcDirection.y, -a_rcDirection.z, a_rcDirection.w );
			TFLOAT   fBackDistance = -a_rfDistance;
			TVector4 vecBackNormal;

			if ( CollideSweptSphere( vecStart, a_fRadius, vecBackDirection, fBackDistance, vecBackNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE ) )
			{
				TVector4 vecOffset;
				vecOffset.Multiply( vecBackDirection, fBackDistance );
				a_rOutPosition.Add( vecStart, vecOffset );
				return TTRUE;
			}
		}
	}

	TVector4 vecPosition      = vecStart;
	TVector4 vecMoveDirection = a_rcDirection;
	TFLOAT   fRemaining       = a_rfDistance;

	TFLOAT fPushBack = TVector4::DotProduct3( vecNormal, vecMoveDirection ) * 0.0001f;
	fPushBack        = ( fPushBack < 0.0f ) ? fPushBack : 0.0f;

	if ( fRemaining < 0.0f )
	{
		fPushBack          = 0.0f;
		fRemaining         = -fRemaining;
		vecMoveDirection.x = -vecMoveDirection.x;
		vecMoveDirection.y = -vecMoveDirection.y;
		vecMoveDirection.z = -vecMoveDirection.z;
		fTotalDistance     = fRemaining + fTotalDistance;
	}

	TFLOAT fDistance = fRemaining;
	TFLOAT fMove     = fPushBack + fRemaining;

	if ( !( 0.0f < fMove ) )
		fMove = 0.0f;

	// Move to the hit
	TVector4 vecOffset;
	vecOffset.Multiply( vecMoveDirection, fMove );

	TVector4 vecTarget( vecOffset.x + vecPosition.x, vecOffset.y + vecPosition.y, vecOffset.z + vecPosition.z, vecPosition.w );
	TVector4 vecPrevDirection = vecMoveDirection;

	TMath::SlideAlongPlane( a_rcDirection, vecNormal, a_fSpeedScale, fTotalDistance, fDistance + fPushBack, fRemaining, vecMoveDirection, 0.1f, 0.004f, 1.0f );
	a_rOutDirection = vecMoveDirection;

	if ( a_rcDirection.y > 0.9f )
		ScaleSpeedBySlopeSoft( fRemaining, vecMoveDirection, a_fMinSlope, a_fMaxSlope );

	fTotalDistance   = fRemaining;
	vecPrevDirection = vecMoveDirection;
	fDistance        = fRemaining;

	TINT iNumIterations = 0;

	while ( bHit )
	{
		if ( fDistance <= 0.001f || fRemaining <= 0.001f || TVector4::DotProduct3( vecPrevDirection, vecMoveDirection ) <= -0.001f )
			break;

		if ( iNumIterations >= 15 )
			break;

		bHit = CollideSweptSphere( vecTarget, a_fRadius, vecMoveDirection, fDistance, vecNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );

		fPushBack = TVector4::DotProduct3( vecNormal, vecMoveDirection ) * 0.0001f;
		fPushBack = ( fPushBack < 0.0f ) ? fPushBack : 0.0f;

		if ( bHit )
		{
			fMove = fDistance + fPushBack;

			if ( fMove <= 0.0f )
			{
				vecOffset.Multiply( vecMoveDirection, fDistance );
				vecTarget.x = vecOffset.x + vecTarget.x;
				vecTarget.y = vecOffset.y + vecTarget.y;
				vecTarget.z = vecOffset.z + vecTarget.z;
			}
			else
			{
				vecOffset.Multiply( vecMoveDirection, fMove );
				vecTarget.x = vecOffset.x + vecTarget.x;
				vecTarget.y = vecOffset.y + vecTarget.y;
				vecTarget.z = vecOffset.z + vecTarget.z;

				vecPrevDirection = vecMoveDirection;

				TMath::SlideAlongPlane( a_rcDirection, vecNormal, a_fSpeedScale, fTotalDistance, fDistance + fPushBack, fRemaining, vecMoveDirection, 0.1f, 0.004f, 1.0f );
				a_rOutDirection = vecMoveDirection;

				if ( a_rcDirection.y > 0.5f )
					ScaleSpeedBySlopeSoft( fRemaining, vecMoveDirection, a_fMinSlope, a_fMaxSlope );

				fTotalDistance = fRemaining;
				fDistance      = fPushBack + fRemaining;
			}
		}
		else
		{
			vecOffset.Multiply( vecMoveDirection, fDistance );
			vecTarget.x = vecOffset.x + vecTarget.x;
			vecTarget.y = vecOffset.y + vecTarget.y;
			vecTarget.z = vecOffset.z + vecTarget.z;
		}

		iNumIterations++;
	}

	// Too many iterations, stay at the start
	a_rOutPosition = ( iNumIterations == 15 ) ? vecStart : vecTarget;
	return TTRUE;
}

// $Barnyard: FUNCTION 005eaee0
TBOOL ATerrainInterface::SlideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, TVector4& a_rOutPosition, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope )
{
	TVector4 vecDirection;
	return SlideSweptSphere( a_rcCenter, a_fRadius, a_rcDirection, a_rfDistance, a_fMaxStepHeight, a_fSpeedScale, a_rfStepHeight, a_rOutPosition, vecDirection, a_fMinSlope, a_fMaxSlope );
}

// $Barnyard: FUNCTION 005e8ae0
void ATerrainInterface::ScaleSpeedBySlopeSoft( TFLOAT& a_rfSpeed, const TVector4& a_rcNormal, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope )
{
	if ( a_rcNormal.y > a_fMinSlope )
	{
		if ( a_rcNormal.y > a_fMaxSlope )
		{
			TFLOAT fScale = TMath::Pow( a_rcNormal.y - a_fMaxSlope, 0.25f ) * ( 1.0f / ( 1.0f - a_fMaxSlope ) );
			a_rfSpeed     = a_rfSpeed * TMath::Min( fScale, 1.0f );
		}
		else
		{
			a_rfSpeed = 0.0f;
		}
	}
}
