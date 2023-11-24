#include "pch.h"
#include "ATerrainVISGroup.h"
#include "ATerrain.h"
#include "Assets/AAssetStreaming.h"
#include "Assets/AMaterialLibraryManager.h"

void ATerrainVISGroup::LoadCollision()
{
	TIMPLEMENT();
}

void ATerrainVISGroup::LoadModels()
{
	TIMPLEMENT();
}

void ATerrainVISGroup::LoadMatlib(ATerrainLODType a_eLODType)
{
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
		m_pMatLibHighTRB = new Toshi::TTRB();
		pBlock->SetupTRB(m_pMatLibHighTRB, pBlock);

		pMatlibJob->InitJob(m_szHighLODMatLibName, m_pMatLibHighTRB, m_pMatLibHigh, pBlock->GetHeap());
	}
	else
	{
		m_pMatLibLowTRB = new Toshi::TTRB();
		pBlock->SetupTRB(m_pMatLibLowTRB, pBlock);

		pMatlibJob->InitJob(m_szLowLODMatLibName, m_pMatLibLowTRB, m_pMatLibLow, pBlock->GetHeap());
	}

	AAssetStreaming::GetSingleton()->AddMainThreadJob(pMatlibJob);
}

void ATerrainVISGroup::UnloadMatlib(ATerrainLODType a_eLODType)
{
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

		eNewFlags = m_eFlags & ~(BITFIELD(0) | BITFIELD(2) | BITFIELD(4));
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

		eNewFlags = m_eFlags & ~(BITFIELD(1) | BITFIELD(3) | BITFIELD(5));
	}

	m_eFlags = eNewFlags;
	SetLODCreatedFlags(a_eLODType, TFALSE);
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

void ATerrainVISGroup::SetLODCreatedFlags(ATerrainLODType a_eLODType, TBOOL a_bCreated)
{
	TASSERT(a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low);

	if (a_bCreated)
	{
		m_eFlags |= (64 << a_eLODType);
	}
	else
	{
		m_eFlags &= ~(64 << a_eLODType);
	}
}
