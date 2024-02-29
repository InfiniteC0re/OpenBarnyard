#include "pch.h"
#include "ATerrain.h"
#include "Tasks/ARootTask.h"
#include "Assets/AAssetStreaming.h"
#include "ALoadScreen.h"

#include <Toshi/T2FixedString.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ATerrain::ATerrain(TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup)
{
	m_bIsLoaded = TTRUE;
	m_pTerrainVIS = TNULL;
	m_iNumChunks = 16;
	m_iPreloadTerrainBlockSize = a_iPreloadTerrainBlockSize;

	m_pJobs = new JobSlot[NUM_TOTAL_JOBS];
	auto pJobSlot = &m_pJobs[0];

	for (TUINT i = 0; i < MAX_NUM_MODEL_LOADER_JOBS; i++)
	{
		pJobSlot->pJob = &m_aModelLoaderJobs[i];
		m_FreeModelLoaderJobs.PushFront(pJobSlot);
		pJobSlot++;
	}

	for (TUINT i = 0; i < MAX_NUM_KEYLIB_LOADER_JOBS; i++)
	{
		pJobSlot->pJob = &m_aKeylibLoaderJobs[i];
		m_FreeKeylibLoaderJobs.PushFront(pJobSlot);
		pJobSlot++;
	}

	for (TUINT i = 0; i < MAX_NUM_MATLIB_LOADER_JOBS; i++)
	{
		pJobSlot->pJob = &m_aMatlibLoaderJobs[i];
		m_FreeMatlibLoaderJobs.PushFront(pJobSlot);
		pJobSlot++;
	}

	for (TUINT i = 0; i < MAX_NUM_SECTION_LOADER_JOBS; i++)
	{
		pJobSlot->pJob = &m_aSectionDoneJobs[i];
		m_FreeSectionLoaderJobs.PushFront(pJobSlot);
		pJobSlot++;
	}

	for (TUINT i = 0; i < MAX_NUM_TRB_LOADER_JOBS; i++)
	{
		pJobSlot->pJob = &m_aTRBLoaderJobs[i];
		m_FreeTRBLoaderJobs.PushFront(pJobSlot);
		pJobSlot++;
	}

	for (TUINT i = 0; i < MAX_NUM_SKELETON_LOADER_JOBS; i++)
	{
		pJobSlot->pJob = &m_aSkeletonDoneJobs[i];
		m_FreeSkeletonLoaderJobs.PushFront(pJobSlot);
		pJobSlot++;
	}

	for (TUINT i = 0; i < MAX_NUM_COLLISION_LOADER_JOBS; i++)
	{
		pJobSlot->pJob = &m_aCollisionDoneJobs[i];
		m_FreeCollisionLoaderJobs.PushFront(pJobSlot);
		pJobSlot++;
	}

	TTODO("Call FUN_00619040() and initialise some other values");
	m_iCurrentSection = a_iStartVISGroup;
	m_iPreviousSection = -1;
	m_cbOnVISGroupChanged = TNULL;
	m_fnGetCurrentVISGroup = GetCurrentSectionID;
}

ATerrain::~ATerrain()
{
	TIMPLEMENT();

	m_bIsLoaded = TFALSE;
	delete[] m_pJobs;
}

void ATerrain::Update()
{
	if (!m_pTerrainVIS) return;

	if (m_fnGetCurrentVISGroup)
	{
		TINT iCurrentGroup = m_fnGetCurrentVISGroup();

		if (iCurrentGroup != -1)
		{
			m_iCurrentSection = iCurrentGroup;
		}
	}

	if (m_iCurrentSection >= 0)
	{
		if (m_iPreviousSection != m_iCurrentSection)
		{
			UpdateUsedBlocks(ATerrainLODType_High);
			UpdateUsedBlocks(ATerrainLODType_Low);

			UnqueueStreamingAssets();
			CancelUnrequiredJobs();

			if (m_cbOnVISGroupChanged)
			{
				auto pPrevious = m_iPreviousSection >= 0 ? &m_pTerrainVIS->m_pSections[m_iPreviousSection] : TNULL;
				auto pCurrent = m_iCurrentSection >= 0 ? &m_pTerrainVIS->m_pSections[m_iCurrentSection] : TNULL;

				m_cbOnVISGroupChanged(pPrevious, pCurrent);
			}

			m_iPreviousSection = m_iCurrentSection;
			QueueStreamingAssets();
		}

		if (m_bIsLoaded && !IsLoaded())
		{
			TFIXME("Create and push ATerrainLoadingStallState if needed");
		}

		TBOOL bSkipLoadingModels = TFALSE;

		for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
		{
			if (m_pTerrainVIS->m_pSections[i].IsLODLoading(ATerrainLODType_High) ||
				m_pTerrainVIS->m_pSections[i].IsLODLoading(ATerrainLODType_Low))
			{
				// There are already some models loading, don't create any new jobs
				bSkipLoadingModels = TTRUE;
				break;
			}
		}

		if (!bSkipLoadingModels)
		{
			if (HasAnyLODsQueued())
			{
				auto pGroups = m_pTerrainVIS->m_pSections;

				if (m_pOrderDVIS)
				{
					// Use precomputed order of queued groups to load
					auto pDVIS = &m_pOrderDVIS[m_iCurrentSection];
					
					for (TINT i = 0; i < pDVIS->iCount; i++)
					{
						auto uiGroupIndex = pDVIS->pInfo[i].uiGroupIndex;
						auto eLODType = pDVIS->pInfo[i].eLODType;

						if (pGroups[uiGroupIndex].IsLODQueued(eLODType))
						{
							pGroups[uiGroupIndex].LoadModels(eLODType);
							break;
						}
					}
				}
				else
				{
					// Brute force order of queued groups to load
					if (pGroups[m_iCurrentSection].IsLODQueued(ATerrainLODType_High))
					{
						pGroups[m_iCurrentSection].LoadModels(ATerrainLODType_High);
					}
					else
					{
						for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
						{
							if (pGroups[i].IsLODQueued(ATerrainLODType_High))
							{
								pGroups[i].LoadModels(ATerrainLODType_High);
								break;
							}
							else if (pGroups[i].IsLODQueued(ATerrainLODType_Low))
							{
								pGroups[i].LoadModels(ATerrainLODType_Low);
								break;
							}
						}
					}
				}
			}
		}

		TFIXME("Restore value of m_fnGetCurrentVISGroup if it was saved");

		// Wait till queued LODs are ready
		for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
		{
			if (m_pTerrainVIS->m_pSections[i].IsLODQueued(ATerrainLODType_High) ||
				m_pTerrainVIS->m_pSections[i].IsLODQueued(ATerrainLODType_Low))
			{
				FlushJobs();
				return;
			}
		}

		// Wait till all LODs are loaded
		for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
		{
			if (m_pTerrainVIS->m_pSections[i].IsLODLoading(ATerrainLODType_High) ||
				m_pTerrainVIS->m_pSections[i].IsLODLoading(ATerrainLODType_Low))
			{
				FlushJobs();
				return;
			}
		}

		if (m_pOrderDVIS)
		{
			auto pDVIS = &m_pOrderDVIS[m_iCurrentSection];

			for (TINT i = 0; i < pDVIS->iCount; i++)
			{
				auto pInfo = &pDVIS->pInfo[i];
				auto pGroup = &m_pTerrainVIS->m_pSections[pInfo->uiGroupIndex];

				if (pGroup->IsLODLoaded(pInfo->eLODType) && !pGroup->IsLODEmpty(pInfo->eLODType))
				{
					TRenderInterface::GetSingleton()->BeginEndSceneHAL();
					pGroup->DestroyLOD(pInfo->eLODType);
					QueueStreamingAssets();
				}
			}
		}
	}

	FlushJobs();
}

TBOOL ATerrain::IsLoaded() const
{
	if (m_pTerrainVIS == TNULL)
	{
		return TTRUE;
	}

	if (m_pTerrainVIS->GetLocatorManager() && m_pTerrainVIS->GetLocatorManager()->GetTRB())
	{
		for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
		{
			if (HASFLAG(m_pTerrainVIS->m_pSections[i].m_eFlags & 0xF3C))
			{
				return TFALSE;
			}
		}

		return TTRUE;
	}

	return TFALSE;
}

void ATerrain::LoadFromFile(const TCHAR* a_szFilePath, TBOOL a_bLoadLater, TBOOL a_bStreamCollision)
{
	TFIXME("Do something with ATerrainSectionHudElement");

	m_bIsLoaded = TFALSE;

	m_VISTRB.Close();
	m_VISTRB.Load(a_szFilePath, 0);

	m_pTerrainVIS = m_VISTRB.CastSymbol<ATerrainVIS>("terrainvis");
	m_pOrderDVIS = m_VISTRB.CastSymbol<ATerrainOrderDVIS>("orderdvis");

	if (m_VISTRB.GetSymbolAddress("preloadcollision"))
	{
		// We won't stream collision models if there's any preload collision
		a_bStreamCollision = TFALSE;
		m_iPreloadTerrainBlockSize = m_pTerrainVIS->m_uiPersistantTerrainBlockSize;
	}

	m_VISTRB.DeleteSymbolTable();

	if (!a_bStreamCollision)
	{
		m_pTerrainVIS->m_uiPersistantTerrainBlockSize = m_iPreloadTerrainBlockSize;
	}

	if (m_pTerrainVIS->m_uiPersistantTerrainBlockSize > 0)
	{
		m_pTerrainVIS->m_pPersistantTerrainBlock = new ATerrainLODBlock(
			m_pTerrainVIS->m_uiPersistantTerrainBlockSize,
			"Terrain_Persistant"
		);
	}

	// Create HighLOD blocks
	if (m_pTerrainVIS->m_iNumHighBlocks > 0)
	{
		m_pTerrainVIS->m_ppHighBlocks = new ATerrainLODBlock*[m_pTerrainVIS->m_iNumHighBlocks];

		for (TINT i = 0; i < m_pTerrainVIS->m_iNumHighBlocks; i++)
		{
			TCHAR szBlockName[64];
			TStringManager::String8Format(szBlockName, sizeof(szBlockName), "Terrain_HighLOD_Block%d", i);

			m_pTerrainVIS->m_ppHighBlocks[i] = new ATerrainLODBlock(
				m_pTerrainVIS->m_uiHighBlockSize,
				szBlockName
			);
		}
	}

	// Create LowLOD blocks
	if (m_pTerrainVIS->m_iNumLowBlocks > 0)
	{
		m_pTerrainVIS->m_ppLowBlocks = new ATerrainLODBlock*[m_pTerrainVIS->m_iNumLowBlocks];

		for (TINT i = 0; i < m_pTerrainVIS->m_iNumLowBlocks; i++)
		{
			TCHAR szBlockName[64];
			TStringManager::String8Format(szBlockName, sizeof(szBlockName), "Terrain_LowLOD_Block%d", i);

			m_pTerrainVIS->m_ppLowBlocks[i] = new ATerrainLODBlock(
				m_pTerrainVIS->m_uiLowBlockSize,
				szBlockName
			);
		}
	}

	m_pTerrainVIS->LoadSkeleton();
	
	// Load global matlib if it is specified
	if (*m_pTerrainVIS->m_szMatLibrary != '\0')
	{
		auto pMatlibJob = GetFreeMatlibLoaderJob();
		auto pPersitantBlock = m_pTerrainVIS->m_pPersistantTerrainBlock;

		auto pTRB = new (pPersitantBlock->GetMemBlock()) TTRB();
		m_pTerrainVIS->m_pMaterialLibraryTRB = pTRB;
		pPersitantBlock->SetupTRB(pTRB, pPersitantBlock);

		pMatlibJob->InitJob(
			m_pTerrainVIS->m_szMatLibrary,
			m_pTerrainVIS->m_pMaterialLibraryTRB,
			m_pTerrainVIS->m_pMaterialLibrary,
			pPersitantBlock->GetMemBlock()
		);

		AAssetStreaming::GetSingleton()->AddMainThreadJob(pMatlibJob);
	}

	m_bStreamCollision = a_bStreamCollision;

	// If collision is not streamed, create jobs to load it for every section
	if (!a_bStreamCollision)
	{
		for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
		{
			m_pTerrainVIS->m_pSections[i].LoadCollision();
		}
	}

	// Wait until all assets are loaded
	if (!a_bLoadLater)
	{
		while (AAssetStreaming::GetSingleton()->HasActiveJobs())
		{
			AAssetStreaming::GetSingleton()->Update();
			Sleep(20);
		}

		WaitUntilLoaded();
	}

	m_bIsLoaded = TTRUE;
}

void ATerrain::WaitUntilLoaded()
{
	while (TTRUE)
	{
		Update();

		if (IsLoaded()) return;
		TRenderInterface::GetSingleton()->Update(1.0f / 1000.0f);
		AAssetStreaming::GetSingleton()->Update();
		Sleep(20);
	}
}

void ATerrain::DestroyModelData(ATerrainSection::ModelNode* a_pModelData)
{
	TVALIDPTR(a_pModelData);

	if (a_pModelData->IsLinked())
	{
		m_ModelDatas.Remove(a_pModelData, a_pModelData);
	}

	if (a_pModelData)
	{
		delete a_pModelData;
	}
}

void ATerrain::UpdateUsedBlocks(ATerrainLODType a_eLODType)
{
	TINT iNumBlocks;
	ATerrainLODBlock** ppBlocks;

	if (a_eLODType == ATerrainLODType_High)
	{
		iNumBlocks = m_pTerrainVIS->m_iNumHighBlocks;
		ppBlocks = m_pTerrainVIS->m_ppHighBlocks;
	}
	else
	{
		iNumBlocks = m_pTerrainVIS->m_iNumLowBlocks;
		ppBlocks = m_pTerrainVIS->m_ppLowBlocks;
	}

	auto pCurrentVISGroup = m_pTerrainVIS->m_pSections + m_iCurrentSection;

	for (TINT i = 0; i < iNumBlocks; i++)
	{
		auto pBlock = ppBlocks[i];
		pBlock->SetUnused();

		if (a_eLODType == ATerrainLODType_High && pBlock->m_pVISGroup == pCurrentVISGroup)
		{
			pBlock->SetUsed();
		}
		else
		{
			auto pVIS = m_pTerrainVIS;

			for (TINT k = 0; k < pVIS->m_iNumSections; k++)
			{
				if (pCurrentVISGroup->m_pVisibility[k] == a_eLODType &&
					pBlock->m_pVISGroup == &pVIS->m_pSections[k])
				{
					pBlock->SetUsed();
				}
			}
		}
	}
}

void ATerrain::QueueStreamingAssets()
{
	TASSERT(m_iCurrentSection != -1);

	auto pCurrentGroup = &m_pTerrainVIS->m_pSections[m_iCurrentSection];

	for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
	{
		auto eLODType = pCurrentGroup->m_pVisibility[i];

		if (eLODType > ATerrainLODType_None)
		{
			auto pGroup = &m_pTerrainVIS->m_pSections[i];

			if ((pGroup->m_eFlags & (21 << eLODType)) == 0)
			{
				pGroup->SetLODQueued(eLODType, TTRUE);
			}
		}
	}

	if (!HASFLAG(pCurrentGroup->m_eFlags &
		(ATerrainSection::FLAGS_HIGH_LOD_LOADED | ATerrainSection::FLAGS_HIGH_LOD_LOADING)))
	{
		pCurrentGroup->SetLODQueued(ATerrainLODType_High, TTRUE);
	}
}

void ATerrain::UnqueueStreamingAssets()
{
	for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
	{
		m_pTerrainVIS->m_pSections[i].RemoveFromStreamingQueue();
	}
}

TBOOL ATerrain::HasAnyLODsQueued()
{
	for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
	{
		auto pGroup = &m_pTerrainVIS->m_pSections[i];

		if (pGroup->m_eFlags & (ATerrainSection::FLAGS_HIGH_LOD_QUEUED | ATerrainSection::FLAGS_LOW_LOD_QUEUED))
		{
			return TTRUE;
		}
	}

	return TFALSE;
}

ATerrainLODBlock* ATerrain::AllocateLODBlock(ATerrainLODType a_eLODType, ATerrainSection* a_pVISGroup)
{
	TINT iNumBlocks;
	ATerrainLODBlock** ppBlocks;
	
	if (a_eLODType == ATerrainLODType_High)
	{
		iNumBlocks = m_pTerrainVIS->m_iNumHighBlocks;
		ppBlocks = m_pTerrainVIS->m_ppHighBlocks;
	}
	else
	{
		iNumBlocks = m_pTerrainVIS->m_iNumLowBlocks;
		ppBlocks = m_pTerrainVIS->m_ppLowBlocks;
	}
	
	TFLOAT fMinAccessTime = TMath::MAXFLOAT;
	TINT iPrevFoundIndex = -1;
	TINT iFoundIndex;

	for (TINT i = 0; i < iNumBlocks; i++)
	{
		auto pBlock = ppBlocks[i];
		iFoundIndex = iPrevFoundIndex;

		if (!pBlock->IsUsed())
		{
			iFoundIndex = i;
			if (pBlock->m_pVISGroup == TNULL) break;

			iFoundIndex = iPrevFoundIndex;
			if (!pBlock->m_pVISGroup->IsLODLoading(a_eLODType))
			{
				iFoundIndex = i;
				if (pBlock->m_pVISGroup = TNULL) break;

				iFoundIndex = iPrevFoundIndex;
				if (pBlock->m_fLastAccessTime < fMinAccessTime)
				{
					fMinAccessTime = pBlock->m_fLastAccessTime;
					iFoundIndex = i;
				}
			}
		}

		iPrevFoundIndex = iFoundIndex;
	}

	if (iFoundIndex >= 0)
	{
		auto pBlock = ppBlocks[iFoundIndex];
		auto pVISGroup = pBlock->m_pVISGroup;

		if (pVISGroup)
		{
			if (pVISGroup->IsLODEmpty(pBlock->m_eLODType))
			{
				pVISGroup->SetLODEmpty(pBlock->m_eLODType, TFALSE);
				return TNULL;
			}

			if (pVISGroup)
			{
				if ((pVISGroup->m_eFlags & (21 << a_eLODType)) != 0)
				{
					if (pVISGroup->IsMatLibLoaded(pBlock->m_eLODType))
					{
						pVISGroup->SetLODEmpty(pBlock->m_eLODType, TFALSE);

						if (pBlock->m_pVISGroup)
						{
							pBlock->m_pVISGroup->UnloadMatlib(pBlock->m_eLODType);
						}
					}
					else if (pBlock->m_pVISGroup)
					{
						pBlock->m_pVISGroup->DestroyLOD(pBlock->m_eLODType);
					}

					return TNULL;
				}

				if (pVISGroup)
				{
					pVISGroup->DestroyLOD(pBlock->m_eLODType);
				}
			}
		}

		pBlock->Assign(a_pVISGroup, a_eLODType);
		return pBlock;
	}

	return TNULL;
}

ATRBLoaderJob* ATerrain::GetFreeTRBLoaderJob()
{
	TASSERT(m_FreeTRBLoaderJobs.Size() > 0, "No free ATRBLoaderJobs left!");

	if (m_FreeTRBLoaderJobs.Size() > 0)
	{
		auto pTRBJobSlot = m_FreeTRBLoaderJobs.PopFront();
		m_UsedTRBLoaderJobs.PushFront(pTRBJobSlot);

		return TSTATICCAST(ATRBLoaderJob*, pTRBJobSlot->pJob);
	}
	else
	{
		return TNULL;
	}
}

ASkeletonDoneJob* ATerrain::GetFreeSkeletonLoaderJob()
{
	TASSERT(m_FreeSkeletonLoaderJobs.Size() > 0, "No free ASkeletonDoneJobs left!");

	if (m_FreeSkeletonLoaderJobs.Size() > 0)
	{
		auto pTRBJobSlot = m_FreeSkeletonLoaderJobs.PopFront();
		m_UsedSkeletonLoaderJobs.PushFront(pTRBJobSlot);

		return TSTATICCAST(ASkeletonDoneJob*, pTRBJobSlot->pJob);
	}
	else
	{
		return TNULL;
	}
}

AMatLibLoaderJob* ATerrain::GetFreeMatlibLoaderJob()
{
	TASSERT(m_FreeMatlibLoaderJobs.Size() > 0, "No free AMatlibLoaderJobs left!");

	if (m_FreeMatlibLoaderJobs.Size() > 0)
	{
		auto pTRBJobSlot = m_FreeMatlibLoaderJobs.PopFront();
		m_UsedMatlibLoaderJobs.PushFront(pTRBJobSlot);

		return TSTATICCAST(AMatLibLoaderJob*, pTRBJobSlot->pJob);
	}
	else
	{
		return TNULL;
	}
}

ACollisionDoneJob* ATerrain::GetFreeCollisionLoaderJob()
{
	TASSERT(m_FreeCollisionLoaderJobs.Size() > 0, "No free ACollisionDoneJob left!");

	if (m_FreeCollisionLoaderJobs.Size() > 0)
	{
		auto pTRBJobSlot = m_FreeCollisionLoaderJobs.PopFront();
		m_UsedCollisionLoaderJobs.PushFront(pTRBJobSlot);

		return TSTATICCAST(ACollisionDoneJob*, pTRBJobSlot->pJob);
	}
	else
	{
		return TNULL;
	}
}

AModelLoaderJob* ATerrain::GetFreeModelLoaderJob()
{
	TASSERT(m_FreeModelLoaderJobs.Size() > 0, "No free AModelLoaderJob left!");

	if (m_FreeModelLoaderJobs.Size() > 0)
	{
		auto pTRBJobSlot = m_FreeModelLoaderJobs.PopFront();
		m_UsedModelLoaderJobs.PushFront(pTRBJobSlot);

		return TSTATICCAST(AModelLoaderJob*, pTRBJobSlot->pJob);
	}
	else
	{
		return TNULL;
	}
}

ASectionDoneJob* ATerrain::GetFreeSectionLoaderJob()
{
	TASSERT(m_FreeSectionLoaderJobs.Size() > 0, "No free ASectionDoneJob left!");

	if (m_FreeSectionLoaderJobs.Size() > 0)
	{
		auto pTRBJobSlot = m_FreeSectionLoaderJobs.PopFront();
		m_UsedSectionLoaderJobs.PushFront(pTRBJobSlot);

		return TSTATICCAST(ASectionDoneJob*, pTRBJobSlot->pJob);
	}
	else
	{
		return TNULL;
	}
}

void ATerrain::FlushJobs()
{
	MoveAllFinishedJobs(m_FreeModelLoaderJobs, m_UsedModelLoaderJobs);
	MoveAllFinishedJobs(m_FreeUnknownLoaderJobs, m_UsedUnknownLoaderJobs);
	MoveAllFinishedJobs(m_FreeSectionLoaderJobs, m_UsedSectionLoaderJobs);
	MoveAllFinishedJobs(m_FreeKeylibLoaderJobs, m_UsedKeylibLoaderJobs);
	MoveAllFinishedJobs(m_FreeMatlibLoaderJobs, m_UsedMatlibLoaderJobs);
	MoveAllFinishedJobs(m_FreeTRBLoaderJobs, m_UsedTRBLoaderJobs);
	MoveAllFinishedJobs(m_FreeSkeletonLoaderJobs, m_UsedSkeletonLoaderJobs);
	MoveAllFinishedJobs(m_FreeCollisionLoaderJobs, m_UsedCollisionLoaderJobs);
}

void ATerrain::MoveAllFinishedJobs(Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs)
{
	for (auto it = a_rUsedJobs.Begin(); it != a_rUsedJobs.End(); )
	{
		auto pNext = it->GetNext();

		if (!it->pJob->IsRunning())
		{
			a_rUsedJobs.Remove(it, it);
			a_rFreeJobs.PushFront(it);
		}

		it = pNext;
	}
}

void ATerrain::CancelUnrequiredJobs()
{
	if (m_iCurrentSection != m_iPreviousSection && m_iPreviousSection >= 0)
	{
		for (TINT i = 0; i < m_pTerrainVIS->m_iNumSections; i++)
		{
			auto pGroup = &m_pTerrainVIS->m_pSections[i];

			if (pGroup->m_eFlags & (ATerrainSection::FLAGS_HIGH_LOD_LOADING | ATerrainSection::FLAGS_LOW_LOD_LOADING))
			{
				if (i < 0) return;
				if (pGroup->m_pVisibility[i] == ((pGroup->m_eFlags >> 2) - 1) % ATerrainLODType_NUMOF) return;
				if (i == m_iCurrentSection) return;

				AAssetStreaming::GetSingleton()->CancelAllJobs();
				return;
			}
		}
	}
}

void ATerrain::UpdateNightMaterials()
{
	TIMPLEMENT();
}

TINT ATerrain::GetCurrentSectionID()
{
	TIMPLEMENT();
	return 0;
}

TINT ATerrain::GetPersistantSectionID()
{
	return ms_iPersistantVISGroupIndex;
}

void ATerrainManager::SetTerrain(TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup)
{
	TIMPLEMENT();

	if (a_eTerrain == ms_eCurrentLevel) return;

	ARootTask::GetSingleton()->SetTerrainReady(TFALSE);
	
	if (ms_pCurrentTerrain)
	{
		delete ms_pCurrentTerrain;
	}

	TRenderInterface::GetSingleton()->FlushDyingResources();

	if (a_eTerrain == Terrain_FrontEnd)
	{
		a_iStartVISGroup = 0;
	}

	ms_pCurrentTerrain = new ATerrain(
		a_iUnused1,
		a_iUnused2,
		a_iPreloadTerrainBlockSize < 1 ? 0x2200000 : a_iPreloadTerrainBlockSize,
		a_iStartVISGroup
	);

	T2FixedString128 terrainFile;
	terrainFile.Format("Data\\Terrain\\%s\\%s.trb", GetTerrainName(a_eTerrain), GetTerrainName(a_eTerrain));
	
	ms_pCurrentTerrain->LoadFromFile(terrainFile.Get(), TTRUE, a_bPersistantCollision);
	ms_eCurrentLevel = a_eTerrain;

	if (!a_bLoadLater)
	{
		StartLoading();
	}

	ARootTask::GetSingleton()->SetTerrainReady(TTRUE);
}

void ATerrainManager::StartLoading()
{
	auto pTerrain = ATerrain::GetSingleton();

	pTerrain->m_bIsLoaded = TFALSE;

	if (ATerrain::ms_bAutoVIS)
	{
		pTerrain->m_fnGetCurrentVISGroup = ATerrain::GetPersistantSectionID;
	}

	pTerrain->Update();

	while (!pTerrain->IsLoaded())
	{
		TRenderInterface::GetSingleton()->Update(1.0f / 1000.0f);
		AAssetStreaming::GetSingleton()->Update();
		Sleep(20);

		pTerrain->Update();
		ALoadScreen::GetGlobalInstance()->Update();
	}

	pTerrain->m_bIsLoaded = TTRUE;

	if (ATerrain::ms_bAutoVIS)
	{
		pTerrain->m_fnGetCurrentVISGroup = ATerrain::GetCurrentSectionID;
	}
}
