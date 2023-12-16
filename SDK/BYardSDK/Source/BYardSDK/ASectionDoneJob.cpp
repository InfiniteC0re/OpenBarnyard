#include "pch.h"
#include "ASectionDoneJob.h"
#include "ATerrain.h"

void ASectionDoneJob::BeginJob()
{
	
}

TBOOL ASectionDoneJob::RunJob()
{
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
