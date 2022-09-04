#include "ToshiPCH.h"
#include "TKernelInterface.h"
#include "TScheduler.h"

namespace Toshi
{
	TOSHI_CLASS_DERIVED_INITIALIZE(TKernelInterface, TObject, MKVERSION(1, 0))
	
	TKernelInterface::TKernelInterface()
	{
		m_Scheduler = new TScheduler(this);
		m_Paused = false;
		m_AverageFPS = 0;
		m_Second = 0;
	}

	bool TKernelInterface::Update()
	{
		if (!m_Paused)
		{
			auto& timer = GetSystemTimer();
			timer.Update();
			
			float deltaTime = timer.GetDelta();
			float newSecondTime = m_Second + deltaTime;

			if (newSecondTime <= 1.0f)
			{
				m_Second = newSecondTime;
			}
			else
			{
				m_AverageFPS = (m_AverageFPS + 1.0f / deltaTime) * 0.5f;
				m_Second = 0.0f;
			}

			m_Scheduler->Update();
		}
		else
		{
			m_Scheduler->UpdateKernelPaused();
		}

		return true;
	}
}