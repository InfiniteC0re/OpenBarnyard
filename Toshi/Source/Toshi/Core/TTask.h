#pragma once
#include "TObject.h"
#include "TKernelInterface.h"
#include "TScheduler.h"

namespace Toshi
{
	enum TTaskState
	{
		TTaskState_Created = BITFIELD(0),
		TTaskState_Active = BITFIELD(1),
		TTaskState_Dying = BITFIELD(2),
	};

	class TTask : public TObject
	{
		TOSHI_CLASS_NO_CREATE_DEFINE(TTask)

	public:
		friend TScheduler;

	public:
		TTask();
		~TTask();
		
		virtual bool Create();
		virtual bool CreateFailed();
		virtual bool OnCreate();
		virtual bool OnUpdate(float deltaTime);
		virtual bool OnUpdateKernelPaused();
		virtual void OnDestroy();
		virtual bool OnChildDying();
		virtual void OnChildDied();
		virtual void OnActivate();
		virtual void OnDeactivate();

		void Activate(bool activate);

		inline TKernelInterface* GetKernel() const { return GetScheduler()->GetKernelInterface(); }
		inline TScheduler* GetScheduler() const { return m_Scheduler; }
		inline uint8_t GetFlags() const { return m_State; }
		inline bool IsCreated() const { return m_State & TTaskState_Created; }
		inline bool IsActive() const { return m_State & TTaskState_Active; }
		inline bool IsCreatedAndActive() const { return m_State & (TTaskState_Created | TTaskState_Active); }
		inline bool IsDying() const { return m_State & TTaskState_Dying; }

	private:
		void* m_Unk1;                    // 0x04
		TNodeTree<TTask>* m_TaskTree;    // 0x08
		TTask* m_Prev;                   // 0x0C
		TTask* m_Next;                   // 0x10
		void* m_UserData;                // 0x14
		TTask* m_SubTask;                // 0x18
		uint8_t m_State;                 // 0x1C
		TScheduler* m_Scheduler;         // 0x20
	};
}