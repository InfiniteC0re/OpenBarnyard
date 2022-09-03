#pragma once
#include "THPTimer.h"

namespace Toshi
{
	class TScheduler;

	class TKernelInterface : public TObject
	{
		TOSHI_CLASS_DEFINE(TKernelInterface)

	public:
		TKernelInterface();

		bool Update();

	public:
		inline TScheduler& GetScheduler() { return *m_Scheduler; }
		inline THPTimer& GetSystemTimer() { return m_SystemTimer; }
		inline bool IsPaused() { return m_Paused; }

	private:
		THPTimer m_SystemTimer;          // 0x08
		TScheduler* m_Scheduler;         // 0x30
		float m_Second;                  // 0x44
		float m_AverageFPS;              // 0x48 (not sure)
		bool m_Paused;                   // 0x54
	};
}