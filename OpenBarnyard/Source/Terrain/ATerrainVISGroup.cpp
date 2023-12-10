#include "pch.h"
#include "ATerrainVISGroup.h"
#include "ATerrain.h"
#include "Assets/AAssetStreaming.h"
#include "Assets/AMaterialLibraryManager.h"

void ATerrainVISGroup::LoadCollision()
{
	auto pTerrain = ATerrain::GetSingleton();

	if (m_szCollisionFile[0] != '\0')
	{
		auto pBlock = (pTerrain->IsCollisionPersistant()) ?
			pTerrain->GetVIS()->m_pPersistantTerrainBlock : 
			m_ppHighLODBlocks[m_ui8AllocCollisionBlock];

		m_pCollisionModelData = new (pBlock->GetMemBlock()) ModelData();

		auto pTRB = new (pBlock->GetMemBlock()) Toshi::TTRB();
		pBlock->SetupTRB(pTRB, pBlock);

		auto pModelLoaderJob = pTerrain->GetFreeModelLoaderJob();
		pModelLoaderJob->InitJob(
			&m_pCollisionModelData->m_ModelRef,
			pTRB,
			m_szCollisionFile,
			pTerrain->IsCollisionPersistant()
		);

		auto pCollisionJob = pTerrain->GetFreeCollisionLoaderJob();
		pCollisionJob->InitJob(this, TFALSE);

		AAssetStreaming::GetSingleton()->AddMainThreadJob(pModelLoaderJob);
		AAssetStreaming::GetSingleton()->AddMainThreadJob(pCollisionJob);
		m_eFlags |= FLAGS_COLLISION_LOADING;
	}
}

void ATerrainVISGroup::LoadModels(ATerrainLODType a_eLODType)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	auto pTerrain = ATerrain::GetSingleton();

	if (!IsLODLoading(a_eLODType) && !IsLODLoaded(a_eLODType))
	{
		const char** ppLODNames;
		TINT iNumLODs;

		if (a_eLODType == ATerrainLODType_High)
		{
			iNumLODs = m_iNumHighLODs;
			ppLODNames = m_ppHighLODNames;
		}
		else
		{
			iNumLODs = m_iNumLowLODs;
			ppLODNames = m_ppLowLODNames;
		}

		if (iNumLODs == 0 && m_szCollisionFile[0] == '\0')
		{
			SetLODQueued(a_eLODType, TFALSE);
			SetLODEmpty(a_eLODType, TTRUE);
			SetLODLoaded(a_eLODType, TTRUE);
		}
		else
		{
			TUINT16 uiNumLODBlocks;
			ATerrainLODBlock** ppLODBlocks;
			TUINT8* pLODToBlock;

			if (a_eLODType == ATerrainLODType_High)
			{
				uiNumLODBlocks = m_ui16NumHighLODBlocks;
				ppLODBlocks = m_ppHighLODBlocks;
				pLODToBlock = m_pHighLODToBlock;
			}
			else
			{
				uiNumLODBlocks = m_ui16NumLowLODBlocks;
				ppLODBlocks = m_ppLowLODBlocks;
				pLODToBlock = m_pLowLODToBlock;
			}

			for (TINT i = 0; i < uiNumLODBlocks; i++)
			{
				if (ppLODBlocks[i] == TNULL)
				{
					ppLODBlocks[i] = pTerrain->AllocateLODBlock(a_eLODType, this);

					if (ppLODBlocks[i] == TNULL)
					{
						TASSERT(TFALSE, "Couldn't allocated new LOD block!");
						return;
					}

					ppLODBlocks[i]->m_bIsUnused = TFALSE;
				}
			}

			if (a_eLODType == ATerrainLODType_High && pTerrain->IsCollisionPersistant())
			{
				LoadCollision();
			}

			LoadMatlib(a_eLODType);

			for (TINT i = 0; i < iNumLODs; i++)
			{
				if (m_ppLODModelsData[a_eLODType][i] == TNULL)
				{
					auto pBlock = ppLODBlocks[pLODToBlock[i]];
					auto pModelData = new (pBlock->GetMemBlock()) ModelData();

					auto pTRB = new (pBlock->GetMemBlock()) Toshi::TTRB();
					pBlock->SetupTRB(pTRB, pBlock);
					m_ppLODModelsData[a_eLODType][i] = pModelData;

					auto pModelLoaderJob = pTerrain->GetFreeModelLoaderJob();
					pModelLoaderJob->InitJob(
						&pModelData->m_ModelRef,
						pTRB,
						ppLODNames[i],
						TTRUE
					);

					AAssetStreaming::GetSingleton()->AddMainThreadJob(pModelLoaderJob);
				}
			}

			auto pSectionJob = pTerrain->GetFreeSectionLoaderJob();
			pSectionJob->InitJob(this, a_eLODType);

			AAssetStreaming::GetSingleton()->AddMainThreadJob(pSectionJob);

			SetLODQueued(a_eLODType, TFALSE);
			SetLODLoading(a_eLODType, TTRUE);
		}
	}	
}

void ATerrainVISGroup::LoadMatlib(ATerrainLODType a_eLODType)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	TUINT8 uiAllocAtBlock;
	ATerrainLODBlock** ppBlocks;

	if (a_eLODType == ATerrainLODType_High)
	{
		if (m_szHighLODMatLibName[0] == '\0') return;
		uiAllocAtBlock = m_ui8AllocMatlibsHighBlock;
		ppBlocks = m_ppHighLODBlocks;
	}
	else
	{
		if (m_szLowLODMatLibName[0] == '\0') return;
		uiAllocAtBlock = m_ui8AllocMatlibsLowBlock;
		ppBlocks = m_ppLowLODBlocks;
	}

	auto pBlock = ppBlocks[uiAllocAtBlock];
	auto pMatlibJob = ATerrain::GetSingleton()->GetFreeMatlibLoaderJob();

	if (a_eLODType == ATerrainLODType_High)
	{
		m_pMatLibHighTRB = new (pBlock->GetMemBlock()) Toshi::TTRB();
		pBlock->SetupTRB(m_pMatLibHighTRB, pBlock);

		pMatlibJob->InitJob(m_szHighLODMatLibName, m_pMatLibHighTRB, m_pMatLibHigh, pBlock->GetMemBlock());
	}
	else
	{
		m_pMatLibLowTRB = new (pBlock->GetMemBlock()) Toshi::TTRB();
		pBlock->SetupTRB(m_pMatLibLowTRB, pBlock);

		pMatlibJob->InitJob(m_szLowLODMatLibName, m_pMatLibLowTRB, m_pMatLibLow, pBlock->GetMemBlock());
	}

	AAssetStreaming::GetSingleton()->AddMainThreadJob(pMatlibJob);
}

void ATerrainVISGroup::UnloadMatlib(ATerrainLODType a_eLODType)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	if (a_eLODType == ATerrainLODType_High)
	{
		if (m_pMatLibHigh)
		{
			AMaterialLibraryManager::GetSingleton()->UnloadTexturesOfLibrary(m_pMatLibHigh);
			m_pMatLibHigh->Destroy();
			m_pMatLibHigh = TNULL;
		}

		if (m_pMatLibHighTRB)
		{
			delete m_pMatLibHighTRB;
			m_pMatLibHighTRB = TNULL;
		}
	}
	else
	{
		if (m_pMatLibLow)
		{
			AMaterialLibraryManager::GetSingleton()->UnloadTexturesOfLibrary(m_pMatLibLow);
			m_pMatLibLow->Destroy();
			m_pMatLibLow = TNULL;
		}

		if (m_pMatLibLowTRB)
		{
			delete m_pMatLibLowTRB;
			m_pMatLibLowTRB = TNULL;
		}
	}
}

void ATerrainVISGroup::DestroyLOD(ATerrainLODType a_eLODType)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	auto pTerrain = ATerrain::GetSingleton();

	TINT iNumLODs = (a_eLODType == ATerrainLODType_High) ? m_iNumHighLODs : m_iNumLowLODs;
	for (TINT i = 0; i < iNumLODs; i++)
	{
		if (m_ppLODModelsData[a_eLODType][i])
		{
			pTerrain->DestroyModelData(m_ppLODModelsData[a_eLODType][i]);
			m_ppLODModelsData[a_eLODType][i] = TNULL;
		}
	}

	UnloadMatlib(a_eLODType);

	TUINT32 eNewFlags;

	if (a_eLODType == ATerrainLODType_High)
	{
		if (m_pCollisionModelData && pTerrain->IsCollisionPersistant())
		{
			pTerrain->DestroyModelData(m_pCollisionModelData);
			m_pCollisionModelData = TNULL;
		}

		for (TUINT i = 0; i < m_ui16NumHighLODBlocks; i++)
		{
			auto pBlock = m_ppHighLODBlocks[i];

			if (pBlock)
			{
				m_ppHighLODBlocks[i] = TNULL;
				pBlock->Assign(TNULL, ATerrainLODType_None);
			}
		}

		eNewFlags = m_eFlags & ~(FLAGS_HIGH_LOD_LOADED | FLAGS_HIGH_LOD_LOADING | FLAGS_HIGH_LOD_QUEUED);
	}
	else
	{
		for (TUINT i = 0; i < m_ui16NumLowLODBlocks; i++)
		{
			auto pBlock = m_ppLowLODBlocks[i];

			if (pBlock)
			{
				m_ppLowLODBlocks[i] = TNULL;
				pBlock->Assign(TNULL, ATerrainLODType_None);
			}
		}

		eNewFlags = m_eFlags & ~(FLAGS_LOW_LOD_LOADED | FLAGS_LOW_LOD_LOADING | FLAGS_LOW_LOD_QUEUED);
	}

	m_eFlags = eNewFlags;
	SetLODEmpty(a_eLODType, TFALSE);
}

void ATerrainVISGroup::RemoveFromStreamingQueue()
{
	for (TINT i = 0; i < ATerrainLODType_NUMOF; i++)
	{
		if (IsLODQueued(i))
		{
			TUINT16 uiNumLODBlocks;
			ATerrainLODBlock** ppLODBlocks;

			if (i == ATerrainLODType_High)
			{
				uiNumLODBlocks = m_ui16NumHighLODBlocks;
				ppLODBlocks = m_ppHighLODBlocks;
			}
			else
			{
				uiNumLODBlocks = m_ui16NumLowLODBlocks;
				ppLODBlocks = m_ppLowLODBlocks;
			}

			for (TINT k = 0; k < uiNumLODBlocks; k++)
			{
				ppLODBlocks[k]->Assign(TNULL, ATerrainLODType_None);
			}

			SetLODQueued(i, TFALSE);
		}
	}
}

TBOOL ATerrainVISGroup::IsMatLibLoaded(ATerrainLODType a_eLODType) const
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	if (a_eLODType == ATerrainLODType_High)
	{
		return m_pMatLibHigh != TNULL;
	}
	else
	{
		return m_pMatLibLow != TNULL;
	}
}

void ATerrainVISGroup::SetLODQueued(ATerrainLODType a_eLODType, TBOOL a_bQueued)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	if (a_bQueued)
	{
		m_eFlags |= (16 << a_eLODType);
	}
	else
	{
		m_eFlags &= ~(16 << a_eLODType);
	}
}

void ATerrainVISGroup::SetLODLoaded(ATerrainLODType a_eLODType, TBOOL a_bLoaded)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	if (a_bLoaded)
	{
		m_eFlags |= (1 << (a_eLODType));
	}
	else
	{
		m_eFlags &= ~(1 << (a_eLODType));
	}
}

void ATerrainVISGroup::SetLODLoading(ATerrainLODType a_eLODType, TBOOL a_bLoading)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	if (a_bLoading)
	{
		m_eFlags |= (1 << (a_eLODType + ATerrainLODType_NUMOF));
	}
	else
	{
		m_eFlags &= ~(1 << (a_eLODType + ATerrainLODType_NUMOF));
	}
}

void ATerrainVISGroup::SetLODEmpty(ATerrainLODType a_eLODType, TBOOL a_bEmpty)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	if (a_bEmpty)
	{
		m_eFlags |= (64 << a_eLODType);
	}
	else
	{
		m_eFlags &= ~(64 << a_eLODType);
	}
}

ATerrainVISGroup::ModelData::~ModelData()
{
	TIMPLEMENT();

	if (m_pModelInstance)
	{
		m_pModelInstance->Delete();
	}
}
