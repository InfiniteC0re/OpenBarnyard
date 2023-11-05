#pragma once
#include "TObject.h"
#include "TNodeTree.h"

namespace Toshi
{
	class TScheduler;

	class TTask :
		public TGenericClassDerived<TTask, TObject, "TTask", TMAKEVERSION(1, 0), TFALSE>,
		public TNodeTree<TTask>::TNode
	{
	public:
		friend TScheduler;

		enum State
		{
			State_Created = BITFIELD(0),
			State_Active = BITFIELD(1),
			State_Dying = BITFIELD(2),
		};

	public:
		TTask();
		virtual ~TTask();
		
		virtual TBOOL Create();
		virtual TBOOL CreateFailed();
		virtual TBOOL Reset();
		virtual TBOOL OnCreate();
		virtual TBOOL OnUpdate(float a_fDeltaTime);
		virtual void OnPreDestroy();
		virtual void OnDestroy();
		virtual TBOOL OnChildDying(TTask* child);
		virtual void OnChildDied(TClass* pClass, TTask* deletedTask);
		virtual void OnActivate();
		virtual void OnDeactivate();

		void Activate(TBOOL activate);

		TScheduler* GetScheduler() const { return m_Scheduler; }
		uint8_t& GetFlags() { return m_State; }
		TBOOL IsCreated() const { return m_State & State_Created; }
		TBOOL IsActive() const { return m_State & State_Active; }
		TBOOL IsCreatedAndActive() const { return (m_State & (State_Created | State_Active)) == (State_Created | State_Active); }
		TBOOL IsDying() const { return m_State & State_Dying; }

	private:
		uint8_t m_State;                 // 0x1C
		TScheduler* m_Scheduler;         // 0x20
	};
}