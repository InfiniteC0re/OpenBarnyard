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

#include "Toshi/Core/TDList.h"

namespace Toshi
{
	class TThread : public TDList<TThread>::TNode
	{
	public:
		typedef int PRIORITY;
		enum PRIORITY_ : PRIORITY
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

	public:
		virtual ~TThread() { };
		virtual void Main() = 0;

		TBOOL Create(size_t a_iStackSize, PRIORITY a_ePriority, uint8_t flag);

		static TBOOL GetPriority(void* a_hThreadHnd, PRIORITY& a_ePriority)
		{
			TASSERT(a_hThreadHnd != NULL, "Thread doesn't exist");
			int iPriority = GetThreadPriority(a_hThreadHnd);
			TASSERT(iPriority != THREAD_PRIORITY_ERROR_RETURN, "Couldn't get thread priority");
			a_ePriority = iPriority;
			return TTRUE;
		}

		static TBOOL SetPriority(void* a_hThreadHnd, PRIORITY a_ePriority)
		{
			TASSERT(a_hThreadHnd != NULL, "Thread doesn't exist");
			BOOL bResult = SetThreadPriority(a_hThreadHnd, a_ePriority);
			TASSERT(bResult != FALSE, "Couldn't set priority");
			return TTRUE;
		}

		static void Exit(TThread* a_pThread);
		//static void Sleep(int milliSeconds) { usleep(100); }

		void* m_hThreadHnd; // 0xC
		DWORD m_iThreadID;  // 0x10
	};

	class TThreadManager : public TSingleton<TThreadManager>
	{
	public:
		TThreadManager() { Create(); }

		void Create() { InitializeCriticalSection(&m_CriticalSection); }
		void Delete() { DeleteCriticalSection(&m_CriticalSection); }

		void RemoveThread(TThread* a_pThread)
		{
			EnterCriticalSection(&m_CriticalSection);
			a_pThread->Remove();
			LeaveCriticalSection(&m_CriticalSection);
		}

		void InsertThread(TThread* a_pThread)
		{
			EnterCriticalSection(&m_CriticalSection);
			a_pThread->InsertAfter(m_Threads.Begin());
			LeaveCriticalSection(&m_CriticalSection);
		}

		friend class TThread;

	private:
		CRITICAL_SECTION m_CriticalSection;
		TDList<TThread> m_Threads;
	};
}


