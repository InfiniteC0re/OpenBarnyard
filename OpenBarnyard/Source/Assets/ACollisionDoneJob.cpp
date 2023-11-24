#include "pch.h"
#include "ACollisionDoneJob.h"

void ACollisionDoneJob::BeginJob()
{

}

TBOOL ACollisionDoneJob::RunJob()
{
	TIMPLEMENT();

	m_pVISGroup->m_eFlags &= ~ATerrainVISGroup::FLAGS_COLLISION_LOADING;
	return TTRUE;
}

TBOOL ACollisionDoneJob::CancelJob()
{
	TIMPLEMENT();
	return TFALSE;
}

void ACollisionDoneJob::InitJob(ATerrainVISGroup* a_pVISGroup, TBOOL a_bFlag)
{
	m_pVISGroup = a_pVISGroup;
	m_bFlag = a_bFlag;
}
