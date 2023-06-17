#include "ToshiPCH.h"
#include "TThread_Win.h"

namespace Toshi
{
	static TThreadManager oThreadManager;

	unsigned long __stdcall ThreadEntry(void* userParam)
	{
		TThread* pThread = static_cast<TThread*>(userParam);
		pThread->Main();
		TThread::Exit(pThread);
		return 0;
	}

	TBOOL TThread::Create(size_t a_iStackSize, PRIORITY a_ePriority, uint8_t flag)
	{
		m_iThreadID = -1;
		m_hThreadHnd = CreateThread(NULL, a_iStackSize, ThreadEntry, this, CREATE_SUSPENDED, &m_iThreadID);

		TASSERT(m_hThreadHnd != NULL, "Couldn't create thread");
		TBOOL bResult = SetThreadPriority(m_hThreadHnd, a_ePriority);
		TASSERT(bResult != TFALSE, "Couldn't set thread priority");

		TThreadManager::GetSingleton()->InsertThread(this);

		if ((flag & 1) == 0)
		{
			DWORD iResult = ResumeThread(m_hThreadHnd);
			TASSERT(iResult != -1, "Couldn't resume thread");
		}

		return TTRUE;
	}

	void TThread::Exit(TThread* a_pThread)
	{
		TASSERT(a_pThread->m_iThreadID == GetCurrentThreadId(), "Thread cannot be closed outside");

		BOOL bResult = CloseHandle(a_pThread->m_hThreadHnd);
		TASSERT(bResult != FALSE, "Couldn't close thread");

		TThreadManager::GetSingleton()->RemoveThread(a_pThread);
		a_pThread->m_hThreadHnd = NULL;
		a_pThread->m_iThreadID = -1;

		_endthreadex(0);
	}
}
