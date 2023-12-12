#include "pch.h"
#include "ATerrainVISGroup.h"
#include "ATerrain.h"
#include "AAssetStreaming.h"

void ATerrainVISGroup::LoadCollision()
{
	
}

void ATerrainVISGroup::LoadModels(ATerrainLODType a_eLODType)
{
	
}

void ATerrainVISGroup::LoadMatlib(ATerrainLODType a_eLODType)
{
	
}

void ATerrainVISGroup::UnloadMatlib(ATerrainLODType a_eLODType)
{
	
}

void ATerrainVISGroup::DestroyLOD(ATerrainLODType a_eLODType)
{
	
}

void ATerrainVISGroup::RemoveFromStreamingQueue()
{
	
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
	
}
