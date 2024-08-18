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

#include "Toshi/T2DList.h"

TOSHI_NAMESPACE_START

class TThread : public T2DList<TThread>::Node
{
public:
	typedef TINT PRIORITY;
	enum PRIORITY_ : PRIORITY
	{
		THREAD_MODE_BACKGROUND_BEGIN  = 0x00010000,
		THREAD_MODE_BACKGROUND_END    = 0x00020000,
		THREAD_PRIORITY_ABOVE_NORMAL  = 1,
		THREAD_PRIORITY_BELOW_NORMAL  = -1,
		THREAD_PRIORITY_HIGHEST       = 2,
		THREAD_PRIORITY_IDLE          = -15,
		THREAD_PRIORITY_LOWEST        = -2,
		THREAD_PRIORITY_NORMAL        = 0,
		THREAD_PRIORITY_TIME_CRITICAL = 15
	};

public:
	virtual void Main() = 0;
	virtual ~TThread(){};

	TBOOL Create( size_t a_iStackSize, PRIORITY a_ePriority, uint8_t flag );
	TBOOL Destroy();

	static TBOOL GetPriority( void* a_hThreadHnd, PRIORITY& a_ePriority );
	static TBOOL SetPriority( void* a_hThreadHnd, PRIORITY a_ePriority );

	static void Exit( TThread* a_pThread );
	//static void Sleep(TINT milliSeconds) { usleep(100); }

	void* m_hThreadHnd; // 0xC
	DWORD m_iThreadID;  // 0x10
};

class TThreadManager : public TSingleton<TThreadManager>
{
public:
	TThreadManager() { Create(); }

	void Create() { InitializeCriticalSection( &m_CriticalSection ); }
	void Delete() { DeleteCriticalSection( &m_CriticalSection ); }

	void RemoveThread( TThread* a_pThread );
	void InsertThread( TThread* a_pThread );

	friend class TThread;

private:
	CRITICAL_SECTION m_CriticalSection;
	T2DList<TThread> m_Threads;
};

TOSHI_NAMESPACE_END
