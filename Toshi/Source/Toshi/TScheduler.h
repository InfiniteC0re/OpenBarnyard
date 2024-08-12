#pragma once
#include "TNodeTree.h"
#include "THPTimer.h"
#include "TSystem.h"
#include "TTask.h"

namespace Toshi
{
class TTask;

class TScheduler : public TObject
{
public:
    TDECLARE_CLASS( TScheduler, TObject );

public:
    using t_DeltaTimeCalculator = TFLOAT ( * )( TFLOAT a_fRealDT );

public:
    TScheduler();
    ~TScheduler();

    TTask* CreateTask( TClass* a_pClass, TTask* a_pParent = TNULL );

    template < class T >
    T* CreateTask( TTask* a_pParent = TNULL )
    {
        return TSTATICCAST( T, CreateTask( &TGetClass( T ), a_pParent ) );
    }

    void Update();

    TFLOAT& GetDebugDeltaTime() { return m_DebugDeltaTime; }
    TFLOAT  GetDebugDeltaTime() const { return m_DebugDeltaTime; }

    TFLOAT& GetDebugDeltaTimeMult() { return m_DebugDeltaTimeMult; }
    TFLOAT  GetDebugDeltaTimeMult() const { return m_DebugDeltaTimeMult; }

    void SetDebugDeltaTimeMult( TBOOL useDebugDeltaTimeMult, TFLOAT debugDeltaTimeMult = 1.0f )
    {
        m_UseDebugDeltaTimeMult = useDebugDeltaTimeMult;
        m_DebugDeltaTimeMult    = debugDeltaTimeMult;
    }

    void SetDebugDeltaTime( TBOOL useDebugDeltaTime, TFLOAT debugDeltaTime = 0.0f )
    {
        m_UseDebugDeltaTime = useDebugDeltaTime;
        m_DebugDeltaTime    = debugDeltaTime;
    }

    void SetDebugPause( TBOOL pause )
    {
        if ( pause )
        {
            m_DebugDeltaTime = 0.0f;
        }

        m_UseDebugDeltaTime = pause;
    }

    void SetDebugSlowTime( TBOOL slowTime )
    {
        m_MaxDeltaTime = slowTime ? s_DebugSlowMaxTimeDeltaAllowed : s_MaxTimeDeltaAllowed;
    }

    TFLOAT GetCurrentDeltaTime() const
    {
        return m_DeltaTime;
    }

    TFLOAT GetTotalTime() const
    {
        return m_TotalTime;
    }

    TUINT GetFrameCount() const
    {
        return m_uiNumFrames;
    }

    TNodeTree< TTask >* GetTree()
    {
        return &m_TaskTree;
    }

    void DeleteTask( TTask* a_pTask )
    {
        a_pTask->OnPreDestroy();
        DeleteTaskRecurse( a_pTask->Child() );
        DeleteTaskAtomic( a_pTask );
    }

    void DestroyTask( TTask* a_pTask )
    {
        TASSERT( a_pTask->IsDying() );
        a_pTask->OnPreDestroy();

        DeleteTaskRecurse( a_pTask->Child() );
        DeleteTaskAtomic( a_pTask->Child() );
    }

private:
    static void DestroyTaskRecurse( TTask* task )
    {
        TTask* currentTask = task;

        while ( currentTask != TNULL && currentTask != task )
        {
            currentTask->GetFlags() |= TTask::State_Dying;

            if ( currentTask->Child() != TNULL )
            {
                DestroyTaskRecurse( currentTask );
            }

            currentTask = currentTask->Next();
        }
    }

    void DeleteTaskAtomic( TTask* task );

    void DeleteTaskRecurse( TTask* task )
    {
        if ( task != TNULL )
        {
            TTask* currentTask = task->Prev();

            while ( currentTask != TNULL )
            {
                TTask* nextTask = ( currentTask->Prev() != task ) ? currentTask->Prev() : TNULL;
                DeleteTaskAtomic( currentTask );
                currentTask = nextTask;
            }
        }
    }

    void DestroyAllTasks()
    {
        if ( m_TaskTree.ChildOfRoot() != TNULL )
        {
            DestroyTaskRecurse( m_TaskTree.ChildOfRoot() );
            DestroyDyingTasks( m_TaskTree.ChildOfRoot() );
        }
    }

    // Destroys all the dying tasks from the first one to the last one
    void DestroyDyingTasks( TTask* task );

    // Updates all the active tasks from the last one to the first one
    void UpdateActiveTasks( TTask* task );

private:
    static TFLOAT s_DebugSlowMaxTimeDeltaAllowed;
    static TFLOAT s_MaxTimeDeltaAllowed;

private:
    TNodeTree< TTask >    m_TaskTree;
    TFLOAT                m_DeltaTime;
    TFLOAT                m_TotalTime;
    TFLOAT                m_MaxDeltaTime;
    TUINT                 m_uiNumFrames;
    t_DeltaTimeCalculator m_fnDeltaTimeCalculator;
    TBOOL                 m_UseDebugDeltaTime;
    TFLOAT                m_DebugDeltaTime;
    TFLOAT                m_Unk1;
    // ...
    TBOOL  m_Unk2;
    TBOOL  m_UseDebugDeltaTimeMult;
    TFLOAT m_DebugDeltaTimeMult;
};
} // namespace Toshi