#include "ToshiPCH.h"
#include "Thread/TThread.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

static TThreadManager oThreadManager;

static void* ThreadEntry( void* a_pUserParam )
{
	TThread* pThread = TSTATICCAST( TThread, a_pUserParam );
	pThread->Main();
	TThread::Exit( pThread );
	return TNULL;
}

TBOOL TThread::Create( TSIZE a_iStackSize, PRIORITY a_ePriority, FLAGS a_eFlags )
{
	pthread_attr_t oAttributes;
	pthread_attr_init( &oAttributes );
	pthread_attr_setdetachstate( &oAttributes, PTHREAD_CREATE_DETACHED );

	if ( a_iStackSize != 0 )
		pthread_attr_setstacksize( &oAttributes, a_iStackSize );

	TThreadManager::GetSingletonSafe()->InsertThread( this );

	m_iThreadID = 1;
	const TINT iResult = pthread_create( &m_oThread, &oAttributes, ThreadEntry, this );
	pthread_attr_destroy( &oAttributes );

	TASSERT( iResult == 0, "Couldn't create thread" );
	m_hThreadHnd = this;

	return iResult == 0;
}

// Threads can't be terminated here, the ones of the game live until the page is closed
TBOOL TThread::Destroy()
{
	if ( m_hThreadHnd == TNULL )
		return TFALSE;

	TThreadManager::GetSingletonSafe()->RemoveThread( this );
	m_hThreadHnd = TNULL;
	m_iThreadID  = -1;

	return TTRUE;
}

TBOOL TThread::GetPriority( void* a_hThreadHnd, PRIORITY& a_ePriority )
{
	a_ePriority = THREAD_PRIORITY_NORMAL;
	return TTRUE;
}

TBOOL TThread::SetPriority( void* a_hThreadHnd, PRIORITY a_ePriority )
{
	return TTRUE;
}

void TThread::Exit( TThread* a_pThread )
{
	TThreadManager::GetSingletonSafe()->RemoveThread( a_pThread );
	a_pThread->m_hThreadHnd = TNULL;
	a_pThread->m_iThreadID  = -1;
}

void TThreadManager::RemoveThread( TThread* a_pThread )
{
	pthread_mutex_lock( &m_Mutex );
	a_pThread->Remove();
	pthread_mutex_unlock( &m_Mutex );
}

void TThreadManager::InsertThread( TThread* a_pThread )
{
	pthread_mutex_lock( &m_Mutex );
	m_Threads.PushFront( a_pThread );
	pthread_mutex_unlock( &m_Mutex );
}

void TThreadManager::Create()
{
	pthread_mutex_init( &m_Mutex, TNULL );
}

void TThreadManager::Delete()
{
	pthread_mutex_destroy( &m_Mutex );
	m_Threads.Clear();
}

TThreadManager::~TThreadManager()
{
	Delete();
}

TOSHI_NAMESPACE_END
