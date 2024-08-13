#pragma once
#include "TEvent.h"
#include "TSingleton.h"
#include "TKernelInterface.h"

#include <utility>

TOSHI_NAMESPACE_START

class TScheduler;
class TPString8Pool;

class TSystemManager
{
public:
	friend TScheduler;

public:
	TSystemManager();

	void Update();

	TFLOAT GetFPS() const
	{
		return m_pKernelInterface->GetFPS();
	}

	TBOOL Pause( TBOOL a_bPause )
	{
		TBOOL bOldState = m_pKernelInterface->SetPaused( a_bPause );
		m_PauseEmitter.Throw( &a_bPause );

		return bOldState;
	}

	TEmitter<TSystemManager, TBOOL>& GetPauseEmitter()
	{
		return m_PauseEmitter;
	}

	TUINT32 GetFrameCount() const
	{
		return m_pKernelInterface->GetNumFrames();
	}

	THPTimer& GetTimer()
	{
		return m_pKernelInterface->GetTimer();
	}

	TScheduler* GetScheduler() const
	{
		return m_pKernelInterface->GetScheduler();
	}

	TBOOL IsPaused() const
	{
		return m_pKernelInterface->IsPaused();
	}

	void SetQuitCallback( TKernelInterface::t_QuitCallback a_fnQuitCallback )
	{
		m_pKernelInterface->SetQuitCallback( a_fnQuitCallback );
	}

public:
	static TBOOL Create();

private:
	inline static TBOOL ms_bWasEverUpdated;

private:
	TEmitter<TSystemManager, TBOOL> m_PauseEmitter;
	TKernelInterface*				m_pKernelInterface;
};

extern TSystemManager g_oSystemManager;

TOSHI_NAMESPACE_END
