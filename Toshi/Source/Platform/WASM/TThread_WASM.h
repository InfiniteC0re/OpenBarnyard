#pragma once
#include "Toshi/T2DList.h"

#include <pthread.h>

TOSHI_NAMESPACE_START

class TThread : public T2DList<TThread>::Node
{
public:
	typedef TINT PRIORITY;
	enum PRIORITY_ : PRIORITY
	{
		THREAD_PRIORITY_ABOVE_NORMAL  = 1,
		THREAD_PRIORITY_BELOW_NORMAL  = -1,
		THREAD_PRIORITY_HIGHEST       = 2,
		THREAD_PRIORITY_IDLE          = -15,
		THREAD_PRIORITY_LOWEST        = -2,
		THREAD_PRIORITY_NORMAL        = 0,
		THREAD_PRIORITY_TIME_CRITICAL = 15
	};

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE      = 0,
		FLAGS_SUSPENDED = BITFLAG( 0 ),
	};

public:
	virtual void Main() = 0;
	virtual ~TThread() {}

	TBOOL Create( size_t a_iStackSize, PRIORITY a_ePriority, FLAGS a_eFlags );
	TBOOL Destroy();

	static TBOOL GetPriority( void* a_hThreadHnd, PRIORITY& a_ePriority );
	static TBOOL SetPriority( void* a_hThreadHnd, PRIORITY a_ePriority );

	static void Exit( TThread* a_pThread );

	void*         m_hThreadHnd;
	unsigned long m_iThreadID;

private:
	pthread_t m_oThread;
};

class TThreadManager : public TSingleton<TThreadManager>
{
public:
	TThreadManager() { Create(); }
	~TThreadManager();

	void Create();
	void Delete();

	void RemoveThread( TThread* a_pThread );
	void InsertThread( TThread* a_pThread );

	friend class TThread;

private:
	pthread_mutex_t  m_Mutex;
	T2DList<TThread> m_Threads;
};

TOSHI_NAMESPACE_END
