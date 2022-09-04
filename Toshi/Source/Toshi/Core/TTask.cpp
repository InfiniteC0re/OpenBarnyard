#include "ToshiPCH.h"
#include "TTask.h"

namespace Toshi
{
	TOSHI_CLASS_DERIVED_INITIALIZE(TTask, TObject, MKVERSION(1, 0))
	
	TTask::TTask()
	{
		m_Unk1 = nullptr;
		m_UserData = nullptr;
		m_SubTask = nullptr;
		m_Prev = this;
		m_Next = this;
		m_TaskTree = nullptr;
		m_Scheduler = nullptr;
		m_State = 0;
	}
	
	TTask::~TTask()
	{
		if (m_Scheduler)
		{
			m_Scheduler->DeleteTask();
		}
	}

	bool TTask::Create()
	{
		if (!IsCreated())
		{
			bool success = OnCreate();
			
			if (!success)
			{
				// couldn't create TTask so destroying it
				OnDestroy();
				// FUN_100253f0(this->field5_0x8,(int)this,'\0');
				Delete();

				return false;
			}

			m_State |= TTaskState_Created;
			Activate(true);
		}

		return true;
	}
	
	bool TTask::CreateFailed()
	{
		if (!IsCreated())
		{
			// FUN_100253f0(this->field5_0x8,(int)this,'\0');
			Delete();
		}

		return false;
	}

	bool TTask::OnCreate()
	{
		return true;
	}

	bool TTask::OnUpdate(float deltaTime)
	{
		return true;
	}

	bool TTask::OnUpdateKernelPaused(float deltaTime)
	{
		return true;
	}

	void TTask::OnDestroy()
	{
		
	}

	bool TTask::OnChildDying()
	{
		return true;
	}

	void TTask::OnChildDied()
	{

	}

	void TTask::OnActivate()
	{

	}

	void TTask::OnDeactivate()
	{

	}

	void TTask::Activate(bool activate)
	{
		uint8_t oldState = m_State;
		uint8_t newFlags = activate ? TTaskState_Active : 0;

		m_State = (m_State & ~TTaskState_Active) | newFlags;

		if (oldState != m_State)
		{
			if (activate)
			{
				OnActivate();
			}
			else
			{
				OnDeactivate();
			}
		}
	}
}