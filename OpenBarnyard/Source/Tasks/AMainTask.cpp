#include "pch.h"
#include "AMainTask.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00403f60, CreateObject 004040c0, CreateObjectInPlace 00404130 (the constructor is inlined in both)
TDEFINE_CLASS( AMainTask );

AMainTask::AMainTask()
{
	m_fTotalTime = 0.0f;
}

// $Barnyard: FUNCTION 00404090
// $Barnyard: FUNCTION 00404480
AMainTask::~AMainTask()
{
}

// $Barnyard: FUNCTION 004040a0
TBOOL AMainTask::OnCreate()
{
	// NOTE: the original jumps to TTask::Create here, not TTask::OnCreate. That would recurse forever, if anything created this task
	return TTask::Create();
}

// $Barnyard: FUNCTION 004040b0
TBOOL AMainTask::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_fTotalTime += a_fDeltaTime;
	return TTRUE;
}
