#pragma once
#include "TNodeTree.h"
#include "THPTimer.h"
#include "TSystem.h"
#include "TTask.h"

TOSHI_NAMESPACE_START

class TTask;

class TScheduler
{
public:
	using t_DeltaTimeCalculator = TFLOAT ( * )( TFLOAT a_fRealDT );

public:
	TScheduler();
	virtual ~TScheduler();

	TTask* CreateTask( TClass* a_pClass, TTask* a_pParent = TNULL );

	template <class T>
	T* CreateTask( TTask* a_pParent = TNULL )
	{
		return TSTATICCAST( T, CreateTask( &TGetClass( T ), a_pParent ) );
	}

	void Update();

	void SetDeltaTimeCalculator( t_DeltaTimeCalculator a_fnCalculator ) { m_fnDeltaTimeCalculator = a_fnCalculator; }

	TFLOAT& GetDebugDeltaTime() { return m_DebugDeltaTime; }
	TFLOAT  GetDebugDeltaTime() const { return m_DebugDeltaTime; }

	TFLOAT& GetDebugDeltaTimeMult() { return m_DebugDeltaTimeMult; }
	TFLOAT  GetDebugDeltaTimeMult() const { return m_DebugDeltaTimeMult; }
	TBOOL   IsDebugDeltaTimeMultUsed() const { return m_UseDebugDeltaTimeMult; }

	void SetDebugDeltaTimeMult( TBOOL useDebugDeltaTimeMult, TFLOAT debugDeltaTimeMult = 1.0f );

	// $Barnyard: FUNCTION 006bc9e0
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

	// $Barnyard: FUNCTION 006bc9c0
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

	TNodeTree<TTask>* GetTree()
	{
		return &m_TaskTree;
	}

	// $Barnyard: FUNCTION 006bcc30
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
	// $Barnyard: FUNCTION 006bca80
	static void DestroyTaskRecurse( TTask* task )
	{
		TTask* currentTask = task;

		if ( currentTask == TNULL )
			return;

		do
		{
			currentTask->GetFlags() |= TTask::State_Dying;

			if ( currentTask->Child() != TNULL )
			{
				DestroyTaskRecurse( currentTask->Child() );
			}

			currentTask = currentTask->Next();
		} while ( currentTask != task && currentTask != TNULL );
	}

	void DeleteTaskAtomic( TTask* task );

	// $Barnyard: FUNCTION 006bcac0
	void DeleteTaskRecurse( TTask* task )
	{
		if ( task != TNULL )
		{
			TTask* currentTask = task->Prev();

			while ( currentTask != TNULL )
			{
				// Deleting the tasks removes them from the list, stop once the last one is deleted
				TTask* nextTask = ( currentTask->Prev() != currentTask ) ? currentTask->Prev() : TNULL;
				DeleteTaskAtomic( currentTask );
				currentTask = nextTask;
			}
		}
	}

public:
	// $Barnyard: FUNCTION 006bcd20
	void DestroyAllTasks()
	{
		if ( m_TaskTree.ChildOfRoot() != TNULL )
		{
			DestroyTaskRecurse( m_TaskTree.ChildOfRoot() );
			DestroyDyingTasks( m_TaskTree.ChildOfRoot() );
		}
	}

private:
	// Destroys all the dying tasks from the first one to the last one
	void DestroyDyingTasks( TTask* task );

	// Updates all the active tasks from the last one to the first one
	void UpdateActiveTasks( TTask* task );

private:
	static TFLOAT s_DebugSlowMaxTimeDeltaAllowed;
	static TFLOAT s_MaxTimeDeltaAllowed;

private:
	TNodeTree<TTask>      m_TaskTree;
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

TOSHI_NAMESPACE_END
