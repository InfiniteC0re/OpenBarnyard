#include "pch.h"
#include "ATRBLoaderJob.h"
#include "AAssetStreaming.h"

void ATRBLoaderJob::BeginJob()
{
}

TBOOL ATRBLoaderJob::RunJob()
{
	return m_oStreamJob.IsProcessed() != TFALSE;
}

void ATRBLoaderJob::InitJob( Toshi::TTRB* a_pTRB, const char* a_szFileName )
{
	m_oStreamJob.Init( a_pTRB, a_szFileName );
}
