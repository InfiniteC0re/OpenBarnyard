#include "pch.h"
#include "ACollisionDoneJob.h"

void ACollisionDoneJob::BeginJob()
{

}

TBOOL ACollisionDoneJob::RunJob()
{
	return TTRUE;
}

TBOOL ACollisionDoneJob::CancelJob()
{
	return TFALSE;
}

void ACollisionDoneJob::InitJob(ATerrainVISGroup* a_pVISGroup, TBOOL a_bFlag)
{
	m_pVISGroup = a_pVISGroup;
	m_bFlag = a_bFlag;
}
