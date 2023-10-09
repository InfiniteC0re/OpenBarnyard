#include "pch.h"
#include "ARootTask.h"

#include <Toshi/Core/TScheduler.h>

TOSHI_NAMESPACE_USING

ARootTask::ARootTask()
{
	TIMPLEMENT();

	auto pSystemManager = TSystemManager::GetSingleton();

	m_pRenderer = TSTATICCAST(ARenderer*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ARenderer)));
}

TBOOL ARootTask::OnCreate()
{
	TIMPLEMENT();

	if (m_pRenderer)
	{
		if (!m_pRenderer->Create())
		{
			return TFALSE;
		}
	}
	
	return TTRUE;
}
