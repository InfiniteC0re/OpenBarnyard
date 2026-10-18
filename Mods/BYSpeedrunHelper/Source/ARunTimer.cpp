#include "pch.h"
#include "ARunTimer.h"
#include "ASplitsServer.h"
#include "AModSettings.h"
#include "AUIManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ARunTimer::ARunTimer()
{
	m_flSyncTimer = 0.1f;
	m_bPaused     = TTRUE;
	m_bIsLoading  = TFALSE;
}

ARunTimer::~ARunTimer()
{
	Destroy();
}

void ARunTimer::Create()
{
	m_LRTTimer.Create();
	m_RTATimer.Create();
}

void ARunTimer::Destroy()
{
	ASplitsServer::GetSingleton()->StopServer();
}

void ARunTimer::Split()
{
	if ( m_bPaused )
		return;

	// Let timeline save info about this split
	m_Timeline.AddEvent( ARunTimeline::EventType::Split );

	// Get actual time
	TDOUBLE time, timeNoLoads;
	m_Timeline.GetDuration( time, timeNoLoads );

	// Get components of the current time
	TINT iMilliseconds, iSeconds, iMinutes, iHours;
	AGUITimer::GetTime( TFLOAT( timeNoLoads ), iMilliseconds, iSeconds, iMinutes, iHours );

	// Send event to LiveSplit
	ASplitsServer::GetSingleton()->Split( iMilliseconds, iSeconds, iMinutes, iHours );
}

void ARunTimer::Start()
{
	m_flSyncTimer = 0.1f;
	m_bPaused     = TFALSE;

	m_Timeline.Start();
	ASplitsServer::GetSingleton()->StartRun();
}

void ARunTimer::End()
{
	if ( m_bPaused )
		return;

	// Stop the timeline and save it as a file
	m_Timeline.Stop();
	m_Timeline.SaveToFile( TNULL );

	// Get actual time
	TDOUBLE time, timeNoLoads;
	m_Timeline.GetDuration( time, timeNoLoads );

	// Get components of the current time
	TINT iMilliseconds, iSeconds, iMinutes, iHours;
	AGUITimer::GetTime( TFLOAT( timeNoLoads ), iMilliseconds, iSeconds, iMinutes, iHours );

	// Send event to LiveSplit
	ASplitsServer::GetSingleton()->EndRun( iMilliseconds, iSeconds, iMinutes, iHours );
	m_bPaused = TTRUE;
}

void ARunTimer::Update()
{
	m_Timer.Update();
	TFLOAT flDelta = m_Timer.GetDelta();

	TDOUBLE time, timeNoLoads;
	m_Timeline.GetDuration( time, timeNoLoads );

	if ( !m_bPaused )
	{
		m_flSyncTimer -= flDelta;

		// Send update to LiveSplit
		if ( m_flSyncTimer <= 0.0f )
		{
			TINT iMilliseconds, iSeconds, iMinutes, iHours;
			AGUITimer::GetTime( TFLOAT( timeNoLoads ), iMilliseconds, iSeconds, iMinutes, iHours );

			ASplitsServer::GetSingleton()->SendTime( iMilliseconds, iSeconds, iMinutes, iHours );
			m_flSyncTimer = 0.1f;
		}
	}

	// Update LRT timer
	if ( m_LRTTimer.IsValid() && g_oSettings.bShowLRTTimer )
	{
		m_LRTTimer.SetTime( TFLOAT( timeNoLoads ) );
		m_LRTTimer.Update();
	}

	// Update RTA timer
	if ( m_RTATimer.IsValid() && g_oSettings.bShowRTATimer )
	{
		m_RTATimer.SetTime( TFLOAT( time ) );
		m_RTATimer.Update();
	}
}

void ARunTimer::Render()
{
	AUIVerticalStack& rUIStack = AUIManager::GetSingleton()->GetLeftSideList();

	const TBOOL bEnablePrefix = g_oSettings.bShowLRTTimer && g_oSettings.bShowRTATimer;
	m_LRTTimer.EnablePrefix( L"LRT: ", bEnablePrefix );
	m_RTATimer.EnablePrefix( L"RTA: ", bEnablePrefix );

	if ( g_oSettings.bShowLRTTimer )
	{
		m_LRTTimer.UpdateUIPosition( rUIStack.flHeight );
		m_LRTTimer.Render();

		rUIStack.AddElement( 24.0f );
	}
	if ( g_oSettings.bShowRTATimer )
	{
		m_RTATimer.UpdateUIPosition( rUIStack.flHeight );
		m_RTATimer.Render();

		rUIStack.AddElement( 24.0f );
	}
}

void ARunTimer::SetIsLoadingScreen( TBOOL a_bLoadingScreen )
{
	if ( m_bPaused )
		return;

	if ( !m_bIsLoading && a_bLoadingScreen )
	{
		Update();

		m_Timeline.AddEvent( ARunTimeline::EventType::LoadStart );
		ASplitsServer::GetSingleton()->SetLoadingStart();

		//TTRACE( "Loading started...\n" );
	}
	else if ( m_bIsLoading && !a_bLoadingScreen )
	{
		// Loading ended
		m_Timeline.AddEvent( ARunTimeline::EventType::LoadEnd );
		ASplitsServer::GetSingleton()->SetLoadingEnd();

		//TTRACE( "Loading has ended and it took %.2f seconds... Total time spent while loading: %.2f seconds\n", flLoadingTime, GetLoadingTime() );
	}

	m_bIsLoading = a_bLoadingScreen;
}
