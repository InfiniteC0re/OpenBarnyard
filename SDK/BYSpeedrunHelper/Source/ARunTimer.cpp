#include "pch.h"
#include "ARunTimer.h"
#include "ASplitsServer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

class ATimerThread : public TThread
{
public:
	ATimerThread( ARunTimer* a_pRunTimer )
	    : m_pRunTimer( a_pRunTimer )
	{
		m_TimeSyncMutex.Create();
		m_flSyncTimerTime = 0.1f;
	}

	virtual void Main() override
	{
		while ( TTRUE )
		{
			m_Timer.Update();
			TFLOAT flDelta = m_Timer.GetDelta();

			// Update time
			if ( !m_pRunTimer->m_bPaused && !m_pRunTimer->m_bIsLoading )
				m_pRunTimer->m_flTime += flDelta;

			m_flSyncTimerTime -= flDelta;

			if ( m_flSyncTimerTime <= 0.0f && !m_pRunTimer->m_bPaused )
			{
				T2MUTEX_LOCK_SCOPE( m_TimeSyncMutex );

				TINT iMilliseconds, iSeconds, iMinutes, iHours;
				AGUITimer::GetTime( m_pRunTimer->GetRunTime(), iMilliseconds, iSeconds, iMinutes, iHours );

				ASplitsServer::GetSingleton()->SendTime( iMilliseconds, iSeconds, iMinutes, iHours );
				m_flSyncTimerTime = 0.1f;
			}

			ThreadSleep( 5 );
		}
	}

public:
	ARunTimer* m_pRunTimer;
	THPTimer   m_Timer;
	TFLOAT     m_flSyncTimerTime;
	T2Mutex    m_TimeSyncMutex;
	TBOOL      m_bClosed;
};

ARunTimer::ARunTimer()
{
	m_pTimerThread = TNULL;
	m_flTime       = 0.0f;
	m_bPaused      = TTRUE;
}

ARunTimer::~ARunTimer()
{
	Destroy();
}

void ARunTimer::Create()
{
	m_UITimer.Create();

	m_pTimerThread = new ATimerThread( this );
	m_pTimerThread->Create( 128, TThread::THREAD_PRIORITY_LOWEST, 0 );
}

void ARunTimer::Destroy()
{
	ASplitsServer::GetSingleton()->StopServer();

	if ( m_pTimerThread )
	{
		m_pTimerThread->Destroy();
		delete m_pTimerThread;
		m_pTimerThread = TNULL;
	}
}

void ARunTimer::Reset()
{
	m_flTotalLoadingTime = 0.0f;
	m_flTime             = 0.0f;
	m_bPaused            = TTRUE;

	ASplitsServer::GetSingleton()->Reset();
}

void ARunTimer::Split()
{
	if ( m_bPaused )
		return;

	TINT iMilliseconds, iSeconds, iMinutes, iHours;
	AGUITimer::GetTime( GetRunTime(), iMilliseconds, iSeconds, iMinutes, iHours );

	ASplitsServer::GetSingleton()->Split( iMilliseconds, iSeconds, iMinutes, iHours );
}

void ARunTimer::Start()
{
	m_flTotalLoadingTime = 0.0f;
	m_flTime             = 0.0f;
	m_bPaused            = TFALSE;

	ASplitsServer::GetSingleton()->StartRun();
}

void ARunTimer::End()
{
	if ( m_bPaused )
		return;

	m_bPaused = TTRUE;

	TINT iMilliseconds, iSeconds, iMinutes, iHours;
	AGUITimer::GetTime( GetRunTime(), iMilliseconds, iSeconds, iMinutes, iHours );

	ASplitsServer::GetSingleton()->EndRun( iMilliseconds, iSeconds, iMinutes, iHours );
}

void ARunTimer::Pause()
{
	if ( m_bPaused )
		return;

	m_bPaused = TTRUE;
	ASplitsServer::GetSingleton()->Pause();
}

void ARunTimer::Resume()
{
	if ( !m_bPaused )
		return;

	m_bPaused = TFALSE;
	ASplitsServer::GetSingleton()->Pause();
}

void ARunTimer::Update()
{
	if ( m_UITimer.IsValid() )
	{
		m_UITimer.SetTime( GetRunTime() );
		m_UITimer.Update();
	}
}

void ARunTimer::Render()
{
	m_UITimer.Render();
}

void ARunTimer::SetIsLoadingScreen( TBOOL a_bLoadingScreen )
{
	if ( m_bPaused )
		return;

	if ( !m_bIsLoading && a_bLoadingScreen )
	{
		// Loading started
		m_LoadingTimer.Reset();

		ASplitsServer::GetSingleton()->SetLoadingStart();

		TTRACE( "Loading started...\n" );
	}
	else if ( m_bIsLoading && !a_bLoadingScreen )
	{
		// Loading ended
		m_LoadingTimer.Update();

		TFLOAT flLoadingTime = m_LoadingTimer.GetDelta();
		m_flTotalLoadingTime += flLoadingTime;

		ASplitsServer::GetSingleton()->SetLoadingEnd();

		TTRACE( "Loading has ended and it took %.2f seconds... Total time spent while loading: %.2f seconds\n", flLoadingTime, m_flTotalLoadingTime );
	}

	m_bIsLoading = a_bLoadingScreen;
}
