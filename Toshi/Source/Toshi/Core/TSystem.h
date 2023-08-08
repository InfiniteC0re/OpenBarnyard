#pragma once
#include "TEvent.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Core/THPTimer.h"

#ifdef TOSHI_ENABLE_DEPRECATED
#include "Toshi/Strings/TCStringPool.h"
#else
#include "Toshi/Strings/TPooledCString.h"
#endif // TOSHI_ENABLE_DEPRECATED

#include <utility>

namespace Toshi
{
	class TScheduler;

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
		static TBOOL Create();

#ifdef TOSHI_ENABLE_DEPRECATED
		static TCStringPool* CreateCStringPoolExplicit(int unk, int unk2);
		static TCStringPool* CreateCStringPoolExplicit(const char* a_szFileName, int unk, int unk2);

		static inline TCStringPool* ms_poTCStringPool = TNULL;
#else
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
#endif // TOSHI_ENABLE_DEPRECATED

	private:
		TEmitter<TSystemManager, TBOOL> m_Emitter; // 0x00
		TScheduler* m_Scheduler;
		TBOOL m_Paused;
		uint32_t m_Unk2;
		THPTimer m_Timer;
		uint32_t m_Unk3;
		float m_Second;
		float m_AverageFps;
		uint32_t m_FrameCount;
		TBOOL m_Unk7;
	};
}


