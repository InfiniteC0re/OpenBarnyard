#pragma once
#include "THPTimer.h"

namespace Toshi
{
	class TScheduler;

	class TKernelInterface : public TObject
	{
		TOSHI_CLASS_NO_CREATE_DEFINE(TKernelInterface)

	public:
		TKernelInterface();

		bool Update();

	public:
		inline TScheduler& GetScheduler() const { return *m_Scheduler; }
		inline THPTimer& GetSystemTimer() { return m_SystemTimer; }
		inline float GetAverageFPS() const { return m_AverageFPS; }
		inline bool IsPaused() const { return m_Paused; }

	private:
		THPTimer m_SystemTimer;          // 0x08
		STL::Ref<TScheduler> m_Scheduler;     // 0x30
		float m_Second;                  // 0x44
		float m_AverageFPS;              // 0x48
		bool m_Paused;                   // 0x54
	};
}