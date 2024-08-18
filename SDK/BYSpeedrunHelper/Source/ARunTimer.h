#pragma once
#include "AGUITimer.h"

#include <Toshi/THPTimer.h>
#include <Toshi/TSingleton.h>

class ATimerThread;

//-----------------------------------------------------------------------------
// Purpose: Wrapper for AGUITimer that also sends commands to ASplitsServer.
//-----------------------------------------------------------------------------
class ARunTimer :
    public Toshi::TSingleton<ARunTimer>
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

	void Resume();
	void Pause();

	void Update();
	void Render();

	AGUITimer& GetUIElement() { return m_UITimer; }

	void SetIsLoadingScreen( TBOOL a_bLoadingScreen );

	TFLOAT GetRunTime() const { return m_flTime; }
	TFLOAT GetLoadingTime() const { return m_flTotalLoadingTime; }
	TFLOAT GetTotalTime() const { return m_flTime + m_flTotalLoadingTime; }

private:
	ATimerThread* m_pTimerThread;
	AGUITimer     m_UITimer;
	TFLOAT        m_flTime;
	TBOOL         m_bPaused;

	TBOOL           m_bIsLoading;
	Toshi::THPTimer m_LoadingTimer;
	TFLOAT          m_flTotalLoadingTime;
};
