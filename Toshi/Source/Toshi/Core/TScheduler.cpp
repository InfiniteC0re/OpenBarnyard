#include "pch.h"
#include "TScheduler.h"
#include "TTask.h"

namespace Toshi
{
	TOSHI_CLASS_DERIVED_INITIALIZE(TScheduler, TObject, MKVERSION(1, 0))
		
	TScheduler::TScheduler(TKernelInterface* kernelInterface)
	{
		m_TaskCount = 0;
		m_KernelInterface = kernelInterface;
		m_Unk3 = 0;
		m_SomeTask = nullptr;
		m_Unk1 = &m_TaskCount;
		m_Unk2 = &m_TaskCount;
		m_TaskTree.m_pRoot = nullptr;
		m_Unk4 = nullptr;
		m_CurrentTimeDelta = 0.0;
		m_TotalTime = 0.0;
		m_TimeDeltaLimit = 1.0;
		m_FrameCount = 0;
	}

	TTask* TScheduler::CreateTask(TClass* toshiClass, TTask* task)
	{
		TTask* taskObject = static_cast<TTask*>(toshiClass->CreateObject());
		
#ifdef TOSHI_DEBUG
		TClass* found = TClass::FindRecurse(taskObject->GetClass()->GetName(), &TTask::s_Class, true);
		TASSERT(found != nullptr, "TScheduler::CreateTask - passed TClass isn't inherited from TTask");
#endif
		
		TScheduler* taskScheduler = taskObject->GetScheduler();

		if (taskScheduler != this)
		{
			if (taskScheduler)
			{
				taskScheduler->DeleteTask();
			}	

			taskObject->m_Scheduler = this;
			m_TaskCount += 1;
		}

		TTODO("FUN_100253f0 and everything else");

		return nullptr;
	}
}