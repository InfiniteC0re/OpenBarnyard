#pragma once
#include "AGUITimer.h"
#include "ARunTimeline.h"

#include <Toshi/THPTimer.h>

class ATimerThread;

//-----------------------------------------------------------------------------
// Purpose: Wrapper for AGUITimer that also sends commands to ASplitsServer.
//-----------------------------------------------------------------------------
class ARunTimer
{
public:
	friend ATimerThread;

public:
	ARunTimer();
	~ARunTimer();

	void Create();
	void Destroy();

	void Start();
	void End();

	void Split();

	void Update();
	void Render();

	AGUITimer& GetLRTTimerElement() { return m_LRTTimer; }
	AGUITimer& GetRTATimerElement() { return m_RTATimer; }

	void SetIsLoadingScreen( TBOOL a_bLoadingScreen );

private:
	ARunTimeline     m_Timeline;
	Toshi::THPTimer  m_Timer;
	AGUITimer        m_LRTTimer;
	AGUITimer        m_RTATimer;
	//TFLOAT           m_flTime;
	//TFLOAT           m_flGlobalTime;
	TBOOL            m_bPaused;
	TBOOL            m_bIsLoading;
	TFLOAT           m_flSyncTimer;
};
