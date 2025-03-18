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
	m_flTime       = 0.0f;
	m_flGlobalTime = 0.0f;
	m_flSyncTimer  = 0.1f;
	m_bPaused      = TTRUE;
	m_bIsLoading   = TFALSE;
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

void ARunTimer::Reset()
{
	Update();
	m_flGlobalTime = 0.0f;
	m_flTime       = 0.0f;
	m_bPaused      = TTRUE;

	ASplitsServer::GetSingleton()->Reset();
}

void ARunTimer::Split()
{
	if ( m_bPaused )
		return;

	Update();
	
	TINT iMilliseconds, iSeconds, iMinutes, iHours;
	AGUITimer::GetTime( GetRunTime(), iMilliseconds, iSeconds, iMinutes, iHours );

	ASplitsServer::GetSingleton()->Split( iMilliseconds, iSeconds, iMinutes, iHours );
}

void ARunTimer::Start()
{
	m_flGlobalTime = 0.0f;
	m_flTime       = 0.0f;
	m_flSyncTimer  = 0.1f;
	m_bPaused      = TFALSE;

	Update();
	ASplitsServer::GetSingleton()->StartRun();
}

void ARunTimer::End()
{
	if ( m_bPaused )
		return;

	Update();
	m_bPaused = TTRUE;

	TINT iMilliseconds, iSeconds, iMinutes, iHours;
	AGUITimer::GetTime( GetRunTime(), iMilliseconds, iSeconds, iMinutes, iHours );

	ASplitsServer::GetSingleton()->EndRun( iMilliseconds, iSeconds, iMinutes, iHours );
}

void ARunTimer::Update()
{
	m_Timer.Update();
	TFLOAT flDelta = m_Timer.GetDelta();

	// Update time
	if ( !m_bPaused && !m_bIsLoading )
		m_flTime += flDelta;

	if ( !m_bPaused )
	{
		m_flGlobalTime += flDelta;
		m_flSyncTimer -= flDelta;

		// Send update to LiveSplit
		if ( m_flSyncTimer <= 0.0f )
		{
			TINT iMilliseconds, iSeconds, iMinutes, iHours;
			AGUITimer::GetTime( GetRunTime(), iMilliseconds, iSeconds, iMinutes, iHours );

			ASplitsServer::GetSingleton()->SendTime( iMilliseconds, iSeconds, iMinutes, iHours );
			m_flSyncTimer = 0.1f;
		}
	}

	// Update LRT timer
	if ( m_LRTTimer.IsValid() && g_oSettings.bShowLRTTimer )
	{
		m_LRTTimer.SetTime( GetRunTime() );
		m_LRTTimer.Update();
	}

	// Update RTA timer
	if ( m_RTATimer.IsValid() && g_oSettings.bShowRTATimer )
	{
		m_RTATimer.SetTime( GetTotalTime() );
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

		ASplitsServer::GetSingleton()->SetLoadingStart();

		TTRACE( "Loading started...\n" );
	}
	else if ( m_bIsLoading && !a_bLoadingScreen )
	{
		// Loading ended, reset timer
		m_Timer.Update();
		TFLOAT flLoadingTime = m_Timer.GetDelta();

		m_flGlobalTime += flLoadingTime;
		ASplitsServer::GetSingleton()->SetLoadingEnd();

		TTRACE( "Loading has ended and it took %.2f seconds... Total time spent while loading: %.2f seconds\n", flLoadingTime, GetLoadingTime() );
	}

	m_bIsLoading = a_bLoadingScreen;
}
