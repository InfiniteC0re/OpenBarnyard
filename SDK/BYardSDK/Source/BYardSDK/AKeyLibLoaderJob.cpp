#include "pch.h"
#include "AKeyLibLoaderJob.h"
#include "AAssetStreaming.h"

void AKeyLibLoaderJob::BeginJob()
{
}

TBOOL AKeyLibLoaderJob::RunJob()
{
	return TFALSE;
}

void AKeyLibLoaderJob::InitJob( const char* a_szFileName )
{
	m_oStreamJob.Init( a_szFileName );
}
