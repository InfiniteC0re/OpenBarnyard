#include "pch.h"
#include "AAssetStreaming.h"
#include "AAssetStreamingJobs.h"

AAssetStreaming::AAssetStreaming()
{
	// Create thread and run it
	m_FileStream.Create(0, Toshi::TThread::THREAD_PRIORITY_ABOVE_NORMAL, 0);
	m_Jobs = T2DList<AMainThreadJob2>();
	m_FreeList = T2DList<AMainThreadJob2>();
	m_FileStream = TFileStream();
	m_pCurrentJob = TNULL;
	TTODO("member at 0xD8 = TNULL");

	for (size_t i = 0; i < JOB_COUNT; i++)
	{
		m_FreeList.PushBack(new ATRBLoaderJob());
	}
}

void AAssetStreaming::Update()
{
	if (m_pCurrentJob == TNULL)
	{
		if (!m_Jobs.IsEmpty())
		{
			RunJob();
		}
	}
	else
	{
		//m_pCurrentJob->RunJob();
	}
}

void AAssetStreaming::ReleaseJob(AMainThreadJob2* a_pJob)
{
	a_pJob->m_streamJob = Toshi::TTRBStreamJob();
	TASSERT(m_FreeList.Size() < JOB_COUNT);
	m_FreeList.PushBack(a_pJob);
}
