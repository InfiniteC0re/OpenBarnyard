#include "pch.h"
#include "ASectionDoneJob.h"

void ASectionDoneJob::BeginJob()
{

}

TBOOL ASectionDoneJob::RunJob()
{
	TIMPLEMENT();
	return TTRUE;
}

TBOOL ASectionDoneJob::CancelJob()
{
	return TFALSE;
}
