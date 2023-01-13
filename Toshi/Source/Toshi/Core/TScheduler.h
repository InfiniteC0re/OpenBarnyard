#pragma once
#include "TNodeTree.h"
#include "TTask.h"

namespace Toshi
{
	class TTask;

	class TScheduler :
		public TGenericClassDerived<TScheduler, TObject, "TScheduler", TMAKEVERSION(1, 0), false>
	{
	public:
		TScheduler();

		virtual ~TScheduler() override
		{
			DestroyAllTasks();
		}

		// Creates task from TClass and binds it to this scheduler
		TTask* CreateTask(TClass* pClass, TTask* pParent = TNULL);
		
		// Updates tasks
		void Update()
		{
			float deltaTime = TSystemManager::GetSingleton()->GetTimer()->GetDelta();
			m_FrameCount += 1;
			m_TotalTime += deltaTime;
			m_DeltaTime = deltaTime;

			// Control delta time
			if (deltaTime >= 0.0f)
			{
				if (deltaTime >= m_MaxDeltaTime)
				{
					deltaTime = m_MaxDeltaTime;
				}
			}
			else
			{
				deltaTime = 0.0f;
			}

			if (m_UseDebugDeltaTime)
			{
				// Constant delta time
				m_DeltaTime = m_DebugDeltaTime;
			}

			if (m_UseDebugDeltaTimeMult)
			{
				// Delta time multiplicator
				m_DeltaTime *= m_DebugDeltaTimeMult;
			}

			m_Timer.Update();
			DestroyDyingTasks(m_TaskTree.AttachedToRoot());
			UpdateActiveTasks(m_TaskTree.AttachedToRoot());
			m_Timer.Update();

			deltaTime = m_Timer.GetDelta();
			m_TasksUpdateTime = deltaTime;

#ifdef TOSHI_PLATFORM_WINDOWS
			if (m_UseFixedMaxFps && deltaTime < m_FixedMaxFps)
			{
				Sleep(static_cast<DWORD>((m_FixedMaxFps - deltaTime) * 1000));
			}
#endif
		}

		bool IsFixedMaxFps() const
		{
			return m_UseFixedMaxFps;
		}

		float GetDebugDeltaTime() const
		{
			return m_DebugDeltaTime;
		}

		void SetFixedMaxFps(bool useFixedMaxFps, float fixedMaxFps = 60.0f)
		{
			m_UseFixedMaxFps = useFixedMaxFps;
			m_FixedMaxFps = 1 / fixedMaxFps;
		}

		void SetDebugDeltaTimeMult(bool useDebugDeltaTimeMult, float debugDeltaTimeMult = 1.0f)
		{
			m_UseDebugDeltaTimeMult = useDebugDeltaTimeMult;
			m_DebugDeltaTimeMult = debugDeltaTimeMult;
		}

		void SetDebugDeltaTime(bool useDebugDeltaTime, float debugDeltaTime = 0.0f)
		{
			m_UseDebugDeltaTime = useDebugDeltaTime;
			m_DebugDeltaTime = debugDeltaTime;
		}

		void SetDebugPause(bool pause)
		{
			if (pause)
			{
				m_DebugDeltaTime = 0.0f;
			}

			m_UseDebugDeltaTime = pause;
		}

		void SetDebugSlowTime(bool slowTime)
		{
			m_MaxDeltaTime = slowTime ? s_DebugSlowMaxTimeDeltaAllowed : s_MaxTimeDeltaAllowed;
		}

		float GetCurrentDeltaTime() const
		{
			return m_DeltaTime;
		}

		float GetTotalTime() const
		{
			return m_TotalTime;
		}
		
		uint32_t GetFrameCount() const
		{
			return m_FrameCount;
		}
		
		TNodeTree<TTask>* GetTree()
		{
			return &m_TaskTree;
		}

		void DeleteTask(TTask* task)
		{
			task->OnPreDestroy();
			DeleteTaskRecurse(task->Attached());
			DeleteTaskAtomic(task);
		}

		void DestroyTask(TTask* task)
		{
			TASSERT(task->IsDying());
			task->OnPreDestroy();

			DeleteTaskRecurse(task->Attached());
			DeleteTaskAtomic(task->Attached());
		}

	private:
		static void DestroyTaskRecurse(TTask* task)
		{
			TTask* currentTask = task;

			while (currentTask != TNULL && currentTask != task)
			{
				currentTask->GetFlags() |= TTask::State_Dying;
				
				if (currentTask->Attached() != TNULL)
				{
					DestroyTaskRecurse(currentTask);
				}

				currentTask = currentTask->Next();
			}
		}

		void DeleteTaskAtomic(TTask* task);

		void DeleteTaskRecurse(TTask* task)
		{
			if (task != TNULL)
			{
				TTask* currentTask = task->Prev();

				while (currentTask != TNULL)
				{
					TTask* nextTask = (currentTask->Prev() != task) ? currentTask->Prev() : TNULL;
					DeleteTaskAtomic(currentTask);
					currentTask = nextTask;
				}
			}
		}

		void DestroyAllTasks()
		{
			if (m_TaskTree.AttachedToRoot() != TNULL)
			{
				DestroyTaskRecurse(m_TaskTree.AttachedToRoot());
				DestroyDyingTasks(m_TaskTree.AttachedToRoot());
			}
		}

		// Destroys all the dying tasks from the first one to the last one
		void DestroyDyingTasks(TTask* task);

		// Updates all the active tasks from the last one to the first one
		void UpdateActiveTasks(TTask* task);

	private:
		static float s_DebugSlowMaxTimeDeltaAllowed;
		static float s_MaxTimeDeltaAllowed;
			
	private:
		uint32_t m_Unk1;              // 0x04
		TNodeTree<TTask> m_TaskTree;  // 0x08
		float m_DeltaTime;            // 0x20
		float m_TasksUpdateTime;      // 0x24
		float m_TotalTime;            // 0x28
		float m_MaxDeltaTime;         // 0x2C
		uint32_t m_FrameCount;        // 0x30
		uint32_t m_Unk2;              // 0x34
		THPTimer m_Timer;             // 0x38
		bool m_UseDebugDeltaTime;     // 0x60
		float m_DebugDeltaTime;       // 0x64
		bool m_UseDebugDeltaTimeMult; // 0x6D
		float m_DebugDeltaTimeMult;   // 0x70
		bool m_UseFixedMaxFps;        // 0x74
		float m_FixedMaxFps;          // 0x78
	};
}