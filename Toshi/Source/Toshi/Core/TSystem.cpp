#include "ToshiPCH.h"
#include "TSystem.h"
#include "TScheduler.h"

namespace Toshi
{
	TSystemManager::TSystemManager() : m_Emitter(this)
	{
		m_Scheduler = TNULL;
		m_Paused = false;
		m_Second = 0;
		m_AverageFps = 0;
		m_FrameCount = 0;
		m_Unk7 = false;
	}

	void TSystemManager::Update()
	{
		// Calculating average fps
		m_Timer.Update();
		float deltaTime = m_Timer.GetDelta();
		m_Second += deltaTime;

		if (m_Second > 1.0f)
		{
			m_Second = 0.0;
			m_AverageFps = (1.0f / deltaTime) * 0.5f + m_AverageFps * 0.5f;
		}

		m_FrameCount += 1;
		
		// Update the scheduler
		m_Scheduler->Update();
	}

	bool TSystemManager::Create()
	{
		TSystemManager::CreateSingleton()->m_Scheduler = new TScheduler();
		return true;
	}
	TCStringPool* TSystemManager::CreateCStringPoolExplicit(int unk, int unk2)
	{
		if (ms_poTCStringPool == TNULL)
		{
			ms_poTCStringPool = new TCStringPool(unk, unk2);
		}
		return ms_poTCStringPool;
	}
	TCStringPool* TSystemManager::CreateCStringPoolExplicit(const char* a_szFileName, int unk, int unk2)
	{
		if (ms_poTCStringPool == TNULL)
		{
			ms_poTCStringPool = new TCStringPool(a_szFileName, unk, unk2);
		}
		return ms_poTCStringPool;
	}
}