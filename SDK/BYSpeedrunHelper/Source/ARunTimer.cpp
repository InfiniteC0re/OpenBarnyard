#include "pch.h"
#include "ARunTimer.h"
#include "ASplitsServer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

class ATimerThread :
	public TThread
{
public:
	ATimerThread( ARunTimer* a_pRunTimer ) :
		m_pRunTimer( a_pRunTimer )
	{
		m_flLastSentGameTime = 0.0f;
	}

	virtual void Main() override
	{
		while ( TTRUE )
		{
			m_Timer.Update();
			TFLOAT flDelta = m_Timer.GetDelta();

			// Update time
			if ( !m_pRunTimer->m_bPaused )
				m_pRunTimer->m_flTime += flDelta;

			if ( m_pRunTimer->m_flTime - m_flLastSentGameTime >= 0.2f )
			{
				TINT iMilliseconds, iSeconds, iMinutes, iHours;
				AGUITimer::GetTime( m_pRunTimer->m_flTime, iMilliseconds, iSeconds, iMinutes, iHours );

				ASplitsServer::GetSingleton()->SendTime( iMilliseconds, iSeconds, iMinutes, iHours );
				m_flLastSentGameTime = m_pRunTimer->m_flTime;
			}

			ThreadSleep( 10 );
		}
	}

public:
	ARunTimer* m_pRunTimer;
	THPTimer m_Timer;
	TFLOAT m_flLastSentGameTime;
};

ARunTimer::ARunTimer()
{
	m_pTimerThread = TNULL;
	m_flTime = 0.0f;
	m_bPaused = TTRUE;
}

ARunTimer::~ARunTimer()
{
	if ( m_pTimerThread )
	{
		TThread::Exit( m_pTimerThread );
		delete m_pTimerThread;
		m_pTimerThread = TNULL;
	}
}

void ARunTimer::Create()
{
	m_UITimer.Create();

	m_pTimerThread = new ATimerThread( this );
	m_pTimerThread->Create( 128, TThread::THREAD_PRIORITY_LOWEST, 0 );
}

void ARunTimer::Reset()
{
	m_flTime = 0.0f;
	m_bPaused = TTRUE;

	ASplitsServer::GetSingleton()->Reset();
}

void ARunTimer::Start()
{
	m_flTime = 0.0f;
	m_bPaused = TFALSE;

	ASplitsServer::GetSingleton()->StartRun();
}

void ARunTimer::Pause()
{
	m_bPaused = TTRUE;

	ASplitsServer::GetSingleton()->Pause();
}

void ARunTimer::Resume()
{
	m_bPaused = TFALSE;

	ASplitsServer::GetSingleton()->Resume();
}

void ARunTimer::Update()
{
	if ( m_UITimer.IsValid() )
	{
		m_UITimer.SetTime( m_flTime );
		m_UITimer.Update();
	}
}

void ARunTimer::Render()
{
	m_UITimer.Render();
}