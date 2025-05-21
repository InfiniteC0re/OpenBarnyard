#pragma once
#include "TEvent.h"
#include "TSingleton.h"
#include "THPTimer.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/TSystemDLL_Win.h"
#endif // TOSHI_SKU_WINDOWS

TOSHI_NAMESPACE_START

class TScheduler;
class TPString8Pool;

class TKernelInterfaceDLL :
#ifdef TOSHI_SKU_WINDOWS
    public TSystemDLLWin
    ,
#else
#  error Unsupported platform
#endif
    public TNodeList<TKernelInterfaceDLL>::TNode
{
public:
	TKernelInterfaceDLL()  = default;
	~TKernelInterfaceDLL() = default;
};

//-----------------------------------------------------------------------------
// Purpose: Provides an interface to work with other interfaces.
// Note: This class is completely removed in the future branches of TOSHI.
//       It's also the reason why it's almost empty compared to it's
//       implementation in JPOG.
//-----------------------------------------------------------------------------
class TKernelInterface : public TSingleton<TKernelInterface>
{
public:
	TKernelInterface()  = default;
	~TKernelInterface() = default;

	TKernelInterfaceDLL* FindInterface( const TString8& a_rcName );
	TKernelInterfaceDLL* LoadInterface( const TString8& a_rcName );

private:
	TNodeList<TKernelInterfaceDLL> m_Interfaces;
};

class TSystemTimer
{
public:
	using QuitCallback = void ( * )();

public:
	TSystemTimer();
	~TSystemTimer();

	void Update();

	THPTimer& GetTimer()
	{
		return m_Timer;
	}

	TUINT GetNumFrames() const
	{
		return m_uiNumFrames;
	}

	TScheduler* GetScheduler() const
	{
		return m_Scheduler;
	}

	void SetScheduler( TScheduler* a_pScheduler )
	{
		m_Scheduler = a_pScheduler;
	}

	TBOOL IsPaused() const
	{
		return m_bIsPaused;
	}

	TBOOL SetPaused( TBOOL a_bPaused )
	{
		TBOOL bOldState = m_bIsPaused;
		m_bIsPaused     = a_bPaused;
		return bOldState;
	}

	TFLOAT GetFPS() const
	{
		return 1.0f / m_Timer.GetDelta();
	}

	void SetQuitCallback( QuitCallback a_fnQuitCallback )
	{
		m_fnQuitCallback = a_fnQuitCallback;
	}

private:
	TUINT        m_Unk1;
	TScheduler*  m_Scheduler;
	TBOOL        m_bIsPaused;
	TUINT32      m_Unk2;
	THPTimer     m_Timer;
	TUINT32      m_Unk3;
	TFLOAT       m_fSecond;
	TFLOAT       m_fAverageFps;
	TUINT32      m_uiNumFrames;
	TBOOL        m_Unk4;
	QuitCallback m_fnQuitCallback;
	TUINT32      m_Unk6;
};

class TSystemManager
{
public:
	friend TScheduler;

public:
	TSystemManager();
	~TSystemManager();

	void Update();

	// $Barnyard: FUNCTION 006bbc30
	TFLOAT GetFPS() const
	{
		return m_pSystemTimer->GetFPS();
	}

	// $Barnyard: FUNCTION 006bbc50
	TBOOL Pause( TBOOL a_bPause )
	{
		TBOOL bOldState = m_pSystemTimer->SetPaused( a_bPause );
		m_PauseEmitter.Throw( &a_bPause );

		return bOldState;
	}

	TEmitter<TSystemManager, TBOOL>& GetPauseEmitter()
	{
		return m_PauseEmitter;
	}

	// $Barnyard: FUNCTION 006bbc40
	TUINT32 GetFrameCount() const
	{
		return m_pSystemTimer->GetNumFrames();
	}

	// $Barnyard: FUNCTION 006bbc00
	THPTimer& GetTimer()
	{
		return m_pSystemTimer->GetTimer();
	}

	// $Barnyard: FUNCTION 006bbc10
	TScheduler* GetScheduler() const
	{
		return m_pSystemTimer->GetScheduler();
	}

	// $Barnyard: FUNCTION 006bbc20
	TBOOL IsPaused() const
	{
		return m_pSystemTimer->IsPaused();
	}

	// $Barnyard: FUNCTION 006bbba0
	void SetQuitCallback( TSystemTimer::QuitCallback a_fnQuitCallback )
	{
		m_pSystemTimer->SetQuitCallback( a_fnQuitCallback );
	}

public:
	static TBOOL Create();

private:
	inline static TBOOL ms_bWasEverUpdated = TFALSE;

private:
	TEmitter<TSystemManager, TBOOL> m_PauseEmitter;
	TSystemTimer*                   m_pSystemTimer;
};

extern TSystemManager g_oSystemManager;

TOSHI_NAMESPACE_END
