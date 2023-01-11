#pragma once
#include "TObject.h"
#include "TNodeTree.h"

namespace Toshi
{
	class TScheduler;

	class TTask :
		public TGenericClassDerived<TTask, TObject, "TTask", TMAKEVERSION(1, 0), false>,
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
		
		virtual bool Create();
		virtual bool CreateFailed();
		virtual bool Reset();
		virtual bool OnCreate();
		virtual bool OnUpdate(float deltaTime);
		virtual void OnPreDestroy();
		virtual void OnDestroy();
		virtual bool OnChildDying(TTask* child);
		virtual void OnChildDied(TClass* pClass, TTask* deletedTask);
		virtual void OnActivate();
		virtual void OnDeactivate();

		void Activate(bool activate);

		TScheduler* GetScheduler() const { return m_Scheduler; }
		uint8_t& GetFlags() { return m_State; }
		bool IsCreated() const { return m_State & State_Created; }
		bool IsActive() const { return m_State & State_Active; }
		bool IsCreatedAndActive() const { return (m_State & (State_Created | State_Active)) == (State_Created | State_Active); }
		bool IsDying() const { return m_State & State_Dying; }

	private:
		void* m_Unk1;                    // 0x04
		uint8_t m_State;                 // 0x1C
		TScheduler* m_Scheduler;         // 0x20
	};
}