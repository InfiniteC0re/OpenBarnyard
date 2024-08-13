#include "pch.h"
#include "ASkeletonDoneJob.h"

void ASkeletonDoneJob::BeginJob()
{
}

TBOOL ASkeletonDoneJob::RunJob()
{
	return TTRUE;
}

void ASkeletonDoneJob::InitJob( ATerrainVIS* a_pVIS, ATRBLoaderJob* a_pTRBJob )
{
	m_pVIS			= a_pVIS;
	m_pTRBLoaderJob = a_pTRBJob;
}
