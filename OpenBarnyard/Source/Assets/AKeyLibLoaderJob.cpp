#include "pch.h"
#include "AKeyLibLoaderJob.h"
#include "AAssetStreaming.h"

void AKeyLibLoaderJob::BeginJob()
{
	AAssetStreaming::GetSingleton()->GetFileStream().AddStream(
		&m_oStreamJob
	);
}

TBOOL AKeyLibLoaderJob::RunJob()
{
	if (m_oStreamJob.IsProcessed())
	{
		if (m_ppOutKeyframeLib)
		{
			*m_ppOutKeyframeLib = m_oStreamJob.GetLibrary();
		}

		return TTRUE;
	}

	return TFALSE;
}

void AKeyLibLoaderJob::InitJob(const char* a_szFileName)
{
	m_oStreamJob.Init(a_szFileName);
}