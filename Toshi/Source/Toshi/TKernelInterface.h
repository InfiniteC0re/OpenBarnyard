#pragma once
#include "THPTimer.h"

namespace Toshi
{

class TScheduler;

class TKernelInterface
{
public:
    using t_QuitCallback = void ( * )();

public:
    TKernelInterface();

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

    void SetQuitCallback( t_QuitCallback a_fnQuitCallback )
    {
        m_fnQuitCallback = a_fnQuitCallback;
    }

private:
    TUINT          m_Unk1;
    TScheduler*    m_Scheduler;
    TBOOL          m_bIsPaused;
    TUINT32        m_Unk2;
    THPTimer       m_Timer;
    TUINT32        m_Unk3;
    TFLOAT         m_fSecond;
    TFLOAT         m_fAverageFps;
    TUINT32        m_uiNumFrames;
    TBOOL          m_Unk4;
    t_QuitCallback m_fnQuitCallback;
    TUINT32        m_Unk6;
};

} // namespace Toshi
