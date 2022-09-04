#include "ToshiPCH.h"
#include "TScheduler.h"
#include "TTask.h"

namespace Toshi
{
	TOSHI_CLASS_DERIVED_INITIALIZE(TScheduler, TObject, TMAKEVERSION(1, 0))

	float TScheduler::s_MaxTimeDeltaAllowed = 0.01f;
	float TScheduler::s_DebugSlowMaxTimeDeltaAllowed = 1.0f;

	TScheduler::TScheduler(TKernelInterface* kernelInterface)
	{
		m_TaskCount = 0;
		m_KernelInterface = kernelInterface;
		m_Unk3 = 0;
		m_LastTask = nullptr;
		m_Unk1 = &m_TaskCount;
		m_Unk2 = &m_TaskCount;
		m_TaskTree.m_pRoot = nullptr;
		m_Unk4 = 0;
		m_CurrentTimeDelta = 0.0;
		m_TotalTime = 0.0;
		m_DeltaTimeLimit = 1.0;
		m_FrameCount = 0;
	}

	TTask* TScheduler::CreateTask(TClass* toshiClass, TTask* parentTask)
	{
		TTask* task = static_cast<TTask*>(toshiClass->CreateObject());
		
#ifdef TOSHI_DEBUG
		TClass* found = TClass::FindRecurse(task->GetClass()->GetName(), &TTask::s_Class, true);
		TASSERT(found != nullptr, "TScheduler::CreateTask - passed TClass isn't inherited from TTask");
#endif
		
		TScheduler* taskScheduler = task->GetScheduler();

		if (taskScheduler != this)
		{
			if (taskScheduler)
			{
				taskScheduler->DeleteTask();
			}	

			task->m_Scheduler = this;
			m_TaskCount += 1;
		}

		TTODO("TKernelInterface.dll: FUN_100253f0 at 0x10024cd9");

		if (m_LastTask)
		{
			task->m_Prev = m_LastTask;
			task->m_Next = m_LastTask->m_Next;
			task->m_Next->m_Prev = task;
			m_LastTask->m_Next = task;
		}

		task->m_TaskTree = &m_TaskTree;
		task->m_UserData = &m_TaskCount;
		m_LastTask = task;
		m_Unk4 += 1;

		if (parentTask != nullptr)
		{
			TTODO("TKernelInterface.dll: FUN_100253f0 at 0x10024d1d");

			TTask* subTask = parentTask->m_SubTask;
			
			if (subTask == nullptr)
			{
				parentTask->m_SubTask = task;
			}
			else
			{
				task->m_Prev = subTask;
				task->m_Next = subTask->m_Next;
				task->m_Next->m_Prev = task;
				subTask->m_Next = task;
			}

			task->m_UserData = parentTask;
			m_Unk4 += 1;
		}

		return task;
	}

	void TScheduler::Update()
	{
		auto& timer = GetKernelInterface()->GetSystemTimer();
		float deltaTime = timer.GetDelta();

		m_TotalTime += deltaTime;
		m_FrameCount += 1;

		if (0 <= deltaTime)
		{
			m_CurrentTimeDelta = deltaTime <= m_DeltaTimeLimit ? deltaTime : m_DeltaTimeLimit;
		}
		else
		{
			m_CurrentTimeDelta = 0;
		}

		DestroyDyingTasks(m_LastTask);
		UpdateActiveTasks(m_LastTask);
	}

	void TScheduler::SetDebugSlowTime(bool slowTime)
	{
		m_DeltaTimeLimit = slowTime ? s_DebugSlowMaxTimeDeltaAllowed : s_MaxTimeDeltaAllowed;
	}
	
	void TScheduler::DestroyDyingTasks(TTask* rootTask)
	{
		if (rootTask)
		{
			TTask* task = rootTask->m_Next;
			
			while (task != nullptr)
			{
				TTask* nextTask = (task != rootTask) ? task->m_Next : nullptr;

				if (task->IsDying())
				{
					DeleteTask(task);
				}
				else
				{
					TTask* attachedTask = task->m_SubTask;
					
					if (attachedTask)
					{
						DestroyDyingTasks(attachedTask);
					}
				}

				task = nextTask;
			}
		}
	}

	void TScheduler::UpdateActiveTasks(TTask* rootTask)
	{
		TTask* task = rootTask;

		while (task != nullptr)
		{
			TTask* prevTask = task->m_Prev;

			if (prevTask == rootTask)
			{
				prevTask = nullptr;
			}

			bool updateResult = true;

			if (task->IsCreatedAndActive())
			{
				updateResult = task->OnUpdate(m_CurrentTimeDelta);

				// if IsPaused() returns true
				// then it only updates the root task
				// and skips any other tasks

				if (GetKernelInterface()->IsPaused())
				{
					return;
				}
			}

			if (updateResult && task->m_SubTask)
			{
				UpdateActiveTasks(task->m_SubTask);
			}

			task = prevTask;
		}
	}

	void TScheduler::UpdateActiveTasksKernelPaused(TTask* rootTask)
	{
		TTask* task = rootTask;

		while (task != nullptr)
		{
			TTask* prevTask = task->m_Prev;

			if (prevTask == rootTask)
			{
				prevTask = nullptr;
			}

			bool updateResult = true;

			if (task->IsCreatedAndActive())
			{
				updateResult = task->OnUpdateKernelPaused(m_CurrentTimeDelta);
			}

			if (updateResult && task->m_SubTask)
			{
				UpdateActiveTasksKernelPaused(task->m_SubTask);
			}

			task = prevTask;
		}
	}

	void TScheduler::DeleteTask(TTask* task)
	{
		TIMPLEMENT();
	}
}