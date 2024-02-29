#include "pch.h"
#include "ATerrainSection.h"
#include "ATerrain.h"
#include "Assets/AAssetStreaming.h"
#include "Assets/AMaterialLibraryManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

void ATerrainSection::LoadCollision()
{
	auto pTerrain = ATerrain::GetSingleton();

	if (m_szCollisionFilename[0] != '\0')
	{
		auto pBlock = (!pTerrain->IsCollisionStreamed()) ?
			pTerrain->GetVIS()->m_pPersistantTerrainBlock : 
			m_ppHighLODBlocks[m_iCollisionMemBlockID];

		m_pCollisionModelData = new (pBlock->GetMemBlock()) ModelNode();

		auto pTRB = new (pBlock->GetMemBlock()) Toshi::TTRB();
		pBlock->SetupTRB(pTRB, pBlock);

		auto pModelLoaderJob = pTerrain->GetFreeModelLoaderJob();
		pModelLoaderJob->InitJob(
			&m_pCollisionModelData->m_ModelRef,
			pTRB,
			m_szCollisionFilename,
			pTerrain->IsCollisionStreamed()
		);

		auto pCollisionJob = pTerrain->GetFreeCollisionLoaderJob();
		pCollisionJob->InitJob(this, TFALSE);

		AAssetStreaming::GetSingleton()->AddMainThreadJob(pModelLoaderJob);
		AAssetStreaming::GetSingleton()->AddMainThreadJob(pCollisionJob);
		m_eFlags |= FLAGS_COLLISION_LOADING;
	}
}

void ATerrainSection::LoadModels(ATerrainLODType a_eLODType)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	auto pTerrain = ATerrain::GetSingleton();

	if (!IsLODLoading(a_eLODType) && !IsLODLoaded(a_eLODType))
	{
		const TCHAR** ppLODNames;
		TINT iNumLODs;

		if (a_eLODType == ATerrainLODType_High)
		{
			iNumLODs = m_iNumHighModelFiles;
			ppLODNames = m_pszHighModelFiles;
		}
		else
		{
			iNumLODs = m_iNumLowModelFiles;
			ppLODNames = m_pszLowModelFiles;
		}

		if (iNumLODs == 0 && m_szCollisionFilename[0] == '\0')
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
				uiNumLODBlocks = m_iNumHighMemBlocksUsed;
				ppLODBlocks = m_ppHighLODBlocks;
				pLODToBlock = m_pHighLODToBlock;
			}
			else
			{
				uiNumLODBlocks = m_iNumLowMemBlocksUsed;
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

			if (a_eLODType == ATerrainLODType_High && pTerrain->IsCollisionStreamed())
			{
				LoadCollision();
			}

			LoadMatlib(a_eLODType);

			for (TINT i = 0; i < iNumLODs; i++)
			{
				if (m_ppLODModelsData[a_eLODType][i] == TNULL)
				{
					auto pBlock = ppLODBlocks[pLODToBlock[i]];
					auto pModelData = new (pBlock->GetMemBlock()) ModelNode();

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

void ATerrainSection::LoadMatlib(ATerrainLODType a_eLODType)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	TUINT8 uiAllocAtBlock;
	ATerrainLODBlock** ppBlocks;

	if (a_eLODType == ATerrainLODType_High)
	{
		if (m_szHighMatLibFilename[0] == '\0') return;
		uiAllocAtBlock = m_iHighMatLibMemBlockID;
		ppBlocks = m_ppHighLODBlocks;
	}
	else
	{
		if (m_szLowMatLibFilename[0] == '\0') return;
		uiAllocAtBlock = m_iLowMatLibMemBlockID;
		ppBlocks = m_ppLowLODBlocks;
	}

	auto pBlock = ppBlocks[uiAllocAtBlock];
	auto pMatlibJob = ATerrain::GetSingleton()->GetFreeMatlibLoaderJob();

	if (a_eLODType == ATerrainLODType_High)
	{
		m_pMatLibHighTRB = new (pBlock->GetMemBlock()) Toshi::TTRB();
		pBlock->SetupTRB(m_pMatLibHighTRB, pBlock);

		pMatlibJob->InitJob(m_szHighMatLibFilename, m_pMatLibHighTRB, m_pMatLibHigh, pBlock->GetMemBlock());
	}
	else
	{
		m_pMatLibLowTRB = new (pBlock->GetMemBlock()) Toshi::TTRB();
		pBlock->SetupTRB(m_pMatLibLowTRB, pBlock);

		pMatlibJob->InitJob(m_szLowMatLibFilename, m_pMatLibLowTRB, m_pMatLibLow, pBlock->GetMemBlock());
	}

	AAssetStreaming::GetSingleton()->AddMainThreadJob(pMatlibJob);
}

void ATerrainSection::UnloadMatlib(ATerrainLODType a_eLODType)
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

void ATerrainSection::DestroyLOD(ATerrainLODType a_eLODType)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	auto pTerrain = ATerrain::GetSingleton();

	TINT iNumLODs = (a_eLODType == ATerrainLODType_High) ? m_iNumHighModelFiles : m_iNumLowModelFiles;
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
		if (m_pCollisionModelData && pTerrain->IsCollisionStreamed())
		{
			pTerrain->DestroyModelData(m_pCollisionModelData);
			m_pCollisionModelData = TNULL;
		}

		for (TUINT i = 0; i < m_iNumHighMemBlocksUsed; i++)
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
		for (TUINT i = 0; i < m_iNumLowMemBlocksUsed; i++)
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

void ATerrainSection::RemoveFromStreamingQueue()
{
	for (TINT i = 0; i < ATerrainLODType_NUMOF; i++)
	{
		if (IsLODQueued(i))
		{
			TUINT16 uiNumLODBlocks;
			ATerrainLODBlock** ppLODBlocks;

			if (i == ATerrainLODType_High)
			{
				uiNumLODBlocks = m_iNumHighMemBlocksUsed;
				ppLODBlocks = m_ppHighLODBlocks;
			}
			else
			{
				uiNumLODBlocks = m_iNumLowMemBlocksUsed;
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

TBOOL ATerrainSection::IsMatLibLoaded(ATerrainLODType a_eLODType) const
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

void ATerrainSection::SetLODQueued(ATerrainLODType a_eLODType, TBOOL a_bQueued)
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

void ATerrainSection::SetLODLoaded(ATerrainLODType a_eLODType, TBOOL a_bLoaded)
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

void ATerrainSection::SetLODLoading(ATerrainLODType a_eLODType, TBOOL a_bLoading)
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

void ATerrainSection::SetLODEmpty(ATerrainLODType a_eLODType, TBOOL a_bEmpty)
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

ATerrainSection::ModelNode::~ModelNode()
{
	TIMPLEMENT();

	if (m_pModelInstance)
	{
		m_pModelInstance->Delete();
	}
}
