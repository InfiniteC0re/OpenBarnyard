#include "ToshiPCH.h"
#include "TThread.h"

namespace Toshi
{
	bool TThread::GetPriority(HANDLE a_hThreadHnd, PRIORITY& a_ePriority)
	{
		TASSERT(a_hThreadHnd != NULL, "");
		int iPriority = GetThreadPriority(a_hThreadHnd);
		TASSERT(iPriority != THREAD_PRIORITY_ERROR_RETURN, "");
		a_ePriority = static_cast<PRIORITY>(iPriority);
		return true;
	}

	bool TThread::SetPriority(HANDLE a_hThreadHnd, PRIORITY a_ePriority)
	{
		TASSERT(a_hThreadHnd != NULL, "");
		BOOL bResult = SetThreadPriority(a_hThreadHnd, a_ePriority);
		TASSERT(bResult != FALSE, "");
		return true;
	}

	void TThread::Exit(TThread* a_pThread)
	{
		TASSERT(a_pThread->m_iThreadID == GetCurrentThreadId(), "");
		BOOL bResult = CloseHandle(a_pThread->m_hThreadHnd);
		TASSERT(bResult != FALSE, "");
		TThreadManager::s_pThreadManager->RemoveThread(a_pThread);
		a_pThread->m_hThreadHnd = TNULL;
		a_pThread->m_iThreadID = -1;
		_endthreadex(0);
	}

	TThreadManager::TThreadManager()
	{
		//   *(TThreadManager **)(this + 0x18) = this + 0x18;
		//   *(TThreadManager**)(this + 0x1c) = this + 0x18;
		Create();
	}

	void TThreadManager::RemoveThread(TThread* a_pThread)
	{
		EnterCriticalSection((LPCRITICAL_SECTION)this);
		/*
			  **(undefined4 **)(param_1 + 8) = *(undefined4 *)(param_1 + 4);
	  *(undefined4 *)(*(int *)(param_1 + 4) + 4) = *(undefined4 *)(param_1 + 8);
	  *(TThread **)(param_1 + 4) = param_1 + 4;
	  *(TThread **)(param_1 + 8) = param_1 + 4;
		*/
		LeaveCriticalSection((LPCRITICAL_SECTION)this);
	}
}
