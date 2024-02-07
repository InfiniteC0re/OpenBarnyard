#include "ToshiPCH.h"
#include "TScheduler.h"
#include "TSystem.h"
#include "TTask.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TDEFINE_CLASS_NORUNTIME(TScheduler);

	TFLOAT TScheduler::s_MaxTimeDeltaAllowed = 0.25f;
	TFLOAT TScheduler::s_DebugSlowMaxTimeDeltaAllowed = 1.0f;

	TScheduler::TScheduler()
	{
		m_DeltaTime = 0.0f;
		m_TotalTime = 0.0f;
		m_MaxDeltaTime = 0.25f;
		m_uiNumFrames = 0;
		m_fnDeltaTimeCalculator = TNULL;
		m_UseDebugDeltaTime = TFALSE;
		m_DebugDeltaTime = 0.01f;
		m_UseDebugDeltaTimeMult = TFALSE;
		m_DebugDeltaTimeMult = 1.0f;
	}

	TTask* TScheduler::CreateTask(TClass* pClass, TTask* pParent)
	{
		TASSERT(pClass->IsA(&TGetClass(TTask)));
		
		TTask* pTask = static_cast<TTask*>(pClass->CreateObject());
		TASSERT(pTask != TNULL);

		m_TaskTree.InsertAtRoot(pTask);

		if (pParent != TNULL)
		{
			m_TaskTree.Remove(pTask, TFALSE);
			m_TaskTree.Insert(pParent, pTask);
		}

		return pTask;
	}

	void TScheduler::Update()
	{
		if (!g_oSystemManager.ms_bWasEverUpdated)
		{
			g_oSystemManager.ms_bWasEverUpdated = TTRUE;
		}
		
		TFLOAT fDeltaTime = g_oSystemManager.GetTimer().GetDelta();
		m_DeltaTime = fDeltaTime;
		m_TotalTime += fDeltaTime;
		m_uiNumFrames += 1;

		// Control delta time
		if (m_DeltaTime >= 0.0f)
		{
			if (m_DeltaTime >= m_MaxDeltaTime)
			{
				m_DeltaTime = m_MaxDeltaTime;
			}
		}
		else
		{
			m_DeltaTime = 0.0f;
		}

		if (m_fnDeltaTimeCalculator)
		{
			m_DeltaTime = m_fnDeltaTimeCalculator(m_DeltaTime);
		}

		if (m_UseDebugDeltaTime)
		{
			m_DeltaTime = m_DebugDeltaTime;
		}

		if (m_UseDebugDeltaTimeMult)
		{
			m_DeltaTime *= m_DebugDeltaTimeMult;
		}

		DestroyDyingTasks(m_TaskTree.ChildOfRoot());
		UpdateActiveTasks(m_TaskTree.ChildOfRoot());
	}

	void TScheduler::DeleteTaskAtomic(TTask* task)
	{
		if (task != TNULL)
		{
			TTask* taskParent = TNULL;

			if (task->IsChildOfDefaultRoot() == TFALSE)
			{
				taskParent = task->Parent();

				if (taskParent->IsDying() == TFALSE && taskParent->OnChildDying(task) == TFALSE)
				{
					// The parent is not dying and doesn't allow us to kill the task
					return;
				}
			}

			if (task->Child() != TNULL)
			{
				TTask* node = task->Child()->Child();

				while (node != TNULL)
				{
					auto next = (node->Prev() != node) ? node->Prev() : TNULL;
					DeleteTaskAtomic(node);
					node = next;
				}
			}

			TClass* pClass = task->GetClass();
			task->OnDestroy();
			m_TaskTree.Remove(task, TFALSE);
			task->Delete();

			if (taskParent != TNULL)
			{
				taskParent->OnChildDied(pClass, task);
			}
		}
	}

	void TScheduler::DestroyDyingTasks(TTask* task)
	{
		if (task != TNULL)
		{
			TTask* currentTask = task->Prev();

			while (currentTask != TNULL)
			{
				TTask* nextTask = (currentTask->Prev() != task) ? currentTask->Prev() : TNULL;

				if (!currentTask->IsDying())
				{
					if (currentTask->Child() != TNULL)
					{
						DestroyDyingTasks(currentTask->Child());
					}
				}
				else
				{
					DeleteTask(currentTask);
				}

				currentTask = nextTask;
			}
		}
	}

	void TScheduler::UpdateActiveTasks(TTask* task)
	{
		TTask* currentTask = task;

		while (currentTask != TNULL)
		{
			TTask* nextTask = (currentTask->Next() != task) ? currentTask->Next() : TNULL;

			TBOOL recurse = TTRUE;
			if (task->IsCreated() && task->IsActive())
			{
				recurse = currentTask->OnUpdate(m_DeltaTime);
			}

			if (currentTask->Child() != TNULL && recurse)
			{
				UpdateActiveTasks(currentTask->Child());
			}

			currentTask = nextTask;
		}
	}
}