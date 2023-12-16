#include "pch.h"
#include "ASectionDoneJob.h"
#include "Terrain/ATerrain.h"

void ASectionDoneJob::BeginJob()
{

}

TBOOL ASectionDoneJob::RunJob()
{
	TIMPLEMENT();

	if (m_bIsCanceled)
	{
		m_pVISGroup->SetLODLoading(m_eLODType, TFALSE);
		m_pVISGroup->SetLODQueued(m_eLODType, TFALSE);
		m_pVISGroup->SetLODEmpty(m_eLODType, TFALSE);
		m_pVISGroup->DestroyLOD(m_eLODType);

		ATerrain::GetSingleton()->QueueStreamingAssets();
		return TTRUE;
	}
	else
	{
		m_pVISGroup->SetLODLoaded(m_eLODType, TTRUE);
		m_pVISGroup->SetLODLoading(m_eLODType, TFALSE);
		m_pVISGroup->SetLODEmpty(m_eLODType, TTRUE);
		return TTRUE;
	}

	return TTRUE;
}

TBOOL ASectionDoneJob::CancelJob()
{
	m_bIsCanceled = TTRUE;
	return TFALSE;
}

void ASectionDoneJob::InitJob(ATerrainSection* a_pVISGroup, ATerrainLODType a_eLODType)
{
	m_eLODType = a_eLODType;
	m_pVISGroup = a_pVISGroup;
	m_bIsCanceled = TFALSE;
}
