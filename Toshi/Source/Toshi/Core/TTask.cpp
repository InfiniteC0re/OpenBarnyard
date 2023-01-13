#include "ToshiPCH.h"
#include "TTask.h"

namespace Toshi
{
	TTask::TTask()
	{
		m_Unk1 = nullptr;

	}
	
	TTask::~TTask()
	{
		TASSERT(IsLinked() == TFALSE);
		/*if (m_Scheduler)
		{
			m_Scheduler->DeleteTask();
		}*/
	}

	bool TTask::Create()
	{
		TASSERT(IsCreated() == TFALSE);

		if (!IsCreated())
		{
			if (!OnCreate())
			{
				OnDestroy();
				m_Tree->Remove(this, false);
				Delete();

				return false;
			}

			auto oldState = m_State;
			m_State |= State_Created;
			Activate(true);
		}

		return true;
	}
	
	bool TTask::CreateFailed()
	{
		TASSERT(IsCreated() == TFALSE);

		if (!IsCreated())
		{
			m_Tree->Remove(this, false);
			Delete();
		}

		return false;
	}

	bool TTask::Reset()
	{
		TTask* firstAttached = Attached();
		TTask* node = firstAttached;
		bool result = true;

		while (node != TNULL && node != firstAttached)
		{
			result &= node->Reset();
			node = node->Next();
		}

		return result;
	}

	bool TTask::OnCreate()
	{
		return true;
	}

	bool TTask::OnUpdate(float deltaTime)
	{
		return true;
	}

	void TTask::OnPreDestroy()
	{

	}

	void TTask::OnDestroy()
	{
		
	}

	bool TTask::OnChildDying(TTask* child)
	{
		return true;
	}

	void TTask::OnChildDied(TClass* pClass, TTask* deletedTask)
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
		uint8_t newFlags = activate ? State_Active : 0;

		m_State = (m_State & ~State_Active) | newFlags;

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