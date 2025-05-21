#include "pch.h"
#include "ACollisionDoneJob.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

void ACollisionDoneJob::BeginJob()
{
}

// $Barnyard: FUNCTION 005ed0d0
TBOOL ACollisionDoneJob::RunJob()
{
	TIMPLEMENT();

	m_pVISGroup->m_eFlags &= ~ATerrainSection::FLAGS_COLLISION_LOADING;
	return TTRUE;
}

// $Barnyard: FUNCTION 005ed170
TBOOL ACollisionDoneJob::CancelJob()
{
	TIMPLEMENT();
	return TFALSE;
}

void ACollisionDoneJob::InitJob( ATerrainSection* a_pVISGroup, TBOOL a_bFlag )
{
	m_pVISGroup = a_pVISGroup;
	m_bFlag     = a_bFlag;
}
