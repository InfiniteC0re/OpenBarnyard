#include "pch.h"
#include "AAssetStreaming.h"
#include "ATRBLoaderJob.h"

AAssetStreaming::AAssetStreaming()
{
	m_pCurrentJob = TNULL;
	m_Unk = TNULL;
	m_bFlag = TTRUE;
	m_FileStream.Create(0, Toshi::TThread::THREAD_PRIORITY_NORMAL, 0);
}

void AAssetStreaming::Update()
{
	if (m_pCurrentJob == TNULL)
	{
		if (!m_Jobs.IsEmpty())
		{
			m_pCurrentJob = m_Jobs.PopFront();
			m_pCurrentJob->BeginJob();
		}
	}
	else if (m_pCurrentJob->RunJob())
	{
		m_pCurrentJob->m_bIsRunning = TTRUE;
		m_pCurrentJob = TNULL;
	}
}
