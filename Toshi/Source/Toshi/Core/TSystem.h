#pragma once
#include "TEvent.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Core/THPTimer.h"

namespace Toshi
{
	class TScheduler;

	class TSystemManager : public TSingleton<TSystemManager>
	{
	public:
		TSystemManager();
	
		void Update();

		bool Pause(bool pause)
		{
			bool oldState = m_Paused;
			TTODO("FUN_00663190(true)");

			return oldState;
		}

		THPTimer* GetTimer()
		{
			return &m_Timer;
		}

		TScheduler* GetScheduler() const
		{
			return m_Scheduler;
		}

		float GetAverageFps() const
		{
			return m_AverageFps;
		}

	public:
		static bool Create();

	private:
		TEmitter<TSystemManager> m_Emitter; // 0x00
		TScheduler* m_Scheduler;
		bool m_Paused;
		uint32_t m_Unk2;
		THPTimer m_Timer;
		uint32_t m_Unk3;
		float m_Second;
		float m_AverageFps;
		uint32_t m_FrameCount;
		bool m_Unk7;
	};
}


