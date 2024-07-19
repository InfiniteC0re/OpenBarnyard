#include "ToshiPCH.h"
#include "TThread_Win.h"

#include <process.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

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

		TThreadManager::GetSingletonSafe()->InsertThread(this);

		if ((flag & 1) == 0)
		{
			DWORD iResult = ResumeThread(m_hThreadHnd);
			TASSERT(iResult != -1, "Couldn't resume thread");
		}

		return TTRUE;
	}

	TBOOL TThread::Destroy()
	{
		TASSERT( m_iThreadID != GetCurrentThreadId() );

		BOOL bResult = TerminateThread( m_hThreadHnd, 0 );
		TASSERT( bResult != FALSE );

		bResult = CloseHandle( m_hThreadHnd );
		TASSERT( bResult != FALSE );

		TThreadManager::GetSingletonSafe()->RemoveThread( this );
		m_hThreadHnd = NULL;
		m_iThreadID = -1;

		return TTRUE;
	}

	TBOOL TThread::GetPriority( void* a_hThreadHnd, PRIORITY& a_ePriority )
	{
		TASSERT( a_hThreadHnd != NULL, "Thread doesn't exist" );
		TINT iPriority = GetThreadPriority( a_hThreadHnd );
		TASSERT( iPriority != THREAD_PRIORITY_ERROR_RETURN, "Couldn't get thread priority" );
		a_ePriority = iPriority;
		return TTRUE;
	}

	TBOOL TThread::SetPriority( void* a_hThreadHnd, PRIORITY a_ePriority )
	{
		TASSERT( a_hThreadHnd != NULL, "Thread doesn't exist" );
		BOOL bResult = SetThreadPriority( a_hThreadHnd, a_ePriority );
		TASSERT( bResult != FALSE, "Couldn't set priority" );
		return TTRUE;
	}

	void TThread::Exit( TThread* a_pThread )
	{
		TASSERT(a_pThread->m_iThreadID == GetCurrentThreadId(), "Thread cannot be closed outside");

		BOOL bResult = CloseHandle(a_pThread->m_hThreadHnd);
		TASSERT(bResult != FALSE, "Couldn't close thread");

		TThreadManager::GetSingletonSafe()->RemoveThread(a_pThread);
		a_pThread->m_hThreadHnd = NULL;
		a_pThread->m_iThreadID = -1;

		_endthreadex(0);
	}

	void TThreadManager::RemoveThread( TThread* a_pThread )
	{
		EnterCriticalSection( &m_CriticalSection );
		a_pThread->Remove();
		LeaveCriticalSection( &m_CriticalSection );
	}

	void TThreadManager::InsertThread( TThread* a_pThread )
	{
		EnterCriticalSection( &m_CriticalSection );
		m_Threads.PushFront( a_pThread );
		LeaveCriticalSection( &m_CriticalSection );
	}

}
