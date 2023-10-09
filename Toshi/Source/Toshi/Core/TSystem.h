#pragma once
#include "TEvent.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Core/THPTimer.h"

#include <utility>

namespace Toshi
{
	class TScheduler;
	class TPString8Pool;

	class TSystemManager : public TSingleton<TSystemManager>
	{
	public:
		TSystemManager();
	
		void Update();

		TBOOL Pause(TBOOL pause)
		{
			TBOOL oldState = m_Paused;

			m_Paused = pause;
			m_Emitter.Throw(&pause);

			return oldState;
		}

		uint32_t GetFrameCount()
		{
			return m_FrameCount;
		}

		THPTimer* GetTimer()
		{
			return &m_Timer;
		}

		TScheduler* GetScheduler() const
		{
			return m_Scheduler;
		}

		TFLOAT GetAverageFps() const
		{
			return m_AverageFps;
		}

		TBOOL IsPaused() const
		{
			return m_Paused;
		}

	public:
		static TBOOL Create();

	public:
		static void CreateStringPool()
		{
			TASSERT(ms_poStringPool == TNULL);
			ms_poStringPool = new TPString8Pool*;
			*ms_poStringPool = TNULL;
		}

		static TPString8Pool* GetStringPool()
		{
			TASSERT(ms_poStringPool != TNULL);
			return *ms_poStringPool;
		}

		static TPString8Pool* SetStringPool(TPString8Pool* a_pStringPool)
		{
			return std::exchange(*ms_poStringPool, a_pStringPool);
		}

	private:
		inline static TPString8Pool** ms_poStringPool;

	private:
		TEmitter<TSystemManager, TBOOL> m_Emitter; // 0x00
		TScheduler* m_Scheduler;
		TBOOL m_Paused;
		TUINT32 m_Unk2;
		THPTimer m_Timer;
		TUINT32 m_Unk3;
		TFLOAT m_Second;
		TFLOAT m_AverageFps;
		TUINT32 m_FrameCount;
		TBOOL m_Unk7;
	};
}


