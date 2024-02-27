#include "pch.h"
#include "ATRBLoaderJob.h"
#include "AAssetStreaming.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

void ATRBLoaderJob::BeginJob()
{
	AAssetStreaming::GetSingleton()->GetFileStream().AddStream(
		&m_oStreamJob
	);
}

TBOOL ATRBLoaderJob::RunJob()
{
	return m_oStreamJob.IsProcessed() != TFALSE;
}

void ATRBLoaderJob::InitJob(Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName)
{
	m_oStreamJob.Init(a_pTRB, a_szFileName);
}
