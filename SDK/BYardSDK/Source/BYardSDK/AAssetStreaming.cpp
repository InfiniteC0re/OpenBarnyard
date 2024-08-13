#include "pch.h"
#include "AAssetStreaming.h"
#include "ATRBLoaderJob.h"
#include "SDKHooks.h"

void AAssetStreaming::Update()
{
	if ( m_pCurrentJob == TNULL )
	{
		if ( !m_Jobs.IsEmpty() )
		{
			m_pCurrentJob = m_Jobs.PopFront();
			m_pCurrentJob->BeginJob();
		}
	}
	else if ( m_pCurrentJob->RunJob() )
	{
		m_pCurrentJob->m_bIsRunning = TTRUE;
		m_pCurrentJob				= TNULL;
	}
}

void AAssetStreaming::CancelAllJobs()
{
	for ( auto it = m_Jobs.Begin(); it != m_Jobs.End(); )
	{
		auto next = it->Next();

		if ( it->CancelJob() )
		{
			it->m_bIsRunning = TTRUE;
			it->Remove();
		}

		it = next;
	}
}
