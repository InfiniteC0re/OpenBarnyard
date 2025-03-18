#pragma once
#include "AGUITimer.h"

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

	void Reset();
	void Split();

	void Update();
	void Render();

	AGUITimer& GetLRTTimerElement() { return m_LRTTimer; }
	AGUITimer& GetRTATimerElement() { return m_RTATimer; }

	void SetIsLoadingScreen( TBOOL a_bLoadingScreen );

	TFLOAT GetRunTime() const { return m_flTime; }
	TFLOAT GetLoadingTime() const { return m_flGlobalTime - m_flTime; }
	TFLOAT GetTotalTime() const { return m_flGlobalTime; }

private:
	Toshi::THPTimer  m_Timer;
	AGUITimer        m_LRTTimer;
	AGUITimer        m_RTATimer;
	TFLOAT           m_flTime;
	TFLOAT           m_flSyncTimer;
	TFLOAT           m_flGlobalTime;
	TBOOL            m_bPaused;
	TBOOL            m_bIsLoading;
};
