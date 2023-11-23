#include "pch.h"
#include "ATerrain.h"
#include "Tasks/ARootTask.h"
#include "Assets/AAssetStreaming.h"

#include <Toshi/Render/TRenderInterface.h>

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
	m_iCurrentGroup = a_iStartVISGroup;
	m_iPreviousGroup = -1;
	m_fnGetCurrentVISGroup = GetCurrentVISGroupIndex;
}

ATerrain::~ATerrain()
{
	TIMPLEMENT();

	m_bIsLoaded = TFALSE;
	delete[] m_pJobs;
}

void ATerrain::Update()
{
	TIMPLEMENT();
}

TBOOL ATerrain::IsLoaded()
{
	if (m_pTerrainVIS == TNULL)
	{
		return TTRUE;
	}

	if (m_pTerrainVIS->GetLocatorManager() && m_pTerrainVIS->GetLocatorManager()->GetTRB())
	{
		for (TINT i = 0; i < m_pTerrainVIS->m_iNumGroups; i++)
		{
			if (HASFLAG(m_pTerrainVIS->m_pGroups[i].m_eFlags & 0xF3C))
			{
				return TFALSE;
			}
		}

		return TTRUE;
	}

	return TFALSE;
}

void ATerrain::LoadFromFile(const char* a_szFilePath, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision)
{
	TFIXME("Do something with ATerrainSectionHudElement");

	m_bIsLoaded = TFALSE;

	m_VISTRB.Close();
	m_VISTRB.Load(a_szFilePath, 0);

	m_pTerrainVIS = m_VISTRB.CastSymbol<ATerrainVIS>("terrainvis");
	m_pOrderDVIS = m_VISTRB.CastSymbol<void*>("orderdvis");

	if (m_VISTRB.GetSymbolAddress("preloadcollision"))
	{
		a_bPersistantCollision = TFALSE;
		m_iPreloadTerrainBlockSize = m_pTerrainVIS->m_uiPersistantTerrainBlockSize;
	}

	m_VISTRB.DeleteSymbolTable();

	if (!a_bPersistantCollision)
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

	if (m_pTerrainVIS->m_iHighLODNum > 0)
	{
		m_pTerrainVIS->m_ppHighLODs = new ATerrainLODBlock*[m_pTerrainVIS->m_iHighLODNum];

		for (TINT i = 0; i < m_pTerrainVIS->m_iHighLODNum; i++)
		{
			char szBlockName[64];
			TStringManager::String8Format(szBlockName, sizeof(szBlockName), "Terrain_HighLOD_Block%d", i);

			m_pTerrainVIS->m_ppHighLODs[i] = new ATerrainLODBlock(
				m_pTerrainVIS->m_uiHighLODSize,
				szBlockName
			);
		}
	}

	if (m_pTerrainVIS->m_iLowLODNum > 0)
	{
		m_pTerrainVIS->m_ppLowLODs = new ATerrainLODBlock*[m_pTerrainVIS->m_iLowLODNum];

		for (TINT i = 0; i < m_pTerrainVIS->m_iLowLODNum; i++)
		{
			char szBlockName[64];
			TStringManager::String8Format(szBlockName, sizeof(szBlockName), "Terrain_LowLOD_Block%d", i);

			m_pTerrainVIS->m_ppLowLODs[i] = new ATerrainLODBlock(
				m_pTerrainVIS->m_uiLowLODSize,
				szBlockName
			);
		}
	}

	m_pTerrainVIS->LoadSkeleton();
	
	if (*m_pTerrainVIS->m_szMatLibrary != '\0')
	{
		auto pMatlibJob = GetFreeMatlibLoaderJob();
		auto pPersitantBlock = m_pTerrainVIS->m_pPersistantTerrainBlock;

		auto pTRB = new (pPersitantBlock->GetHeap()) TTRB();
		m_pTerrainVIS->m_pMaterialLibraryTRB = pTRB;
		pPersitantBlock->SetupTRB(pTRB, pPersitantBlock);

		pMatlibJob->InitJob(
			m_pTerrainVIS->m_szMatLibrary,
			m_pTerrainVIS->m_pMaterialLibraryTRB,
			m_pTerrainVIS->m_pMaterialLibrary,
			pPersitantBlock->GetHeap()
		);

		AAssetStreaming::GetSingleton()->AddMainThreadJob(pMatlibJob);
	}

	m_bPersistantCollision = a_bPersistantCollision;

	if (!m_bPersistantCollision)
	{
		for (TINT i = 0; i < m_pTerrainVIS->m_iNumGroups; i++)
		{
			m_pTerrainVIS->m_pGroups[i].LoadCollision();
		}
	}

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

TINT ATerrain::GetCurrentVISGroupIndex()
{
	TIMPLEMENT();
	return 0;
}

TINT ATerrain::GetPersistantVISGroupIndex()
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

	char terrainFile[128];
	TStringManager::String8Format(terrainFile, sizeof(terrainFile), "Data\\Terrain\\%s\\%s.trb", GetTerrainName(a_eTerrain), GetTerrainName(a_eTerrain));
	ms_pCurrentTerrain->LoadFromFile(terrainFile, TTRUE, a_bPersistantCollision);
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
		pTerrain->m_fnGetCurrentVISGroup = ATerrain::GetPersistantVISGroupIndex;
	}

	pTerrain->Update();

	while (!pTerrain->IsLoaded())
	{
		TRenderInterface::GetSingleton()->Update(1.0f / 1000.0f);
		AAssetStreaming::GetSingleton()->Update();
		Sleep(20);
		pTerrain->Update();
	}

	pTerrain->m_bIsLoaded = TTRUE;

	if (ATerrain::ms_bAutoVIS)
	{
		pTerrain->m_fnGetCurrentVISGroup = ATerrain::GetCurrentVISGroupIndex;
	}
}
