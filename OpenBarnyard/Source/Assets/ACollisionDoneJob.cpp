#include "pch.h"
#include "ACollisionDoneJob.h"

void ACollisionDoneJob::BeginJob()
{

}

TBOOL ACollisionDoneJob::RunJob()
{
	TIMPLEMENT();
	return TTRUE;
}

TBOOL ACollisionDoneJob::CancelJob()
{
	TIMPLEMENT();
	return TFALSE;
}
