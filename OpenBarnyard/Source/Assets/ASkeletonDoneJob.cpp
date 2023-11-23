#include "pch.h"
#include "ASkeletonDoneJob.h"

void ASkeletonDoneJob::BeginJob()
{

}

TBOOL ASkeletonDoneJob::RunJob()
{
	Toshi::TMemoryHeapSwap heapSwap;
	m_pVIS->GetLocatorManager()->LoadFromTRB(m_pTRBLoaderJob->GetStream().GetTRB());

	return TTRUE;
}
