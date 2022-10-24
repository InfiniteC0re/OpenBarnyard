#pragma once

#undef THREAD_MODE_BACKGROUND_BEGIN
#undef THREAD_MODE_BACKGROUND_END
#undef THREAD_PRIORITY_ABOVE_NORMAL
#undef THREAD_PRIORITY_BELOW_NORMAL
#undef THREAD_PRIORITY_HIGHEST
#undef THREAD_PRIORITY_IDLE
#undef THREAD_PRIORITY_LOWEST
#undef THREAD_PRIORITY_NORMAL
#undef THREAD_PRIORITY_TIME_CRITICAL

#include "Toshi/Core/TGenericDList.h"

namespace Toshi
{
	class TThread : public TDList<TThread>::TNode
	{
		enum PRIORITY
		{
			THREAD_MODE_BACKGROUND_BEGIN = 0x00010000,
			THREAD_MODE_BACKGROUND_END = 0x00020000,
			THREAD_PRIORITY_ABOVE_NORMAL = 1,
			THREAD_PRIORITY_BELOW_NORMAL = -1,
			THREAD_PRIORITY_HIGHEST = 2,
			THREAD_PRIORITY_IDLE = -15,
			THREAD_PRIORITY_LOWEST = -2,
			THREAD_PRIORITY_NORMAL = 0,
			THREAD_PRIORITY_TIME_CRITICAL = 15
		};

		static bool GetPriority(HANDLE a_hThreadHnd, PRIORITY& a_ePriority);
		static bool SetPriority(HANDLE a_hThreadHnd, PRIORITY a_ePriority);
		static void Exit(TThread* a_pThread);
		//static void Sleep(int milliSeconds) { usleep(100); }

		HANDLE m_hThreadHnd; // 0xC
		int m_iThreadID; // 0x10
	};

	class TThreadManager
	{

	public:
		static TThreadManager* s_pThreadManager;

		TThreadManager();

		inline void Create() { TASSERT(s_pThreadManager == TNULL, ""); s_pThreadManager = this;  InitializeCriticalSection((LPCRITICAL_SECTION)this); }
		inline void Delete() { TASSERT(s_pThreadManager != TNULL, ""); s_pThreadManager = TNULL; DeleteCriticalSection((LPCRITICAL_SECTION)this); }
		inline static TThreadManager* GetThreadManager() { return s_pThreadManager; }
		void RemoveThread(TThread* a_pThread);
	};
}


