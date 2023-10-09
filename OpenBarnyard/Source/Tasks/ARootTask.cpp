#include "pch.h"
#include "ARootTask.h"
#include "Memory/AMemory.h"
#include <Toshi/Core/TScheduler.h>

TOSHI_NAMESPACE_USING

ARootTask::ARootTask()
{
	TIMPLEMENT();

	m_pOptions = AOptions::CreateSingleton();
	AMemory::CreatePool(AMemory::POOL_Sound);

	auto pSystemManager = TSystemManager::GetSingleton();
	m_pRenderer = TSTATICCAST(ARenderer*, pSystemManager->GetScheduler()->CreateTask(&TGetClass(ARenderer)));

	m_bRenderScene = TFALSE;
}

TBOOL ARootTask::OnCreate()
{
	TIMPLEMENT();

	AMemory::CreatePool(AMemory::POOL_Misc);
	AMemory::CreatePool(AMemory::POOL_Collision);
	AMemory::CreatePool(AMemory::POOL_Viewport);

	if (m_pRenderer)
	{
		if (!m_pRenderer->Create())
		{
			return TFALSE;
		}
	}
	
	return TTRUE;
}
