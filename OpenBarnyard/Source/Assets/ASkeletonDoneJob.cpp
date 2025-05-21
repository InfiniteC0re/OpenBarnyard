#include "pch.h"
#include "ASkeletonDoneJob.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

void ASkeletonDoneJob::BeginJob()
{
}

// $Barnyard: FUNCTION 005ed0a0
TBOOL ASkeletonDoneJob::RunJob()
{
	m_pVIS->GetLocatorList()->LoadFromTRB( m_pTRBLoaderJob->GetStream().GetTRB() );

	return TTRUE;
}

void ASkeletonDoneJob::InitJob( ATerrainVIS* a_pVIS, ATRBLoaderJob* a_pTRBJob )
{
	m_pVIS          = a_pVIS;
	m_pTRBLoaderJob = a_pTRBJob;
}
