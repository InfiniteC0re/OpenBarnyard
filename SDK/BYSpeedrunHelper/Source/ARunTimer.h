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

	void Start();
	void Reset();

	void Resume();
	void Pause();

	void Update();
	void Render();

	AGUITimer& GetUIElement() { return m_UITimer; }

private:
	ATimerThread* m_pTimerThread;
	AGUITimer m_UITimer;
	TFLOAT m_flTime;
	TBOOL m_bPaused;
};
